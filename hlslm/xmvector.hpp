#pragma once
#ifndef _HLSL_XM_VECTOR_H
#define _HLSL_XM_VECTOR_H

#include "detail/xmvector_impl.inl"

#if defined(_M_IX86) && defined(_DEBUG)
#define XM_VECTOR_LOAD_CTOR 
//explicit
#else
#define XM_VECTOR_LOAD_CTOR
#endif

namespace DirectX
{
	namespace hlsl
	{
		using index_t = size_t;
		using uint = uint32_t;

		template <typename _T>
		struct xmscalar;

		template <typename _T, size_t _Size>
		struct xmvector;

		template <typename _T, index_t... _SwzArgs>
		struct xmswizzler;

		// strong-typed workload for vector math
		// SIMD register vector type wrapper for __mm128 / float32x4_t / etc...
		// features:
		// Element(s)-Select-Swizzle methods, like hlsl, v.yw() = v.xx();
		// Arthmetic type: operator [+-*/] are overloaded to element wise 
		// Bitwise type: operator [&|^] are overloaded for integer vectors
		// Vector type: operator[](int) are overloaded, but be aware of the poor performance
		// example:
		// xmvector<float,4> v = {.0f,.1f,0.2f,0.3f};
		// v.yz() *= v.xx(); 
		// Implemtation detail:
		// Why not inherit xmvector_base here but through the detail::swizzle_operator_base ?
		// An struct that contains an vector can be pass by register through __vectorcall
		// We refer to this type of struct as "vector struct"
		// "vector struct" should be construct by following rules
		// 0) ! empty structs are __not__ "vector struct" !
		// 1) A struct does not have any inherience, and only contains __mm128(i/d) field
		// 2) A struct inherit from only "vector struct" types, and only contains __mm128(i/d) field
		template <typename _T, size_t _Size>
		struct XM_ALIGNATTR XM_EMPTY_BASE xmvector : public detail::logical_bitwise_operator_base<xmvector<_T, _Size>,_T,_Size>
		{
			//using components_name_enums = detail::components_name_enums;
			using this_type = xmvector<_T, _Size>;
			static constexpr size_t Size = _Size;
			static constexpr size_t size = Size;
			using Scalar = _T;
			using scalar_type = _T;
			typedef xmvector SelfType;
			using intrinsic_vector = detail::get_intrinsic_vector_t<_T, _Size>;

			static_assert(Size > 0 && sizeof(Scalar) * Size <= 16, "Instantiate xmvector of dimension 0 or greater than 4.");

			inline xmvector() = default;

			inline explicit xmvector(CXMVECTOR xmv) { v = xmv; }

			inline explicit xmvector(Scalar s) {
				v = detail::replicate_scalar(s);
			}

			inline xmvector(Scalar _x, Scalar _y, Scalar _z = Scalar(0), Scalar _w = Scalar(0)) {
				v = detail::set_vector(_x, _y, _z, _w);
			}

			this_type& operator= (const this_type& rhs) { this->v = rhs.v; return *this; }

			inline operator XMVECTOR () const
			{
				return v;
			}

			// reinterpret cast this vector
			template <typename _NewType, size_t _NewSize = _Size>
			inline const xmvector<_NewType, _NewSize>& as() const {
				return reinterpret_cast<const xmvector<_NewType, _NewSize>&>(*this);
			}

			// reinterpret cast this vector
			template <typename _NewType, size_t _NewSize = _Size>
			inline xmvector<_NewType, _NewSize>& as() {
				return reinterpret_cast<xmvector<_NewType, _NewSize>&>(*this);
			}

			// reinterpret cast this vector
			template <size_t _NewSize>
			inline const xmvector<Scalar, _NewSize>& as() const {
				return reinterpret_cast<const xmvector<Scalar, _NewSize>&>(*this);
			}

