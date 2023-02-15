#include "ptpch.h"
#include "ScriptGlue.h"
#include "Proton/Core/Log.h"
#include "ScriptEngine.h"
#include "Proton/Scene/Entity.h"
#include "Proton/Scene/Components.h"
#include "Proton/Core/UUID.h"
#include "Proton/Core/KeyCodes.h"
#include "Proton/Core/Input.h"

#include "mono/metadata/object.h"
#include <DirectXMath.h>
#include "mono/metadata/reflection.h"

namespace Proton
{
	static std::unordered_map<MonoType*, std::function<bool(Entity)>> s_EntityHasComponentFuncs;

#define PT_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Proton.InternalCalls::" #Name, Name)

	static bool Entity_HasComponent(uint64_t entityID, MonoReflectionType* componentType)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		PT_CORE_ASSERT(scene);
		Entity entity = scene->GetEntityByUUID(entityID);
		PT_CORE_ASSERT(entity);

		MonoType* monoComponentType = mono_reflection_type_get_type(componentType); 
		PT_CORE_ASSERT(s_EntityHasComponentFuncs.find(monoComponentType) != s_EntityHasComponentFuncs.end());
		return s_EntityHasComponentFuncs.at(monoComponentType)(entity);
	}

	static void TransformComponent_GetPosition(UUID entityID, DirectX::XMFLOAT3* outPosition)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntityByUUID(entityID);

		*outPosition = entity.GetComponent<TransformComponent>().position;
	}

	static void TransformComponent_SetPosition(UUID entityID, DirectX::XMFLOAT3* position)
	{
		Scene* scene = ScriptEngine::GetSceneContext();
		Entity entity = scene->GetEntityByUUID(entityID);

		entity.GetComponent<TransformComponent>().position = *position;
	}

	static bool Input_IsKeyDown(KeyCode keyCode)
	{
		return Input::IsKeyPressed(keyCode);
	}

	void ScriptGlue::RegisterFunctions()
	{
		PT_ADD_INTERNAL_CALL(TransformComponent_GetPosition);
		PT_ADD_INTERNAL_CALL(TransformComponent_SetPosition);

		PT_ADD_INTERNAL_CALL(Input_IsKeyDown);
	}

	template<typename Component>
	static void RegisterComponent()
	{
		std::string_view typeName = typeid(Component).name();
		size_t pos = typeName.find_last_of(':');
		std::string_view structName = typeName.substr(pos + 1);
		std::string managedTypename = fmt::format("Proton.{}", structName);

		MonoType* managedType = mono_reflection_type_from_name(managedTypename.data(), ScriptEngine::GetCoreAssemblyImage());
		s_EntityHasComponentFuncs[managedType] = [](Entity entity) { return entity.HasComponent<Component>(); };
	}

	void ScriptGlue::RegisterComponents()
	{
		RegisterComponent<TransformComponent>();
	}
}