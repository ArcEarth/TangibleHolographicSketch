#include "SpaceCurve.h"

using namespace Geometrics;
using namespace DirectX;


SpaceCurve::SpaceCurve(const std::vector<Vector3>& Trajectory)
{
	for (auto& point : Trajectory)
	{
		this->push_back(point);
	}
}


std::vector<Vector3> SpaceCurve::FixCountSampling(unsigned int SampleSegmentCount, bool Smooth/* = true*/) const
{
	if (Anchors.size() <= 1) {
		return std::vector<Vector3>();
	}
	float Interval = length() / SampleSegmentCount;
	// Allocate C+1 size array for storage the result
	std::vector<Vector3> sample(SampleSegmentCount + 1);
	Vector3 ptr;
	float r = 0.0f;
	unsigned int k = 1;

	for (unsigned int i = 0; i < SampleSegmentCount + 1; i++, r += Interval)
	{
		while ((k < Anchors.size() - 1) && (Anchors[k].w < r)) k++;
		float t = r - Anchors[k - 1].w;
		float f = Anchors[k].w - Anchors[k - 1].w;
		if (abs(f) < 1.192092896e-7f)
		{
			XMVECTOR v2 = XMLoadFloat4A(&Anchors[k]);
			XMStoreFloat3(&sample[i], v2);
		}
		else
		{
			XMVECTOR v0 = XMLoadFloat4A(&Anchors[k - 1]);
			XMVECTOR v1 = XMLoadFloat4A(&Anchors[k]);
			XMVECTOR v2 = XMVectorLerp(v0, v1, t);
			XMStoreFloat3(&sample[i], v2);
		}
	}

	if (Smooth)
		LaplaianSmoothing(sample, 16);

	return sample;
}

std::vector<Vector3> SpaceCurve::FixCountSampling2(unsigned int SampleSegmentCount) const
{
	auto trajectory = FixCountSampling(SampleSegmentCount * 15, true);
	SpaceCurve smoothSampler(trajectory);
	return smoothSampler.FixCountSampling(SampleSegmentCount, false);
}


std::vector<Vector3> SpaceCurve::FixIntervalSampling(float Interval) const
{
	assert(Interval != 0.0f);
	float r = length();
	unsigned int Count = (int) (r / Interval) + 1;
	auto trajectory = FixCountSampling(Count * 15, true);
	SpaceCurve smoothSampler(trajectory);
	r = smoothSampler.length();
	Count = (int) (r / Interval) + 1;
	return smoothSampler.FixCountSampling(Count, false);
}

void SpaceCurve::push_back(const Vector3& p)
{
	const float* ptr = reinterpret_cast<const float*>(&p);
	XMFLOAT4A content(ptr);
	if (Anchors.empty()) {
		content.w = .0f;
		Anchors.push_back(content);
	}
	else {
		XMVECTOR vtr = XMLoadFloat4A(&content);
		XMVECTOR btr = XMLoadFloat4A(&(Anchors.back()));
		float len = XMVectorGetW(btr + XMVector3Length(vtr - btr));
		if (abs(len - Anchors.back().w) < XM_EPSILON * 8)
			return;
		content.w = len;
	}
	Anchors.push_back(content);
}

DirectX::XMVECTOR SpaceCurve::extract(float t) const
{
	unsigned int a = 0, b = Anchors.size() - 1;
	t = t * length();
	while (b - a > 1)
	{
		unsigned int k = (a + b) / 2;
		if (Anchors[k].w > t) b = k;
		else a = k;
	}
	XMVECTOR v0 = XMLoadFloat4A(&Anchors[a]);
	XMVECTOR v1 = XMLoadFloat4A(&Anchors[b]);
	float rt = (t - Anchors[a].w) / (Anchors[b].w - Anchors[a].w);
	XMVECTOR v2 = XMVectorLerp(v0, v1, rt);
	return v2;
}

void SpaceCurve::LaplaianSmoothing(std::vector<Vector3>& Curve, unsigned IterationTimes /*= 1*/)
{
	unsigned int N = Curve.size();
	std::vector<Vector3> Cache(N);
	Vector3* BUFF[2] = { &Curve[0],&Cache[0] };
	unsigned int src = 0, dst = 1;
	for (unsigned int k = 0; k < IterationTimes; k++)
	{
		BUFF[dst][0] = BUFF[src][0];
		BUFF[dst][N - 1] = BUFF[src][N - 1];
		for (unsigned int i = 1; i < N - 1; i++)
		{
			BUFF[dst][i] = (BUFF[src][i - 1] + BUFF[src][i + 1])*0.5f;
		}
		dst = !dst;
		src = !src;
	}
	if (dst == 0)
	{
		for (unsigned int i = 0; i < N; i++)
		{
			BUFF[0][i] = BUFF[1][i];
		}
	}
}

//float DistanceSegmentToSegment(const LineSegement &S1, const LineSegement &S2);
float DistanceSegmentToSegment(FXMVECTOR S0P0, FXMVECTOR S0P1, FXMVECTOR S1P0, GXMVECTOR S1P1);
//inline float Length(const LineSegement& Ls){
//	return Distance(Ls.first,Ls.second);
//}