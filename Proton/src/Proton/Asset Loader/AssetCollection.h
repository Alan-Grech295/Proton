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
		static void Add(std::string path, T& asset)
		{
			T* copy = new T(asset);
			m_Collection[path] = copy;
		}

		template<typename T>
		static T& Get(std::string path)
		{
			if (m_Collection.find(path) != m_Collection.end())
			{
				return *static_cast<T*>(m_Collection[path]);
			}
			else
			{
				assert("Key not found" && false);
			}
		}
	private:
		static std::unordered_map<std::string, void*> m_Collection;
	};
}