			// reinterpret cast this vector
			template <size_t _NewSize>
			inline xmvector<Scalar, _NewSize>& as() {
				return reinterpret_cast<xmvector<Scalar, _NewSize>&>(*this);
			}

			template <int _NewSize>
			inline explicit operator const xmvector<Scalar, _NewSize>&() const {
				return as<Scalar, _NewSize>();
			}

			template <typename _NewType>
			inline xmvector<_NewType, _Size> XM_CALLCONV cast() const
			{
				xmvector<_NewType, _Size> result;
				result.v = detail::cast_vector<Scalar, _NewType, _Size>(this->v);
				return result;
			}

			template <index_t... selectors>
			inline const xmswizzler<_T, selectors...>& XM_CALLCONV swizzle() const;

			template <index_t... selectors>
			inline xmswizzler<_T, selectors...>& XM_CALLCONV swizzle();

			inline Scalar get(size_t elem_index) const
			{
				assert(elem_index < Size);
				return detail::get<Scalar>(this->v, elem_index);
			}

			template<index_t elem_index>
			inline Scalar get() const
			{
				return detail::get<Scalar, elem_index>(this->v);
			}

			inline void set(size_t elem_index, Scalar value)
			{
				assert(elem_index < Size);
				this->v = detail::set<Scalar>(this->v, elem_index, value);
			}

			template<index_t elem_index>
			inline void set(Scalar value) const
			{
				return detail::set<Scalar, elem_index>(this->v,value);
			}

#if !defined(_XM_NO_INTRINSICS_)
			[[deprecated("For compile constant index, use method v.x()/etc... or get<index>() instead.")]]
#endif
			inline Scalar operator[](size_t elem_index) const
			{ return get(elem_index); }

			// Dynamic swizzler
			inline auto XM_CALLCONV swizzle(uint _x, uint _y = 1, uint _z = 2, uint _w = 3) const
			{ return xmvector<Scalar, 4>(_DXMEXT XMVectorSwizzle(v, _x, _y, _z, _w)); }

			inline this_type XM_CALLCONV operator - () const
			{ this_type ret; ret.v = vector_math::negate<scalar_type, size>::invoke(this->v); return ret; }

			inline this_type& XM_CALLCONV operator += (const this_type rhs)
			{ this->v = vector_math::add<scalar_type, size>::invoke(this->v, rhs.v); return *this; }
			inline this_type& XM_CALLCONV operator -= (const this_type rhs)
			{ this->v = vector_math::subtract<scalar_type, size>::invoke(this->v, rhs.v); return *this; }
			inline this_type& XM_CALLCONV operator *= (const this_type rhs)
			{ this->v = vector_math::multiply<scalar_type, size>::invoke(this->v, rhs.v); return *this; }
			inline this_type& XM_CALLCONV operator /= (const this_type rhs)
			{ this->v = vector_math::divide<scalar_type, size>::invoke(this->v, rhs.v); return *this; }

			inline this_type XM_CALLCONV operator + (const this_type rhs) const
			{ this_type ret; ret.v = vector_math::add<scalar_type, size>::invoke(this->v, rhs.v); return ret; }
			inline this_type XM_CALLCONV operator - (const this_type rhs) const
			{ this_type ret; ret.v = vector_math::subtract<scalar_type, size>::invoke(this->v, rhs.v); return ret; }
			inline this_type XM_CALLCONV operator * (const this_type rhs) const
			{ this_type ret; ret.v = vector_math::multiply<scalar_type, size>::invoke(this->v, rhs.v); return ret; }
			inline this_type XM_CALLCONV operator / (const this_type rhs) const
			{ this_type ret; ret.v = vector_math::divide<scalar_type, size>::invoke(this->v, rhs.v); return ret; }


#if defined(_XM_VECTOR_USE_LOAD_STORE_HELPER_)

			template <typename _Ty, typename _TRet = void>
			using enable_if_loadable = std::enable_if_t<traits::is_memory_type<_Ty>::value && (traits::memery_vector_traits<_Ty>::cols >= Size) && std::is_same<Scalar,typename traits::memery_vector_traits<_Ty>::scalar>::value, _TRet>;

