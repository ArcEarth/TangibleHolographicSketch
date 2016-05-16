#pragma once
#ifndef _HLSL_XM_VECTOR_H
#pragma error("This header are suppose to be inlcuded inside xmvector.h, not to use solely")
#endif

#include "mpl.hpp"
#include "vector_math.h"

//#define XM_EMPTY_BASE __declspec(empty_bases)
#define XM_EMPTY_BASE

namespace DirectX
{
	namespace hlsl
	{
		using index_t = size_t;
		using uint = uint32_t;
		using std::enable_if_t;

		template <typename _T>
		struct xmscalar;

		template <typename _T, size_t _Size>
		struct xmvector;

		template <typename _T, index_t... _SwzArgs>
		struct xmswizzler;

#pragma region details: vector classes base for __mm128 vectors / operators like +-* /&|^ / swizzle operator defines
		// definition of special swizzle operators
		// like v2.xy, v2.yz, v2.xxyy etc...
		// swizzle_operator_base
		namespace detail
		{
			struct XM_ALIGNATTR xmvector_base
			{
				XMVECTOR v;
			};

			template <typename _TDerived, index_t _Size>
			struct XM_ALIGNATTR XM_EMPTY_BASE swizzle_operator_base : public xmvector_base
			{
			};
#ifdef _SWIZZLE_DECL_
#pragma push_macro(_MAKE_SWS_CONST_)
#pragma push_macro(_SWIZZLE_DECL_)
#undef _SWIZZLE_DECL_
#undef _CONST_SWIZZLE_DECL_
#define _SHOULD_UNDEFINE_MAKE_SWS_SOMETHING_STRANGE_
#endif
#define _SWIZZLE_DECL_(name, ...) \
			inline auto& XM_CALLCONV name () { return static_cast< _TDerived * >( this )->swizzle<__VA_ARGS__ >(); } \
			inline const auto& XM_CALLCONV name() const { return static_cast< const _TDerived * >( this )->swizzle<__VA_ARGS__ >(); }
#define _CONST_SWIZZLE_DECL_(name, ...) \
			inline const auto& XM_CALLCONV name() const { return static_cast< const _TDerived * >( this )->swizzle<__VA_ARGS__ >(); }

			template <typename _TDerived>
			struct XM_ALIGNATTR swizzle_operator_base<_TDerived, 2> : public xmvector_base
			{
#include "detail/swizzles_def_2.h"
			};

			template <typename _TDerived>
			struct XM_ALIGNATTR swizzle_operator_base<_TDerived, 3> : public xmvector_base
			{
#include "detail/swizzles_def_3.h"
			};

			template <typename _TDerived>
			struct XM_ALIGNATTR swizzle_operator_base<_TDerived, 4> : public xmvector_base
			{
#include "detail/swizzles_def_4.h"
			};

#if defined(_SHOULD_UNDEFINE_MAKE_SWS_SOMETHING_STRANGE_)
#pragma pop_macro(_SWIZZLE_DECL_)
#pragma pop_macro(_CONST_SWIZZLE_DECL_)
#else
#undef _CONST_SWIZZLE_DECL_
#undef _SWIZZLE_DECL_
#endif
		}

		namespace detail
		{
			template <typename _TDerived, typename _TScalar, index_t _Size>
			struct XM_EMPTY_BASE logical_bitwise_operator_base : public swizzle_operator_base<_TDerived, _Size>
			{
			};

			template <index_t _Size>
			struct XM_EMPTY_BASE logical_bitwise_operator_base<xmvector<uint, _Size>, uint, _Size>
				: public swizzle_operator_base<xmvector<uint, _Size>, _Size>
			{
				inline xmvector<uint, _Size>& XM_CALLCONV operator&= (const xmvector<uint, _Size> rhs)
				{
					this->v = vector_math::and<uint, _Size>::invoke(this->v, rhs.v);
					return static_cast<xmvector<uint, _Size>&>(*this);
				}
				inline xmvector<uint, _Size>& XM_CALLCONV operator|= (const xmvector<uint, _Size> rhs)
				{
					this->v = vector_math:: or <uint, _Size>::invoke(this->v, rhs.v);
					return static_cast<xmvector<uint, _Size>&>(*this);
				}
				inline xmvector<uint, _Size>& XM_CALLCONV operator^= (const xmvector<uint, _Size> rhs)
				{
					this->v = vector_math::xor<uint, _Size>::invoke(this->v, rhs.v);
					return static_cast<xmvector<uint, _Size>&>(*this);
				}
				inline xmvector<uint, _Size> XM_CALLCONV operator& (const xmvector<uint, _Size> rhs) const
				{
					xmvector<uint, _Size> ret;
					ret.v = vector_math::and<uint, _Size>::invoke(this->v, rhs.v);
					return ret;
				}
				inline xmvector<uint, _Size> XM_CALLCONV operator| (const xmvector<uint, _Size> rhs) const
				{
					xmvector<uint, _Size> ret;
					ret.v = vector_math:: or <uint, _Size>::invoke(this->v, rhs.v);
					return ret;
				}
				inline xmvector<uint, _Size> XM_CALLCONV operator^ (const xmvector<uint, _Size> rhs) const
				{
					xmvector<uint, _Size> ret;
					ret.v = vector_math::xor<uint, _Size>::invoke(this->v, rhs.v);
					return ret;
				}
			};

