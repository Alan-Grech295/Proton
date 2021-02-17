#pragma once

namespace Proton
{
	class Sampler
	{
	public:
		virtual ~Sampler() {}

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;

		static Sampler* Create(int slot = 0);
	};
}