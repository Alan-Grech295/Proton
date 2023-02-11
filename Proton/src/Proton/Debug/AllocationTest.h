#pragma once
#include "MemoryDebug.h"

namespace Proton
{
	class MemTest
	{
		struct TestStruct
		{
			//OVERRIDE_NEW
		public:
			TestStruct();

			~TestStruct();
		public:
			char* m_Name;
		};
	public:
		//OVERRIDE_NEW
		MemTest();

		~MemTest();

		void Run();
	private:
		TestStruct* str;
	};
}