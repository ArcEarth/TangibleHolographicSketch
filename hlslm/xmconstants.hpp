#pragma once
#include "xmvector.hpp"

namespace DirectX
{
	namespace hlsl
	{
		//namespace constants
		//{
			inline xmscalar<float> epsilon() { return xmscalar<float>(XMVectorSplatEpsilon()); }
			inline xmscalar<float> infinity() { return xmscalar<float>(XMVectorSplatInfinity()); }
			inline xmscalar<float> one() { return xmscalar<float>(XMVectorSplatOne()); }
			inline xmscalar<float> zero() { return xmscalar<float>(XMVectorZero()); }
			inline xmscalar<float> qnan() { return xmscalar<float>(XMVectorSplatQNaN()); }
			inline xmscalar<float> pi() { return xmscalar<float>(g_XMPi.v); }
			inline xmscalar<uint>  truei() { return xmscalar<uint>(XMVectorTrueInt()); }
			inline xmscalar<uint>  falsei() { return xmscalar<uint>(XMVectorFalseInt()); }

			template <uint i0, uint i1, uint i2 = 0, uint i3 = 0>
			static inline xmvector<uint,4> constant() {
				static const DirectX::XMVECTORU32 u = { i0,i1,i2,i3 };
				return u.v;
			}

			template <int i0, int i1, int i2 = 0, int i3 = 0>
			static inline xmvector<int, 4> constant() {
				static const DirectX::XMVECTORI32 u = { i0,i1,i2,i3 };
				return u.v;
			}

			namespace detail
			{
				inline constexpr bool is_power_of_2(uint n)
				{
					return n==0 ? true : n & 0x1 ? false : is_power_of_2(n >> 2);
				}

				inline constexpr int log_of_2(int n, int base = 0)
				{
					return (n == 1) ? base : log_of_2(n / 2, base + 1);
				}
			}

			template <int Numerator, uint Quotient>
			inline std::enable_if_t <(Numerator > 15) || (Numerator < -16) || !detail::is_power_of_2(Quotient), xmscalar<float>> rational()
			{
				return XMVectorReplicate((float)Numerator / (float)Quotient);
			}

			template <int Numerator, uint Quotient>
			inline std::enable_if_t<(Numerator <= 15) && (Numerator >= -16) && detail::is_power_of_2(Quotient), xmscalar<float>> rational()
			{
				return XMVectorSplatConstant(Numerator, detail::log_of_2(Quotient));
			}
		//}

		//using namespace constants;
	}
}