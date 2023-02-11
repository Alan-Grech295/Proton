#include "ptpch.h"
#include "AllocationTest.h"

namespace Proton
{
	
	MemTest::MemTest()
	{
		str = new TestStruct();
	}

	MemTest::~MemTest()
	{
		delete str;
	}

	void MemTest::Run()
	{
		delete str->m_Name;
		str->m_Name = "Whats up";
	}

	MemTest::TestStruct::TestStruct()
	{
		m_Name = "Hello world";
	}

	MemTest::TestStruct::~TestStruct()
	{
		delete m_Name;
	}
}

