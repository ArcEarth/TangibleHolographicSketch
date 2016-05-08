#pragma once
#include <DirectXMathExtend.h>
namespace DirectX
{
	namespace hlsl
	{
		namespace vector_math
		{
			template <typename _TScalar, size_t _Size>
			struct add;
			template <typename _TScalar, size_t _Size>
			struct subtract;
			template <typename _TScalar, size_t _Size>
			struct multiply;
			template <typename _TScalar, size_t _Size>
			struct divide;
			template <typename _TScalar, size_t _Size>
			struct madd;
			template <typename _TScalar, size_t _Size>
			struct negate;

			template <size_t _Size>
			struct negate<float,_Size>
			{
				static inline XMVECTOR XM_CALLCONV invoke(FXMVECTOR lhs)
				{ return XMVectorNegate(lhs); }
				inline auto XM_CALLCONV operator()(FXMVECTOR v0) { return invoke(v0); };
			};

			template <size_t _Size>
			struct negate<uint, _Size>
			{
				static inline XMVECTOR XM_CALLCONV invoke(FXMVECTOR lhs)
				{ return XMVectorXorInt(lhs, DirectX::g_XMNegativeZero.v); } // This toggles the sign bit of the integer value
				inline auto XM_CALLCONV operator()(FXMVECTOR v0) { return invoke(v0); };
			};

			template <size_t _Size>
			struct negate<int, _Size>
			{
				static inline XMVECTOR XM_CALLCONV invoke(FXMVECTOR lhs)
				{
					return XMVectorXorInt(lhs, DirectX::g_XMNegativeZero.v);
				} // This toggles the sign bit of the integer value
				inline auto XM_CALLCONV operator()(FXMVECTOR v0) { return invoke(v0); };
			};

			template <typename _TScalar, size_t _Size>
			struct and
			{
				static inline XMVECTOR XM_CALLCONV invoke(FXMVECTOR lhs, FXMVECTOR rhs)
				{
					return XMVectorAndInt(lhs, rhs);
				}
				inline auto XM_CALLCONV operator()(FXMVECTOR v0, FXMVECTOR v1) { return invoke(v0, v1); };
				static inline XMVECTOR identity() { return g_XMNegOneMask.v; }
			};

			template <typename _TScalar, size_t _Size>
			struct or
			{
				static inline XMVECTOR XM_CALLCONV invoke(FXMVECTOR lhs, FXMVECTOR rhs)
				{
					return XMVectorOrInt(lhs, rhs);
				}
				inline auto XM_CALLCONV operator()(FXMVECTOR v0, FXMVECTOR v1) { return invoke(v0, v1); };
				static inline XMVECTOR identity() { return XMVectorZero(); }
			};

			template <typename _TScalar, size_t _Size>
			struct xor
			{
				static inline XMVECTOR XM_CALLCONV invoke(FXMVECTOR lhs, FXMVECTOR rhs)
				{
					return XMVectorXorInt(lhs, rhs);
				}
				inline auto XM_CALLCONV operator()(FXMVECTOR v0, FXMVECTOR v1) { return invoke(v0, v1); };
				static inline XMVECTOR identity() { return XMVectorZero(); }
			};


			template <size_t _Size>
			struct add<float, _Size>
			{
				static inline XMVECTOR XM_CALLCONV invoke(FXMVECTOR lhs, FXMVECTOR rhs)
				{ return XMVectorAdd(lhs, rhs); }
				inline auto XM_CALLCONV operator()(FXMVECTOR v0, FXMVECTOR v1) { return invoke(v0, v1); };
				static inline XMVECTOR identity() { return XMVectorZero(); }
			};

			template <size_t _Size>
			struct add<uint, _Size>
			{
				static inline XMVECTOR XM_CALLCONV invoke(FXMVECTOR lhs, FXMVECTOR rhs)
				{ return XMVectorAddInt(lhs, rhs); }
				inline auto XM_CALLCONV operator()(FXMVECTOR v0, FXMVECTOR v1) { return invoke(v0, v1); };
				static inline XMVECTOR identity() { return XMVectorZero(); }
			};

