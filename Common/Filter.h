#pragma once

#include <cmath>
#include <algorithm>

namespace Causality
{

	//////////////////////////////////////////////////////////////////////
	//
	// Filter 
	// 
	//////////////////////////////////////////////////////////////////////
	// Template class for filtering
	//  
	template <class TValue, class TScaler = double>
	class Filter
	{
	public:
		//////////////////////////////////////////////////////////////////////
		// UpdateFrequency (Hz) 
		Filter(TScaler* pUpdateFrequency)
		{
			m_pUpdateFrequency = pUpdateFrequency;
			m_FirstTime = true;
		}

		Filter() = default;

		virtual const TValue& Apply(const TValue &value) = 0;

		const TValue& Delta() const { return m_Delta; }
		const TValue& Value() const { return m_PrevValue; }

		virtual ~Filter(void) { }

		// Reset the filter to it's initial state
		void Reset() { m_FirstTime = true; };
		void SetUpdateFrequency(TScaler* updateFrequency) { m_pUpdateFrequency = updateFrequency; };
		TScaler GetUpdateFrequency() const { return *m_pUpdateFrequency; }


	protected:

		TScaler* m_pUpdateFrequency;

		TValue m_PrevValue;
		TValue m_Delta;
		bool m_FirstTime;

	};

	template <class _Ty, class _TScaler>
	struct default_lerp
	{
		_Ty operator()(const _Ty& lhs, const _Ty&rhs, _TScaler t)
		{
			return lhs + (rhs - lhs) * t;
		}
	};

	template <class _TVector, class _TScaler>
	struct conversion_to_scalar
		: public std::unary_function<_TVector, _TScaler>
	{
		_TScaler operator()(const _TVector& lhs)
		{
			return (_TScaler)lhs;
		}
	};


#ifdef DIRECTX_MATH_VERSION
	struct QuaternionWrapper : public DirectX::Quaternion
	{
		using DirectX::Quaternion::Quaternion;
		using DirectX::Quaternion::operator DirectX::XMVECTOR;

		QuaternionWrapper& operator +=(const QuaternionWrapper& rhs)
		{
			using namespace DirectX;
			XMVECTOR ql = XMLoadFloat4(this);
			XMVECTOR qr = XMLoadFloat4(&rhs);
			ql = XMQuaternionMultiply(ql, qr);
			XMStoreFloat4(this, ql);
		}

		QuaternionWrapper& operator -=(const QuaternionWrapper& rhs)
		{
			using namespace DirectX;
			XMVECTOR ql = XMLoadFloat4(this);
			XMVECTOR qr = XMLoadFloat4(&rhs);
			qr = XMQuaternionInverse(rhs);
			ql = XMQuaternionMultiply(ql, qr);
			XMStoreFloat4(this, ql);
		}

		QuaternionWrapper& operator *=(float rhs)
		{
			using namespace DirectX;

			float angle;
			XMVECTOR q = XMLoadFloat4(this);
			XMVECTOR axis;
			XMQuaternionToAxisAngle(&axis, &angle, q);
			angle *= rhs;
			q = XMQuaternionRotationAxis(axis, angle);
			XMStoreFloat4(this, q);
		}

		static inline QuaternionWrapper SLerp(const QuaternionWrapper& lhs, const QuaternionWrapper&rhs, float t)
		{
			using namespace DirectX;
			QuaternionWrapper result;
			XMVECTOR ql = XMLoadFloat4(&lhs);
			XMVECTOR qr = XMLoadFloat4(&rhs);
			ql = XMQuaternionSlerp(ql, qr, t);
			XMStoreFloat4(&result, ql);
			return result;
		}

		float RotationAngle() const
		{
			using namespace DirectX;
			float angle;
			XMVECTOR q = XMLoadFloat4(this);
			XMVECTOR axis;
			XMQuaternionToAxisAngle(&axis, &angle, q);
			return angle;
		}
	};

	template <>
	struct default_lerp<QuaternionWrapper, float>
	{
		using _Ty = QuaternionWrapper;
		using _TScaler = float;
		_Ty operator()(const _Ty& lhs, const _Ty&rhs, _TScaler t)
		{
			return QuaternionWrapper::SLerp(lhs, rhs, t);
		}
	};

