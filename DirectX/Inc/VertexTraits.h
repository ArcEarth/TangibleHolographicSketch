#pragma once
#include "DirectXMathExtend.h"
#include <DirectXPackedVector.h>
#include <type_traits>

#ifndef XM_HAS_MEMBER_FUNCTION
#define XM_HAS_MEMBER_FUNCTION(member_name, name)                                   \
    template<typename T, typename Signature>                            \
    struct name {                                                       \
        typedef char yes[1];                                            \
        typedef char no [2];                                            \
        template <typename U, U> struct type_check;                     \
        template <typename _1> static yes &chk(type_check<Signature, &_1::member_name > *); \
        template <typename   > static no  &chk(...);                    \
        static bool const value = sizeof(chk<T>(0)) == sizeof(yes);     \
    }
#endif // !HAS_MEM_FUNC

#ifndef XM_HAS_MEMBER
#define XM_HAS_MEMBER(member_name, name)                                   \
    template<typename T>												\
    struct name {                                                       \
        typedef char yes[1];                                            \
        typedef char no [2];                                            \
        template <typename> static yes &chk(decltype(T::member_name)*);	\
        template <typename> static no  &chk(...);						\
        static bool const value = sizeof(chk<T>(0)) == sizeof(yes);     \
    }
#endif // !HAS_MEM_FUNC

namespace DirectX
{
	// provide generic methods for check, access, and modify vertex data
	namespace VertexTraits
	{
		XM_HAS_MEMBER(position, has_position);
		XM_HAS_MEMBER(normal, has_normal);
		XM_HAS_MEMBER(textureCoordinate, has_tex);
		XM_HAS_MEMBER(uv, has_uv);
		XM_HAS_MEMBER(color, has_color);
		XM_HAS_MEMBER(tangent, has_tangent);
		XM_HAS_MEMBER(weights, has_weights);

		XMGLOBALCONST
			XMUINT4 XMWeightIndexZero = {0U,0U,0U,0U};

		template <class TVertex>
		std::enable_if_t<has_position<TVertex>::value>
			XM_CALLCONV set_position(TVertex& vertex, FXMVECTOR position)
		{
			XMStore(vertex.position, position);
		}

		template <class TVertex>
		std::enable_if_t<!has_position<TVertex>::value>
			XM_CALLCONV set_position(TVertex& vertex, FXMVECTOR position)
		{
		}

		template <class TVertex>
		std::enable_if_t<has_position<TVertex>::value, XMVECTOR>
			XM_CALLCONV get_position(const TVertex& vertex, FXMVECTOR default_value = g_XMZero.v)
		{
			return XMLoad(vertex.position);
		}

		template <class TVertex>
		std::enable_if_t<!has_normal<TVertex>::value, XMVECTOR>
			XM_CALLCONV get_position(const TVertex& vertex, FXMVECTOR default_value = g_XMZero.v)
		{
			return default_value;
		}

		template <class TVertex>
		std::enable_if_t<has_normal<TVertex>::value>
			XM_CALLCONV set_normal(TVertex& vertex, FXMVECTOR normal)
		{
			XMStore(vertex.normal, normal);
		}

		template <class TVertex>
		std::enable_if_t<!has_normal<TVertex>::value>
			XM_CALLCONV set_normal(TVertex& vertex, FXMVECTOR normal)
		{
		}

		template <class TVertex>
		std::enable_if_t<has_normal<TVertex>::value, XMVECTOR>
			XM_CALLCONV get_normal(const TVertex& vertex, FXMVECTOR default_value = g_XMZero.v)
		{
			return XMLoad(vertex.normal);
		}

		template <class TVertex>
		std::enable_if_t<!has_normal<TVertex>::value, XMVECTOR>
			XM_CALLCONV get_normal(const TVertex& vertex, FXMVECTOR default_value = g_XMZero.v)
		{
			return default_value;
		}

		template <class TVertex>
		std::enable_if_t<has_color<TVertex>::value && std::is_base_of<DirectX::XMFLOAT4, decltype(TVertex::color)>::value>
			XM_CALLCONV set_color(TVertex& vertex, FXMVECTOR color)
		{
			XMStore(vertex.color, color);
		}

		template <class TVertex>
		std::enable_if_t<has_color<TVertex>::value && std::is_same<decltype(TVertex::color), uint32_t>::value>
			XM_CALLCONV set_color(TVertex& vertex, FXMVECTOR color)
		{
			using namespace ::DirectX::PackedVector;
			XMUBYTEN4 rgba;
			XMStoreUByteN4(&rgba, color);
			vertex.color = rgba.v;
		}

