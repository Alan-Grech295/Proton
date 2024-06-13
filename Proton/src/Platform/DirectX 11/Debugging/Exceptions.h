#pragma once
#include <string>
#include <sstream>
#define WIN32_LEAN_AND_MEAN
#define NOGDICAPMASKS
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOSYSCOMMANDS
#define NORASTEROPS
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOKERNEL
#define NONLS
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#define NORPC
#define NOPROXYSTUB
#define NOIMAGE
#define NOTAPE

#define STRICT

#include <Windows.h>
#include <vector>
#include "dxerr/dxerr.h"
#include "DxgiInfoManager.h"

namespace Proton
{
	class Exception : public std::exception
	{
	public:
		Exception(int line, const char* file) noexcept
			:
			Line(line),
			File(file)
		{}

		const char* what() const noexcept override
		{
			std::ostringstream oss;
			oss << GetType() << std::endl
				<< GetOriginString();
			whatBuffer = oss.str();
			return whatBuffer.c_str();
		}

		virtual const char* GetType() const noexcept { return "Exception"; }
		std::string GetOriginString() const noexcept
		{
			std::ostringstream oss;
			oss << "[File] " << Line << std::endl
				<< "[Line] " << File;
			return oss.str();
		}
	public:
		int Line;
		std::string File;
	protected:
		mutable std::string whatBuffer;
	};

	class HrException : public Exception
	{
	public:
		HrException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMsgs = {}) noexcept
			: Exception(line, file), hr(hr)
		{
			// join all info messages with newlines into single string
			for (const auto& m : infoMsgs)
			{
				info += m;
				info.push_back('\n');
			}

			// remove final newline if exists
			if (!info.empty())
			{
				info.pop_back();
			}
		}

		const char* what() const noexcept override
		{
			std::ostringstream oss;
			oss << GetType() << std::endl
				<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
				<< std::dec << " (" << (unsigned long)GetErrorCode() << ")" << std::endl
				<< "[Error String] " << GetErrorString() << std::endl
				<< "[Description] " << GetErrorDescription() << std::endl;
			if (!info.empty())
			{
				oss << "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
			}
			oss << GetOriginString();
			whatBuffer = oss.str();
			return whatBuffer.c_str();
		}

		const char* GetType() const noexcept override { return "Graphics Exception"; }
		HRESULT GetErrorCode() const noexcept { return hr; }
		std::string GetErrorString() const noexcept { return DXGetErrorStringA(hr); }
		std::string GetErrorDescription() const noexcept
		{
			char buf[512];
			DXGetErrorDescriptionA(hr, buf, sizeof(buf));
			return buf;
		}

		std::string GetErrorInfo() const noexcept { return info; }
	private:
		HRESULT hr;
		std::string info;
	};
	class DeviceRemovedException : public HrException
	{
		using HrException::HrException;
	public:
		const char* GetType() const noexcept override { return "Graphics Exception [Device Removed] (DXGI_ERROR_DEVICE_REMOVED)"; }
	private:
		std::string reason;
	};

#if PT_DEBUG
	inline static DxgiInfoManager infoManager;
	#define GFX_EXCEPT(hr) HrException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )
	#define GFX_THROW_INFO(hrcall) infoManager.Set(); {HRESULT hr; if( FAILED( hr = (hrcall) ) ) throw GFX_EXCEPT(hr);}
	#define GFX_DEVICE_REMOVED_EXCEPT(hr) DeviceRemovedException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )
#else
	#define GFX_EXCEPT(hr) HrException( __LINE__,__FILE__,(hr) )
	#define GFX_THROW_INFO(hrcall) hrcall
	#define GFX_DEVICE_REMOVED_EXCEPT(hr) DeviceRemovedException( __LINE__,__FILE__,(hr)) 
#endif
}