#pragma once

#include "traits.hpp"
#include "xmvector.hpp"
// For XMVectorAddInt etc...
#include <DirectXMathExtend.h>

namespace DirectX
{
	namespace hlsl
	{
		namespace traits
		{
			template <typename lhs_t, typename rhs_t>
			struct enable_if_binary_operator_type : public std::enable_if<
				binary_operator_traits<lhs_t, rhs_t>::overload, typename binary_operator_traits<lhs_t, rhs_t>::return_type >
			{};

			template <typename lhs_t, typename rhs_t>
			using enable_if_binary_operator_t = typename enable_if_binary_operator_type<lhs_t, rhs_t>::type;
		}

		namespace detail
		{
			template <typename _Ty, size_t _Size>
			inline XMVECTOR XM_CALLCONV xmfoward(const xmvector<_Ty, _Size>& xmv)
			{
				return xmv.v;
			};
			template <typename _Ty>
			inline XMVECTOR XM_CALLCONV xmfoward(const xmscalar<_Ty>& xms)
			{
				return xms.v;
			}
			template <typename _Ty>
			inline std::enable_if_t<traits::is_memory_type<_Ty>::value, XMVECTOR> XM_CALLCONV xmfoward(const _Ty& mvector)
			{
				return load(mvector).v;
			}
			template <typename _Ty>
			inline std::enable_if_t<traits::is_expression<_Ty>::value, XMVECTOR> XM_CALLCONV xmfoward(const _Ty& mvector)
			{
				return mvector.eval().v;
			}
		}

		// Comparison functions
		template <size_t _Size>
		inline xmvector<uint, _Size> XM_CALLCONV equal(const xmvector<float, _Size> lhs, const xmvector<float, _Size> rhs)
		{
			xmvector<uint, _Size> ret;
			ret.v = XMVectorEqual(lhs.v, rhs.v);
			return ret;
		}

		template <size_t _Size>
		inline xmvector<uint, _Size> XM_CALLCONV greater_equal(const xmvector<float, _Size> lhs, const xmvector<float, _Size> rhs)
		{
			xmvector<uint, _Size> ret;
			ret.v = XMVectorGreaterOrEqual(lhs.v, rhs.v);
			return ret;
		}

		template <size_t _Size>
		inline xmvector<uint, _Size> XM_CALLCONV greater(const xmvector<float, _Size> lhs, const xmvector<float, _Size> rhs)
		{
			xmvector<uint, _Size> ret;
			ret.v = XMVectorGreater(lhs.v, rhs.v);
			return ret;
		}

		template <size_t _Size>
		inline xmvector<uint, _Size> XM_CALLCONV less_equal(const xmvector<float, _Size> lhs, const xmvector<float, _Size> rhs)
		{
			xmvector<uint, _Size> ret;
			ret.v = XMVectorLessOrEqual(lhs.v, rhs.v);
			return ret;
		}

		template <size_t _Size>
		inline xmvector<uint, _Size> XM_CALLCONV less(const xmvector<float, _Size> lhs, const xmvector<float, _Size> rhs)
		{
			xmvector<uint, _Size> ret;
			ret.v = XMVectorLess(lhs.v, rhs.v);
			return ret;
		}

		template <size_t _Size>
		inline xmvector<uint, _Size> XM_CALLCONV near_equal(const xmvector<float, _Size> lhs, const xmvector<float, _Size> rhs, const xmscalar<float> epsilon)
		{
			xmvector<uint, _Size> ret;
			ret.v = XMVectorNearEqual(lhs.v, rhs.v, epsilon.v);
			return ret;
		}

		template <size_t _Size>
		inline xmvector<uint, _Size> XM_CALLCONV near_equal(const xmvector<float, _Size> lhs, const xmvector<float, _Size> rhs, const xmvector<float, _Size> epsilon)
		{
			xmvector<uint, _Size> ret;
			ret.v = XMVectorNearEqual(lhs.v, rhs.v, epsilon.v);
			return ret;
		}

		// Bitwise operators for uint vectors
		// lhs & ~rhs
		template <size_t _Size>
		inline xmvector<uint, _Size> XM_CALLCONV andnot(const xmvector<uint, _Size> lhs, const xmvector<uint, _Size> rhs)
		{
			xmvector<uint, _Size> ret;
			ret.v = XMVectorAndCInt(lhs.v, rhs.v);
			return ret;
		}

