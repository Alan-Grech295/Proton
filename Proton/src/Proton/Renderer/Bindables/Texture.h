#pragma once
#include <string>
#include <cassert>
#include "Bindable.h"
#include "Proton\Renderer\Renderer.h"

namespace Proton
{
	struct Color
	{
		float r, g, b, a;
	};
	 
	class Texture2D : public Bindable
	{
	public:
		virtual ~Texture2D() {}

		virtual void Bind() = 0;

		virtual void Load(const std::string& path) = 0;
		virtual Color GetPixel(int x, int y) const = 0;

		virtual void* GetTexturePointer() const = 0;

		virtual bool HasAlpha() const = 0;

		virtual std::string GetUID() const noexcept = 0;

		inline void SetSlot(int slot) { m_Slot = slot; }

		template<typename...Ignore>
		static std::string GenerateUID(const std::string& path, Ignore&&...ignore) { return path; }

		static Ref<Texture2D> Create(const std::string& path, int slot = 0);

		static Scope<Texture2D> CreateUnique(std::string path, int slot = 0);
		static Scope<Texture2D> CreateUnique(Ref<Bindable> other);
	protected:
		std::string m_Path;
		uint32_t m_Slot;
	};
}