			template <index_t... _SwzArg, size_t _Size>
			struct XM_EMPTY_BASE logical_bitwise_operator_base<xmswizzler<uint, _SwzArg...>, uint, _Size>
				: public swizzle_operator_base<xmswizzler<uint, _SwzArg...>, _Size>
			{
			private:
				using Scalar = uint;
				using Derived = xmswizzler<uint, _SwzArg...>;
				using xmvector_type = xmvector<uint, sizeof...(_SwzArg)>;
			public:
				// Case of swizzler + vector(scalar)
				inline xmvector_type XM_CALLCONV operator & (const xmvector_type rhs) const
				{
					return rhs.operator&(static_cast<const Derived&>(*this));
				}
				inline xmvector_type XM_CALLCONV operator | (const xmvector_type rhs) const
				{
					return rhs.operator|(static_cast<const Derived&>(*this));
				}
				inline xmvector_type XM_CALLCONV operator ^ (const xmvector_type rhs) const
				{
					return rhs.operator^(static_cast<const Derived&>(*this));
				}

				template <index_t... _SrcSwz> inline void XM_CALLCONV operator&=(const xmswizzler<Scalar, _SrcSwz...>& src)
				{
					static_cast<Derived&>(*this).invoke_operator_assign<typename vector_math::and, _SrcSwz...>(src);
				}
				template <index_t... _SrcSwz> inline void XM_CALLCONV operator|=(const xmswizzler<Scalar, _SrcSwz...>& src)
				{
					static_cast<Derived&>(*this).invoke_operator_assign<typename vector_math:: or , _SrcSwz...>(src);
				}
				template <index_t... _SrcSwz> inline void XM_CALLCONV operator^=(const xmswizzler<Scalar, _SrcSwz...>& src)
				{
					static_cast<Derived&>(*this).invoke_operator_assign<typename vector_math::xor, _SrcSwz...>(src);
				}

				// identity to any assignment
				inline void XM_CALLCONV operator&=(const xmvector_type rhs)
				{
					auto& srcv = reinterpret_cast<const detail::swizzle_from_indecis_t<Scalar, std::make_index_sequence<Size>>&>(rhs);
					this->operator&=(srcv);
				}
				inline void XM_CALLCONV operator|=(const xmvector_type rhs)
				{
					auto& srcv = reinterpret_cast<const detail::swizzle_from_indecis_t<Scalar, std::make_index_sequence<Size>>&>(rhs);
					this->operator|=(srcv);
				}
				inline void XM_CALLCONV operator^=(const xmvector_type rhs)
				{
					auto& srcv = reinterpret_cast<const detail::swizzle_from_indecis_t<Scalar, std::make_index_sequence<Size>>&>(rhs);
					this->operator^=(srcv);
				}
			};
		};

		// bunch of helpers
		namespace detail
		{
			template <typename _Ty>
			inline const _Ty & as_const(_Ty& ref)
			{
				return const_cast<const _Ty&>(ref);
			}

			template <typename _Ty>
			inline const _Ty && as_const(_Ty&& ref)
			{
				return const_cast<const _Ty&&>(ref);
			}

			template <typename _Ty, bool aligned, size_t cols, size_t rows = 1>
			struct storage_helper;

			inline XMVECTOR XM_CALLCONV replicate_scalar(uint s)
			{
				return XMVectorReplicateInt(s);
			}

			inline XMVECTOR XM_CALLCONV replicate_scalar(float s)
			{
				return XMVectorReplicate(s);
			}

			inline XMVECTOR XM_CALLCONV set_vector(float x, float y = .0f, float z = .0f, float w = .0f)
			{
				return XMVectorSet(x, y, z, w);
			}

