#include "ptpch.h"
#include "ScriptEngine.h"
#include "ScriptGlue.h"

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/object.h"

#include "Proton/Scene/Entity.h"

#include <assert.h>

namespace Proton
{
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

		static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& assemblyPath)
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
	}

	struct ScriptEngineData
	{
		MonoDomain* RootDomain = nullptr;
		MonoDomain* AppDomain = nullptr;

		MonoAssembly* CoreAssembly = nullptr;
		MonoImage* CoreAssemblyImage = nullptr;

		ScriptClass EntityClass;

		std::unordered_map<std::string, Ref<ScriptClass>> EntityClasses;
		//TODO: Entity UUID
		std::unordered_map<UUID, Ref<ScriptInstance>> EntityInstances;

		//Runtime
		Scene* SceneContext = nullptr;
	};

	static ScriptEngineData* s_Data;

	std::unordered_map<std::string, Ref<ScriptClass>>& ScriptEngine::GetEntityClasses()
	{
		return s_Data->EntityClasses;
	}

	void ScriptEngine::Init()
	{
		s_Data = new ScriptEngineData();

		InitMono();
		LoadAssembly("Resources/Scripts/Proton-ScriptCore.dll");
		LoadAssemblyClasses(s_Data->CoreAssembly);

		Utils::PrintAssemblyTypes(s_Data->CoreAssembly);

		ScriptGlue::RegisterFunctions();
		ScriptGlue::RegisterComponents();

		//Retrieve and instantiate class with constructor
		s_Data->EntityClass = ScriptClass("Proton", "Entity");

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

		MonoDomain* rootDomain = mono_jit_init("ProtonJITRuntime");
		assert("No root domain" && rootDomain);

		s_Data->RootDomain = rootDomain;
	}

	void ScriptEngine::ShutdownMono()
	{
		//Shutdown not working, to do later
		/*mono_domain_unload(s_Data->AppDomain);
		mono_jit_cleanup(s_Data->RootDomain);*/
		s_Data->AppDomain = nullptr;
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

	void ScriptEngine::OnCreateEntity(Entity entity)
	{
		const auto& sc = entity.GetComponent<ScriptComponent>();

		if (ScriptEngine::EntityClassExists(sc.ClassName))
		{
			Ref<ScriptInstance> instance = CreateRef<ScriptInstance>(s_Data->EntityClasses[sc.ClassName], entity);
			s_Data->EntityInstances[entity.GetUUID()] = instance;
			instance->InvokeOnCreate();
		}
	}

	void ScriptEngine::OnUpdateEntity(Entity entity, float ts)
	{
		UUID entityUUID = entity.GetUUID();
		assert(s_Data->EntityInstances.find(entityUUID) != s_Data->EntityInstances.end());

		Ref<ScriptInstance> instance = s_Data->EntityInstances[entityUUID];
		instance->InvokeOnUpdate(ts);
	}

	Scene* ScriptEngine::GetSceneContext()
	{
		return s_Data->SceneContext;
	}

	void ScriptEngine::OnRuntimeStop()
	{
		s_Data->SceneContext = nullptr;

		s_Data->EntityInstances.clear();
	}

	void ScriptEngine::LoadAssembly(const std::filesystem::path& filepath)
	{
		//Create app domain
		s_Data->AppDomain = mono_domain_create_appdomain("ProtonScriptRuntime", nullptr);
		mono_domain_set(s_Data->AppDomain, true);

		s_Data->CoreAssembly = Utils::LoadMonoAssembly(filepath);
		s_Data->CoreAssemblyImage = mono_assembly_get_image(s_Data->CoreAssembly);
		//PrintAssemblyTypes(s_Data->CoreAssembly);
	}

	void ScriptEngine::LoadAssemblyClasses(MonoAssembly* assembly)
	{
		s_Data->EntityClasses.clear();

		MonoImage* image = mono_assembly_get_image(assembly);
		const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
		int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

		MonoClass* entityClass = mono_class_from_name(image, "Proton", "Entity");

		for (int32_t i = 0; i < numTypes; i++)
		{
			uint32_t cols[MONO_TYPEDEF_SIZE];
			mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

			const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
			const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
			std::string fullName;
			if (strlen(nameSpace) != 0)
				fullName = fmt::format("{}.{}", nameSpace, name);
			else
				fullName = name;

			MonoClass* monoClass = mono_class_from_name(image, nameSpace, name);

			if(monoClass == entityClass)
				continue;

			bool isEntity = mono_class_is_subclass_of(monoClass, entityClass, false);

			if (isEntity)
				s_Data->EntityClasses[fullName] = CreateRef<ScriptClass>(nameSpace, name);
		}
	}

	MonoImage* ScriptEngine::GetCoreAssemblyImage()
	{
		return s_Data->CoreAssemblyImage;
	}

	MonoObject* ScriptEngine::InstantiateClass(MonoClass* monoClass)
	{
		MonoObject* instance = mono_object_new(s_Data->AppDomain, monoClass);
		mono_runtime_object_init(instance);
		return instance;
	}

	ScriptClass::ScriptClass(const std::string& classNamespace, const std::string& className)
		:
		m_ClassNamespace(classNamespace),
		m_ClassName(className)
	{
		m_MonoClass = mono_class_from_name(s_Data->CoreAssemblyImage, classNamespace.c_str(), className.c_str());
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
		return mono_runtime_invoke(monoMethod, instance, params, nullptr);
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

}