	template <>
	struct conversion_to_scalar<DirectX::XMFLOAT2, float>
		: public std::unary_function<DirectX::XMFLOAT2, float>
	{
		float operator()(const DirectX::XMFLOAT2& lhs)
	{
		DirectX::XMVECTOR v = DirectX::XMLoadFloat2(&lhs);
		v = DirectX::XMVector2Length(v);
		return DirectX::XMVectorGetX(v);
	}
	};
	template <>
	struct conversion_to_scalar<DirectX::XMFLOAT2A, float>
		: public std::unary_function<DirectX::XMFLOAT2A, float>
	{
		float operator()(const DirectX::XMFLOAT2A& lhs)
	{
		DirectX::XMVECTOR v = DirectX::XMLoadFloat2A(&lhs);
		v = DirectX::XMVector2Length(v);
		return DirectX::XMVectorGetX(v);
	}
	};
	template <>
	struct conversion_to_scalar<DirectX::XMFLOAT3, float>
		: public std::unary_function<DirectX::XMFLOAT3, float>
	{
		float operator()(const DirectX::XMFLOAT3& lhs)
	{
		DirectX::XMVECTOR v = DirectX::XMLoadFloat3(&lhs);
		v = DirectX::XMVector3Length(v);
		return DirectX::XMVectorGetX(v);
	}
	};
	template <>
	struct conversion_to_scalar<DirectX::XMFLOAT3A, float>
		: public std::unary_function<DirectX::XMFLOAT3A, float>
	{
		float operator()(const DirectX::XMFLOAT3A& lhs)
	{
		DirectX::XMVECTOR v = DirectX::XMLoadFloat3A(&lhs);
		v = DirectX::XMVector3Length(v);
		return DirectX::XMVectorGetX(v);
	}
	};
	template <>
	struct conversion_to_scalar<DirectX::XMFLOAT4, float>
		: public std::unary_function<DirectX::XMFLOAT4, float>
	{
		float operator()(const DirectX::XMFLOAT4& lhs)
	{
		DirectX::XMVECTOR v = DirectX::XMLoadFloat4(&lhs);
		v = DirectX::XMVector4Length(v);
		return DirectX::XMVectorGetX(v);
	}
	};
	template <>
	struct conversion_to_scalar<DirectX::XMFLOAT4A, float>
		: public std::unary_function<DirectX::XMFLOAT4A, float>
	{
		float operator()(const DirectX::XMFLOAT4A& lhs)
	{
		DirectX::XMVECTOR v = DirectX::XMLoadFloat4A(&lhs);
		v = DirectX::XMVector4Length(v);
		return DirectX::XMVectorGetX(v);
	}
	};
#endif


	//////////////////////////////////////////////////////////////////////
	//
	// Low Pass Filter 
	// 
	//////////////////////////////////////////////////////////////////////
	// set the following before using: 
	//		mCutoffFrequency
	//

	template <class TValue, class TScaler = double>
	class LowPassFilter
		: public Filter<TValue, TScaler>
	{
	public:
		LowPassFilter(TScaler* pUpdateFrequency, TScaler cutOffFrequency = 0)
			: Filter<TValue, TScaler>(pUpdateFrequency), m_CutoffFrequency(cutOffFrequency) {};

		LowPassFilter() = default;

		virtual const TValue& Apply(const TValue &value) override
		{
			/*

			Let's say Pnf the filtered position, Pn the non filtered position and Pn-1f the previous filtered position,
			Te the sampling period (in second) and tau a time constant calculated from the cut-off frequency fc.

			tau = 1 / (2 * pi * fc)
			Pnf = ( Pn + tau/Te * Pn-1f ) * 1/(1+ tau/Te)

			Attention: tau >= 10 * Te
			*/

			if (m_FirstTime)
			{
				m_PrevValue = value;
				m_FirstTime = false;
			}

			const TScaler one = TScaler(1), zero = TScaler(0), twopi = TScaler(2 * 3.14159265);

			TScaler updateFrequency = *m_pUpdateFrequency;

			TScaler Te(one / updateFrequency);		// the sampling period (in seconds)
			TScaler Tau(one / (twopi * m_CutoffFrequency));	// a time constant calculated from the cut-off frequency

			auto t = one / (one + (Tau / Te));

			m_Delta = t * (value - m_PrevValue);

			m_PrevValue += m_Delta;

			return m_PrevValue;
		}

		void SetCutoffFrequency(TScaler f) { m_CutoffFrequency = f; };

		TScaler GetCutoffFrequency(void) { return m_CutoffFrequency; };

	protected:
		TScaler m_CutoffFrequency;

	};


#ifdef DX_MATH_EXT_H
	template <>
	struct conversion_to_scalar<DirectX::Vector2, float>
		: public std::unary_function<DirectX::Vector2, float>
	{
		float operator()(const DirectX::Vector2& lhs)
		{
			DirectX::XMVECTOR v = DirectX::XMLoadFloat2(&lhs);
			v = DirectX::XMVector2Length(v);
			return DirectX::XMVectorGetX(v);
		}
	};	
	template <>
	struct conversion_to_scalar<DirectX::Vector3, float>
		: public std::unary_function<DirectX::Vector3, float>
	{
		float operator()(const DirectX::Vector3& lhs)
		{
			DirectX::XMVECTOR v = DirectX::XMLoadFloat3(&lhs);
			v = DirectX::XMVector3Length(v);
			return DirectX::XMVectorGetX(v);
		}
	};
	template <>
	struct conversion_to_scalar<DirectX::Vector4, float>
		: public std::unary_function<DirectX::Vector4, float>
	{
		float operator()(const DirectX::Vector4& lhs)
		{
			DirectX::XMVECTOR v = DirectX::XMLoadFloat4(&lhs);
			v = DirectX::XMVector4Length(v);
			return DirectX::XMVectorGetX(v);
		}
	};

