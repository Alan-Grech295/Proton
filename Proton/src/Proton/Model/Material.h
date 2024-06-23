#pragma once
#include "Proton/Renderer/Bindables/Binds.h"
#include "Proton/Core/UUID.h"

namespace Proton
{
	class Material
	{
	public:
		Material(const std::string& passName, const std::string& name, UUID assetID);
		Material(const std::string& passName);
		Material(int passID, const std::string& name, UUID assetID);
		Material();

		void AddBindable(Ref<Bindable> bindable)
		{
			m_Bindables.push_back(bindable);
		}

		template<typename T>
		const Ref<T> GetBindable() const
		{
			static_assert(std::is_base_of<Bindable, T>::value, "Type has to be a bindable");
			for (Ref<Bindable> bind : m_Bindables)
			{
				if (dynamic_cast<T*>(bind.get()))
				{
					return CastRef<T>(bind);
				}
			}

			return nullptr;
		}

		template<typename T>
		void SetBindable(Ref<T> bindable)
		{
			static_assert(std::is_base_of<Bindable, T>::value, "Type has to be a bindable");
			for (Ref<Bindable>& bind : m_Bindables)
			{
				if (dynamic_cast<T*>(bind.get()))
				{
					bind = bindable;
					return;
				}
			}
		}

		Ref<Material> Clone();
		
	public:
		std::string m_Name;
		Ref<VertexShader> m_VertexShader;
		Ref<PixelShader> m_PixelShader;
		std::vector<Ref<Bindable>> m_Bindables;
		int m_PassID = -1;

		bool hasSpecular = false;
		bool hasNormalMap = false;
		bool hasDiffuseMap = false;

		UUID assetID;
	};
}