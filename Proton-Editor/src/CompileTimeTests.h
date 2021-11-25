#pragma once
#include <Proton.h>
#include <vector>
#include <string>
#include <assert.h>

namespace Proton
{
	class Base
	{
	public:
		virtual void Update() = 0;
	};

	class Base2 : Base
	{
		friend class Other;
	public:
		virtual void Update() = 0;

	private:
		void Bind() {}
	};

	class Derived : Base2
	{
	public:
		virtual void Update() override {}
	};

	class Other
	{
	public:
		Other()
		{
			Base2* b;
		}
	};
}