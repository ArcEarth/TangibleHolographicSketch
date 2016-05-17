#pragma once

#ifndef _HLSL_XM_VECTOR_H
#pragma error("This header are suppose to be inlcuded after xmvector.h, not to use solely")
#endif
#define _HLSL_XM_SWIZZLER_H

#include "traits.hpp"
#include "xmvector.hpp"

namespace DirectX
{
	namespace hlsl
	{
		namespace detail
		{
			using std::conditional;

			template <typename IS>
			struct swizzle_impl;

			using namespace mpl;

			template <index_t... SwizzleArgs>
			struct swizzle_impl<index_sequence<SwizzleArgs...>>
			{
				inline static XMVECTOR XM_CALLCONV invoke(FXMVECTOR v)
				{
					return _DXMEXT XMVectorSwizzle<uint32_t(SwizzleArgs)...>(v);
				}
			};

			template <typename IS>
			struct permute_impl;

			template <index_t... PermutesArgs>
			struct permute_impl<index_sequence<PermutesArgs...>>
			{
				inline static XMVECTOR XM_CALLCONV invoke(FXMVECTOR v0, FXMVECTOR v1)
				{
					return _DXMEXT XMVectorPermute<uint32_t(PermutesArgs)...>(v0, v1);
				}
			};

			template <typename IS>
			struct select_impl;

			template <index_t... Masks>
			struct select_impl<index_sequence<Masks...>>
			{
				static constexpr uint32_t zero_or_all(index_t val)
				{
					return val ? 0xFFFFFFFF : 0;
				}

				inline static XMVECTOR XM_CALLCONV invoke(FXMVECTOR v0, FXMVECTOR v1)
				{
					//XMVECTOR control = XMVectorSetInt(zero_or_all(Masks)...);
					return _DXMEXT XMVectorSelect<((bool)Masks)...>(v0, v1);
				}
			};

			template <typename Mask, typename IS>
			struct sequence_to_mask_impl;

			template <index_t... Masks, index_t Head, index_t... Indices>
			struct sequence_to_mask_impl<index_sequence<Masks...>, index_sequence<Head, Indices...>>
				: public sequence_to_mask_impl< set_element_t<Head, 1, index_sequence<Masks...>>, index_sequence<Indices...>>
			{};

			template <index_t... Masks>
			struct sequence_to_mask_impl<index_sequence<Masks...>, index_sequence<>>
			{
				using type = index_sequence<Masks...>;
			};

			template <typename IS>
			struct sequence_to_mask : public sequence_to_mask_impl<index_sequence<0, 0, 0, 0>, IS>
			{};

			template <typename IS1, typename IS2>
			struct concat_swizzle_sequence;

			template <index_t... IS1, index_t... IS2>
			struct concat_swizzle_sequence <index_sequence<IS1...>, index_sequence<IS2...>>
			{
				using type = index_sequence<(get_element<IS2, index_sequence<IS1...>>::value)...>;
			};

			template <typename IS>
			struct is_permutation_sequence : public std::false_type {};

			template <index_t I0>
			struct is_permutation_sequence<index_sequence<I0>> : public std::true_type {};

			template <index_t I0, index_t I1>
			struct is_permutation_sequence<index_sequence<I0,I1>> : public std::integral_constant<bool, I0 != I1> {};

			template <index_t I0, index_t I1, index_t I2>
			struct is_permutation_sequence<index_sequence<I0, I1, I2>> : public std::integral_constant<bool, I0 != I1 && I0 != I2 && I1 != I2> {};

			template <index_t I0, index_t I1, index_t I2, index_t I3>
			struct is_permutation_sequence<index_sequence<I0, I1, I2, I3>> : public std::integral_constant<bool, I0 != I1 && I0 != I2 && I1 != I2 && I0 != I3 && I1 != I3 && I2 != I3> {};


			template <index_t Divisor, typename IS1>
			struct div_sequence;

