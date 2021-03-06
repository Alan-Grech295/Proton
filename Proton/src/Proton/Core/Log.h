#pragma once

#include "Core.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

namespace Proton
{
	class Log
	{
	public:
		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};
}

//Core Log Macros
#define PT_CORE_TRACE(...)     ::Proton::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define PT_CORE_INFO(...)      ::Proton::Log::GetCoreLogger()->info(__VA_ARGS__)
#define PT_CORE_WARN(...)      ::Proton::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define PT_CORE_ERROR(...)     ::Proton::Log::GetCoreLogger()->error(__VA_ARGS__)
#define PT_CORE_FATAL(...)     ::Proton::Log::GetCoreLogger()->fatal(__VA_ARGS__)

//Client Log Macros
#define PT_TRACE(...)          ::Proton::Log::GetClientLogger()->trace(__VA_ARGS__)
#define PT_INFO(...)           ::Proton::Log::GetClientLogger()->info(__VA_ARGS__)
#define PT_WARN(...)           ::Proton::Log::GetClientLogger()->warn(__VA_ARGS__)
#define PT_ERROR(...)          ::Proton::Log::GetClientLogger()->error(__VA_ARGS__)
#define PT_FATAL(...)          ::Proton::Log::GetClientLogger()->fatal(__VA_ARGS__)

#ifdef PT_PLATFORM_WINDOWS
#define GET_ERROR(hr) char* msgBuf = nullptr;\
					     DWORD msgLen = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | \
												   FORMAT_MESSAGE_FROM_SYSTEM | \
												   FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, hr, \
												   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<LPSTR>(&msgBuf), 0, nullptr);\
						 if(msgLen == 0)\
						 {\
							 PT_CORE_ERROR("Unidentified error code!");\
						 }\
						 std::string errorString = msgBuf;\
						 if(hr == 0)\
						 {\
							PT_CORE_INFO("{0}", errorString);\
						 }\
						 else\
						 {\
							LocalFree(msgBuf);\
							PT_CORE_ERROR("Error at line {0} in file {1} \n\t\t[Error Code] {2}\n\t\t[Description] {3}", __LINE__, __FILE__, hr, errorString);\
						 }

#define LAST_ERROR() GET_ERROR(GetLastError());
#endif