			// Load from storage types
			template <typename _Ty>
			inline enable_if_loadable<_Ty> operator=(const _Ty& memory_vector)
			{
				using traits = traits::memery_vector_traits<_Ty>;
				using load_imple = detail::storage_helper<typename traits::scalar, is_aligned<_Ty>::value, Size>;
				this->v = load_imple::load(reinterpret_cast<const typename traits::scalar*>(&memory_vector));
			}

			// explicit is an walk around for compiler internal error in x86|Debug mode
			template <typename _Ty>
			inline XM_VECTOR_LOAD_CTOR xmvector(const _Ty& memory_vector, enable_if_loadable<_Ty> *junk = nullptr)
			{ this->operator=<_Ty>(memory_vector); }

			template <typename _Ty>
			inline typename traits::enable_memery_traits_t<_Ty>::void_type XM_CALLCONV store(_Ty& storage) const
			{
				using traits = traits::memery_vector_traits<_Ty>;
				using load_imple = detail::storage_helper<typename traits::scalar, is_aligned<_Ty>::value, traits::cols, traits::rows>;
				load_imple::store(reinterpret_cast<typename traits::scalar*>(&storage), this->v);
			}

			template <typename _Ty>
			inline typename traits::enable_memery_traits_t<_Ty>::void_type XM_CALLCONV store_a(_Ty& storage) const
			{
				using traits = traits::memery_vector_traits<_Ty>;
				using load_imple = detail::storage_helper<typename traits::scalar, true, traits::cols, traits::rows>;
				load_imple::store(reinterpret_cast<typename traits::scalar*>(&storage), this->v);
			}
#endif
		};

		template <typename _T>
		struct XM_ALIGNATTR XM_EMPTY_BASE xmscalar : public xmvector<_T, 1>
		{
			using base_type = xmvector<_T, 1>;
			using this_type = xmscalar<_T>;
			using typename base_type::scalar_type;
			using typename base_type::Scalar;

			static constexpr size_t impl_size = 4;

			inline xmscalar() = default;

			// x86 Debug mode crush by these type of implicit constructor
			inline XM_VECTOR_LOAD_CTOR xmscalar(Scalar s) {
				this->v = detail::replicate_scalar(s);
			}

			inline explicit xmscalar(CXMVECTOR xmv) {
				this->v = xmv;
			}

			inline xmscalar& operator=(const xmscalar& rhs) {
				this->v = rhs.v; return *this;
			}

			inline xmscalar& operator=(Scalar s) {
				this->v = detail::replicate_scalar(s);
				return *this;
			}

			inline operator Scalar () const {
				return this->get<0>();
			}

			template <size_t _Size>
			inline XM_CALLCONV operator const xmvector<Scalar, _Size>&() const
			{
				return reinterpret_cast<const xmvector<Scalar, _Size>&>(*this);
			}


			inline this_type& XM_CALLCONV operator += (const this_type rhs)
			{
				this->v = vector_math::add<scalar_type, impl_size>::invoke(this->v, rhs.v);
				return *this;
			}

			inline this_type& XM_CALLCONV operator -= (const this_type rhs)
			{
				this->v = vector_math::subtract<scalar_type, impl_size>::invoke(this->v, rhs.v);
				return *this;
			}

			inline this_type& XM_CALLCONV operator *= (const this_type rhs)
			{
				this->v = vector_math::multiply<scalar_type, impl_size>::invoke(this->v, rhs.v);
				return *this;
			}

			inline this_type& XM_CALLCONV operator /= (const this_type rhs)
			{
				this->v = vector_math::divide<scalar_type, impl_size>::invoke(this->v, rhs.v);
				return *this;
			}

