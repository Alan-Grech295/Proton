#include "ptpch.h"
#include "ScriptEngine.h"
#include "ScriptGlue.h"
#include "Proton/Scene/Entity.h"
#include "FileWatch.h"
#include "Proton/Core/Application.h"

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>
#include <mono/metadata/tabledefs.h>
#include <mono/metadata/mono-debug.h>
#include <mono/metadata/threads.h>

#include <assert.h>

namespace Proton
{
	static std::unordered_map<std::string, ScriptFieldType> s_ScriptFieldTypeMap =
	{
		{"System.Single", ScriptFieldType::Float},
		{"System.Double", ScriptFieldType::Double},
		
		{"System.Char", ScriptFieldType::Char},
		{"System.Boolean", ScriptFieldType::Bool},
		{"System.SByte", ScriptFieldType::SByte},
		{"System.Int16", ScriptFieldType::Short},
		{"System.Int32", ScriptFieldType::Int},
		{"System.Int64", ScriptFieldType::Long},

		{"System.Byte", ScriptFieldType::Byte},
		{"System.UInt16", ScriptFieldType::UShort},
		{"System.UInt32", ScriptFieldType::UInt},
		{"System.UInt64", ScriptFieldType::ULong},

		//{"System.String", ScriptFieldType::String},

		{"Proton.Vector2", ScriptFieldType::Vector2},
		{"Proton.Vector3", ScriptFieldType::Vector3},
		{"Proton.Vector4", ScriptFieldType::Vector4},

		{"Proton.Entity", ScriptFieldType::Entity},
	};

	namespace Utils
	{
		//TODO: Move to FileSystem class 
		static char* ReadBytes(const std::filesystem::path& filepath, uint32_t* outSize)
		{
			std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

			if (!stream)
			{
				PT_CORE_ERROR("Error opening file");
				return nullptr;
			}

			std::streampos end = stream.tellg();
			stream.seekg(0, std::ios::beg);
			uint32_t size = end - stream.tellg();

			if (size == 0)
			{
				PT_CORE_ERROR("File is empty");
				return nullptr;
			}

			char* buffer = new char[size];
			stream.read(buffer, size);
			stream.close();

			*outSize = (uint32_t)size;
			return buffer;
		}

		static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& assemblyPath, bool loadPDB = false)
		{
			uint32_t fileSize = 0;
			char* fileData = ReadBytes(assemblyPath, &fileSize);

			MonoImageOpenStatus status;
			MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);

			if (status != MONO_IMAGE_OK)
			{
				PT_CORE_ERROR(mono_image_strerror(status));
				return nullptr;
			}

			if (loadPDB)
			{
				std::filesystem::path pdbPath = assemblyPath;
				pdbPath.replace_extension(".pdb");

				if (std::filesystem::exists(pdbPath))
				{
					uint32_t pdbFileSize;
					char* pdbFileData = ReadBytes(pdbPath, &pdbFileSize);

					mono_debug_open_image_from_memory(image, (const mono_byte*)pdbFileData, pdbFileSize);
					PT_CORE_INFO("Loaded PDB {}", pdbPath);
					delete[] pdbFileData;
				}
			}

			std::string pathString = assemblyPath.string();
			MonoAssembly* assembly = mono_assembly_load_from_full(image, pathString.c_str(), &status, 0);
			mono_image_close(image);

			delete[] fileData;