	template <>
	struct conversion_to_scalar<QuaternionWrapper, float>
		: public std::unary_function<QuaternionWrapper, float>
	{
		float operator()(const QuaternionWrapper& lhs)
		{
			return lhs.RotationAngle();
		}
	};
#endif

	//////////////////////////////////////////////////////////////////////
	//
	// Low Pass Dynamic (or Adjustable) Filter 
	// 
	//////////////////////////////////////////////////////////////////////
	// adjusts the cutoff filter depending on the velocity
	// set the following before using: 
	//		mUpdateFrequency (Hz) 
	//		mCutoffFrequency the lower cuttoff frequency
	//		mCutoffFrequencyHigh
	//		mVelocityLow (mm/s) the speed at which mCutoffFrequencyLow is reached 
	//		mVelocityHigh (mm/s) the speed at which mCutoffFrequencyHigh is reached
	template <class TValue, class TScaler = double, class TNormalOperator = conversion_to_scalar<TValue, TScaler>, class TLerpOperator = default_lerp<TValue,TScaler>>
	class LowPassDynamicFilter : public LowPassFilter<TValue, TScaler>
	{
	protected:
		LowPassFilter<TValue>	m_VelocityFilter;
		TValue					m_LastPositionForVelocity;
		TScaler					m_CutoffFrequencyHigh, m_VelocityLow, m_VelocityHigh;
		TNormalOperator			mf_GetNormal;
		TLerpOperator			mf_Lerp;

	public:
		LowPassDynamicFilter(TScaler* updateFrequency) : LowPassFilter<TValue, TScaler>(updateFrequency), m_VelocityFilter(updateFrequency) {  };
		LowPassDynamicFilter() = default;
		virtual const TValue& Apply(const TValue &value)
		{
			using namespace std;

			// special case if first time being used
			if (m_FirstTime)
			{
				m_PrevValue = value;
				m_LastPositionForVelocity = value;
				m_FirstTime = false;
			}


			TScaler updateFrequency = *m_pUpdateFrequency;

			// first get an estimate of velocity (with filter)
			TValue mPositionForVelocity = m_VelocityFilter.Apply(value);
			TValue vel = (mPositionForVelocity - m_LastPositionForVelocity) * updateFrequency;
			m_LastPositionForVelocity = mPositionForVelocity;

			// constants
			const TScaler one = TScaler(1), zero = TScaler(0), twopi = TScaler(2 * 3.14159265);
			// interpolate between frequencies depending on velocity

			TScaler t = (mf_GetNormal(vel) - m_VelocityLow) / (m_VelocityHigh - m_VelocityLow);
			t = min(max(t, zero), one);
			TScaler cutoff((m_CutoffFrequencyHigh * t) + (m_CutoffFrequency * (one - t)));
			TScaler Te(one / updateFrequency);		// the sampling period (in seconds)
			TScaler Tau(one / (twopi * cutoff));	// a time constant calculated from the cut-off frequency

			t = one / (one + (Tau / Te));


			m_Delta = mf_Lerp(m_PrevValue, value, t);

			m_Delta -= m_PrevValue;

			m_PrevValue += m_Delta;

			return m_PrevValue;
		}

		// Parameter Properties
		TScaler GetCutoffFrequencyHigh() const { return m_CutoffFrequencyHigh; }
		TScaler GetCutoffFrequencyLow() const { return m_CutoffFrequency; }
		TScaler GetVelocityLow() const { return *m_VelocityLow; }
		TScaler GetVelocityHigh() const { return *m_VelocityHigh; }

		void SetCutoffFrequencyLow(TScaler f) { m_CutoffFrequency = f; SetCutoffFrequencyVelocity(); };
		void SetCutoffFrequencyHigh(TScaler f) { m_CutoffFrequencyHigh = f; SetCutoffFrequencyVelocity(); };
		void SetVelocityLow(TScaler f) { m_VelocityLow = f; };
		void SetVelocityHigh(TScaler f) { m_VelocityHigh = f; };

	protected:

		// cutoff freq for velocity
		// equal to mCutoffFrequency  + 0.75 * (mCutoffFrequencyHigh - mCutoffFrequency)
		void SetCutoffFrequencyVelocity()
		{
			m_VelocityFilter.SetCutoffFrequency((TScaler) (m_CutoffFrequency + 0.75 * (m_CutoffFrequencyHigh - m_CutoffFrequency)));
		}
	};

	//
	//////////////////////////////////////////////////////////////////////
	// 
}