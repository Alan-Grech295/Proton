#pragma once
#include "Proton/Core/Core.h"
#include "Proton/Scene/Scene.h"

#include <map>
#include <filesystem>
#include <string>

extern "C" {
	typedef struct _MonoClass MonoClass;
	typedef struct _MonoObject MonoObject;
	typedef struct _MonoMethod MonoMethod;
	typedef struct _MonoAssembly MonoAssembly;
	typedef struct _MonoImage MonoImage;
	typedef struct _MonoClassField MonoClassField;
}

namespace Proton
{
	class Entity;

//TODO: Add string
#define SCRIPT_FIELD_TYPES \
		X(None,		int)\
		X(Float,	float)\
		X(Double,	double)\
		X(Bool,		bool)\
		X(Char,		char)\
		X(SByte,	int8_t)\
		X(Short,	int16_t)\
		X(Int,		int32_t)\
		X(Long,		int64_t)\
		X(Byte,		uint8_t)\
		X(UShort,	uint16_t)\
		X(UInt,		uint32_t)\
		X(ULong,	uint64_t)\
		X(Vector2,	DirectX::XMFLOAT2)\
		X(Vector3,	DirectX::XMFLOAT3)\
		X(Vector4,	DirectX::XMFLOAT4)\
		X(Entity,	UUID)

	//X(String,	std::string)
		

	enum class ScriptFieldType
	{
#define X(t) t,
		SCRIPT_FIELD_TYPES
#undef X
	};

	struct ScriptField
	{
		ScriptFieldType Type;
		std::string Name;

		MonoClassField* ClassField;
	};

	struct ScriptFieldInstance
	{
		friend class ScriptInstance;
		friend class ScriptEngine;
	public:
		ScriptField Field;

		ScriptFieldInstance()
		{
			memset(m_Buffer, 0, sizeof(m_Buffer));
		}

	public:
		template<typename T>
		T GetValue(const std::string& name)
		{
			return *(T*)m_Buffer;
		}

		template<typename T>
		void SetValue(T value)
		{
			static_assert(sizeof(T) <= 16, "Type too large!");
			memcpy(m_Buffer, &value, sizeof(T));
		}
	private:
		uint8_t m_Buffer[16];
	};

	using ScriptFieldMap = std::unordered_map<std::string, ScriptFieldInstance>;

	class ScriptClass
	{
		friend class ScriptEngine;
	public:
		ScriptClass() = default;

		ScriptClass(const std::string& classNamespace, const std::string& className, bool isCore = false);

		MonoObject* Instantiate();

		MonoMethod* GetMethod(const std::string& name, int parameterCount);
		MonoObject* InvokeMethod(MonoObject* instance, MonoMethod* monoMethod, void** params = nullptr);

		const std::map<std::string, ScriptField>& GetFields() { return m_Fields; }
	private:
		std::string m_ClassNamespace;
		std::string m_ClassName;

		std::map<std::string, ScriptField> m_Fields;

		MonoClass* m_MonoClass = nullptr;
	};

	class ScriptInstance
	{
		friend struct ScriptFieldInstance;
		friend class ScriptEngine;
	public:
		ScriptInstance(Ref<ScriptClass> scriptClass, Entity entity);

		void InvokeOnCreate();
		void InvokeOnUpdate(float ts);

		Ref<ScriptClass> GetScriptClass() { return m_ScriptClass; }

		template<typename T>
		T GetFieldValue(const std::string& name) const
		{
			static_assert(sizeof(T) <= 16, "Type too large!");

			bool success = GetFieldValueInternal(name, s_FieldValueBuffer);
			if (!success)
				return T();

			return *(T*)s_FieldValueBuffer;
		}

		template<typename T>
		bool SetFieldValue(const std::string& name, T value)
		{
			static_assert(sizeof(T) <= 16, "Type too large!");

			return SetFieldValueInternal(name, &value);
		}

		MonoObject* GetManagedObject() { return m_Instance; }

	private:
		bool GetFieldValueInternal(const std::string& name, void* buffer) const;
		bool SetFieldValueInternal(const std::string& name, const void* value);
	private:
		Ref<ScriptClass> m_ScriptClass;

		MonoObject* m_Instance = nullptr;
		MonoMethod* m_Constructor = nullptr;
		MonoMethod* m_OnCreateMethod = nullptr;
		MonoMethod* m_OnUpdateMethod = nullptr;

		inline static char s_FieldValueBuffer[16];
	};

	class ScriptEngine
	{
		friend class ScriptClass;
		friend class ScriptGlue;
	public:
		static void Init();
		static void Shutdown();

		static void LoadAssembly(const std::filesystem::path& filepath);
		static void LoadAppAssembly(const std::filesystem::path& filepath);
		static void OnRuntimeStart(Scene* scene);
		static void OnRuntimeStop();

		static bool EntityClassExists(const std::string& fullClassName);
		static const char* const* GetEntityClassNames();
		static const char* GetEntityClassNameByIndex(uint32_t index);
		static int GetEntityClassIndexFromName(std::string_view name);
		
		static void OnCreateEntity(Entity entity);
		static void OnUpdateEntity(Entity entity, float ts);

		static Scene* GetSceneContext();
		static Ref<ScriptInstance> GetEntityScriptInstance(UUID entityID);

		static const ScriptInstance& GetScriptInstanceFromClass(const std::string& className);

		static Ref<ScriptClass> GetEntityClass(const std::string& name);
		static std::unordered_map<std::string, Ref<ScriptClass>>& GetEntityClasses();
		static ScriptFieldMap& GetScriptFieldMap(Entity entity);

		static MonoImage* GetCoreAssemblyImage();

		static MonoObject* GetManagedInstance(UUID id);
	private:
		static void InitMono();
		static void ShutdownMono();

		static MonoObject* InstantiateClass(MonoClass* monoClass);
		static void LoadAssemblyClasses();
	};

	namespace Utils
	{
		inline const char* ScriptFieldTypeToString(ScriptFieldType fieldType)
		{
			switch (fieldType)
			{
#define X(t) case ScriptFieldType::t: return #t;
				SCRIPT_FIELD_TYPES
#undef X
			}

			PT_CORE_ASSERT(false, "Unknown ScriptFieldType");
			return "None";
		}

		inline ScriptFieldType ScriptFieldTypeFromString(std::string_view fieldType)
		{
#define X(t) if(fieldType == #t) { return ScriptFieldType::t; }
			SCRIPT_FIELD_TYPES
#undef X

			PT_CORE_ASSERT(false, "Unknown ScriptFieldType");
			return ScriptFieldType::None;
		}
	}
}