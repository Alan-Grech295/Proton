#pragma once
#include <string>

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

		virtual void Bind(int slot = 0) const = 0;
		virtual void Unbind() const = 0;
		virtual void Load(std::string path) = 0;
		virtual Color GetPixel(int x, int y) const = 0;

		static Texture2D* Create(std::string path);
	};
}