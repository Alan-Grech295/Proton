#pragma once
#include <xhash>

namespace Proton
{
	//TODO: Maybe convert to 128-bit UUID
	class UUID
	{
	public:
		UUID();
		UUID(uint64_t uuid);
		UUID(const UUID&) = default;

		operator uint64_t() const { return m_UUID; }
	
	public:
		static UUID Null;

	private:
		uint64_t m_UUID;
	};
}

namespace std
{
	template<>
	struct hash<Proton::UUID>
	{
		std::size_t operator()(const Proton::UUID& uuid) const
		{
			return hash<uint64_t>()((uint64_t)uuid);
		}
	};
}