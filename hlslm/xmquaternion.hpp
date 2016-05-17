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
			using scalar_type = _T;
			using vector3_type = xmvector<_T, 3>;

			static_assert(std::is_same<_T, float>::value, "xmquaternion only supports float right now");

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

			inline this_type operator~() const {return this->conjugated(); }
			
			// reinterpret this quaternion as a 4-vector
			inline const base_type& vector() const { return reinterpret_cast<const base_type&>(*this); }
			// reinterpret this quaternion as a 4-vector
			inline base_type& vector() { return reinterpret_cast<base_type&>(*this); }

			inline void XM_CALLCONV conjugate(){
				this->v = XMQuaternionConjugate(this->v);
			}
			
			inline this_type XM_CALLCONV conjugated() const
			{
				this_type ret;
				ret.v = XMQuaternionConjugate(this->v);
				return ret;
			}

			inline void XM_CALLCONV inverse(){
				this->v = XMQuaternionInverse(this->v);
			}
			
			inline this_type XM_CALLCONV inversed() const
			{
				this_type ret;
				ret.v = XMQuaternionInverse(this->v);
				return ret;
			}

			static inline this_type rotation_x(float angle);
			static inline this_type rotation_y(float angle);
			static inline this_type rotation_z(float angle);

			inline const vector3_type& axis() const{
				return this->as<3>();
			}

			xmscalar<scalar_type> angle() const {
				return 2.0f * acos(base_type(detail::splat<3>(this->v)));
			}

			// Decomposite this quaternion into A-B-C Euler Angles
			template <unsigned _X, unsigned _Y, unsigned _Z>
			inline vector3_type euler_abc() const
			{
				vector3_type ret;
				ret.v = DirectX::XMQuaternionEulerAngleABC<_X,_Y,_Z>(this->v);
				return ret;
			}

			// Decomposite this quaternion into classical euler angles: the yaw-pitch-roll rotation sequence
			inline vector3_type yaw_pitch_roll() const
			{
				vector3_type ret;
				ret.v = DirectX::XMQuaternionEulerAngleABC<_X,_Y,_Z>(this->v);
				return ret;
			}
			
			// Convert this rotation quaternion into matrix form
			inline xmmatrix<scalar_type,3,3> matrix() const
			{
				return xmmatrix<scalar_type, 3, 3>();
			}
			
		};

		using xmquaternionf = xmquaternion<float>;
	}
}