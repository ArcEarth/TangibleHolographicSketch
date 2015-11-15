#include <vector>
#include <memory>
#include "DirectXMathExtend.h"

namespace Geometrics
{
	using DirectX::Vector3;
	using DirectX::XMVECTOR;
	using DirectX::XMFLOAT4A;

	// Class to represent a spatial curve with anchor points
	// Provide method for linear sampling from it
	class SpaceCurve
	{
	public:

		SpaceCurve() {}
		~SpaceCurve() {}
		explicit SpaceCurve(const std::vector<Vector3>& Trajectory);

		std::size_t size() const { return Anchors.size(); }
		bool empty() const { return Anchors.empty(); }
		float length() const
		{
			if (Anchors.empty()) return 0.0f;
			return Anchors.back().w;
		}
		void clear() { Anchors.clear(); }
		void push_back(const Vector3& p);
		XMVECTOR back() const { return XMLoadFloat4A(&Anchors.back()); }
		//		void push_front(const Vector3& p);

		// Retrive the point at parameter position 't' belongs to [0,1]
		// This O(LogN) level operation
		// perform a binary search in anchors 
		// returns a 4D vector where xyz represent the Position 
		// w is the length from start to point;
		XMVECTOR extract(float t) const;
		XMVECTOR operator[](float t) const { return extract(t); }

		std::vector<Vector3> FixIntervalSampling(float Interval) const;
		std::vector<Vector3> FixCountSampling(unsigned int SampleSegmentCount, bool Smooth = true) const;
		std::vector<Vector3> FixCountSampling2(unsigned int SampleSegmentCount) const;

		static void LaplaianSmoothing(std::vector<Vector3>& Curve, unsigned IterationTimes = 2);

		const std::vector<XMFLOAT4A, DirectX::AlignedAllocator<XMFLOAT4A>>& ControlPoints() const
		{
			return Anchors;
		}

	private:
		// The data we stored is actually aligned on 16-byte boundary , so , use it as a XMFLOAT4A
		std::vector<XMFLOAT4A, DirectX::AlignedAllocator<XMFLOAT4A>> Anchors;
	};

}