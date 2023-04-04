#pragma once
#include "Proton\Core\Base.h"
#include "Proton\Core\Core.h"
#include "Proton/Renderer/Bindables/Buffer.h"
#include "Proton/Renderer/Bindables/Shader.h"
#include <DirectXMath.h>
#include <vector>

namespace Proton
{						//Format, Clear Colour Size
#define TEXTURE_FORMAT	X(RGBA8, DirectX::XMFLOAT4) \
						X(RINT, int) \
						X(DEPTH32F, float) \
						X(DEPTH24STENCIL8, DirectX::XMFLOAT2)

	enum class FramebufferTextureFormat
	{
		None = 0,
		
		// Color
#define X(format, col) format,
		TEXTURE_FORMAT
#undef X

		//Defaults
		DEPTH = DEPTH32F
	};

	inline static std::unordered_map<FramebufferTextureFormat, std::string> TextureFormatToString = {
#define X(format, col) {FramebufferTextureFormat::format, #format},
			TEXTURE_FORMAT
		#undef X
	};

	/*enum class FramebufferTextureUsage
	{
		None = 0,

		GPU_RW,
		GPU_R,
		GPU_R_CPU_W,
		GPU_CPU_RW
	};*/

	struct FramebufferTextureSpecification
	{
		FramebufferTextureSpecification() = default;

		template<typename T>
		FramebufferTextureSpecification(FramebufferTextureFormat format, const T& clearCol)
			: TextureFormat(format)
		{
#define X(fmt, col) if(format == FramebufferTextureFormat::fmt) \
					{ \
						if (std::is_same<T, col>::value) \
						{ \
							ClearColor = malloc(sizeof(T)); \
							memcpy(ClearColor, &clearCol, sizeof(T)); \
						} \
						else { PT_CORE_ERROR("Clear colour does not match format!\nFormat given: {}\nColour given: {}\nExpected Colour: {}", TextureFormatToString.at(format), typeid(T).name(), #col); } \
					}
			TEXTURE_FORMAT
#undef X

				// Setting shaders and buffers
				switch (format)
				{
				case FramebufferTextureFormat::RINT:
					ClearVS = VertexShader::Create(CoreUtils::CORE_PATH_STR + "Proton\\IntClearVS.cso");
					ClearPS = PixelShader::Create(CoreUtils::CORE_PATH_STR + "Proton\\IntClearPS.cso");
					DCB::RawLayout layout;
					layout.Add(DCB::Type::Int, "clearCol");
					ClearCBuf = PixelConstantBuffer::CreateUnique(0, DCB::CookedLayout(std::move(layout)));
					(*ClearCBuf)["clearCol"] = *(int*)ClearColor;
					break;
				}
		}

		FramebufferTextureFormat TextureFormat = FramebufferTextureFormat::None;
		void* ClearColor = nullptr;
		
		Ref<VertexShader> ClearVS;
		Ref<PixelShader> ClearPS;
		Ref<PixelConstantBuffer> ClearCBuf;
		//TODO: Filtering/wrap
	};

	struct FramebufferAttachmentSpecification
	{
		FramebufferAttachmentSpecification() = default;
		FramebufferAttachmentSpecification(std::initializer_list<FramebufferTextureSpecification> attachments)
			:
			Attachments(attachments) {}

		std::vector<FramebufferTextureSpecification> Attachments;
	};

	struct FramebufferDescription
	{
		uint32_t Width, Height;
		FramebufferAttachmentSpecification Attachments;
		uint32_t Samples = 1;
		float* ClearColor = new float[4]{ 0, 0, 0, 0 };

		//UNUSED
		bool SwapChainTarget = false;
	};

	class Framebuffer
	{
	public:
		virtual void Bind() = 0;

		virtual const FramebufferDescription& GetDescription() const = 0;

		virtual void* GetRenderTextureID(uint32_t index) = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;

		template<typename T>
		inline T ReadPixel(uint32_t targetIndex, int x, int y) { return *(T*)ReadPixel_Impl(targetIndex, x, y, (uint32_t)sizeof(T)); }

		virtual void* ReadPixel_Impl(uint32_t targetIndex, int x, int y, uint32_t size) = 0;

		virtual void Clear() = 0;

		static Ref<Framebuffer> Create(const FramebufferDescription& desc);
	};
}