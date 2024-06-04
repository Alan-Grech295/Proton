#pragma once
#include "Proton/Renderer/Bindables/Binds.h"

namespace Proton
{
	class Material
	{
	public:
		class Pass
		{
		public:
			Pass(const std::string& passName);

			void AddBindable(Ref<Bindable> bindable)
			{
				m_Bindables.push_back(bindable);
			}
		public:
			Ref<VertexShader> m_VertexShader;
			Ref<PixelShader> m_PixelShader;
			std::vector<Ref<Bindable>> m_Bindables;
			int m_PassID = -1;
		};

	public:
		Material()
		{}

		void AddPass(Ref<Pass> pass)
		{
			m_Passes.push_back(std::move(pass));
		}
	public:
		std::vector<Ref<Pass>> m_Passes;

		bool hasSpecular = false;
		bool hasNormalMap = false;
		bool hasDiffuseMap = false;
	};
}