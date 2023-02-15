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

namespace Proton
{
#define PT_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Proton.InternalCalls::" #Name, Name)

	static void NativeLog(MonoString* message, int parameter)
	{
		char* cStr = mono_string_to_utf8(message);
		std::string str(cStr);
		mono_free(cStr);

		PT_CORE_TRACE("{}, {}", str, parameter);
	}

	static void NativeLog_Vec(DirectX::XMFLOAT3* param, DirectX::XMFLOAT3* outResult)
	{
		PT_CORE_WARN("{}, {}, {}", param->x, param->y, param->z);

		*outResult = DirectX::XMFLOAT3(param->z, param->y, param->x);
	}

	static void Entity_GetTranslation(UUID entityID, DirectX::XMFLOAT3* outTranslation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();

		Entity entity = scene->GetEntityByUUID(entityID);
		*outTranslation = entity.GetComponent<TransformComponent>().position;
	}

	static void Entity_SetTranslation(UUID entityID, DirectX::XMFLOAT3* translation)
	{
		Scene* scene = ScriptEngine::GetSceneContext();

		Entity entity = scene->GetEntityByUUID(entityID);
		entity.GetComponent<TransformComponent>().position = *translation;
	}

	static bool Input_IsKeyDown(KeyCode keyCode)
	{
		return Input::IsKeyPressed(keyCode);
	}

	void ScriptGlue::RegisterFunctions()
	{
		PT_ADD_INTERNAL_CALL(NativeLog);
		PT_ADD_INTERNAL_CALL(NativeLog_Vec);

		PT_ADD_INTERNAL_CALL(Entity_GetTranslation);
		PT_ADD_INTERNAL_CALL(Entity_SetTranslation);

		PT_ADD_INTERNAL_CALL(Input_IsKeyDown);
	}
}