			inline this_type XM_CALLCONV operator - () const
			{
				this_type ret; ret.v = vector_math::negate<scalar_type, impl_size>::invoke(this->v); return ret;
			}

			inline this_type XM_CALLCONV operator + (const this_type rhs) const
			{
				this_type ret = *this; ret.operator+=(rhs); return ret;
			}
			inline this_type XM_CALLCONV operator - (const this_type rhs) const
			{
				this_type ret = *this; ret.operator-=(rhs); return ret;
			}
			inline this_type XM_CALLCONV operator * (const this_type rhs) const
			{
				this_type ret = *this; ret.operator*=(rhs); return ret;
			}
			inline this_type XM_CALLCONV operator / (const this_type rhs) const
			{
				this_type ret = *this; ret.operator/=(rhs); return ret;
			}

			// Case of scalar + vector
			template <size_t _RhsSize>
			inline xmvector<Scalar, _RhsSize> XM_CALLCONV operator + (const xmvector<Scalar, _RhsSize> rhs) const
			{ return rhs.operator+(*this); }
			template <size_t _RhsSize>
			inline xmvector<Scalar, _RhsSize> XM_CALLCONV operator - (const xmvector<Scalar, _RhsSize> rhs) const
			{ return rhs.operator-(*this); }
			template <size_t _RhsSize>
			inline xmvector<Scalar, _RhsSize> XM_CALLCONV operator * (const xmvector<Scalar, _RhsSize> rhs) const
			{ return rhs.operator*(*this); }
			template <size_t _RhsSize>
			inline xmvector<Scalar, _RhsSize> XM_CALLCONV operator / (const xmvector<Scalar, _RhsSize> rhs) const
			{ return reinterpret_cast<const xmvector<Scalar, _RhsSize>&>(*this).operator/(rhs); }

			template <index_t... _SrcSwz> 
			inline xmvector<Scalar,sizeof...(_SrcSwz)> XM_CALLCONV operator+(const xmswizzler<Scalar, _SrcSwz...>& rhs) const
			{ return rhs.operator+(reinterpret_cast<const xmswizzler<Scalar, _SrcSwz...>&>(*this)); }
			template <index_t... _SrcSwz> 
			inline xmvector<Scalar, sizeof...(_SrcSwz)> XM_CALLCONV operator-(const xmswizzler<Scalar, _SrcSwz...>& rhs) const
			{ return rhs.operator-(reinterpret_cast<const xmswizzler<Scalar, _SrcSwz...>&>(*this)); }
			template <index_t... _SrcSwz> 
			inline xmvector<Scalar, sizeof...(_SrcSwz)> XM_CALLCONV operator*(const xmswizzler<Scalar, _SrcSwz...>& rhs) const
			{ return rhs.operator*(reinterpret_cast<const xmswizzler<Scalar, _SrcSwz...>&>(*this)); }
			template <index_t... _SrcSwz> 
			inline xmvector<Scalar, sizeof...(_SrcSwz)> XM_CALLCONV operator/(const xmswizzler<Scalar, _SrcSwz...>& rhs) const
			{ return (reinterpret_cast<const xmvector<Scalar, sizeof...(_SrcSwz)>&>(*this)).operator/(rhs.eval()); }

		};

		// Untyped Get Set implementation
		// struct storage_helper definition
		namespace detail
		{
			template <uint32_t Elem>
			// broadcast an element to all dimension, like xyzw -> xxxx
			inline XMVECTOR splat(FXMVECTOR xmv);

			template <>
			inline XMVECTOR splat<0>(FXMVECTOR xmv) { return _DXMEXT XMVectorSplatX(xmv); }
			template <>
			inline XMVECTOR splat<1>(FXMVECTOR xmv) { return _DXMEXT XMVectorSplatY(xmv); }
			template <>
			inline XMVECTOR splat<2>(FXMVECTOR xmv) { return _DXMEXT XMVectorSplatZ(xmv); }
			template <>
			inline XMVECTOR splat<3>(FXMVECTOR xmv) { return _DXMEXT XMVectorSplatW(xmv); }


