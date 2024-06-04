#pragma once
#include "Proton/Core/UUID.h"

namespace Proton
{
	class AssetHandle
	{
	public:
		enum AssetType { MODEL, IMAGE };

		AssetHandle() = delete;
		AssetHandle(UUID uuid, AssetType type)
			: UUID(uuid), Type(type)
		{}

		~AssetHandle() = default;
	public:
		UUID UUID;
		AssetType Type;
	};
}