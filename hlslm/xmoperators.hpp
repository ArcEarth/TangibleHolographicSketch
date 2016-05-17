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

			template <typename expect_scalar_t, typename lhs_t, typename rhs_t>
			struct enable_binary_operator_on_scalar : public std::enable_if<
				binary_operator_traits<lhs_t, rhs_t>::overload &&
				std::is_same<
					typename binary_operator_traits<lhs_t, rhs_t>::scalar_type,
					expect_scalar_t>::value,
				typename binary_operator_traits<lhs_t, rhs_t>::return_type >
			{};

			template <typename expect_scalar_t, typename lhs_t, typename rhs_t>
			using enable_binary_operator_on_scalar_t = 
				typename enable_binary_operator_on_scalar<expect_scalar_t, lhs_t, rhs_t>::type;

			template <typename expect_scalar_t, typename lhs_t>
			struct enable_unary_operator_on_scalar : public std::enable_if<
				unary_operator_traits<lhs_t>::overload &&
				std::is_same<typename unary_operator_traits<lhs_t>::scalar_type,
				expect_scalar_t>::value,
				typename unary_operator_traits<lhs_t>::return_type>
			{};

			template <typename expect_scalar_t, typename lhs_t>
			using enable_unary_operator_on_scalar_t =
				typename enable_unary_operator_on_scalar<expect_scalar_t, lhs_t>::type;

		}

		namespace detail
		{
			template <typename _Ty, size_t _Size>
			__forceinline XMVECTOR XM_CALLCONV xmfoward(const xmvector<_Ty, _Size>& xmv)
			{
				return xmv.v;
			};
			template <typename _Ty>
			__forceinline XMVECTOR XM_CALLCONV xmfoward(const xmscalar<_Ty>& xms)
			{
				return xms.v;
			}

			// Load int / float
			template <typename _Ty>
			inline std::enable_if_t<traits::scalar_traits<_Ty>::value, XMVECTOR> XM_CALLCONV xmfoward(const _Ty& mvector)
			{
				return detail::replicate_scalar(mvector);
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

#define XM_MAKE_BINARY_CWISE_OPERATOR(stype,func,XMFunc,XMNS) \
		template <typename lhs_t, typename rhs_t> \
		inline traits::enable_binary_operator_on_scalar_t<stype, lhs_t, rhs_t> \
			XM_CALLCONV func(const lhs_t& lhs, const rhs_t& rhs) \
		{	traits::binary_operator_return_type<lhs_t, rhs_t> ret; \
			ret.v = XMNS XMFunc(detail::xmfoward(lhs), detail::xmfoward(rhs)); \
			return ret;}

#define XM_MAKE_UNARY_CWISE_OPERATOR(stype,func,XMFunc,XMNS) \
		template <typename lhs_t> \
		inline traits::enable_unary_operator_on_scalar_t<stype, lhs_t> \
			XM_CALLCONV func(const lhs_t& lhs) \
		{	typename traits::unary_operator_traits<lhs_t>::return_type ret; \
			ret.v = XMNS XMFunc(detail::xmfoward(lhs)); \
			return ret;}


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
		XM_MAKE_BINARY_CWISE_OPERATOR(uint, andnot, XMVectorAndCInt, XM_NAMES)
		XM_MAKE_BINARY_CWISE_OPERATOR(uint, nor, XMVectorNorInt, XM_NAMES)
		XM_MAKE_BINARY_CWISE_OPERATOR(uint, equal, XMVectorEqualInt, XM_NAMES)
		XM_MAKE_BINARY_CWISE_OPERATOR(uint, not_equal, XMVectorNotEqualInt, XM_NAMES)
		XM_MAKE_BINARY_CWISE_OPERATOR(uint, and, XMVectorAndInt, XM_NAMES)
		XM_MAKE_BINARY_CWISE_OPERATOR(uint, or, XMVectorOrInt, XM_NAMES)
		XM_MAKE_BINARY_CWISE_OPERATOR(uint, xor , XMVectorXorInt, XM_NAMES)

		// Float vector cwise functions
		XM_MAKE_BINARY_CWISE_OPERATOR(float, min, XMVectorMin, XM_NAMES)
		XM_MAKE_BINARY_CWISE_OPERATOR(float, max, XMVectorMax, XM_NAMES)

		XM_MAKE_UNARY_CWISE_OPERATOR(float, round, XMVectorRound, _DXMEXT)
		XM_MAKE_UNARY_CWISE_OPERATOR(float, ceil, XMVectorRound, _DXMEXT)
		XM_MAKE_UNARY_CWISE_OPERATOR(float, floor, XMVectorFloor, _DXMEXT)
		XM_MAKE_UNARY_CWISE_OPERATOR(float, trunc, XMVectorTruncate, _DXMEXT)
		XM_MAKE_UNARY_CWISE_OPERATOR(float, saturate, XMVectorSaturate, XM_NAMES)
		XM_MAKE_UNARY_CWISE_OPERATOR(float, abs, XMVectorAbs, XM_NAMES)
		XM_MAKE_UNARY_CWISE_OPERATOR(float, rcp, XMVectorReciprocal, XM_NAMES)
		XM_MAKE_UNARY_CWISE_OPERATOR(float, rcpf, XMVectorReciprocalEst, XM_NAMES)
		XM_MAKE_UNARY_CWISE_OPERATOR(float, sqrt, XMVectorSqrt, XM_NAMES)
		XM_MAKE_UNARY_CWISE_OPERATOR(float, sqrtf, XMVectorSqrtEst, XM_NAMES)
		XM_MAKE_UNARY_CWISE_OPERATOR(float, rsqrt, XMVectorReciprocalSqrt, XM_NAMES)
		XM_MAKE_UNARY_CWISE_OPERATOR(float, rsqrtf, XMVectorReciprocalSqrtEst, XM_NAMES)
		XM_MAKE_UNARY_CWISE_OPERATOR(float, exp, XMVectorExpE, XM_NAMES)
		XM_MAKE_UNARY_CWISE_OPERATOR(float, exp2, XMVectorExp2, XM_NAMES)
		XM_MAKE_UNARY_CWISE_OPERATOR(float, log, XMVectorLogE, XM_NAMES)
		XM_MAKE_UNARY_CWISE_OPERATOR(float, log2, XMVectorLog2, XM_NAMES)
		XM_MAKE_UNARY_CWISE_OPERATOR(float, sinh, XMVectorSinH, XM_NAMES)
		XM_MAKE_UNARY_CWISE_OPERATOR(float, cosh, XMVectorCosH, XM_NAMES)
		XM_MAKE_UNARY_CWISE_OPERATOR(float, tanh, XMVectorTanH, XM_NAMES)
		XM_MAKE_UNARY_CWISE_OPERATOR(float, sin, XMVectorSin, XM_NAMES)
		XM_MAKE_UNARY_CWISE_OPERATOR(float, cos, XMVectorCos, XM_NAMES)
		XM_MAKE_UNARY_CWISE_OPERATOR(float, tan, XMVectorTan, XM_NAMES)
		XM_MAKE_UNARY_CWISE_OPERATOR(float, sinf, XMVectorSinEst, XM_NAMES)
		XM_MAKE_UNARY_CWISE_OPERATOR(float, cosf, XMVectorCosEst, XM_NAMES)
		XM_MAKE_UNARY_CWISE_OPERATOR(float, tanf, XMVectorTanEst, XM_NAMES)
		XM_MAKE_UNARY_CWISE_OPERATOR(float, asin, XMVectorASin, XM_NAMES)
		XM_MAKE_UNARY_CWISE_OPERATOR(float, acos, XMVectorACos, XM_NAMES)
		XM_MAKE_UNARY_CWISE_OPERATOR(float, atan, XMVectorATan, XM_NAMES)
		XM_MAKE_UNARY_CWISE_OPERATOR(float, asinf, XMVectorASinEst, XM_NAMES)
		XM_MAKE_UNARY_CWISE_OPERATOR(float, acosf, XMVectorACosEst, XM_NAMES)
		XM_MAKE_UNARY_CWISE_OPERATOR(float, atanf, XMVectorATanEst, XM_NAMES)

		XM_MAKE_BINARY_CWISE_OPERATOR(float, pow, XMVectorPow, XM_NAMES)
		XM_MAKE_BINARY_CWISE_OPERATOR(float, atan2, XMVectorATan2, XM_NAMES)
		XM_MAKE_BINARY_CWISE_OPERATOR(float, atan2f, XMVectorATan2Est, XM_NAMES)

		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV clamp(const xmvector<float, _Size> lhs, const xmvector<float, _Size> _min, const xmvector<float, _Size> _max)
		{
			xmvector<float, _Size> ret;
			ret.v = XM_NAMES XMVectorClamp(lhs.v, _min.v, _max.v);
			return ret;
		}

		inline xmscalar<float> XM_CALLCONV clamp(const xmscalar<float> lhs, const xmscalar<float> _min, const xmscalar<float> _max)
		{
			xmscalar<float> ret;
			ret.v = XM_NAMES XMVectorClamp(lhs.v, _min.v, _max.v);
			return ret;
		}

		template <size_t _Size>
		inline xmvector<float, _Size> XM_CALLCONV lerp(const xmvector<float, _Size> lhs, const xmvector<float, _Size> rhs, const xmscalar<float> t)
		{
			xmvector<float, _Size> ret;
			ret.v = XM_NAMES XMVectorLerpV(lhs.v, rhs.v, t.v);
			return ret;
		}

		inline xmscalar<float> XM_CALLCONV lerp(const xmscalar<float> lhs, const xmscalar<float> rhs, const xmscalar<float> t)
		{
			xmscalar<float> ret;
			ret.v = XM_NAMES XMVectorLerpV(lhs.v, rhs.v, t.v);
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