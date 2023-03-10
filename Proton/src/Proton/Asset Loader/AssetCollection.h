#pragma once
#include "Proton/Core/Core.h"
#include <unordered_map>
#include <string>
#include <assert.h>

namespace Proton
{
	class AssetCollection
	{
	public:
		template<typename T>
		static void Add(std::string path, Ref<T> asset)
		{
			m_Collection[path] = asset;
		}

		template<typename T>
		static Ref<T> Get(std::string path)
		{
			PT_CORE_ASSERT(m_Collection.find(path) != m_Collection.end(), "Key not found");
			return std::static_pointer_cast<T>(m_Collection.at(path));
		}
	private:
		static std::unordered_map<std::string, Ref<void>> m_Collection;
	};
}