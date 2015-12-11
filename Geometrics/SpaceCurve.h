#pragma once

#include <vector>
#include <memory>
#include "DirectXMathExtend.h"

namespace Geometrics
{
	using DirectX::Vector3;
	using DirectX::XMVECTOR;
	using DirectX::XMFLOAT4A;
	using DirectX::FXMVECTOR;
	// Class to represent a spatial curve with anchor points
	// Provide method for linear sampling from it
	class SpaceCurve
	{
	public:

		SpaceCurve();
		~SpaceCurve();
		explicit SpaceCurve(const std::vector<Vector3>& Trajectory);

		std::size_t size() const { return m_anchors.size(); }
		bool empty() const { return m_anchors.empty(); }
		float length() const
		{
			if (m_anchors.empty()) return 0.0f;
			return m_anchors.back().w;
		}
		XMFLOAT4A* data() { return m_anchors.data(); }
		const XMFLOAT4A* data() const { return m_anchors.data(); }
		void clear() { m_anchors.clear(); }
		void push_back(const Vector3& p);
		void XM_CALLCONV push_back(FXMVECTOR p);
		XMVECTOR back() const { return XMLoadFloat4A(&m_anchors.back()); }
		//		void push_front(const Vector3& p);

		// Retrive the point at parameter position 't' belongs to [0,1]
		// This O(LogN) level operation
		// perform a binary search in anchors 
		// returns a 4D vector where xyz represent the Position 
		// w is the length from start to point;
		XMVECTOR extract(float t) const;
		XMVECTOR position(float t) const { return extract(t); }
		XMVECTOR position(int idx) const {
			using namespace DirectX;
			XMVECTOR v = XMLoadFloat4A(&m_anchors[idx]); 
			v = XMVectorSelect(v, g_XMIdentityR3.v, g_XMIdentityR3.v);
			return v;
		}
		// eval tangent at parameter position t
		XMVECTOR tangent(float t) const;
		// eval tangent at anchor position idx
		XMVECTOR tangent(int idx) const;

		inline XMVECTOR operator[](int idx) const { return XMLoadFloat4A(&m_anchors[idx]); }
		inline XMVECTOR operator()(float t) const { return extract(t); }

		std::vector<Vector3> FixIntervalSampling(float Interval) const;
		std::vector<Vector3> FixCountSampling(unsigned int SampleSegmentCount, bool Smooth = true) const;
		std::vector<Vector3> FixCountSampling2(unsigned int SampleSegmentCount) const;

		void resample(size_t anchorCount, bool smooth = true);

		static void LaplaianSmoothing(std::vector<Vector3>& Curve, unsigned IterationTimes = 2);

		void updateLength();

		const XMFLOAT4A& anchor(int idx) const
		{
			return m_anchors[idx];
		}

		XMFLOAT4A& anchor(int idx)
		{
			return m_anchors[idx];
		}

		const std::vector<XMFLOAT4A, DirectX::AlignedAllocator<XMFLOAT4A>>& anchors() const
		{
			return m_anchors;
		}

	private:
		// The data we stored is actually aligned on 16-byte boundary , so , use it as a XMFLOAT4A
		std::vector<XMFLOAT4A, DirectX::AlignedAllocator<XMFLOAT4A>> m_anchors;
	};

}