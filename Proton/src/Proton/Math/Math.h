#pragma once
#include <DirectXMath.h>

namespace Proton
{
#define IS_VEC_FLT_EQUAL(x) (x & 2)
#define IS_VEC_MOD_LAST(x) (x & 1)

#define MOD_VEC(x) x = 1
#define MOD_FLT(x) x = 0
#define VEC_FLT_EQUAL(x) x = 2

	class Vector2
	{
	public:
		Vector2()
			:
			m_Float2({0, 0}),
			m_Vector(DirectX::XMVectorSet(0, 0, 0, 0)),
			m_Modified(0)
		{
		}

		Vector2(float x, float y)
			:
			m_Float2({x, y}),
			m_Vector(DirectX::XMVectorSet(x, y, 0, 0)),
			m_Modified(0)
		{
		}

		static float Dot(Vector2& a, Vector2& b)
		{
			a.UpdateVec();
			b.UpdateVec();

			return DirectX::XMVectorGetX(DirectX::XMVector2Dot(a.m_Vector, b.m_Vector));
		}

		//Vector-Vector Operations
		Vector2 operator+(Vector2& other)
		{
			UpdateVec();
			other.UpdateVec();

			return Vector2(DirectX::XMVectorAdd(m_Vector, other.m_Vector));
		}

		Vector2 operator-(Vector2& other)
		{
			UpdateVec();
			other.UpdateVec();

			return Vector2(DirectX::XMVectorSubtract(m_Vector, other.m_Vector));
		}

		Vector2 operator*(Vector2& other)
		{
			UpdateVec();
			other.UpdateVec();

			return Vector2(DirectX::XMVectorMultiply(m_Vector, other.m_Vector));
		}

		Vector2 operator/(Vector2& other)
		{
			UpdateVec();
			other.UpdateVec();

			return Vector2(DirectX::XMVectorDivide(m_Vector, other.m_Vector));
		}

		//Equal Operators
		void operator+=(Vector2& other)
		{
			UpdateVec();
			other.UpdateVec();

			m_Vector = DirectX::XMVectorAdd(m_Vector, other.m_Vector);
			MOD_VEC(m_Modified);
		}

		void operator-=(Vector2& other)
		{
			UpdateVec();
			other.UpdateVec();

			m_Vector = DirectX::XMVectorSubtract(m_Vector, other.m_Vector);
			MOD_VEC(m_Modified);
		}

		void operator*=(Vector2& other)
		{
			UpdateVec();
			other.UpdateVec();

			m_Vector = DirectX::XMVectorMultiply(m_Vector, other.m_Vector);
			MOD_VEC(m_Modified);
		}

		void operator/=(Vector2& other)
		{
			UpdateVec();
			other.UpdateVec();

			m_Vector = DirectX::XMVectorDivide(m_Vector, other.m_Vector);
			MOD_VEC(m_Modified);
		}

		//Vector-Scalar Operations
		Vector2 operator+(float other)
		{
			UpdateVec();

			return Vector2(DirectX::XMVectorAdd(m_Vector, DirectX::XMVectorSet(other, other, 0, 0)));
		}

		Vector2 operator-(float other)
		{
			UpdateVec();

			return Vector2(DirectX::XMVectorSubtract(m_Vector, DirectX::XMVectorSet(other, other, 0, 0)));
		}

		Vector2 operator*(float other)
		{
			UpdateVec();

			return Vector2(DirectX::XMVectorMultiply(m_Vector, DirectX::XMVectorSet(other, other, 0, 0)));
		}

		Vector2 operator/(float other)
		{
			UpdateVec();

			return Vector2(DirectX::XMVectorDivide(m_Vector, DirectX::XMVectorSet(other, other, 0, 0)));
		}

		//Equal Operators
		void operator+=(float other)
		{
			UpdateVec();

			m_Vector = DirectX::XMVectorAdd(m_Vector, DirectX::XMVectorSet(other, other, 0, 0));
			MOD_VEC(m_Modified);
		}

		void operator-=(float other)
		{
			UpdateVec();

			m_Vector = DirectX::XMVectorSubtract(m_Vector, DirectX::XMVectorSet(other, other, 0, 0));
			MOD_VEC(m_Modified);
		}

		void operator*=(float other)
		{
			UpdateVec();

			m_Vector = DirectX::XMVectorMultiply(m_Vector, DirectX::XMVectorSet(other, other, 0, 0));
			MOD_VEC(m_Modified);
		}

		void operator/=(float other)
		{
			UpdateVec();

			m_Vector = DirectX::XMVectorDivide(m_Vector, DirectX::XMVectorSet(other, other, 0, 0));
			MOD_VEC(m_Modified);
		}

