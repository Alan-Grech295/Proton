#pragma once
#include <DirectXMath.h>
#include "Renderer\Buffer.h"
#include "Renderer\Shader.h"

namespace Proton
{
	class SolidSphere
	{
	public:
		SolidSphere(float radius);
		void Update(float dt) noexcept;
		void SetPos(DirectX::XMFLOAT3 pos);
		DirectX::XMMATRIX GetTransformXM() const noexcept;
		void Bind();
	private:
		DirectX::XMFLOAT3 pos = { 1.0f, 1.0f, 1.0f };
		struct PSColorConstant
		{
			DirectX::XMFLOAT3 color = { 1.0f,1.0f,1.0f };
			float padding;
		} colorConst;

		struct Transforms
		{
			DirectX::XMMATRIX modelViewProj;
			DirectX::XMMATRIX model;
		};
	public:
		std::unique_ptr<VertexBuffer> m_VertBuffer;
		std::unique_ptr<IndexBuffer> m_IndexBuffer;
		std::unique_ptr<VertexShader> m_VertShader;
		std::unique_ptr<PixelShader> m_PixelShader;
		std::unique_ptr<VertexConstantBuffer> m_TransformCBuf;
		std::unique_ptr<PixelConstantBuffer> m_MaterialCBuf;
	};
}