			template <index_t Divisor, index_t... Vals>
			struct div_sequence<Divisor, index_sequence<Vals...>>
			{
				using quotient = index_sequence<(Vals / Divisor)...>;
				using remainder = index_sequence<(Vals % Divisor)...>;
			};

			// sort the sequence IS1, swizzling IS2 as the way to form IS1
			template <typename IS1, typename IS2>
			struct dual_sort_sequence;

			template <size_t... IS1, size_t... IS2>
			struct dual_sort_sequence<index_sequence<IS1...>, index_sequence<IS2...>>
			{
				static_assert(sizeof...(IS1) == sizeof...(IS2), "Sequence length must agree");
				using mad_seq = typename index_sequence<(IS1 * 4 + IS2)...>::type;
				using sorted_mad = typename sort_sequence<mad_seq>::type;
				using type = typename div_sequence<4, sorted_mad>::remainder;
			};


			template <typename IS>
			struct inverse_swizzle_sequence;

			template <typename IS1, typename IS2>
			using concat_swizzle_sequence_t = typename concat_swizzle_sequence<IS1, IS2>::type;

		}

		namespace detail
		{
			template <typename _T, typename IS>
			struct swizzle_from_indecis;

			template <typename _T, index_t... _SwzArgs>
			struct swizzle_from_indecis<_T, index_sequence<_SwzArgs...>>
			{
				using type = xmswizzler<_T, _SwzArgs...>;
			};

			template <typename _T, typename IS>
			using swizzle_from_indecis_t = typename swizzle_from_indecis<_T, IS>::type;

			template <typename XMV_S1, index_t... SW2>
			struct xmswizzler_concat;

			template <typename _T, index_t... SW1, index_t... SW2>
			struct xmswizzler_concat<xmswizzler<_T, SW1...>, SW2...>
			{
				using type = swizzle_from_indecis_t <
					_T,
					concat_swizzle_sequence_t <
					index_sequence<SW1...>,
					index_sequence<SW2... >> >;
			};

			template <typename XMV_S1, index_t... SW2>
			using xmswizzler_concat_t = typename xmswizzler_concat<XMV_S1, SW2...>::type;

			//using test = xmswizzler_concat_t<xmswizzler<float, 1, 2, 3>, 3, 2, 1>;
		}