		//Retrieve components
		const float GetX()
		{
			UpdateFlt();
			return m_Float2.x;
		}

		const float GetY()
		{
			UpdateFlt();
			return m_Float2.y;
		}

		//Set components
		void SetX(float x)
		{
			m_Float2.x = x;
			MOD_FLT(m_Modified);
		}

		void SetY(float y)
		{
			m_Float2.y = y;
			MOD_FLT(m_Modified);
		}
	private:
		Vector2(DirectX::XMFLOAT2& flt)
			:
			m_Float2(flt),
			m_Vector(DirectX::XMLoadFloat2(&flt)),
			m_Modified(2)
		{
		}

		Vector2(DirectX::XMVECTOR& vec)
			:
			m_Float2({0, 0}),
			m_Vector(vec),
			m_Modified(2)
		{
			DirectX::XMStoreFloat2(&m_Float2, vec);
		}

		void UpdateVec()
		{
			if (!IS_VEC_FLT_EQUAL(m_Modified) && !IS_VEC_MOD_LAST(m_Modified))
			{
				m_Vector = DirectX::XMLoadFloat2(&m_Float2);
				VEC_FLT_EQUAL(m_Modified);
			}
		}

		void UpdateFlt()
		{
			if (!IS_VEC_FLT_EQUAL(m_Modified) && IS_VEC_MOD_LAST(m_Modified))
			{
				DirectX::XMStoreFloat2(&m_Float2, m_Vector);
				VEC_FLT_EQUAL(m_Modified);
			}
		}
	private:
		DirectX::XMVECTOR m_Vector;
		DirectX::XMFLOAT2 m_Float2;
		char m_Modified;
	};

	class Vector3
	{
	public:
		Vector3()
			:
			m_Float3({0, 0, 0}),
			m_Vector(DirectX::XMVectorSet(0, 0, 0, 0)),
			m_Modified(0)
		{
		}

		Vector3(float x, float y, float z)
			:
			m_Float3({ x, y, z }),
			m_Vector(DirectX::XMVectorSet(x, y, z, 0)),
			m_Modified(0)
		{
		}

		static float Dot(Vector3& a, Vector3& b)
		{
			a.UpdateVec();
			b.UpdateVec();

			return DirectX::XMVectorGetX(DirectX::XMVector3Dot(a.m_Vector, b.m_Vector));
		}

		//Vector-Vector Operations
		Vector3 operator+(Vector3& other)
		{
			UpdateVec();
			other.UpdateVec();

			return Vector3(DirectX::XMVectorAdd(m_Vector, other.m_Vector));
		}

		Vector3 operator-(Vector3& other)
		{
			UpdateVec();
			other.UpdateVec();

			return Vector3(DirectX::XMVectorSubtract(m_Vector, other.m_Vector));
		}

		Vector3 operator*(Vector3& other)
		{
			UpdateVec();
			other.UpdateVec();

			return Vector3(DirectX::XMVectorMultiply(m_Vector, other.m_Vector));
		}

		Vector3 operator/(Vector3& other)
		{
			UpdateVec();
			other.UpdateVec();

			return Vector3(DirectX::XMVectorDivide(m_Vector, other.m_Vector));
		}

		//Equal Operators
		void operator+=(Vector3& other)
		{
			UpdateVec();
			other.UpdateVec();

			m_Vector = DirectX::XMVectorAdd(m_Vector, other.m_Vector);
			MOD_VEC(m_Modified);
		}

		void operator-=(Vector3& other)
		{
			UpdateVec();
			other.UpdateVec();

			m_Vector = DirectX::XMVectorSubtract(m_Vector, other.m_Vector);
			MOD_VEC(m_Modified);
		}

		void operator*=(Vector3& other)
		{
			UpdateVec();
			other.UpdateVec();

			m_Vector = DirectX::XMVectorMultiply(m_Vector, other.m_Vector);
			MOD_VEC(m_Modified);
		}

		void operator/=(Vector3& other)
		{
			UpdateVec();
			other.UpdateVec();

			m_Vector = DirectX::XMVectorDivide(m_Vector, other.m_Vector);
			MOD_VEC(m_Modified);
		}

		//Vector-Scalar Operations
		Vector3 operator+(float other)
		{
			UpdateVec();

			return Vector3(DirectX::XMVectorAdd(m_Vector, DirectX::XMVectorSet(other, other, other, 0)));
		}

		Vector3 operator-(float other)
		{
			UpdateVec();

			return Vector3(DirectX::XMVectorSubtract(m_Vector, DirectX::XMVectorSet(other, other, other, 0)));
		}

		Vector3 operator*(float other)
		{
			UpdateVec();

			return Vector3(DirectX::XMVectorMultiply(m_Vector, DirectX::XMVectorSet(other, other, other, 0)));
		}

