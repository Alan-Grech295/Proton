#pragma once
#include "Bindable.h"
#include <string>

namespace Proton
{
	class Rasterizer : public Bindable
	{
	public:
		virtual ~Rasterizer() {}

		virtual void Bind() = 0;

		virtual Ref<Bindable> Clone() override
		{
			return Clone(*this);
		}

		virtual std::string GetUID() const noexcept = 0;

		template<typename...Ignore>
		static std::string GenerateUID(const std::string& tag, bool twoSided, Ignore&&...ignore) { return tag + (twoSided ? "t" : "n"); }

		static Ref<Rasterizer> Create(const std::string& tag, bool twoSided);

		static Scope<Rasterizer> CreateUnique(bool twoSided);

		static Scope<Rasterizer> CreateUnique(Ref<Bindable> other);
		static Ref<Rasterizer> Clone(const Rasterizer& other);
	protected:
		bool m_TwoSided;
	};
}