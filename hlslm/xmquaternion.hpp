#pragma once

#include "traits.hpp"
#include "xmvector.hpp"
#include <DirectXMathExtend.h>

#ifndef _HLSLM_QUATERNION_COLUMN_MAJOR
#define _HLSLM_QUATERNION_COLUMN_MAJOR 0
#endif

namespace DirectX
{
	namespace hlsl
	{
		template <typename _T>
		struct xmquaternion : public xmvector<_T,4>
		{
		public:
			using base_type = xmvector<_T, 4>;
			using this_type = xmquaternion<_T>;

			static_assert(std::is_same<_T, float>, "xmquaternion only supports float right now");

			inline static XMVECTOR XM_CALLCONV qmul(FXMVECTOR lhs, FXMVECTOR rhs)
			{
#if !_HLSLM_QUATERNION_COLUMN_MAJOR
				// DirectX Math uses row major vector, and the quaternion rotation is also applied from left to right
				return XMQuaternionMultiply(lhs.v, rhs.v);
#else
				// Flip the multilication order so that it fit column major vector, like the old book
				return XMQuaternionMultiply(rhs.v, lhs.v);
#endif
			}

			inline this_type& XM_CALLCONV operator *= (const this_type rhs)
			{
				this->v = qmul(this->v, rhs.v);
			}

			inline this_type XM_CALLCONV operator * (const this_type rhs) const
			{
				this_type ret; 
				ret.v = qmul(this->v, rhs.v);
				return ret;
			}
		};

		using xmquaternionf = xmquaternion<float>;
	}
}