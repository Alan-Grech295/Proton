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

		static Texture2D* Create(std::string path, int slot = 0);
	};
}