		template <size_t _Size>
		inline xmvector<uint, _Size> XM_CALLCONV nor(const xmvector<uint, _Size> lhs, const xmvector<uint, _Size> rhs)
		{
			xmvector<uint, _Size> ret;
			ret.v = XMVectorNorInt(lhs.v, rhs.v);
			return ret;
		}

		template <size_t _Size>
		inline xmvector<uint, _Size> XM_CALLCONV equal(const xmvector<uint, _Size> lhs, const xmvector<uint, _Size> rhs)
		{
			xmvector<uint, _Size> ret;
			ret.v = XMVectorEqualInt(lhs.v, rhs.v);
			return ret;
		}

		template <size_t _Size>
		inline xmvector<uint, _Size> XM_CALLCONV not_equal(const xmvector<uint, _Size> lhs, const xmvector<uint, _Size> rhs)
		{
			xmvector<uint, _Size> ret;
			ret.v = XMVectorNotEqualInt(lhs.v, rhs.v);
			return ret;
		}

		template <size_t _Size>
		inline xmvector<uint, _Size> XM_CALLCONV and(const xmvector<uint, _Size> lhs, const xmvector<uint, _Size> rhs)
		{
			xmvector<uint, _Size> ret;
			ret.v = XMVectorAndInt(lhs.v, rhs.v);
			return ret;
		}

		template <size_t _Size>
		inline xmvector<uint, _Size> XM_CALLCONV or(const xmvector<uint, _Size> lhs, const xmvector<uint, _Size> rhs)
		{
			xmvector<uint, _Size> ret;
			ret.v = XMVectorOrInt(lhs.v, rhs.v);
			return ret;
		}

		template <size_t _Size>
		inline xmvector<uint, _Size> XM_CALLCONV xor(const xmvector<uint, _Size> lhs, const xmvector<uint, _Size> rhs)
		{
			xmvector<uint, _Size> ret;
			ret.v = XMVectorXorInt(lhs.v, rhs.v);
			return ret;
		}

		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV min(const xmvector<float, _Size> lhs, const xmvector<float, _Size> rhs)
		{
			xmvector<float, _Size> ret;
			ret.v = XMVectorMin(lhs.v, rhs.v);
			return ret;
		}

		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV max(const xmvector<float, _Size> lhs, const xmvector<float, _Size> rhs)
		{
			xmvector<float, _Size> ret;
			ret.v = XMVectorMax(lhs.v, rhs.v);
			return ret;
		}

		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV round(const xmvector<float, _Size> lhs)
		{
			xmvector<float, _Size> ret;
			ret.v = _DXMEXT XMVectorRound(lhs.v);
			return ret;
		}

		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV ceil(const xmvector<float, _Size> lhs)
		{
			xmvector<float, _Size> ret;
			ret.v = _DXMEXT XMVectorCeil(lhs.v);
			return ret;
		}

		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV floor(const xmvector<float, _Size> lhs)
		{
			xmvector<float, _Size> ret;
			ret.v = _DXMEXT XMVectorFloor(lhs.v);
			return ret;
		}

		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV trunc(const xmvector<float, _Size> lhs)
		{
			xmvector<float, _Size> ret;
			ret.v = _DXMEXT XMVectorTruncate(lhs.v);
			return ret;
		}

		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV saturate(const xmvector<float, _Size> lhs)
		{
			xmvector<float, _Size> ret;
			ret.v = XMVectorSaturate(lhs.v);
			return ret;
		}

		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV clamp(const xmvector<float, _Size> lhs, const xmvector<float, _Size> _min, const xmvector<float, _Size> _max)
		{
			xmvector<float, _Size> ret;
			ret.v = XMVectorClamp(lhs.v,_min.v,_max.v);
			return ret;
		}



		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV abs(const xmvector<float, _Size> lhs)
		{
			xmvector<float, _Size> ret;
			ret.v = XMVectorAbs(lhs.v);
			return ret;
		}

		// Reciprocal of the value
		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV rcp(const xmvector<float, _Size> lhs)
		{
			xmvector<float, _Size> ret;
			ret.v = XMVectorReciprocal(lhs.v);
			return ret;
		}

