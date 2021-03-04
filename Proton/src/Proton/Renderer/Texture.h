#pragma once
#include <string>
#include <cassert>
#include "Proton\Renderer\Renderer.h"

namespace Proton
{
	struct Color
	{
		float r, g, b, a;
	};

	class Texture2D
	{
	public:
		virtual ~Texture2D() {}

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual void Load(std::string path) = 0;
		virtual Color GetPixel(int x, int y) const = 0;

		virtual bool HasAlpha() const = 0;

		virtual std::string GetUID() const noexcept = 0;

		template<typename...Ignore>
		static std::string GenerateUID(const std::string& path, Ignore&&...ignore) { return path; }

		static Ref<Texture2D> Create(std::string path, int slot = 0);

		static Scope<Texture2D> CreateUnique(std::string path, int slot = 0);
	};
}