		// This type is a wrapper for containing the swizzle and mask information of a xmvector
		// Your should never construct this type like xmswizzler<float,_y,_z> xmv;
		// instead, use function swizzle on an existed xmvector to construct
		// helps to optimize in the case case of masked/swizzled assignment: v.yzw = p, v.wyz = p.xzw;
		template <typename _T, index_t... _SwzArgs>
		struct XM_ALIGNATTR XM_EMPTY_BASE xmswizzler 
			: public detail::logical_bitwise_operator_base<xmswizzler<_T, _SwzArgs...>, _T, sizeof...(_SwzArgs)>
		{
			using this_type = xmswizzler<_T, _SwzArgs...>;
			using Scalar = _T;
			static constexpr size_t Size = sizeof...(_SwzArgs);

			static_assert(Size <= 4, "Swizzle element count out of 4");
			static_assert(mpl::conjunction < std::integral_constant<bool, _SwzArgs < 4>...>::value, "Swizzle index out of [0,3]");

			static constexpr size_t size = Size;
			using scalar_type = Scalar;
			using Indices = std::index_sequence<_SwzArgs...>;
			using index_type = Indices;
			using xmvector_type = xmvector<Scalar, Size>;
			using return_type = xmvector_type;
			static constexpr bool is_lvalue = detail::is_permutation_sequence<Indices>::value;
			using MergedIndices = std::conditional_t<Size == 4, Indices, mpl::overwrite_sequence_t<Indices, std::index_sequence<0, 1, 2, 3>>>;

			xmswizzler() = delete;
			// copy construct is an special form of the general assign operator
			xmswizzler(const this_type& rhs) = delete;
			xmswizzler(this_type&& rhs) = delete;
			//		void operator=(const this_type& rhs) = delete;

			inline xmvector_type XM_CALLCONV eval() const
			{
				xmvector_type ret;
				ret.v = detail::swizzle_impl<MergedIndices>::invoke(this->v);
				return ret;
			}

			template <typename _NewType>
			inline xmvector<_NewType, Size> XM_CALLCONV cast() const
			{
				return this->eval().cast<_NewType>();
			}

			inline XM_CALLCONV operator XMVECTOR () const
			{ return eval().v; }

			inline XM_CALLCONV operator xmvector_type() const
			{ return eval(); }

			// Scalar Selection 
			template <typename U = Scalar>
			inline XM_CALLCONV operator enable_if_t<Size == 1 && std::is_same<U, Scalar>::value, U>() const
			{ return detail::get<Scalar, _SwzArgs...>(this->v); }

			template <typename U = Scalar>
			inline XM_CALLCONV operator enable_if_t<Size == 1 && std::is_same<U, Scalar>::value, xmscalar<U>>() const
			{ return xmscalar<U>(detail::splat<_SwzArgs...>(this->v)); }

			template <index_t... _NewSwzArgs>
			inline const detail::xmswizzler_concat_t<this_type, _NewSwzArgs...>&
				XM_CALLCONV swizzle() const {
				static_assert(detail::valiad_swizzle_args<Size, _SwzArgs...>::value, "Swizzle index out of range");
				return reinterpret_cast<const detail::xmswizzler_concat_t<this_type, _NewSwzArgs...>&>(*this);
			}

			template <index_t... _NewSwzArgs>
			inline detail::xmswizzler_concat_t<this_type, _NewSwzArgs...>&
				XM_CALLCONV swizzle() {
				static_assert(detail::valiad_swizzle_args<Size, _SwzArgs...>::value, "Swizzle index out of range");
				return reinterpret_cast<detail::xmswizzler_concat_t<this_type, _NewSwzArgs...>&>(*this);
			}

			// When src and dst swizzle are same, this becomes an XMVectorSelect Masked Assignment Problem
			// v4.zxy = v4.zxy
			template <typename U = Scalar>
			inline enable_if_t<(Size < 4) && std::is_same<U, Scalar>::value>
				XM_CALLCONV
				assign(const xmswizzler<U, _SwzArgs...>& src)
			{
				static_assert(is_lvalue, "Swizzle expression contains duplication (like v.xxx) is not l-valued, and can not be assign to");
				using mask_seq = typename detail::sequence_to_mask<std::index_sequence<_SwzArgs...>>::type;
				this->v = detail::select_impl<mask_seq>::invoke(this->v, src.v);
			}

			// any to any assignment, expcept swz == src_swz
			// v4.yz = v3.zy;
			template <index_t... _SrcSwz>
			inline enable_if_t<(Size < 4) && !std::is_same<std::index_sequence<_SwzArgs...>, std::index_sequence<_SrcSwz...>>::value && (sizeof...(_SwzArgs) == sizeof...(_SrcSwz))>
				XM_CALLCONV
				assign(const xmswizzler<Scalar, _SrcSwz...>& src)
			{
				static_assert(is_lvalue, "Swizzle expression contains duplication (like v.xxx) is not l-valued, and can not be assign to");
				using permute_sequence = typename mpl::indirect_assign<std::index_sequence<0, 1, 2, 3>, std::index_sequence<_SwzArgs...>, std::index_sequence<(_SrcSwz + 4)...>>::type;
				this->v = detail::permute_impl<permute_sequence>::invoke(this->v, src.v);
			}

			// 4 to 4 assignment
			// v4.wyxz = v4.yxwz;
			template <index_t... _SrcSwz>
			inline enable_if_t<sizeof...(_SwzArgs) == 4 && sizeof...(_SrcSwz) == 4>
				XM_CALLCONV
				assign(const xmswizzler<Scalar, _SrcSwz...>& src)
			{
				static_assert(is_lvalue, "Swizzle expression contains duplication (like v.xxx) is not l-valued, and can not be assign to");
				using permute_sequence = typename mpl::indirect_assign<std::index_sequence<-1, -1, -1, -1>, std::index_sequence<_SwzArgs...>, std::index_sequence<_SrcSwz...>>::type;
				this->v = detail::swizzle_impl<permute_sequence>::invoke(src.v);
			}

			// identity to any assignment
			// v4.yxz = v3;
			template <index_t SrcSize>
			inline enable_if_t<sizeof...(_SwzArgs) == SrcSize>
				XM_CALLCONV
				assign(const xmvector<Scalar, SrcSize> src)
			{
				static_assert(is_lvalue, "Swizzle expression contains duplication (like v.xxx) is not l-valued, and can not be assign to");
				using SrcSwizzlerType = detail::swizzle_from_indecis_t<Scalar, std::make_index_sequence<SrcSize>>;
				auto& srcv = reinterpret_cast<const SrcSwizzlerType&>(src);
				this->assign(srcv);
			}

			template <typename _Ty>
			inline typename traits::enable_memery_traits_t<_Ty>::void_type XM_CALLCONV store(_Ty& storage) const
			{
				using traits = traits::memery_vector_traits<_Ty>;
				auto temp = this->eval();
				using sl_impl = detail::storage_helper<typename traits::scalar, is_aligned<_Ty>::value, traits::cols, traits::rows>;
				sl_impl::store(reinterpret_cast<typename traits::scalar*>(&storage), temp);
			}

			// Load from storage types
			template <typename _Ty>
			inline typename traits::enable_memery_traits_t<_Ty>::void_type operator=(const _Ty& memery_vector)
			{
				static_assert(is_lvalue, "Swizzle expression contains duplication (like v.xxx) is not l-valued, and can not be assign to");
				using traits = traits::memery_vector_traits<_Ty>;
				using sl_impl = detail::storage_helper<typename traits::scalar, is_aligned<_Ty>::value, traits::cols, traits::rows>;
				auto temp = sl_impl::load(reinterpret_cast<const typename traits::scalar*>(&memery_vector));
				this->assign(temp);
			}

			// identity to any assignment
			// v4.yxz = v3;
			template <index_t SrcSize>
			inline enable_if_t<sizeof...(_SwzArgs) == SrcSize>
				XM_CALLCONV	operator=(const xmvector<Scalar, SrcSize> src)
			{ this->assign(src); }

			template <typename U = Scalar>
			inline enable_if_t<Size == 1 && std::is_same<U, Scalar>::value> 
				XM_CALLCONV operator= (U scalar)
			{ this->v = detail::set<Scalar, _SwzArgs...>(this->v, scalar); }

			inline void	XM_CALLCONV operator= (const xmscalar<Scalar> scalar)
			{ this->assign(reinterpret_cast<const this_type&>(scalar)); }

			//template <typename U>
			//inline enable_if_t<(Size < 4) && std::is_same<U, Scalar>::value>
			void XM_CALLCONV operator=(const this_type& src)
			{ this->assign(src); }

			// any to any assignment
			// v4.yz = v3.zy;
			template <index_t... _SrcSwz>
			inline enable_if_t<(sizeof...(_SwzArgs) == sizeof...(_SrcSwz)) && !std::is_same<std::index_sequence<_SrcSwz...>, std::index_sequence<_SwzArgs...>>::value>
				XM_CALLCONV operator=(const xmswizzler<Scalar, _SrcSwz...>& src)
			{ this->assign(src); }

			template <template<typename, size_t> typename _TOperator, index_t... _SrcSwz>
			inline void XM_CALLCONV invoke_operator_assign(const xmswizzler<Scalar, _SrcSwz...>& src)
			{
				static_assert(is_lvalue, "Swizzle expression contains duplication (like v.xxx) is not l-valued, and can not be assign to");
				static_assert(sizeof...(_SrcSwz) == Size, "swizzler dimension must agree");
				using permute_sequence = typename mpl::indirect_assign<std::index_sequence<0, 1, 2, 3>, std::index_sequence<_SwzArgs...>, std::index_sequence<(_SrcSwz + 4)...>>::type;
				using opr = _TOperator<scalar_type, 4>;
				XMVECTOR i = opr::identity();
				XMVECTOR temp = detail::permute_impl<permute_sequence>::invoke(i, src.v);
				this->v = opr::invoke(this->v, temp);
			}

			// Case of swizzler + vector(scalar)
			inline xmvector_type XM_CALLCONV operator - () const
			{ return this->eval().operator-(); }
			inline xmvector_type XM_CALLCONV operator + (const xmvector_type rhs) const
			{ return rhs.operator+(*this); }
			inline xmvector_type XM_CALLCONV operator - (const xmvector_type rhs) const
			{ return rhs.operator-(*this); }
			inline xmvector_type XM_CALLCONV operator * (const xmvector_type rhs) const
			{ return rhs.operator*(*this); }
			inline xmvector_type XM_CALLCONV operator / (const xmvector_type rhs) const
			{ return rhs.operator/(*this); }

			template <index_t... _SrcSwz> inline void XM_CALLCONV operator+=(const xmswizzler<Scalar, _SrcSwz...>& src)
			{ invoke_operator_assign<vector_math::add, _SrcSwz...>(src); }
			template <index_t... _SrcSwz> inline void XM_CALLCONV operator-=(const xmswizzler<Scalar, _SrcSwz...>& src)
			{ invoke_operator_assign<vector_math::subtract, _SrcSwz...>(src); }
			template <index_t... _SrcSwz> inline void XM_CALLCONV operator*=(const xmswizzler<Scalar, _SrcSwz...>& src)
			{ invoke_operator_assign<vector_math::multiply, _SrcSwz...>(src); }
			template <index_t... _SrcSwz> inline void XM_CALLCONV operator/=(const xmswizzler<Scalar, _SrcSwz...>& src)
			{ invoke_operator_assign<vector_math::divide, _SrcSwz...>(src); }

			// identity to any assignment
			inline void XM_CALLCONV operator+=(const xmvector_type rhs)
			{
				auto& srcv = reinterpret_cast<const detail::swizzle_from_indecis_t<Scalar, std::make_index_sequence<Size>>&>(rhs);
				this->operator+=(srcv);
			}
			inline void XM_CALLCONV operator-=(const xmvector_type rhs)
			{
				auto& srcv = reinterpret_cast<const detail::swizzle_from_indecis_t<Scalar, std::make_index_sequence<Size>>&>(rhs);
				this->operator-=(srcv);
			}
			inline void XM_CALLCONV operator*=(const xmvector_type rhs)
			{
				auto& srcv = reinterpret_cast<const detail::swizzle_from_indecis_t<Scalar, std::make_index_sequence<Size>>&>(rhs);
				this->operator*=(srcv);
			}
			inline void XM_CALLCONV operator/=(const xmvector_type rhs)
			{
				auto& srcv = reinterpret_cast<const detail::swizzle_from_indecis_t<Scalar, std::make_index_sequence<Size>>&>(rhs);
				this->operator/=(srcv);
			}
		};