		// Reciprocal of the value, fast pass
		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV rcpf(const xmvector<float, _Size> lhs)
		{
			xmvector<float, _Size> ret;
			ret.v = XMVectorReciprocalEst(lhs.v);
			return ret;
		}

		// Square Root
		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV sqrt(const xmvector<float, _Size> lhs)
		{
			xmvector<float, _Size> ret;
			ret.v = XMVectorSqrt(lhs.v);
			return ret;
		}

		// Square Root, fast pass
		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV sqrtf(const xmvector<float, _Size> lhs)
		{
			xmvector<float, _Size> ret;
			ret.v = XMVectorSqrtEst(lhs.v);
			return ret;
		}

		// Sqrt Reciprocal
		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV rsqrt(const xmvector<float, _Size> lhs)
		{
			xmvector<float, _Size> ret;
			ret.v = XMVectorReciprocalSqrt(lhs.v);
			return ret;
		}

		// Sqrt Reciprocal, fast pass
		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV rsqrtf(const xmvector<float, _Size> lhs)
		{
			xmvector<float, _Size> ret;
			ret.v = XMVectorReciprocalSqrtEst(lhs.v);
			return ret;
		}

		// lhs^rhs
		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV pow(const xmvector<float, _Size> lhs, const xmvector<float, _Size> rhs)
		{
			xmvector<float, _Size> ret;
			ret.v = XMVectorPow(lhs.v, rhs.v);
			return ret;
		}

		// e^lhs
		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV exp(const xmvector<float, _Size> lhs)
		{
			xmvector<float, _Size> ret;
			ret.v = XMVectorExpE(lhs.v);
			return ret;
		}

		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV exp2(const xmvector<float, _Size> lhs)
		{
			xmvector<float, _Size> ret;
			ret.v = XMVectorExp2(lhs.v);
			return ret;
		}

		// log_e(lhs)
		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV log(const xmvector<float, _Size> lhs)
		{
			xmvector<float, _Size> ret;
			ret.v = XMVectorLogE(lhs.v);
			return ret;
		}

		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV log2(const xmvector<float, _Size> lhs)
		{
			xmvector<float, _Size> ret;
			ret.v = XMVectorLog2(lhs.v);
			return ret;
		}

		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV sinh(const xmvector<float, _Size> lhs)
		{
			xmvector<float, _Size> ret;
			ret.v = XMVectorSinH(lhs.v);
			return ret;
		}

		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV cosh(const xmvector<float, _Size> lhs)
		{
			xmvector<float, _Size> ret;
			ret.v = XMVectorCosH(lhs.v);
			return ret;
		}

		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV tanh(const xmvector<float, _Size> lhs)
		{
			xmvector<float, _Size> ret;
			ret.v = XMVectorTanH(lhs.v);
			return ret;
		}


		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV sin(const xmvector<float, _Size> lhs)
		{
			xmvector<float, _Size> ret;
			ret.v = XMVectorSin(lhs.v);
			return ret;
		}

		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV cos(const xmvector<float, _Size> lhs)
		{
			xmvector<float, _Size> ret;
			ret.v = XMVectorCos(lhs.v);
			return ret;
		}

		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV tan(const xmvector<float, _Size> lhs)
		{
			xmvector<float, _Size> ret;
			ret.v = XMVectorTan(lhs.v);
			return ret;
		}

		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV asin(const xmvector<float, _Size> lhs)
		{
			xmvector<float, _Size> ret;
			ret.v = XMVectorASin(lhs.v);
			return ret;
		}

		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV acos(const xmvector<float, _Size> lhs)
		{
			xmvector<float, _Size> ret;
			ret.v = XMVectorACos(lhs.v);
			return ret;
		}

		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV atan(const xmvector<float, _Size> lhs)
		{
			xmvector<float, _Size> ret;
			ret.v = XMVectorATan(lhs.v);
			return ret;
		}

		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV atan2(const xmvector<float, _Size> Y, const xmvector<float, _Size> X)
		{
			xmvector<float, _Size> ret;
			ret.v = XMVectorATan2(lhs.v, rhs.v);
			return ret;
		}

		// fast pass of sin function
		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV sinf(const xmvector<float, _Size> lhs)
		{
			xmvector<float, _Size> ret;
			ret.v = XMVectorSinEst(lhs.v);
			return ret;
		}