		template <class TVertex>
		std::enable_if_t<!has_color<TVertex>::value>
			XM_CALLCONV set_color(TVertex& vertex, FXMVECTOR color)
		{
		}

		template <class TVertex>
		std::enable_if_t<has_color<TVertex>::value && std::is_base_of<DirectX::XMFLOAT4, decltype(TVertex::color)>::value, XMVECTOR>
			XM_CALLCONV get_color(const TVertex& vertex, FXMVECTOR default_value = g_XMZero.v)
		{
			return XMLoad(vertex.color);
		}

		template <class TVertex>
		std::enable_if_t<has_color<TVertex>::value && std::is_same<decltype(TVertex::color), uint32_t>::value, XMVECTOR>
			XM_CALLCONV get_color(const TVertex& vertex, FXMVECTOR default_value = g_XMZero.v)
		{
			using namespace ::DirectX::PackedVector;
			XMUBYTEN4 rgba;
			rgba.v = vertex.color;
			return XMLoadUByteN4(&rgba);
		}

		template <class TVertex>
		std::enable_if_t<!has_color<TVertex>::value, XMVECTOR>
			XM_CALLCONV get_color(const TVertex& vertex, FXMVECTOR default_value = g_XMZero.v)
		{
			return default_value;
		}

		template <class TVertex>
		std::enable_if_t<has_tangent<TVertex>::value>
			XM_CALLCONV set_tangent(TVertex& vertex, FXMVECTOR tangent)
		{
			XMStore(vertex.tangent, tangent);
		}

		template <class TVertex>
		std::enable_if_t<!has_tangent<TVertex>::value>
			XM_CALLCONV set_tangent(TVertex& vertex, FXMVECTOR tangent)
		{
		}

		template <class TVertex>
		std::enable_if_t<has_tangent<TVertex>::value, XMVECTOR>
			XM_CALLCONV get_tangent(const TVertex& vertex, FXMVECTOR default_value = g_XMZero.v)
		{
			return XMLoad(vertex.tangent);
		}

		template <class TVertex>
		std::enable_if_t<!has_tangent<TVertex>::value, XMVECTOR>
			XM_CALLCONV get_tangent(const TVertex& vertex, FXMVECTOR default_value = g_XMZero.v)
		{
			return default_value;
		}

		template <class TVertex>
		std::enable_if_t<has_uv<TVertex>::value>
			XM_CALLCONV set_uv(TVertex& vertex, FXMVECTOR uv)
		{
			XMStore(vertex.uv, uv);
		}

		template <class TVertex>
		std::enable_if_t<!has_uv<TVertex>::value && has_tex<TVertex>::value>
			XM_CALLCONV set_uv(TVertex& vertex, FXMVECTOR uv)
		{
			XMStore(vertex.textureCoordinate, uv);
		}
		template <class TVertex>
		std::enable_if_t<!has_uv<TVertex>::value && !has_tex<TVertex>::value>
			XM_CALLCONV set_uv(TVertex& vertex, FXMVECTOR uv)
		{
		}

		template <class TVertex>
		std::enable_if_t<has_uv<TVertex>::value, XMVECTOR>
			XM_CALLCONV get_uv(const TVertex& vertex, FXMVECTOR default_value = g_XMZero.v)
		{
			return XMLoad(vertex.uv);
		}

		template <class TVertex>
		std::enable_if_t<!has_uv<TVertex>::value && has_tex<TVertex>::value, XMVECTOR>
			XM_CALLCONV get_uv(const TVertex& vertex, FXMVECTOR default_value = g_XMZero.v)
		{
			return XMLoad(vertex.textureCoordinate);
		}

		template <class TVertex>
		std::enable_if_t<!has_uv<TVertex>::value && !has_tex<TVertex>::value, XMVECTOR>
			XM_CALLCONV get_uv(const TVertex& vertex, FXMVECTOR default_value = g_XMZero.v)
		{
			return default_value;
		}

		template <class TVertex>
		std::enable_if_t<has_uv<TVertex>::value>
			XM_CALLCONV set_uv(TVertex& vertex, float u, float v)
		{
			vertex.uv.x = u; vertex.uv.y = v;
		}
		
		template <class TVertex>
		std::enable_if_t<!has_uv<TVertex>::value && has_tex<TVertex>::value>
			XM_CALLCONV set_uv(TVertex& vertex, float u, float v)
		{
			vertex.textureCoordinate.x = u;
			vertex.textureCoordinate.y = v;
		}

		template <class TVertex>
		std::enable_if_t<!has_uv<TVertex>::value && !has_tex<TVertex>::value>
			XM_CALLCONV set_uv(TVertex& vertex, float u, float v)
		{
		}

