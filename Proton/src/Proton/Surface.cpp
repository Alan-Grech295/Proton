#include "ptpch.h"
#include "Surface.h"
#include <algorithm>
#include "Proton/Log.h"
#include <filesystem>

namespace fs = std::filesystem;

namespace Gdiplus
{
	using std::min;
	using std::max;
}
#include <gdiplus.h>
#include <sstream>

#pragma comment( lib,"gdiplus.lib" )
#pragma comment( lib,"d3d11.lib" )
#pragma comment( lib,"DirectXTK.lib" )

namespace Proton
{
	Surface::Surface(unsigned int width, unsigned int height) noexcept
		:
		pBuffer(std::make_unique<Color[]>(width* height)),
		width(width),
		height(height)
	{}

	Surface& Surface::operator=(Surface&& donor) noexcept
	{
		width = donor.width;
		height = donor.height;
		pBuffer = std::move(donor.pBuffer);
		donor.pBuffer = nullptr;
		return *this;
	}

	Surface::Surface(Surface&& source) noexcept
		:
		pBuffer(std::move(source.pBuffer)),
		width(source.width),
		height(source.height)
	{}

	Surface::~Surface()
	{}

	void Surface::Clear(Color fillValue) noexcept
	{
		memset(pBuffer.get(), fillValue.dword, width * height * sizeof(Color));
	}

	void Surface::PutPixel(unsigned int x, unsigned int y, Color c) noexcept
	{
		assert(x >= 0);
		assert(y >= 0);
		assert(x < width);
		assert(y < height);
		pBuffer[y * width + x] = c;
	}

	Surface::Color Surface::GetPixel(unsigned int x, unsigned int y) const noexcept
	{
		assert(x >= 0);
		assert(y >= 0);
		assert(x < width);
		assert(y < height);
		return pBuffer[y * width + x];
	}

	unsigned int Surface::GetWidth() const noexcept
	{
		return width;
	}

	unsigned int Surface::GetHeight() const noexcept
	{
		return height;
	}

	Surface::Color* Surface::GetBufferPtr() noexcept
	{
		return pBuffer.get();
	}

	const Surface::Color* Surface::GetBufferPtr() const noexcept
	{
		return pBuffer.get();
	}

	const Surface::Color* Surface::GetBufferPtrConst() const noexcept
	{
		return pBuffer.get();
	}

	Surface Surface::FromFile(const std::string& name)
	{
		unsigned int width = 0;
		unsigned int height = 0;
		std::unique_ptr<Color[]> pBuffer;

		{
			// convert filenam to wide string (for Gdiplus)
			wchar_t wideName[512];
			mbstowcs_s(nullptr, wideName, name.c_str(), _TRUNCATE);

			Gdiplus::Bitmap bitmap(wideName);
			if (bitmap.GetLastStatus() != Gdiplus::Status::Ok)
			{
				std::stringstream ss;
				ss << "Loading image [" << name << "]: failed to load.";
			}

			width = bitmap.GetWidth();
			height = bitmap.GetHeight();
			pBuffer = std::make_unique<Color[]>(width * height);

			for (unsigned int y = 0; y < height; y++)
			{
				for (unsigned int x = 0; x < width; x++)
				{
					Gdiplus::Color c;
					bitmap.GetPixel(x, y, &c);
					pBuffer[y * width + x] = c.GetValue();
				}
			}
		}

		return Surface(width, height, std::move(pBuffer));
	}

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Surface::GetTextureView(ID3D11Device* device, const std::string& path)
	{
		// convert filenam to wide string
		wchar_t wideName[512];
		mbstowcs_s(nullptr, wideName, path.c_str(), _TRUNCATE);

		Microsoft::WRL::ComPtr<ID3D11Resource> texture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRV;

		HRESULT hr = DirectX::CreateWICTextureFromFile(device, wideName, &texture, &textureSRV);

		if (FAILED(hr))
		{
			GET_ERROR(hr);
		}

		return textureSRV;
	}

	void Surface::Save(const std::string& filename) const
	{
		auto GetEncoderClsid = [&filename](const WCHAR* format, CLSID* pClsid) -> void
		{
			UINT  num = 0;          // number of image encoders
			UINT  size = 0;         // size of the image encoder array in bytes

			Gdiplus::ImageCodecInfo* pImageCodecInfo = nullptr;

			Gdiplus::GetImageEncodersSize(&num, &size);
			if (size == 0)
			{
				std::stringstream ss;
				ss << "Saving surface to [" << filename << "]: failed to get encoder; size == 0.";
			}

			pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
			if (pImageCodecInfo == nullptr)
			{
				std::stringstream ss;
				ss << "Saving surface to [" << filename << "]: failed to get encoder; failed to allocate memory.";
			}

			GetImageEncoders(num, size, pImageCodecInfo);

			for (UINT j = 0; j < num; ++j)
			{
				if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
				{
					*pClsid = pImageCodecInfo[j].Clsid;
					free(pImageCodecInfo);
					return;
				}
			}

			free(pImageCodecInfo);
			std::stringstream ss;
			ss << "Saving surface to [" << filename <<
				"]: failed to get encoder; failed to find matching encoder.";
		};

		CLSID bmpID;
		GetEncoderClsid(L"image/bmp", &bmpID);


		// convert filenam to wide string (for Gdiplus)
		wchar_t wideName[512];
		mbstowcs_s(nullptr, wideName, filename.c_str(), _TRUNCATE);

		Gdiplus::Bitmap bitmap(width, height, width * sizeof(Color), PixelFormat32bppARGB, (BYTE*)pBuffer.get());
		if (bitmap.Save(wideName, &bmpID, nullptr) != Gdiplus::Status::Ok)
		{
			std::stringstream ss;
			ss << "Saving surface to [" << filename << "]: failed to save.";
		}
	}

	void Surface::Copy(const Surface& src) noexcept
	{
		assert(width == src.width);
		assert(height == src.height);
		memcpy(pBuffer.get(), src.pBuffer.get(), width * height * sizeof(Color));
	}

	Surface::Surface(unsigned int width, unsigned int height, std::unique_ptr<Color[]> pBufferParam) noexcept
		:
		width(width),
		height(height),
		pBuffer(std::move(pBufferParam))
	{}
}