		namespace detail
		{
			// any to any assignment
			// v4.yz = v3.zy;
			template <typename _Ty, index_t... _DstSwz, index_t... _SrcSwz>
			inline enable_if_t<sizeof...(_DstSwz) == sizeof...(_SrcSwz), XMVECTOR>
				XM_CALLCONV
				swizzle_assign(xmswizzler<_Ty, _DstSwz...>& dst, const xmswizzler<_Ty, _SrcSwz...>& src)
			{
				using permute_sequence = typename indirect_assign<index_sequence<0, 1, 2, 3>, index_sequence<_DstSwz...>, index_sequence<(_SrcSwz + 4)...>>::type;

				return detail::permute_impl<permute_sequence>::invoke(dst.v, src.v);
			}

			// 4 to 4 assignment
			// v4.wyxz = v4.yxwz;
			template <typename _Ty, index_t... _DstSwz, index_t... _SrcSwz>
			inline enable_if_t<sizeof...(_DstSwz) == 4 && sizeof...(_DstSwz) == 4, XMVECTOR>
				XM_CALLCONV
				swizzle_assign(xmswizzler<_Ty, _DstSwz...>& dst, const  xmswizzler<_Ty, _SrcSwz...>& src)
			{
				using permute_sequence = typename indirect_assign<index_sequence<-1, -1, -1, -1>, index_sequence<_DstSwz...>, index_sequence<_SrcSwz...>>::type;
				return detail::swizzle_impl<permute_sequence>::invoke(src.v);
			}

