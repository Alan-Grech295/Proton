#pragma once
#include "Bindable.h"
#include <string>

namespace Proton
{
	class Blender : public Bindable
	{
	public:
		virtual ~Blender() {}

		virtual void Bind() = 0;

		virtual Ref<Bindable> Clone() override
		{
			return Clone(*this);
		}

		virtual std::string GetUID() const noexcept = 0;

		template<typename...Ignore>
		static std::string GenerateUID(const std::string& tag, bool blending, Ignore&&...ignore) { return tag + (blending ? "b" : "n"); }

		static Ref<Blender> Create(const std::string& tag, bool blending);

		static Scope<Blender> CreateUnique(bool blending);

		static Scope<Blender> CreateUnique(Ref<Bindable> other);
		static Ref<Blender> Clone(const Blender& other);
	protected:
		bool m_Blending;
	};
}