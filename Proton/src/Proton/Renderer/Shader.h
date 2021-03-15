#pragma once
#include <string>
#include "Proton\Core\Core.h"

namespace Proton
{
	class PixelShader
	{
	public:
		virtual ~PixelShader() {}

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual std::string GetUID() const noexcept = 0;

		static std::string GenerateUID(const std::string& path) { return path; }

		static Ref<PixelShader> Create(std::string path);

		static Scope<PixelShader> CreateUnique(std::string path);
	};

	class VertexShader
	{
	public:
		virtual ~VertexShader() {}

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		virtual std::string GetUID() const noexcept = 0;

		static std::string GenerateUID(const std::string& path) { return path; }

		static Ref<VertexShader> Create(std::string path);

		static Scope<VertexShader> CreateUnique(std::string path);
	};
}