			// identity to any assignment
			// v4.yxz = v3;
			template <typename _Ty, index_t... _DstSwz, index_t SrcSize>
			inline enable_if_t<sizeof...(_DstSwz) == SrcSize, XMVECTOR>
				XM_CALLCONV
				swizzle_assign(xmswizzler<_Ty, _DstSwz...>&  dst, const  xmvector<_Ty, SrcSize>& src)
			{
				using src_type = detail::swizzle_from_indecis_t<_Ty, std::make_index_sequence<SrcSize>>;
				const auto& srcv = reinterpret_cast<const src_type&>(src);
				return swizzle_assign(dst, srcv);
			}

			// When src and dst swizzle are same, this becomes an XMVectorSelect Masked Assignment Problem
			// v4.zxy = v4.zxy
			template <typename _Ty, index_t... _SwzzleArgs>
			inline XMVECTOR
				XM_CALLCONV
				swizzle_assign(xmswizzler<_Ty, _SwzzleArgs...>& dst, const  xmswizzler<_Ty, _SwzzleArgs...>& src)
			{
				using mask_seq = typename detail::sequence_to_mask<index_sequence<_SwzzleArgs...>>::type;
				return detail::select_impl<mask_seq>::invoke(src.v, dst.v);
			}
		}

		namespace detail
		{

