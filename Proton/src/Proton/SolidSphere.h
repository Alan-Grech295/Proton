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
	public:
		struct PSColorConstant
		{
			DirectX::XMFLOAT3 color = { 1.0f,1.0f,1.0f };
			float padding = 0.0f;
		} colorConst;
	private:
		DirectX::XMFLOAT3 pos = { 1.0f, 1.0f, 1.0f };

		struct Transforms
		{
			DirectX::XMMATRIX modelViewProj;
			DirectX::XMMATRIX model;
		};
	public:
		Ref<VertexBuffer> m_VertBuffer;
		Ref<IndexBuffer> m_IndexBuffer;
		Ref<VertexShader> m_VertShader;
		Ref<PixelShader> m_PixelShader;
		Ref<VertexConstantBuffer> m_TransformCBuf;
		Ref<PixelConstantBuffer> m_MaterialCBuf;
	};
}