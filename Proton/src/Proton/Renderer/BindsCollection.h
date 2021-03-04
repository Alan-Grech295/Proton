#pragma once
#include <unordered_map>
#include <memory>
#include <type_traits>

namespace Proton
{
	class BindsCollection
	{
	public:
		template<class T, typename...Params>
		static Ref<T> Resolve(Params&&... p)
		{
			return Get().Resolve_<T>(std::forward<Params>(p)...);
		}
	private:
		template<class T, typename...Params>
		Ref<T> Resolve_(Params&&... p)
		{
			using namespace std::string_literals;
			const std::string key = std::string(typeid(T).name()) + "#"s + T::GenerateUID(std::forward<Params>(p)...);
			const auto i = binds.find(key);

			if (i == binds.end())
			{
				auto bind = std::make_shared<T>(std::forward<Params>(p)...);
				binds[key] = bind;
				return bind;
			}
			else
			{
				return std::static_pointer_cast<T>(i->second);
			}
		}

		static BindsCollection& Get()
		{
			static BindsCollection collection;
			return collection;
		}
	private:
		std::unordered_map<std::string, Ref<void>> binds;
	};
}