			template <size_t _Size>
			struct subtract<float, _Size>
			{
				static inline XMVECTOR XM_CALLCONV invoke(FXMVECTOR lhs, FXMVECTOR rhs)
				{
					return XMVectorSubtract(lhs, rhs);
				}
				inline auto XM_CALLCONV operator()(FXMVECTOR v0, FXMVECTOR v1) { return invoke(v0, v1); };
				static inline XMVECTOR identity() { return XMVectorZero(); }
			};

			template <size_t _Size>
			struct subtract<uint, _Size>
			{
				static inline XMVECTOR XM_CALLCONV invoke(FXMVECTOR lhs, FXMVECTOR rhs)
				{
					return XMVectorSubtractInt(lhs, rhs);
				}
				inline auto XM_CALLCONV operator()(FXMVECTOR v0, FXMVECTOR v1) { return invoke(v0, v1); };
				static inline XMVECTOR identity() { return XMVectorZero(); }
			};

			template <size_t _Size>
			struct multiply<float, _Size>
			{
				static inline XMVECTOR XM_CALLCONV invoke(FXMVECTOR lhs, FXMVECTOR rhs)
				{
					return XMVectorMultiply(lhs, rhs);
				}
				inline auto XM_CALLCONV operator()(FXMVECTOR v0, FXMVECTOR v1) { return invoke(v0, v1); };
				static inline XMVECTOR identity() { return XMVectorSplatOne(); }
			};

			template <size_t _Size>
			struct multiply<uint, _Size>
			{
				static inline XMVECTOR XM_CALLCONV invoke(FXMVECTOR lhs, FXMVECTOR rhs)
				{
					return XMVectorMultiplyInt(lhs, rhs);
				}
				inline auto XM_CALLCONV operator()(FXMVECTOR v0, FXMVECTOR v1) { return invoke(v0, v1); };
				static inline XMVECTOR identity() { return XMVectorSplatConstantInt(1); }
			};

			template <size_t _Size>
			struct divide<float, _Size>
			{
				static inline XMVECTOR XM_CALLCONV invoke(FXMVECTOR lhs, FXMVECTOR rhs)
				{
					return XMVectorDivide(lhs, rhs);
				}
				inline auto XM_CALLCONV operator()(FXMVECTOR v0, FXMVECTOR v1) { return invoke(v0, v1); };
				static inline XMVECTOR identity() { return XMVectorSplatOne(); }
			};

			template <size_t _Size>
			struct divide<uint, _Size>
			{
				[[deprecated("Integer Division vectors are extremely slow, considering code to CPU instead.")]]
				static inline XMVECTOR XM_CALLCONV invoke(FXMVECTOR lhs, FXMVECTOR rhs)
				{   // Convert to memory vectors
					XMVECTORU32 ls; XMStoreInt4A(ls.u, lhs);
					XMVECTORU32 rs; XMStoreInt4A(rs.u, rhs);
					ls.u[0] /= rs.u[0];	ls.u[1] /= rs.u[1]; ls.u[2] /= rs.u[2]; ls.u[3] /= rs.u[3];
					return XMLoadInt4A(ls.u);
				}

				inline auto XM_CALLCONV operator()(FXMVECTOR v0, FXMVECTOR v1) { return invoke(v0, v1); };
				static inline XMVECTOR identity() { return XMVectorSplatConstantInt(1); }
			};

			template <size_t _Size>
			struct madd<float, _Size>
			{
				static inline XMVECTOR XM_CALLCONV invoke(FXMVECTOR v1, FXMVECTOR v2, FXMVECTOR v3)
				{
					return _DXMEXT XMVectorMultiplyAdd(v1, v2, v3);
				}
				inline auto XM_CALLCONV operator()(FXMVECTOR v1, FXMVECTOR v2, FXMVECTOR v3) { return invoke(v1, v2, v3); };
				static inline XMVECTOR identity() { return g_XMOne.v; }
			};
		}
	}
}