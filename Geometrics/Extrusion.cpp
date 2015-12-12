#include "Extrusion.h"

using namespace Geometrics;
using namespace DirectX;
using namespace std;

Extrusion::Extrusion()
	: m_top(nullptr), m_bottom(nullptr), m_path(nullptr), m_dirty(1)
{

}

Extrusion::Extrusion(Patch * top, Patch * bottom, Curve * axis)
	: m_top(top), m_bottom(bottom), m_path(axis), m_dirty(1)
{}

Extrusion::~Extrusion()
{

}

int Extrusion::intersect(const Ray & ray, std::vector<Vector3>& intersections)
{
	return RayIntersection(m_mesh, ray.position, ray.direction, reinterpret_cast<std::vector<DirectX::XMFLOAT3>*>(&intersections));
}

MeshType & Extrusion::mesh() { assert(!m_dirty, "triangluate the mesh first"); return m_mesh; }

const MeshType & Extrusion::mesh() const { assert(!m_dirty, "triangluate the mesh first"); return m_mesh; }

void centralize(_Inout_ Curve & curve, FXMVECTOR center, FXMVECTOR rotation, bool rotateRespectPolarAngle)
{
	using namespace DirectX;

	auto n = curve.size();

	// polar angles in X-Z plane
	std::vector<float> angles(n);
	float minA = XM_2PI;
	int minAidx = 0;

	// zero mean and rotate with align to identity axis
	for (int i = 0; i < curve.size(); i++)
	{
		auto& ach = reinterpret_cast<XMFLOAT3A&>(curve.anchor(i));

		XMVECTOR vr = curve.position(i) - center;
		vr = XMVector3Rotate(vr, rotation);

		XMStoreFloat3A(&ach, vr);

		float ang = atan2f(ach.z, ach.x);
		angles[i] = ang;

		if (ang < minA)
		{
			minA = ang;
			minAidx = i;
		}
	}

	if (rotateRespectPolarAngle)
	{
		Curve temp(curve);
		for (int i = 0; i < n; i++)
		{
			curve.anchor(i) = temp.anchor((minAidx + i) % n);
		}

		curve.updateLength();
	}

}

RigidTransform centralize(Curve& curve, FXMVECTOR identityAxis = DirectX::g_XMIdentityR1.v, bool rotateRespectPolarAngle = true);

/// <summary>
///	return the transform from centralized curve to original curve
/// </summary>
/// <param name="curve">curve to transform</param>
/// <param name="identityAxis">the axis that rotate the patch normal into</param>
/// <param name="rotateRespectPolarAngle">should the function zerolize the polar angle</param>
/// <returns>return the transform from centralized curve to original curve r[0] = translation, r[1] = rotation quaternion</returns>
RigidTransform centralize(_Inout_ Curve & curve, FXMVECTOR identityAxis, bool rotateRespectPolarAngle)
{
	using namespace DirectX;

	auto n = curve.size();

	// caculate geo center
	XMVECTOR center = XMVectorZero();
	for (int i = 0; i < n; i++)
	{
		center += curve.position(i);
	}
	center /= n;

	// caculate average 'patch' normal
	XMVECTOR axis = XMVectorZero();
	for (int i = 0; i < n; i++)
	{
		XMVECTOR vr = curve.position(i) - center;
		XMVECTOR vt = curve.tangent(i);
		vt = XMVector3Cross(vr, vt);
		axis += vt;
	}
	axis = XMVector3Normalize(axis);

	XMVECTOR q = XMQuaternionRotationVectorToVector(axis, identityAxis);

	centralize(curve, center, q, rotateRespectPolarAngle);

	RigidTransform transform;
	transform.Translation = center;
	transform.Rotation = XMQuaternionInverse(q);
	return transform;
}

template <typename IndexType>
std::enable_if_t<std::is_integral<IndexType>::value> pushTriangle(vector<IndexType>& indices, IndexType a, IndexType b, IndexType c)
{
	indices.push_back(a);
	indices.push_back(b);
	indices.push_back(c);
}
// triangulate top and bottom with path into m_mesh
MeshType & Extrusion::triangulate(int axisSubdiv, int polarSubdiv)
{
	using namespace DirectX;
	XMVECTOR stdAxis = g_XMIdentityR1.v;

	m_mesh.clear();
	assert(m_bottom != nullptr);
	auto top = m_top->boundry();
	auto bottom = m_bottom->boundry();
	auto& path = *m_path;

	top.resample(polarSubdiv);
	bottom.resample(polarSubdiv);

	XMVECTOR p0 = path.position(0);;
	XMVECTOR p1 = path.tangent((int)(path.size() - 1));
	XMVECTOR t0 = path.tangent(0);
	XMVECTOR t1 = path.tangent((int)(path.size() - 1));

	XMVECTOR r0 = XMQuaternionRotationVectorToVector(stdAxis, path.tangent(0));
	XMVECTOR r1 = XMQuaternionRotationVectorToVector(t0, t1);
	r1 = XMQuaternionMultiply(r0, r1);

	// inverse r0,r1
	r0 = XMQuaternionConjugate(r0);
	r1 = XMQuaternionConjugate(r1);

	centralize(bottom, p0, r0, true);
	centralize(top, p1, r1, true);
	// put r0 back
	r0 = XMQuaternionConjugate(r0);

	auto  length = path.length();
	auto  interval = length / axisSubdiv;
	auto& vertices = m_mesh.vertices;
	auto& indices = m_mesh.indices;
	vertices.reserve(axisSubdiv * polarSubdiv);
	indices.reserve(3 * axisSubdiv * polarSubdiv * 2);

	int idxBase = vertices.size();

	XMDUALVECTOR disp;
	for (int axisIdx = 0; axisIdx <= axisSubdiv; axisIdx++)
	{
		float t = axisIdx * interval;
		XMVECTOR tv = XMVectorReplicate(t);

		// axis posion
		XMVECTOR pt = path.position(t);
		// axis tanget
		auto tt = path.tangent(t);

		auto r0t = XMQuaternionRotationVectorToVector(t0, tt);
		// axis rotation
		XMVECTOR rt = XMQuaternionMultiply(r0, r0t);

		for (int i = 0; i < polarSubdiv; ++i)
		{
			auto v = XMVectorLerpV(bottom.position(i), top.position(i), tv);
			auto vt = XMVectorLerpV(bottom.tangent(i), top.tangent(i), tv);

			v = XMVector3Rotate(v, rt);
			v += pt;
			vt = XMVector3Rotate(vt, rt);

			Vertex vtx;
			vtx.position = v;
			vtx.normal = XMVector3Cross(tt, vt);

			vertices.push_back(vtx);
		}
	}

	for (int axisIdx = 0; axisIdx < axisSubdiv; axisIdx++)
	{
		for (int i = 0; i < polarSubdiv; ++i)
		{
			int idx = axisIdx * polarSubdiv + i;
			int upIdx = idx + polarSubdiv;

			int idx_1 = axisIdx * polarSubdiv + (i + 1) % polarSubdiv;
			int upIdx_1 = idx_1 + polarSubdiv;

			pushTriangle(indices, idx, idx_1, upIdx);
			pushTriangle(indices, upIdx_1, idx_1, idx);
		}
	}

	m_dirty = false;
	return m_mesh;
}

MeshType & Geometrics::Patch::triangulate(size_t subdiv)
{
	assert(!"Not implemented yet");
	m_dirty = false;
	return m_mesh;
}
