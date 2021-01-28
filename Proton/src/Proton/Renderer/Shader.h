#pragma once
#include <string>

namespace Proton
{
	class PixelShader
	{
	public:
		virtual ~PixelShader() {}

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		static PixelShader* Create(std::string path);
	};

	class VertexShader
	{
	public:
		virtual ~VertexShader() {}

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		static VertexShader* Create(std::string path);
	};
}