		Vector3 operator/(float other)
		{
			UpdateVec();

			return Vector3(DirectX::XMVectorDivide(m_Vector, DirectX::XMVectorSet(other, other, other, 0)));;
		}

		//Equal Operators
		void operator+=(float other)
		{
			UpdateVec();

			m_Vector = DirectX::XMVectorAdd(m_Vector, DirectX::XMVectorSet(other, other, other, 0));
			MOD_VEC(m_Modified);
		}

		void operator-=(float other)
		{
			UpdateVec();

			m_Vector = DirectX::XMVectorSubtract(m_Vector, DirectX::XMVectorSet(other, other, other, 0));
			MOD_VEC(m_Modified);
		}

		void operator*=(float other)
		{
			UpdateVec();

			m_Vector = DirectX::XMVectorMultiply(m_Vector, DirectX::XMVectorSet(other, other, other, 0));
			MOD_VEC(m_Modified);
		}

		void operator/=(float other)
		{
			UpdateVec();

			m_Vector = DirectX::XMVectorDivide(m_Vector, DirectX::XMVectorSet(other, other, other, 0));
			MOD_VEC(m_Modified);
		}

		//Retrieve components
		const float GetX()
		{
			UpdateFlt();
			return m_Float3.x;
		}

		const float GetY()
		{
			UpdateFlt();
			return m_Float3.y;
		}

		const float GetZ()
		{
			UpdateFlt();
			return m_Float3.z;
		}

		//Set components
		void SetX(float x)
		{
			m_Float3.x = x;
			MOD_FLT(m_Modified);
		}

		void SetY(float y)
		{
			m_Float3.y = y;
			MOD_FLT(m_Modified);
		}

		void SetZ(float z)
		{
			m_Float3.z = z;
			MOD_FLT(m_Modified);
		}
	private:
		Vector3(DirectX::XMFLOAT3& flt)
			:
			m_Float3(flt),
			m_Vector(DirectX::XMLoadFloat3(&flt)),
			m_Modified(2)
		{
		}

		Vector3(DirectX::XMVECTOR& vec)
			:
			m_Float3({ 0, 0, 0 }),
			m_Vector(vec),
			m_Modified(2)
		{
			DirectX::XMStoreFloat3(&m_Float3, vec);
		}

		void UpdateVec()
		{
			if (!IS_VEC_FLT_EQUAL(m_Modified) && !IS_VEC_MOD_LAST(m_Modified))
			{
				m_Vector = DirectX::XMLoadFloat3(&m_Float3);
				VEC_FLT_EQUAL(m_Modified);
			}
		}

		void UpdateFlt()
		{
			if (!IS_VEC_FLT_EQUAL(m_Modified) && IS_VEC_MOD_LAST(m_Modified))
			{
				DirectX::XMStoreFloat3(&m_Float3, m_Vector);
				VEC_FLT_EQUAL(m_Modified);
			}
		}
	private:
		DirectX::XMVECTOR m_Vector;
		DirectX::XMFLOAT3 m_Float3;
		char m_Modified;
	};

	class Vector4
	{
	public:
		Vector4()
			:
			m_Float4({ 0, 0, 0, 0 }),
			m_Vector(DirectX::XMVectorSet(0, 0, 0, 0)),
			m_Modified(0)
		{
		}

		Vector4(float x, float y, float z, float w)
			:
			m_Float4({ x, y, z, w }),
			m_Vector(DirectX::XMVectorSet(x, y, z, w)),
			m_Modified(0)
		{
		}

		static float Dot(Vector4& a, Vector4& b)
		{
			a.UpdateVec();
			b.UpdateVec();

			return DirectX::XMVectorGetX(DirectX::XMVector4Dot(a.m_Vector, b.m_Vector));
		}

		//Vector-Vector Operations
		Vector4 operator+(Vector4& other)
		{
			UpdateVec();
			other.UpdateVec();

			return Vector4(DirectX::XMVectorAdd(m_Vector, other.m_Vector));
		}

		Vector4 operator-(Vector4& other)
		{
			UpdateVec();
			other.UpdateVec();

			return Vector4(DirectX::XMVectorSubtract(m_Vector, other.m_Vector));
		}

		Vector4 operator*(Vector4& other)
		{
			UpdateVec();
			other.UpdateVec();

			return Vector4(DirectX::XMVectorMultiply(m_Vector, other.m_Vector));
		}

		Vector4 operator/(Vector4& other)
		{
			UpdateVec();
			other.UpdateVec();

			return Vector4(DirectX::XMVectorDivide(m_Vector, other.m_Vector));
		}

