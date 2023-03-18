#pragma once
#include "Proton\Core\Core.h"
#include <DirectXMath.h>
#include <vector>

namespace Proton
{
	enum class FramebufferTextureFormat
	{
		None = 0,
		
		// Color
		RGBA8,
		RINT,

		// Depth
		DEPTH32F,

		// Depth/Stencil
		DEPTH24STENCIL8,

		//Defaults
		DEPTH = DEPTH32F
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
		FramebufferTextureSpecification(FramebufferTextureFormat format)
			: TextureFormat(format)//, TextureUsage(usage) 
		{}

		FramebufferTextureFormat TextureFormat = FramebufferTextureFormat::None;
		//FramebufferTextureUsage TextureUsage = FramebufferTextureUsage::None;

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