		namespace detail
		{
			template <class TVertex>
			inline std::enable_if_t<std::is_same<decltype(TVertex::indices), uint32_t>::value> 
				SetVertexBlendIndices(TVertex& vertex, XMUINT4 const& iindices)
			{
				vertex.indices = ((iindices.w & 0xff) << 24) | ((iindices.z & 0xff) << 16) | ((iindices.y & 0xff) << 8) | (iindices.x & 0xff);
			}

			template <class TVertex>
			inline std::enable_if_t<std::is_same<decltype(TVertex::weights),uint32_t>::value> 
				XM_CALLCONV SetVertexBlendWeights(TVertex& vertex, FXMVECTOR iweights)
			{
				PackedVector::XMUBYTEN4 packed;
				PackedVector::XMStoreUByteN4(&packed, iweights);
				vertex.weights = packed.v;
			}

			template <class TVertex>
			inline std::enable_if_t<std::is_base_of<DirectX::XMFLOAT4 , decltype(TVertex::weights)>::value> 
				XM_CALLCONV SetVertexBlendWeights(TVertex& vertex, FXMVECTOR iweights)
			{
				XMStore(vertex.weights,weights);
			}
		}

		template <class TVertex>
		std::enable_if_t<has_weights<TVertex>::value>
			XM_CALLCONV set_weights(TVertex& vertex, FXMVECTOR weights, const XMUINT4 &index)
		{
			detail::SetVertexBlendWeights(vertex, weights);
			detail::SetVertexBlendIndices(vertex, index);
		}

		template <class TVertex>
		std::enable_if_t<!has_weights<TVertex>::value>
			XM_CALLCONV set_weights(TVertex& vertex, FXMVECTOR weights, const XMUINT4 &index)
		{
		}

		template <class TVertex>
		std::enable_if_t<has_weights<TVertex>::value && std::is_same<decltype(TVertex::weights), uint32_t>::value, XMVECTOR>
			XM_CALLCONV get_weights(const TVertex& vertex, FXMVECTOR default_value = g_XMZero.v)
		{
			PackedVector::XMUBYTEN4 packed;
			packed.v = vertex.weights;
			return DirectX::PackedVector::XMLoadUByte4(&packed);
		}

		template <class TVertex>
		std::enable_if_t<has_weights<TVertex>::value && std::is_base_of<DirectX::XMFLOAT4, decltype(TVertex::weights)>::value, XMVECTOR>
			XM_CALLCONV get_weights(const TVertex& vertex, FXMVECTOR default_value = g_XMZero.v)
		{
			return XMLoad(vertex.weights);
		}

		template <class TVertex>
		std::enable_if_t<!has_weights<TVertex>::value, XMVECTOR>
			XM_CALLCONV get_weights(const TVertex& vertex, FXMVECTOR default_value = g_XMZero.v)
		{
			return default_value;
		}

		template <class TVertex>
		std::enable_if_t<has_weights<TVertex>::value && std::is_same<decltype(TVertex::indices), uint32_t>::value, XMUINT4>
			XM_CALLCONV get_indices(const TVertex& vertex, const XMUINT4& default_value = XMWeightIndexZero)
		{
			XMUINT4 value;
			value.x = (vertex.indices >> 0) & 0xff;
			value.y = (vertex.indices >> 8) & 0xff;
			value.z = (vertex.indices >> 16) & 0xff;
			value.w = (vertex.indices >> 24) & 0xff;
			return value;
		}

		template <class TVertex>
		std::enable_if_t<!has_weights<TVertex>::value, XMUINT4>
			XM_CALLCONV get_indices(const TVertex& vertex, const XMUINT4& default_value = XMWeightIndexZero)
		{
			return default_value;
		}

		template <class TVertex>
		void set_vertex(TVertex& vertex, FXMVECTOR position, FXMVECTOR normal = g_XMZero.v, FXMVECTOR uv = g_XMZero.v, GXMVECTOR tangent = g_XMZero.v, HXMVECTOR color = g_XMZero.v, HXMVECTOR weights = g_XMZero.v, const XMUINT4 &index = XMWeightIndexZero)
		{
			set_position(vertex, position);
			set_normal(vertex, normal);
			set_uv(vertex, uv);
			set_tangent(vertex, tangent);
			set_color(vertex, color);
			set_weights(vertex, weights, index);
		}

		template <class TSrcVertex, class TDstVertex>
		void convert_vertex(const TSrcVertex& src, TDstVertex& dst)
		{
			set_position(dst, get_position(src));
			set_normal(dst, get_normal(src));
			set_uv(dst, get_uv(src));
			set_tangent(dst, get_tangent(src));
			set_color(dst, get_color(src));
			set_weights(dst, get_weights(src), get_indices(src));
		}
	}
}