			template <> inline float XM_CALLCONV get<float, _x>(FXMVECTOR xmv) { return _DXMEXT XMVectorGetX(xmv); }
			template <> inline float XM_CALLCONV get<float, _y>(FXMVECTOR xmv) { return _DXMEXT XMVectorGetY(xmv); }
			template <> inline float XM_CALLCONV get<float, _z>(FXMVECTOR xmv) { return _DXMEXT XMVectorGetZ(xmv); }
			template <> inline float XM_CALLCONV get<float, _w>(FXMVECTOR xmv) { return _DXMEXT XMVectorGetW(xmv); }

			template <> inline uint XM_CALLCONV	 get<uint, _x>(FXMVECTOR xmv) { return _DXMEXT XMVectorGetIntX(xmv); }
			template <> inline uint XM_CALLCONV	 get<uint, _y>(FXMVECTOR xmv) { return _DXMEXT XMVectorGetIntY(xmv); }
			template <> inline uint XM_CALLCONV	 get<uint, _z>(FXMVECTOR xmv) { return _DXMEXT XMVectorGetIntZ(xmv); }
			template <> inline uint XM_CALLCONV	 get<uint, _w>(FXMVECTOR xmv) { return _DXMEXT XMVectorGetIntW(xmv); }

			template <> inline void XM_CALLCONV  get_ptr<float, _x>(float* ptr, FXMVECTOR xmv) { return _DXMEXT XMVectorGetXPtr(ptr, xmv); }
			template <> inline void XM_CALLCONV  get_ptr<float, _y>(float* ptr, FXMVECTOR xmv) { return _DXMEXT XMVectorGetYPtr(ptr, xmv); }
			template <> inline void XM_CALLCONV  get_ptr<float, _z>(float* ptr, FXMVECTOR xmv) { return _DXMEXT XMVectorGetZPtr(ptr, xmv); }
			template <> inline void XM_CALLCONV  get_ptr<float, _w>(float* ptr, FXMVECTOR xmv) { return _DXMEXT XMVectorGetWPtr(ptr, xmv); }

			template <> inline void XM_CALLCONV	 get_ptr<uint, _x>(uint* ptr, FXMVECTOR xmv) { return _DXMEXT XMVectorGetIntXPtr(ptr, xmv); }
			template <> inline void XM_CALLCONV	 get_ptr<uint, _y>(uint* ptr, FXMVECTOR xmv) { return _DXMEXT XMVectorGetIntYPtr(ptr, xmv); }
			template <> inline void XM_CALLCONV	 get_ptr<uint, _z>(uint* ptr, FXMVECTOR xmv) { return _DXMEXT XMVectorGetIntZPtr(ptr, xmv); }
			template <> inline void XM_CALLCONV	 get_ptr<uint, _w>(uint* ptr, FXMVECTOR xmv) { return _DXMEXT XMVectorGetIntWPtr(ptr, xmv); }

			template <>	inline FXMVECTOR XM_CALLCONV set<float, _x>(FXMVECTOR xmv, float val) { return _DXMEXT XMVectorSetX(xmv, val); }
			template <>	inline FXMVECTOR XM_CALLCONV set<float, _y>(FXMVECTOR xmv, float val) { return _DXMEXT XMVectorSetY(xmv, val); }
			template <>	inline FXMVECTOR XM_CALLCONV set<float, _z>(FXMVECTOR xmv, float val) { return _DXMEXT XMVectorSetZ(xmv, val); }
			template <>	inline FXMVECTOR XM_CALLCONV set<float, _w>(FXMVECTOR xmv, float val) { return _DXMEXT XMVectorSetW(xmv, val); }