		// fast pass of cos function
		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV cosf(const xmvector<float, _Size> lhs)
		{
			xmvector<float, _Size> ret;
			ret.v = XMVectorCosEst(lhs.v);
			return ret;
		}


		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV tanf(const xmvector<float, _Size> lhs)
		{
			xmvector<float, _Size> ret;
			ret.v = XMVectorTanEst(lhs.v);
			return ret;
		}

		// fast pass of asin function
		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV asinf(const xmvector<float, _Size> lhs)
		{
			xmvector<float, _Size> ret;
			ret.v = XMVectorASinEst(lhs.v);
			return ret;
		}

		// fast pass of acos function
		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV acosf(const xmvector<float, _Size> lhs)
		{
			xmvector<float, _Size> ret;
			ret.v = XMVectorACosEst(lhs.v);
			return ret;
		}

		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV atanf(const xmvector<float, _Size> lhs)
		{
			xmvector<float, _Size> ret;
			ret.v = XMVectorATanEst(lhs.v);
			return ret;
		}

		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV atan2f(const xmvector<float, _Size> Y, const xmvector<float, _Size> X)
		{
			xmvector<float, _Size> ret;
			ret.v = XMVectorATan2Est(lhs.v, rhs.v);
			return ret;
		}


		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV lerp(const xmvector<float, _Size> lhs, const xmvector<float, _Size> rhs, const xmscalar<float> t)
		{
			xmvector<float, _Size> ret;
			ret.v = XMVectorLerpV(lhs.v, rhs.v, t.v);
			return ret;
		}

		inline xmfloat XM_CALLCONV dot(const xmvector2f a, const xmvector2f b)
		{
			xmfloat ret;
			ret.v = _DXMEXT XMVector2Dot(a.v, b.v);
			return ret;
		}
		inline xmfloat XM_CALLCONV dot(const xmvector3f a, const xmvector3f b)
		{ 
			xmfloat ret;
			ret.v = _DXMEXT XMVector3Dot(a.v, b.v);
			return ret;
		}
		inline xmfloat XM_CALLCONV dot(const xmvector4f a, const xmvector4f b)
		{
			xmfloat ret;
			ret.v = _DXMEXT XMVector4Dot(a.v, b.v);
			return ret;
		}
		inline xmfloat XM_CALLCONV length(const xmvector2f a)
		{
			xmfloat ret;
			ret.v = _DXMEXT XMVector2Length(a.v);
			return ret;
		}
		inline xmfloat XM_CALLCONV length(const xmvector3f a)
		{
			xmfloat ret;
			ret.v = _DXMEXT XMVector3Length(a.v);
			return ret;
		}
		inline xmfloat XM_CALLCONV length(const xmvector4f a)
		{
			xmfloat ret;
			ret.v = _DXMEXT XMVector4Length(a.v);
			return ret;
		}
		inline xmvector2f XM_CALLCONV normalize(const xmvector2f a)
		{
			xmvector2f ret;
			ret.v = _DXMEXT XMVector2Normalize(a.v);
			return ret;
		}
		inline xmvector3f XM_CALLCONV normalize(const xmvector3f a)
		{
			xmvector3f ret;
			ret.v = _DXMEXT XMVector3Normalize(a.v);
			return ret;
		}
		inline xmvector4f XM_CALLCONV normalize(const xmvector4f a)
		{
			xmvector4f ret;
			ret.v = _DXMEXT XMVector4Normalize(a.v);
			return ret;
		}
		// 2d vector cross product
		inline xmfloat XM_CALLCONV cross(const xmvector2f a, const xmvector2f b)
		{
			xmfloat ret;
			ret.v = XMVector2Cross(a.v, b.v);
			return ret;
		}
		// 3d vector cross product
		inline xmvector3f XM_CALLCONV cross(const xmvector3f a, const xmvector3f b)
		{
			xmvector3f ret;
			ret.v = XMVector3Cross(a.v, b.v);
			return ret;
		}
		// 4d vector cross product
		inline xmvector4f XM_CALLCONV cross(const xmvector4f a, const xmvector4f b, const xmvector4f c)
		{
			xmvector4f ret;
			ret.v = XMVector4Cross(a.v, b.v, c.v);
			return ret;
		}
	}
}