		//Equal Operators
		void operator+=(Vector4& other)
		{
			UpdateVec();
			other.UpdateVec();

			m_Vector = DirectX::XMVectorAdd(m_Vector, other.m_Vector);
			MOD_VEC(m_Modified);
		}

		void operator-=(Vector4& other)
		{
			UpdateVec();
			other.UpdateVec();

			m_Vector = DirectX::XMVectorSubtract(m_Vector, other.m_Vector);
			MOD_VEC(m_Modified);
		}

		void operator*=(Vector4& other)
		{
			UpdateVec();
			other.UpdateVec();

			m_Vector = DirectX::XMVectorMultiply(m_Vector, other.m_Vector);
			MOD_VEC(m_Modified);
		}

		void operator/=(Vector4& other)
		{
			UpdateVec();
			other.UpdateVec();

			m_Vector = DirectX::XMVectorDivide(m_Vector, other.m_Vector);
			MOD_VEC(m_Modified);
		}

		//Vector-Scalar Operations
		Vector4 operator+(float other)
		{
			UpdateVec();

			return Vector4(DirectX::XMVectorAdd(m_Vector, DirectX::XMVectorSet(other, other, other, other)));
		}

		Vector4 operator-(float other)
		{
			UpdateVec();

			return Vector4(DirectX::XMVectorSubtract(m_Vector, DirectX::XMVectorSet(other, other, other, other)));
		}

		Vector4 operator*(float other)
		{
			UpdateVec();

			return Vector4(DirectX::XMVectorMultiply(m_Vector, DirectX::XMVectorSet(other, other, other, other)));
		}

		Vector4 operator/(float other)
		{
			UpdateVec();

			return Vector4(DirectX::XMVectorDivide(m_Vector, DirectX::XMVectorSet(other, other, other, other)));;
		}

		//Equal Operators
		void operator+=(float other)
		{
			UpdateVec();

			m_Vector = DirectX::XMVectorAdd(m_Vector, DirectX::XMVectorSet(other, other, other, other));
			MOD_VEC(m_Modified);
		}

		void operator-=(float other)
		{
			UpdateVec();

			m_Vector = DirectX::XMVectorSubtract(m_Vector, DirectX::XMVectorSet(other, other, other, other));
			MOD_VEC(m_Modified);
		}

		void operator*=(float other)
		{
			UpdateVec();

			m_Vector = DirectX::XMVectorMultiply(m_Vector, DirectX::XMVectorSet(other, other, other, other));
			MOD_VEC(m_Modified);
		}

		void operator/=(float other)
		{
			UpdateVec();

			m_Vector = DirectX::XMVectorDivide(m_Vector, DirectX::XMVectorSet(other, other, other, other));
			MOD_VEC(m_Modified);
		}

		//Retrieve components
		const float GetX()
		{
			UpdateFlt();
			return m_Float4.x;
		}

		const float GetY()
		{
			UpdateFlt();
			return m_Float4.y;
		}

		const float GetZ()
		{
			UpdateFlt();
			return m_Float4.z;
		}

		const float GetW()
		{
			UpdateFlt();
			return m_Float4.w;
		}

		//Set components
		void SetX(float x)
		{
			m_Float4.x = x;
			MOD_FLT(m_Modified);
		}

		void SetY(float y)
		{
			m_Float4.y = y;
			MOD_FLT(m_Modified);
		}

		void SetZ(float z)
		{
			m_Float4.z = z;
			MOD_FLT(m_Modified);
		}

		void SetW(float w)
		{
			m_Float4.w = w;
			MOD_FLT(m_Modified);
		}
	private:
		Vector4(DirectX::XMFLOAT4& flt)
			:
			m_Float4(flt),
			m_Vector(DirectX::XMLoadFloat4(&flt)),
			m_Modified(2)
		{
		}

		Vector4(DirectX::XMVECTOR& vec)
			:
			m_Float4({ 0, 0, 0, 0 }),
			m_Vector(vec),
			m_Modified(2)
		{
			DirectX::XMStoreFloat4(&m_Float4, vec);
		}

		void UpdateVec()
		{
			if (!IS_VEC_FLT_EQUAL(m_Modified) && !IS_VEC_MOD_LAST(m_Modified))
			{
				m_Vector = DirectX::XMLoadFloat4(&m_Float4);
				VEC_FLT_EQUAL(m_Modified);
			}
		}

		void UpdateFlt()
		{
			if (!IS_VEC_FLT_EQUAL(m_Modified) && IS_VEC_MOD_LAST(m_Modified))
			{
				DirectX::XMStoreFloat4(&m_Float4, m_Vector);
				VEC_FLT_EQUAL(m_Modified);
			}
		}
	private:
		DirectX::XMVECTOR m_Vector;
		DirectX::XMFLOAT4 m_Float4;
		char m_Modified;
	};
}