			return assembly;
		}

		void PrintAssemblyTypes(MonoAssembly* assembly)
		{
			MonoImage* image = mono_assembly_get_image(assembly);
			const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
			int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

			for (int32_t i = 0; i < numTypes; i++)
			{
				uint32_t cols[MONO_TYPEDEF_SIZE];
				mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

				const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
				const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

				PT_CORE_TRACE("{}.{}", nameSpace, name);
			}
		}

		ScriptFieldType MonoTypeToScriptFieldType(MonoType* monoType)
		{
			std::string typeName = mono_type_get_name(monoType);
			auto it = s_ScriptFieldTypeMap.find(typeName);
			if(it == s_ScriptFieldTypeMap.end())
				return ScriptFieldType::None;

			return it->second;
		}
	}

	struct ScriptEngineData
	{
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;

		MonoAssembly* CoreAssembly = nullptr;
		MonoImage* CoreAssemblyImage = nullptr;

		MonoAssembly* AppAssembly = nullptr;
		MonoImage* AppAssemblyImage = nullptr;

		std::filesystem::path CoreAssemblyFilePath;
		std::filesystem::path AppAssemblyFilePath;

		ScriptClass EntityClass;

		std::unordered_map<std::string, Ref<ScriptClass>> EntityClasses;
		std::vector<const char*> EntityClassNames;

		std::unordered_map<UUID, Ref<ScriptInstance>> EntityInstances;
		std::unordered_map<std::string, Scope<ScriptInstance>> EntityClassInstances;
		std::unordered_map<UUID, ScriptFieldMap> EntityScriptFields;

		Scope<filewatch::FileWatch<std::string>> AppAssemblyFileWatcher;
		bool AssemblyReloadPending = false;

		bool EnableDebugging = true;

		//Runtime
		Scene* SceneContext = nullptr;
	};

	static ScriptEngineData* s_Data;

	Ref<ScriptClass> ScriptEngine::GetEntityClass(const std::string& name)
	{
		auto it = s_Data->EntityClasses.find(name);
		if (it == s_Data->EntityClasses.end())
			return nullptr;

		return it->second;
	}

	std::unordered_map<std::string, Ref<ScriptClass>>& ScriptEngine::GetEntityClasses()
	{
		return s_Data->EntityClasses;
	}

	ScriptFieldMap& ScriptEngine::GetScriptFieldMap(Entity entity)
	{
		PT_CORE_ASSERT(entity);
		return s_Data->EntityScriptFields[entity.GetUUID()];
	}

	void ScriptEngine::Init()
	{
		s_Data = new ScriptEngineData();

		InitMono();
		ScriptGlue::RegisterFunctions();

		LoadAssembly("Resources/Scripts/Proton-ScriptCore.dll");
		LoadAppAssembly("SandboxProject/Assets/Scripts/Binaries/Sandbox.dll");

		//Retrieve and instantiate class with constructor
		s_Data->EntityClass = ScriptClass("Proton", "Entity", true);

		LoadAssemblyClasses();
		ScriptGlue::RegisterComponents();

		//Utils::PrintAssemblyTypes(s_Data->CoreAssembly);
#if 0
		//Retrieve and instantiate class with constructor
		s_Data->EntityClass = ScriptClass("Proton", "Entity");

		MonoObject* instance = s_Data->EntityClass.Instantiate();

		//Call method
		MonoMethod* printMessageFunc = s_Data->EntityClass.GetMethod("PrintMessage", 0);
		s_Data->EntityClass.InvokeMethod(instance, printMessageFunc, nullptr);

		MonoMethod* printIntFunc = s_Data->EntityClass.GetMethod("PrintInt", 1);

		int value = 5;
		void* param = &value;

		s_Data->EntityClass.InvokeMethod(instance, printIntFunc, &param);

		MonoMethod* printCustomMessageFunc = s_Data->EntityClass.GetMethod("PrintCustomMessage", 1);
		MonoString* string = mono_string_new(s_Data->AppDomain, "Hello world from C++!");
		s_Data->EntityClass.InvokeMethod(instance, printCustomMessageFunc, (void**)&string);
#endif
	}

	void ScriptEngine::Shutdown()
	{
		ShutdownMono();

		delete s_Data;
	}

	void ScriptEngine::InitMono()
	{
		mono_set_assemblies_path("mono/lib");

		if (s_Data->EnableDebugging)
		{
			const char* argv[2] = {
				"--debugger-agent=transport=dt_socket,address=127.0.0.1:2550,server=y,suspend=n,loglevel=3,logfile=MonoDebugger.log",
				"--soft-breakpoints"
			};
			mono_jit_parse_options(2, (char**)argv);
			mono_debug_init(MONO_DEBUG_FORMAT_MONO);
		}

		MonoDomain* rootDomain = mono_jit_init("ProtonJITRuntime");
		PT_CORE_ASSERT(rootDomain);

		s_Data->RootDomain = rootDomain;
		
		if(s_Data->EnableDebugging)
			mono_debug_domain_create(s_Data->RootDomain);

		mono_thread_set_main(mono_thread_current());
	}

	void ScriptEngine::ShutdownMono()
	{
		mono_domain_set(mono_get_root_domain(), false);

		//Shutdown not working, to do later
		mono_domain_unload(s_Data->AppDomain);
		s_Data->AppDomain = nullptr;

		mono_jit_cleanup(s_Data->RootDomain);
		s_Data->RootDomain = nullptr;
	}

	void ScriptEngine::OnRuntimeStart(Scene* scene)
	{
		s_Data->SceneContext = scene;
	}

	bool ScriptEngine::EntityClassExists(const std::string& fullClassName)
	{
		return s_Data->EntityClasses.find(fullClassName) != s_Data->EntityClasses.end();
	}

	const char*const* ScriptEngine::GetEntityClassNames()
	{
		return s_Data->EntityClassNames.data();
	}

	const char* ScriptEngine::GetEntityClassNameByIndex(uint32_t index)
	{
		return s_Data->EntityClassNames[index];
	}

	int ScriptEngine::GetEntityClassIndexFromName(std::string_view name)
	{
		for (int i = 0; i < s_Data->EntityClassNames.size(); i++)
		{
			if (name == s_Data->EntityClassNames[i])
				return i;
		}

		return -1;
	}

	void ScriptEngine::OnCreateEntity(Entity entity)
	{
		const auto& sc = entity.GetComponent<ScriptComponent>();

		if (ScriptEngine::EntityClassExists(sc.ClassName))
		{
			UUID entityID = entity.GetUUID();
			Ref<ScriptInstance> instance = CreateRef<ScriptInstance>(s_Data->EntityClasses[sc.ClassName], entity);
			s_Data->EntityInstances[entityID] = instance;

			//Copy field values
			if (s_Data->EntityScriptFields.find(entityID) != s_Data->EntityScriptFields.end())
			{
				const ScriptFieldMap& fieldMap = s_Data->EntityScriptFields.at(entityID);
				for (const auto& [name, fieldInstance] : fieldMap)
				{
					instance->SetFieldValueInternal(name, fieldInstance.m_Buffer);
				}
			}

			instance->InvokeOnCreate();
		}
	}

	void ScriptEngine::OnUpdateEntity(Entity entity, float ts)
	{
		UUID entityUUID = entity.GetUUID();
		PT_CORE_ASSERT(s_Data->EntityInstances.find(entityUUID) != s_Data->EntityInstances.end());

		Ref<ScriptInstance> instance = s_Data->EntityInstances[entityUUID];
		instance->InvokeOnUpdate(ts);
	}

	Scene* ScriptEngine::GetSceneContext()
	{
		return s_Data->SceneContext;
	}

	Ref<ScriptInstance> ScriptEngine::GetEntityScriptInstance(UUID entityID)
	{
		auto it = s_Data->EntityInstances.find(entityID);
		if (it == s_Data->EntityInstances.end())
			return nullptr;

		return it->second;
	}

	const ScriptInstance& ScriptEngine::GetScriptInstanceFromClass(const std::string& className)
	{
		auto it = s_Data->EntityClassInstances.find(className);
		PT_CORE_ASSERT(it != s_Data->EntityClassInstances.end());

		return *(it->second.get());
	}

	void ScriptEngine::OnRuntimeStop()
	{
		s_Data->SceneContext = nullptr;

		s_Data->EntityInstances.clear();
	}

	static void OnAppAssemblyFileSystemEvent(const std::string& path, const filewatch::Event change_type)
	{
		if (!s_Data->AssemblyReloadPending && change_type == filewatch::Event::modified)
		{
			s_Data->AssemblyReloadPending = true;

			Application::Get().SubmitToMainThread([]()
			{
				s_Data->AppAssemblyFileWatcher.reset();
				ScriptEngine::ReloadAssembly(); 
			});
		}
	}

	void ScriptEngine::LoadAppAssembly(const std::filesystem::path& filepath)
	{
		s_Data->AppAssemblyFilePath = filepath;
		s_Data->AppAssembly = Utils::LoadMonoAssembly(filepath, s_Data->EnableDebugging);
		s_Data->AppAssemblyImage = mono_assembly_get_image(s_Data->AppAssembly);
		//PrintAssemblyTypes(s_Data->CoreAssembly);

		s_Data->AppAssemblyFileWatcher = CreateScope<filewatch::FileWatch<std::string>>(filepath.string(), OnAppAssemblyFileSystemEvent);
		s_Data->AssemblyReloadPending = false;
	}

	void ScriptEngine::ReloadAssembly()
	{
		mono_domain_set(mono_get_root_domain(), false);

		mono_domain_unload(s_Data->AppDomain);
		//mono_domain_free(s_Data->AppDomain, true);

		LoadAssembly(s_Data->CoreAssemblyFilePath);
		LoadAppAssembly(s_Data->AppAssemblyFilePath);

		//Retrieve and instantiate class with constructor
		s_Data->EntityClass = ScriptClass("Proton", "Entity", true);

		LoadAssemblyClasses();

		ScriptGlue::RegisterComponents();
	}

	void ScriptEngine::LoadAssembly(const std::filesystem::path& filepath)
	{
		//Create app domain
		s_Data->AppDomain = mono_domain_create_appdomain("ProtonScriptRuntime", nullptr);
		mono_domain_set(s_Data->AppDomain, true);

		s_Data->CoreAssemblyFilePath = filepath;
		s_Data->CoreAssembly = Utils::LoadMonoAssembly(filepath, s_Data->EnableDebugging);
		s_Data->CoreAssemblyImage = mono_assembly_get_image(s_Data->CoreAssembly);
		//PrintAssemblyTypes(s_Data->CoreAssembly);
	}

	void ScriptEngine::LoadAssemblyClasses()
	{
		s_Data->EntityClasses.clear();
		s_Data->EntityClassNames.clear();

		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(s_Data->AppAssemblyImage, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);
		MonoClass* entityClass = mono_class_from_name(s_Data->CoreAssemblyImage, "Proton", "Entity");

		s_Data->EntityClassNames.reserve(numTypes);
		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(s_Data->AppAssemblyImage, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* className = mono_metadata_string_heap(s_Data->AppAssemblyImage, cols[MONO_TYPEDEF_NAME]);
			std::string fullName;
			if (strlen(nameSpace) != 0)
				fullName = fmt::format("{}.{}", nameSpace, className);
			else
				fullName = className;

			MonoClass* monoClass = mono_class_from_name(s_Data->AppAssemblyImage, nameSpace, className);

			if(monoClass == entityClass)
				continue;

			bool isEntity = mono_class_is_subclass_of(monoClass, entityClass, false);

			if (!isEntity)
				continue;

			Ref<ScriptClass> scriptClass = CreateRef<ScriptClass>(nameSpace, className);
			auto [it, succeded] = s_Data->EntityClasses.emplace(fullName, scriptClass);
			s_Data->EntityClassNames.push_back(it->first.c_str());

			PT_CORE_WARN("{} fields:", className);
			void* iterator = nullptr;
			while (MonoClassField* field = mono_class_get_fields(monoClass, &iterator))
			{
				const char* fieldName = mono_field_get_name(field);
				uint32_t flags = mono_field_get_flags(field);
				if (flags & FIELD_ATTRIBUTE_PUBLIC)
				{
					MonoType* type = mono_field_get_type(field);
					ScriptFieldType fieldType = Utils::MonoTypeToScriptFieldType(type);
					PT_CORE_WARN("   {} ({})", fieldName, Utils::ScriptFieldTypeToString(fieldType));

					scriptClass->m_Fields[fieldName] = { fieldType, fieldName, field };
				}
			}

			s_Data->EntityClassInstances[fullName] = CreateScope<ScriptInstance>(scriptClass, Entity::Null);
		}
	}

	MonoImage* ScriptEngine::GetCoreAssemblyImage()
	{
		return s_Data->CoreAssemblyImage;
	}

	MonoObject* ScriptEngine::GetManagedInstance(UUID id)
	{
		PT_CORE_ASSERT(s_Data->EntityInstances.find(id) != s_Data->EntityInstances.end());
		return s_Data->EntityInstances.at(id)->GetManagedObject();
	}

	MonoObject* ScriptEngine::InstantiateClass(MonoClass* monoClass)
	{
		MonoObject* instance = mono_object_new(s_Data->AppDomain, monoClass);
		mono_runtime_object_init(instance);
		return instance;
	}

	ScriptClass::ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore)
		:
		m_ClassNamespace(classNamespace),
		m_ClassName(className)
	{
		m_MonoClass = mono_class_from_name(isCore ? s_Data->CoreAssemblyImage : s_Data->AppAssemblyImage, classNamespace.c_str(), className.c_str());
	}

	MonoObject* ScriptClass::Instantiate()
	{
		return ScriptEngine::InstantiateClass(m_MonoClass);
	}

	MonoMethod* ScriptClass::GetMethod(const std::string& name, int parameterCount)
	{
		return mono_class_get_method_from_name(m_MonoClass, name.c_str(), parameterCount);
	}

	MonoObject* ScriptClass::InvokeMethod(MonoObject* instance, MonoMethod* monoMethod, void** params)
	{
		if (monoMethod)
		{
			MonoObject* exception = nullptr;
			return mono_runtime_invoke(monoMethod, instance, params, &exception);
		}

		return nullptr;
	}
		
	ScriptInstance::ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity)
		:
		m_ScriptClass(scriptClass)
	{
		m_Instance = scriptClass->Instantiate();

		m_Constructor = s_Data->EntityClass.GetMethod(".ctor", 1);
		m_OnCreateMethod = scriptClass->GetMethod("OnCreate", 0);
		m_OnUpdateMethod = scriptClass->GetMethod("OnUpdate", 1);

		//Call constructor
		if(entity) 
		{
			UUID entityID = entity.GetUUID();
			void* param = &entityID;
			m_ScriptClass->InvokeMethod(m_Instance, m_Constructor, &param);
		}
	}

	void ScriptInstance::InvokeOnCreate()
	{
		m_ScriptClass->InvokeMethod(m_Instance, m_OnCreateMethod);
	}

	void ScriptInstance::InvokeOnUpdate(float ts)
	{
		void* param = &ts;
		m_ScriptClass->InvokeMethod(m_Instance, m_OnUpdateMethod, &param);
	}

	bool ScriptInstance::GetFieldValueInternal(const std::string& name, void* buffer) const
	{
		const auto& fields = m_ScriptClass->GetFields();
		auto it = fields.find(name);
		if (it == fields.end())
			return false;

		const ScriptField field = it->second;
		mono_field_get_value(m_Instance, field.ClassField, buffer);
		return true;
	}

	bool ScriptInstance::SetFieldValueInternal(const std::string& name, const void* value)
	{
		const auto& fields = m_ScriptClass->GetFields();
		auto it = fields.find(name);
		if (it == fields.end())
			return false;

		const ScriptField field = it->second;
		mono_field_set_value(m_Instance, field.ClassField, (void*)value);
		return true;
	}
}