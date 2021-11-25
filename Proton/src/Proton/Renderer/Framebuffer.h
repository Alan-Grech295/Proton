#pragma once
#include "Proton\Core\Core.h"
#include <DirectXMath.h>

namespace Proton
{
	enum class FramebufferTextureFormat
	{
		None = 0,
		
		//Color
		RGBA8 = 1,

		//Depth
		DEPTH32F,

		//Defaults
		DEPTH = DEPTH32F
	};

	struct FramebufferTextureSpecification
	{
		FramebufferTextureSpecification() = default;
		FramebufferTextureSpecification(FramebufferTextureFormat format)
			:
			TextureFormat(format) {}

		FramebufferTextureFormat TextureFormat = FramebufferTextureFormat::None;
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

		bool SwapChainTarget = false;
	};

	class Framebuffer
	{
	public:
		virtual void Bind() = 0;

		virtual const FramebufferDescription& GetDescription() const = 0;

		virtual void* GetRenderTextureID(uint32_t index) = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;

		virtual void Clear() = 0;

		static Ref<Framebuffer> Create(const FramebufferDescription& desc);
	};
}