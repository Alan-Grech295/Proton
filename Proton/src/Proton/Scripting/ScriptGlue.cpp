#include "ptpch.h"
#include "ScriptGlue.h"
#include "Proton/Core/Log.h"

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

	void ScriptGlue::RegisterFunctions()
	{
		PT_ADD_INTERNAL_CALL(NativeLog);
		PT_ADD_INTERNAL_CALL(NativeLog_Vec);
	}
}