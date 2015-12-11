#include "SpaceCurve.h"

using namespace Geometrics;
using namespace DirectX;


SpaceCurve::SpaceCurve() {}

SpaceCurve::~SpaceCurve() {}

SpaceCurve::SpaceCurve(const std::vector<Vector3>& Trajectory)
{
	for (auto& point : Trajectory)
	{
		this->push_back(point);
	}
}


std::vector<Vector3> SpaceCurve::FixCountSampling(unsigned int SampleSegmentCount, bool Smooth/* = true*/) const
{
	if (m_anchors.size() <= 1) {
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
		while ((k < m_anchors.size() - 1) && (m_anchors[k].w < r)) k++;
		float t = r - m_anchors[k - 1].w;
		float f = m_anchors[k].w - m_anchors[k - 1].w;
		if (abs(f) < 1.192092896e-7f)
		{
			XMVECTOR v2 = XMLoadFloat4A(&m_anchors[k]);
			XMStoreFloat3(&sample[i], v2);
		}
		else
		{
			XMVECTOR v0 = XMLoadFloat4A(&m_anchors[k - 1]);
			XMVECTOR v1 = XMLoadFloat4A(&m_anchors[k]);
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

void SpaceCurve::resample(size_t anchorCount, bool smooth)
{
	auto sample = FixCountSampling(anchorCount, smooth);

	m_anchors.resize(sample.size());
	for (int i = 0; i < sample.size(); i++)
		reinterpret_cast<Vector3&>(m_anchors[i]) = sample[i];
	updateLength();
}

std::vector<Vector3> SpaceCurve::FixIntervalSampling(float Interval) const
{
	assert(Interval != 0.0f);
	float r = length();
	unsigned int Count = (int)(r / Interval) + 1;
	auto trajectory = FixCountSampling(Count * 15, true);
	SpaceCurve smoothSampler(trajectory);
	r = smoothSampler.length();
	Count = (int)(r / Interval) + 1;
	return smoothSampler.FixCountSampling(Count, false);
}

void SpaceCurve::push_back(const Vector3& p)
{
	const float* ptr = reinterpret_cast<const float*>(&p);
	XMFLOAT4A content(ptr);
	if (m_anchors.empty()) {
		content.w = .0f;
		m_anchors.push_back(content);
	}
	else {
		XMVECTOR vtr = XMLoadFloat4A(&content);
		XMVECTOR btr = XMLoadFloat4A(&(m_anchors.back()));
		float len = XMVectorGetW(btr + XMVector3Length(vtr - btr));
		if (abs(len - m_anchors.back().w) < XM_EPSILON * 8)
			return;
		content.w = len;
	}
	m_anchors.push_back(content);
}

void XM_CALLCONV SpaceCurve::push_back(FXMVECTOR p)
{
	push_back(Vector3(p));
}

// Tangent vector at anchor index
XMVECTOR SpaceCurve::extract(float t) const
{
	assert(t <= length() && t >= .0f, "t must belongs to [0,length()]");

	unsigned int a = 0, b = m_anchors.size() - 1;
	t = t * length();
	while (b - a > 1)
	{
		unsigned int k = (a + b) / 2;
		if (m_anchors[k].w > t) b = k;
		else a = k;
	}

	XMVECTOR v0 = XMLoadFloat4A(&m_anchors[a]);
	XMVECTOR v1 = XMLoadFloat4A(&m_anchors[b]);
	float rt = (t - m_anchors[a].w) / (m_anchors[b].w - m_anchors[a].w);
	XMVECTOR v2 = XMVectorLerp(v0, v1, rt);

	// set w = 1.0
	v2 = XMVectorSelect(v2, g_XMIdentityR3.v, g_XMIdentityR3.v);
	return v2;
}

XMVECTOR SpaceCurve::tangent(int idx) const
{
	using namespace DirectX;
	int pid = idx - (idx > 0);
	int rid = idx + (idx + 1 < m_anchors.size());

	XMVECTOR v0 = XMLoadFloat4A(&m_anchors[pid]);
	XMVECTOR v1 = XMLoadFloat4A(&m_anchors[rid]);
	v1 = XMVectorSubtract(v1, v0);
	v0 = XMVectorSplatW(v1);
	v1 = XMVectorDivide(v1, v0);

	// set w = 0 as tangent are pure direction
	v1 = XMVectorSelect(v1, XMVectorZero(), g_XMIdentityR3.v);
	return v1;
}

XMVECTOR SpaceCurve::tangent(float t) const
{
	assert(t <= length() && t >= .0f, "t must belongs to [0,length()]");

	// binary search for parameter
	int a = 0, b = m_anchors.size() - 1;
	t = t * length();
	while (b - a > 1)
	{
		unsigned int k = (a + b) / 2;
		if (m_anchors[k].w > t) b = k;
		else a = k;
	}

	XMVECTOR v0 = tangent(a);
	XMVECTOR v1 = tangent(b);
	float rt = (t - m_anchors[a].w) / (m_anchors[b].w - m_anchors[a].w);
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

void SpaceCurve::updateLength()
{
	m_anchors[0].w = 0;
	XMVECTOR p0 = XMLoadFloat4A(&m_anchors[0]);
	XMVECTOR p1;
	for (int i = 1; i < m_anchors.size(); i++)
	{
		p1 = XMLoadFloat4A(&m_anchors[i]);
		float dt = XMVectorGetX(XMVector3Length(p1 - p0));
		m_anchors[i].w = m_anchors[i - 1].w + dt;
		p0 = p1;
	}
}

//float DistanceSegmentToSegment(const LineSegement &S1, const LineSegement &S2);
float DistanceSegmentToSegment(FXMVECTOR S0P0, FXMVECTOR S0P1, FXMVECTOR S1P0, GXMVECTOR S1P1);
//inline float Length(const LineSegement& Ls){
//	return Distance(Ls.first,Ls.second);
//}