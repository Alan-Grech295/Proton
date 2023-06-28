#pragma once
#include "Proton/Core/UUID.h"

namespace Proton
{
	class Asset
	{
	public:
		enum AssetType { Model, Material, Image };

		Asset() = delete;
		Asset(UUID uuid)
			: m_UUID(uuid)
		{}
		virtual ~Asset() {};

		template<typename T>
		T& As() 
		{
			static_assert(std::is_base_of<Asset, T>, "");
			return *static_cast<T*>(this);
		}

	public:
		UUID m_UUID;
	};

	class AssetMetaData
	{
	public:
		AssetMetaData(Asset::AssetType type)
			: m_AssetType(type)
		{}

	public:
		Asset::AssetType m_AssetType;
	};
}