			template <>	inline FXMVECTOR XM_CALLCONV set<uint, _x>(FXMVECTOR xmv, uint val) { return _DXMEXT XMVectorSetIntX(xmv, val); }
			template <>	inline FXMVECTOR XM_CALLCONV set<uint, _y>(FXMVECTOR xmv, uint val) { return _DXMEXT XMVectorSetIntY(xmv, val); }
			template <>	inline FXMVECTOR XM_CALLCONV set<uint, _z>(FXMVECTOR xmv, uint val) { return _DXMEXT XMVectorSetIntZ(xmv, val); }
			template <>	inline FXMVECTOR XM_CALLCONV set<uint, _w>(FXMVECTOR xmv, uint val) { return _DXMEXT XMVectorSetIntW(xmv, val); }

			template <>	inline FXMVECTOR XM_CALLCONV set_ptr<float, _x>(FXMVECTOR xmv, float* ptr) { return _DXMEXT XMVectorSetXPtr(xmv, ptr); }
			template <>	inline FXMVECTOR XM_CALLCONV set_ptr<float, _y>(FXMVECTOR xmv, float* ptr) { return _DXMEXT XMVectorSetYPtr(xmv, ptr); }
			template <>	inline FXMVECTOR XM_CALLCONV set_ptr<float, _z>(FXMVECTOR xmv, float* ptr) { return _DXMEXT XMVectorSetZPtr(xmv, ptr); }
			template <>	inline FXMVECTOR XM_CALLCONV set_ptr<float, _w>(FXMVECTOR xmv, float* ptr) { return _DXMEXT XMVectorSetWPtr(xmv, ptr); }

			template <>	inline FXMVECTOR XM_CALLCONV set_ptr<uint, _x>(FXMVECTOR xmv, uint* ptr) { return _DXMEXT XMVectorSetIntXPtr(xmv, ptr); }
			template <>	inline FXMVECTOR XM_CALLCONV set_ptr<uint, _y>(FXMVECTOR xmv, uint* ptr) { return _DXMEXT XMVectorSetIntYPtr(xmv, ptr); }
			template <>	inline FXMVECTOR XM_CALLCONV set_ptr<uint, _z>(FXMVECTOR xmv, uint* ptr) { return _DXMEXT XMVectorSetIntZPtr(xmv, ptr); }
			template <>	inline FXMVECTOR XM_CALLCONV set_ptr<uint, _w>(FXMVECTOR xmv, uint* ptr) { return _DXMEXT XMVectorSetIntWPtr(xmv, ptr); }

			template <bool aligned>
			struct storage_helper <float, aligned, false, 1> {
				static XMVECTOR XM_CALLCONV load(const float* pSource) { return XMVectorReplicatePtr(pSource); }
				static void XM_CALLCONV store(float* pDst, FXMVECTOR xmv) { XMStoreFloat(pDst, xmv); }
			};

			template <>
			struct storage_helper <float, false, 2> {
				static XMVECTOR XM_CALLCONV load(const float* pSource) { return XMLoadFloat2(reinterpret_cast<const XMFLOAT2*>(pSource)); }
				static void XM_CALLCONV store(float* pDst, FXMVECTOR xmv) { XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(pDst), xmv); }
			};

