#pragma once
#include <Windows.h>
#include <string>
#include <vector>
#include <dxgidebug.h>

namespace Proton
{
	class DxgiInfoManager
	{
	public:
		DxgiInfoManager();
		~DxgiInfoManager();
		DxgiInfoManager( const DxgiInfoManager& ) = delete;
		DxgiInfoManager& operator=( const DxgiInfoManager& ) = delete;
		void Set() noexcept;
		std::vector<std::string> GetMessages() const;
	private:
		unsigned long long next = 0u;
		struct IDXGIInfoQueue* pDxgiInfoQueue = nullptr;
	};
}