#pragma once
#include <string>
#include "Proton\Core\Core.h"
#include "Bindable.h"

namespace Proton
{
	class PixelShader : public Bindable
	{
	public:
		virtual ~PixelShader() {}

		virtual void Bind() = 0;

		virtual Ref<Bindable> Clone() override
		{
			return Clone(*this);
		}

		virtual std::string GetUID() const noexcept = 0;

		static std::string GenerateUID(const std::string& path) { return path; }

		static Ref<PixelShader> Create(std::string path);

		static Scope<PixelShader> CreateUnique(std::string path);
		static Scope<PixelShader> CreateUnique(Ref<Bindable> other);
		static Ref<PixelShader> Clone(const PixelShader& other);
	protected:
		std::string m_Path;
	};

	class VertexShader : public Bindable
	{
	public:
		virtual ~VertexShader() {}

		virtual void Bind() = 0;

		virtual Ref<Bindable> Clone() override
		{
			return Clone(*this);
		}

		virtual std::string GetUID() const noexcept = 0;

		static std::string GenerateUID(const std::string& path) { return path; }

		static Ref<VertexShader> Create(std::string path);

		static Scope<VertexShader> CreateUnique(std::string path);
		static Scope<VertexShader> CreateUnique(Ref<Bindable> other);
		static Ref<VertexShader> Clone(const VertexShader& other);
	protected:
		std::string m_Path;
	};
}