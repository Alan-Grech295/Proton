#pragma once
#include "Bindable.h"
#include <string>

namespace Proton
{
	class Sampler : public Bindable
	{
	public:
		virtual ~Sampler() {}

		virtual void Bind() = 0;

		virtual std::string GetUID() const noexcept = 0;

		template<typename...Ignore>
		static std::string GenerateUID(const std::string& tag, Ignore&&...ignore) { return tag; }

		static Ref<Sampler> Create(const std::string& tag, int slot = 0);

		static Scope<Sampler> CreateUnique(int slot = 0);
	};
}