			template <>
			struct storage_helper <float, false, 3> {
				static XMVECTOR XM_CALLCONV load(const float* pSource) { return XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(pSource)); }
				static void XM_CALLCONV store(float* pDst, FXMVECTOR xmv) { XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(pDst), xmv); }
			};

			template <>
			struct storage_helper <float, false, 4> {
				static XMVECTOR XM_CALLCONV load(const float* pSource) { return XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(pSource)); }
				static void XM_CALLCONV store(float* pDst, FXMVECTOR xmv) { XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(pDst), xmv); }
			};

			template <>
			struct storage_helper <float, true, 2> {
				static XMVECTOR XM_CALLCONV load(const float* pSource) { return XMLoadFloat2A(reinterpret_cast<const XMFLOAT2A*>(pSource)); }
				static void XM_CALLCONV store(float* pDst, FXMVECTOR xmv) { XMStoreFloat2A(reinterpret_cast<XMFLOAT2A*>(pDst), xmv); }
			};

			template <>
			struct storage_helper <float, true, 3> {
				static XMVECTOR XM_CALLCONV load(const float* pSource) { return XMLoadFloat3A(reinterpret_cast<const XMFLOAT3A*>(pSource)); }
				static void XM_CALLCONV store(float* pDst, FXMVECTOR xmv) { XMStoreFloat3A(reinterpret_cast<XMFLOAT3A*>(pDst), xmv); }
			};

			template <>
			struct storage_helper <float, true, 4> {
				static XMVECTOR XM_CALLCONV load(const float* pSource) { return XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(pSource)); }
				static void XM_CALLCONV store(float* pDst, FXMVECTOR xmv) { XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(pDst), xmv); }
			};

			template <bool aligned>
			struct storage_helper <uint, aligned, false, 1> {
				static XMVECTOR XM_CALLCONV load(const uint* pSource) { return XMVectorReplicateIntPtr(pSource); }
				static void XM_CALLCONV store(uint* pDst, FXMVECTOR xmv) { XMStoreInt(pDst, xmv); }
			};

			template <>
			struct storage_helper <uint, false, 2> {
				static XMVECTOR XM_CALLCONV load(const uint* pSource) { return XMLoadInt2(pSource); }
				static void XM_CALLCONV store(uint* pDst, FXMVECTOR xmv) { XMStoreInt2(pDst, xmv); }
			};

			template <>
			struct storage_helper <uint, false, 3> {
				static XMVECTOR XM_CALLCONV load(const uint* pSource) { return XMLoadInt3(pSource); }
				static void XM_CALLCONV store(uint* pDst, FXMVECTOR xmv) { XMStoreInt3(pDst, xmv); }
			};

			template <>
			struct storage_helper <uint, false, 4> {
				static XMVECTOR XM_CALLCONV load(const uint* pSource) { XMLoadInt4(pSource); }
				static void XM_CALLCONV store(uint* pDst, FXMVECTOR xmv) { XMStoreInt4(pDst, xmv); }
			};
			template <>
			struct storage_helper <uint, true, 2> {
				static XMVECTOR XM_CALLCONV load(const uint* pSource) {XMLoadInt2A(pSource); }
				static void XM_CALLCONV store(uint* pDst, FXMVECTOR xmv) { XMStoreInt2A(pDst, xmv); }
			};

			template <>
			struct storage_helper <uint, true, 3> {
				static XMVECTOR XM_CALLCONV load(const uint* pSource) { return XMLoadInt3A(pSource); }
				static void XM_CALLCONV store(uint* pDst, FXMVECTOR xmv) { XMStoreInt3A(pDst, xmv); }
			};

			template <>
			struct storage_helper <uint, true, 4> {
				static auto XM_CALLCONV load(const uint* pSource) { return XMLoadInt4A(pSource); }
				static void XM_CALLCONV store(uint* pDst, FXMVECTOR xmv) { XMStoreInt4A(pDst, xmv); }
			};

		}

		using xmfloat	 = xmscalar<float>;
		using xmvector1f = xmvector<float, 1>;
		using xmvector2f = xmvector<float, 2>;
		using xmvector3f = xmvector<float, 3>;
		using xmvector4f = xmvector<float, 4>;

		using xmuint	 = xmscalar<uint>;
		using xmvector1i = xmvector<uint, 1>;
		using xmvector2i = xmvector<uint, 2>;
		using xmvector3i = xmvector<uint, 3>;
		using xmvector4i = xmvector<uint, 4>;
	}

	// Extend methods for XMLoad / XMStore
	inline XMVECTOR XM_CALLCONV XMLoad(const hlsl::xmvector1f& src){ return src.v; }
	inline XMVECTOR XM_CALLCONV XMLoadA(const hlsl::xmvector1f& src){ return src.v; }
	inline XMVECTOR XM_CALLCONV XMLoad(const hlsl::xmvector2f& src){ return src.v; }
	inline XMVECTOR XM_CALLCONV XMLoadA(const hlsl::xmvector2f& src){ return src.v; }
	inline XMVECTOR XM_CALLCONV XMLoad(const hlsl::xmvector3f& src){ return src.v; }
	inline XMVECTOR XM_CALLCONV XMLoadA(const hlsl::xmvector3f& src){ return src.v; }
	inline XMVECTOR XM_CALLCONV XMLoad(const hlsl::xmvector4f& src){ return src.v; }
	inline XMVECTOR XM_CALLCONV XMLoadA(const hlsl::xmvector4f& src){ return src.v; }

	inline XMVECTOR XM_CALLCONV XMLoad(const hlsl::xmvector1i& src){ return src.v; }
	inline XMVECTOR XM_CALLCONV XMLoadA(const hlsl::xmvector1i& src){ return src.v; }
	inline XMVECTOR XM_CALLCONV XMLoad(const hlsl::xmvector2i& src){ return src.v; }
	inline XMVECTOR XM_CALLCONV XMLoadA(const hlsl::xmvector2i& src){ return src.v; }
	inline XMVECTOR XM_CALLCONV XMLoad(const hlsl::xmvector3i& src){ return src.v; }
	inline XMVECTOR XM_CALLCONV XMLoadA(const hlsl::xmvector3i& src){ return src.v; }
	inline XMVECTOR XM_CALLCONV XMLoad(const hlsl::xmvector4i& src){ return src.v; }
	inline XMVECTOR XM_CALLCONV XMLoadA(const hlsl::xmvector4i& src){ return src.v; }

	inline void XM_CALLCONV XMStore(hlsl::xmvector1f& dest, FXMVECTOR v) { dest.v = v; }
	inline void XM_CALLCONV XMStoreA(hlsl::xmvector1f& dest, FXMVECTOR v) { dest.v = v; }
	inline void XM_CALLCONV XMStore(hlsl::xmvector2f& dest, FXMVECTOR v) { dest.v = v; }
	inline void XM_CALLCONV XMStoreA(hlsl::xmvector2f& dest, FXMVECTOR v) { dest.v = v; }
	inline void XM_CALLCONV XMStore(hlsl::xmvector3f& dest, FXMVECTOR v) { dest.v = v; }
	inline void XM_CALLCONV XMStoreA(hlsl::xmvector3f& dest, FXMVECTOR v) { dest.v = v; }
	inline void XM_CALLCONV XMStore(hlsl::xmvector4f& dest, FXMVECTOR v) { dest.v = v; }
	inline void XM_CALLCONV XMStoreA(hlsl::xmvector4f& dest, FXMVECTOR v) { dest.v = v; }

	inline void XM_CALLCONV XMStore(hlsl::xmvector1i& dest, FXMVECTOR v) { dest.v = v; }
	inline void XM_CALLCONV XMStoreA(hlsl::xmvector1i& dest, FXMVECTOR v) { dest.v = v; }
	inline void XM_CALLCONV XMStore(hlsl::xmvector2i& dest, FXMVECTOR v) { dest.v = v; }
	inline void XM_CALLCONV XMStoreA(hlsl::xmvector2i& dest, FXMVECTOR v) { dest.v = v; }
	inline void XM_CALLCONV XMStore(hlsl::xmvector3i& dest, FXMVECTOR v) { dest.v = v; }
	inline void XM_CALLCONV XMStoreA(hlsl::xmvector3i& dest, FXMVECTOR v) { dest.v = v; }
	inline void XM_CALLCONV XMStore(hlsl::xmvector4i& dest, FXMVECTOR v) { dest.v = v; }
	inline void XM_CALLCONV XMStoreA(hlsl::xmvector4i& dest, FXMVECTOR v) { dest.v = v; }

}

#endif