			inline XMVECTOR XM_CALLCONV set_vector(uint x, uint y = 0, uint z = 0, uint w = 0)
			{
				return XMVectorSetInt(x, y, z, w);
			}

			enum components_name_enums
			{
				_x = 0,
				_y = 1,
				_z = 2,
				_w = 3,
			};

			template <typename _Ty>
			inline _Ty XM_CALLCONV get(FXMVECTOR xmv, size_t elem_index);
			template <>
			inline float XM_CALLCONV get<float>(FXMVECTOR xmv, size_t elem_index)
			{
				return XMVectorGetByIndex(xmv, elem_index);
			}
			template <>
			inline uint XM_CALLCONV get<uint>(FXMVECTOR xmv, size_t elem_index)
			{
				return XMVectorGetIntByIndex(xmv, elem_index);
			}

			template <typename _Ty>
			inline XMVECTOR XM_CALLCONV set(FXMVECTOR xmv, size_t elem_index, _Ty value);
			template <>
			inline XMVECTOR XM_CALLCONV set<float>(FXMVECTOR xmv, size_t elem_index, float value)
			{
				return XMVectorSetByIndex(xmv, value, elem_index);
			}
			template <>
			inline XMVECTOR XM_CALLCONV set<uint>(FXMVECTOR xmv, size_t elem_index, uint value)
			{
				return XMVectorSetIntByIndex(xmv, value, elem_index);
			}


			template <typename _Ty, uint32_t Elem>
			inline _Ty XM_CALLCONV get(FXMVECTOR xmv);

			template <typename _Ty, uint32_t Elem>
			inline void XM_CALLCONV get_ptr(_Ty* ptr, FXMVECTOR xmv);

			template <typename _Ty, uint32_t Elem>
			inline FXMVECTOR XM_CALLCONV set(FXMVECTOR xmv, _Ty val);

			template <typename _Ty, uint32_t Elem>
			inline FXMVECTOR XM_CALLCONV set_ptr(FXMVECTOR xmv, _Ty* val);

			template <size_t _Size, index_t... _SwzArgs>
			struct valiad_swizzle_args
			{
				//static_assert(sizeof...(_SwzArgs) <= 4, "Swizzle element count out of 4");
				//static_assert(conjunction < std::integral_constant<bool, (_SwzArgs < _Size || _SwzArgs == size_t(-1))>...>::value, "Swizzle index out of source vector size");

				static constexpr bool value = (sizeof...(_SwzArgs) <= 4) && (conjunction < std::integral_constant<bool, (_SwzArgs < _Size || _SwzArgs == size_t(-1))>...>::value);
			};

			using std::conditional;
			using std::conditional_t;
			using std::is_same;

			template <typename Scalar, size_t Size>
			struct intrinsic_vector
			{
				using type = void;
			};