			template <index_t... _SwzArgs, typename _T, size_t _Size>
			inline auto& XM_CALLCONV swizzle(const xmvector<_T, _Size>& xmv) {
				static_assert(valiad_swizzle_args<_Size, _SwzArgs...>::value, "Swizzle index out of range");
				return reinterpret_cast<const xmswizzler<_T, _SwzArgs...>&>(xmv);
			}

			template <index_t... _SwzArgs, typename _T, size_t _Size>
			inline auto& XM_CALLCONV swizzle(xmvector<_T, _Size>& xmv) {
				static_assert(valiad_swizzle_args<_Size, _SwzArgs...>::value, "Swizzle index out of range");
				return reinterpret_cast<xmswizzler<_T, _SwzArgs...>&>(xmv);
			}

			template <index_t... _SwzArgs, typename _T, index_t... _PrevSwzArgs>
			inline auto& XM_CALLCONV swizzle(xmswizzler<_T, _PrevSwzArgs...>&& xmv) {
				static constexpr size_t _Size = sizeof...(_PrevSwzArgs);
				static_assert(valiad_swizzle_args<_Size, _SwzArgs...>::value, "Swizzle index out of range");
				using src_t = xmswizzler<_T, _PrevSwzArgs...>;
				return reinterpret_cast<const detail::xmswizzler_concat_t<src_t, _SwzArgs...>&>(xmv);
			}

		}

		template<typename _T, size_t _Size>
		template<index_t ..._SwzArgs>
		inline xmswizzler<_T, _SwzArgs...>& XM_CALLCONV xmvector<_T, _Size>::swizzle()
		{
			static_assert(detail::valiad_swizzle_args<_Size, _SwzArgs...>::value, "Swizzle index out of range");
			return reinterpret_cast<xmswizzler<_T, _SwzArgs...>&>(*this);
		}

		template<typename _T, size_t _Size>
		template<index_t ..._SwzArgs>
		inline const xmswizzler<_T, _SwzArgs...>& XM_CALLCONV xmvector<_T, _Size>::swizzle() const
		{
			static_assert(detail::valiad_swizzle_args<_Size, _SwzArgs...>::value, "Swizzle index out of range");
			return reinterpret_cast<const xmswizzler<_T, _SwzArgs...>&>(*this);
		}
	}
}