			template <typename Scalar, size_t Size>
			struct __mvector
			{
				Scalar m[Size];
			};


#if defined(_XM_SSE_INTRINSICS_) && !defined(_XM_NO_INTRINSICS_)
			template <>struct intrinsic_vector<float, 1> { using type = __m128; };
			template <>struct intrinsic_vector<float, 2> { using type = __m128; };
			template <>struct intrinsic_vector<float, 3> { using type = __m128; };
			template <>struct intrinsic_vector<float, 4> { using type = __m128; };
			template <>struct intrinsic_vector<float, 5> { using type = __m256; };
			template <>struct intrinsic_vector<float, 6> { using type = __m256; };
			template <>struct intrinsic_vector<float, 7> { using type = __m256; };
			template <>struct intrinsic_vector<float, 8> { using type = __m256; };
			template <>struct intrinsic_vector<double, 1> { using type = __m128d; };
			template <>struct intrinsic_vector<double, 2> { using type = __m128d; };
			template <>struct intrinsic_vector<double, 3> { using type = __m256d; };
			template <>struct intrinsic_vector<double, 4> { using type = __m256d; };
			template <size_t Size >struct intrinsic_vector<int32_t, Size> {
				using scalar = int32_t;
				using type = conditional_t<(Size*sizeof(scalar) <= 16), __m128i, conditional_t<(Size*sizeof(scalar) <= 32), __m256i, void>>;
			};
			template <size_t Size >struct intrinsic_vector<uint32_t, Size> {
				using scalar = uint32_t;
				using type = conditional_t<(Size*sizeof(scalar) <= 16), __m128i, conditional_t<(Size*sizeof(scalar) <= 32), __m256i, void>>;
			};
			template <size_t Size >struct intrinsic_vector<int64_t, Size> {
				using scalar = int64_t;
				using type = conditional_t<(Size*sizeof(scalar) <= 16), __m128i, conditional_t<(Size*sizeof(scalar) <= 32), __m256i, void>>;
			};
			template <size_t Size >struct intrinsic_vector<uint64_t, Size> {
				using scalar = uint64_t;
				using type = conditional_t<(Size*sizeof(scalar) <= 16), __m128i, conditional_t<(Size*sizeof(scalar) <= 32), __m256i, void>>;
			};
			template <size_t Size >struct intrinsic_vector<int16_t, Size> {
				using scalar = int16_t;
				using type = conditional_t<(Size*sizeof(scalar) <= 16), __m128i, conditional_t<(Size*sizeof(scalar) <= 32), __m256i, void>>;
			};
			template <size_t Size >struct intrinsic_vector<uint16_t, Size> {
				using scalar = uint16_t;
				using type = conditional_t<(Size*sizeof(scalar) <= 16), __m128i, conditional_t<(Size*sizeof(scalar) <= 32), __m256i, void>>;
			};
			template <size_t Size >struct intrinsic_vector<int8_t, Size> {
				using scalar = int8_t;
				using type = conditional_t<(Size*sizeof(scalar) <= 16), __m128i, conditional_t<(Size*sizeof(scalar) <= 32), __m256i, void>>;
			};
			template <size_t Size >struct intrinsic_vector<uint8_t, Size> {
				using scalar = uint8_t;
				using type = conditional_t<(Size*sizeof(scalar) <= 16), __m128i, conditional_t<(Size*sizeof(scalar) <= 32), __m256i, void>>;
			};
#elif defined(_XM_ARM_NEON_INTRINSICS_) && !defined(_XM_NO_INTRINSICS_)
			template <>struct intrinsic_vector<float, 1> { using type = float32x2_t; };
			template <>struct intrinsic_vector<float, 2> { using type = float32x2_t; };
			template <>struct intrinsic_vector<float, 3> { using type = float32x4_t; };
			template <>struct intrinsic_vector<float, 4> { using type = float32x4_t; };
			template <>struct intrinsic_vector<float, 5> { using type = float32x4x2_t; };
			template <>struct intrinsic_vector<float, 6> { using type = float32x4x2_t; };
			template <>struct intrinsic_vector<float, 7> { using type = float32x4x2_t; };
			template <>struct intrinsic_vector<float, 8> { using type = float32x4x2_t; };

			template <>struct intrinsic_vector<uint, 1> { using type = uint32x2_t; };
			template <>struct intrinsic_vector<uint, 2> { using type = uint32x2_t; };
			template <>struct intrinsic_vector<uint, 3> { using type = uint32x4_t; };
			template <>struct intrinsic_vector<uint, 4> { using type = uint32x4_t; };
			template <>struct intrinsic_vector<uint, 5> { using type = uint32x4x2_t; };
			template <>struct intrinsic_vector<uint, 6> { using type = uint32x4x2_t; };
			template <>struct intrinsic_vector<uint, 7> { using type = uint32x4x2_t; };
			template <>struct intrinsic_vector<uint, 8> { using type = uint32x4x2_t; };
#else
			template <typename Scalar, size_t Size>
			using intrinsic_vector_t = Scarlar[Size];
#endif

			template <typename Scalar, size_t Size>
			using intrinsic_vector_t = typename intrinsic_vector<Scalar, Size>::type;

			template <typename Scalar, size_t Size>
			using get_intrinsic_vector_t =
				conditional_t<
				is_same<intrinsic_vector_t<Scalar, Size>, void>::value,
				intrinsic_vector_t<Scalar, Size>,
				__mvector<Scalar, Size>
				>;

			template <typename _TSrc, typename _Dst>
			inline XMVECTOR XM_CALLCONV cast_vector_4(FXMVECTOR V);

			template <>
			inline XMVECTOR XM_CALLCONV cast_vector_4<float, uint32_t>(FXMVECTOR V)
			{
				return XMVectorCastFloatToInt(V);
			}

			template <>
			inline XMVECTOR XM_CALLCONV cast_vector_4<uint32_t, float>(FXMVECTOR V)
			{
				return XMVectorCastFloatToInt(V);
			}

			template <typename _TSrc, typename _Dst, size_t _Size>
			inline XMVECTOR XM_CALLCONV cast_vector(FXMVECTOR V)
			{
				static_assert(_Size <= 4, "size must not exceed 4");
				return cast_vector_4<_TSrc, _TDst>(V);
			}

		}

#pragma endregion
	}
}