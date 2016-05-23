#pragma once
#include <vector>
#include <cassert>
#include "csg.h"
#include "SpaceCurve.h"

namespace Geometrics
{
	using DirectX::Vector2;
	using DirectX::Vector3;
	using DirectX::Ray;
	using DirectX::FXMVECTOR;

	using Curve = SpaceCurve;

	struct DefaultVertex
	{
		Vector3 position;
		Vector3 normal;
		Vector2 uv;
	};

	//using MeshType = TriangleMesh<DefaultVertex, uint16_t>;

	// A Patch is a closed curve on a surface
	// And the region within
	template <class _MeshType>
	class SurfacePatch
	{
	public:
		static_assert(concept_mesh_type<_MeshType>::value, "MeshType not valiad");
		using MeshType = _MeshType;
		//typedef TriangleMesh<_VertexType, IndexType> MeshType;
		//typedef _VertexType VertexType;
		//typedef _IndexType IndexType;
	protected:
		MeshType*	m_surface;
		Curve		m_uvCurve;

		// the face id of each uv points
		std::vector<int> m_fids;

		Curve		m_boundry;
		MeshType	m_mesh;		// triangluated mesh
		int			m_dirty;

	public:
		MeshType& surface() { return *m_surface; }
		const MeshType& surface() const { return *m_surface; }
		void setSurface(MeshType* surface) { m_surface = surface; m_dirty = 1; };

		inline const Curve & boundry() const { return m_boundry; }

		inline MeshType & mesh() { return m_mesh; }

		inline const MeshType & mesh() const { return m_mesh; }

		inline void clear()
		{
			m_boundry.clear();
			m_uvCurve.clear();
		}

		//Curve& uvBoundry();
		const Curve& uvBoundry() const { return m_uvCurve; }

		bool append(XMVECTOR position, int fid);

		void closeLoop()
		{
			m_boundry.closeLoop();
			m_uvCurve.closeLoop();
		}

		// convert uv to positon in world space
		XMVECTOR XM_CALLCONV unproject(XMVECTOR uv, int& fid);

		// unproject uv curve to the spatial curve
		void unprojectBoundry(int startFid);

		// seperate and re-triangluate the patch from exist surface
		MeshType& triangulate(size_t subdiv);
	};


	using std::vector;

	template <class _MeshType>
	class GeneralizedExtrusion
	{
		static_assert(concept_mesh_type<_MeshType>::value, "MeshType not valiad");
		using MeshType = _MeshType;
		using SurfacePatch = ::Geometrics::SurfacePatch<MeshType>;

		SurfacePatch *	m_top, *m_bottom;
		Curve *			m_path;
		MeshType		m_mesh;
		int				m_dirty;

	public:

		GeneralizedExtrusion()
			: m_top(nullptr), m_bottom(nullptr), m_path(nullptr), m_dirty(1)
		{}

		GeneralizedExtrusion(SurfacePatch * top, SurfacePatch * bottom, Curve * axis)
			: m_top(top), m_bottom(bottom), m_path(axis), m_dirty(1)
		{}

		~GeneralizedExtrusion()
		{}

		bool valiad() const { return m_top != nullptr && m_bottom != nullptr && m_path && !m_dirty; }

		MeshType & mesh() { assert(!m_dirty); return m_mesh; }

		const MeshType & mesh() const { assert(!m_dirty); return m_mesh; }

		SurfacePatch& top() { return *m_top; }
		const SurfacePatch& top() const { return *m_top; }
		void setTop(SurfacePatch *path) { m_dirty = (m_top == path); m_top = path; }

		SurfacePatch& bottom() { return *m_bottom; }
		const SurfacePatch& bottom() const { return *m_bottom; }
		void setBottom(SurfacePatch *path) { m_dirty = (m_bottom == path); m_bottom = path; }

		Curve& axis() { return *m_path; }
		const Curve& axis() const { return *m_path; }
		void setAxis(Curve *axis) { m_dirty = (m_path == axis); m_path = axis; }

		// triangulate top and bottom with path into m_mesh
		MeshType& triangulate(int axisSubdiv, int polarSubdiv);
	};


	template <class _MeshType>
	inline bool SurfacePatch<_MeshType>::append(XMVECTOR position, int fid)
	{
		using namespace DirectX;
		using namespace DirectX::VertexTraits;

		if (m_boundry.append(position))
		{
			auto& tri = m_surface->facet(fid);
			auto& vertics = m_surface->vertices;

			auto BC = TriangleTests::BarycentricCoordinate(position, get_position(vertics[tri[0]]), get_position(vertics[tri[1]]), get_position(vertics[tri[2]]));
			auto uv = XMVectorBaryCentricV(get_uv(vertics[tri[0]]), get_uv(vertics[tri[1]]), get_uv(vertics[tri[2]]), BC);

			// append uv forcely
			m_uvCurve.append(uv, true);
		}
		m_dirty |= 2;
		return false;
	}

	template <class _MeshType>
	inline XMVECTOR XM_CALLCONV SurfacePatch<_MeshType>::unproject(XMVECTOR uv, int& fid)
	{
		using namespace DirectX;
		using namespace DirectX::VertexTraits;
		auto& vertics = m_surface->vertices;
		auto& tri = m_surface->facet(fid);
		auto BC = TriangleTests::BarycentricCoordinate(uv, get_uv(vertics[tri[0]]), get_uv(vertics[tri[1]]), get_uv(vertics[tri[2]]));


		XMVECTOR vali = XMVectorLessOrEqual(BC, XMVectorZero());
		XMUINT3 out;
		XMStoreUInt3(&out, vali);

		if (out.x)
			fid = m_surface->adjacentFacet(fid, 0);
		else if (out.y)
			fid = m_surface->adjacentFacet(fid, 1);
		else if (out.z)
			fid = m_surface->adjacentFacet(fid, 2);
		else
			return XMVectorBaryCentricV(get_position(vertics[tri[0]]), get_position(vertics[tri[1]]), get_position(vertics[tri[2]]), BC);
		return unproject(uv, fid);
	}

	template<class _MeshType>
	inline void SurfacePatch<_MeshType>::unprojectBoundry(int startFid)
	{
		assert(!"Not implemented yet");
		m_dirty = 0;
	}

	template<class _MeshType>
	inline _MeshType & SurfacePatch<_MeshType>::triangulate(size_t subdiv)
	{
		assert(!"Not implemented yet");
		m_dirty = 0;
		return m_mesh;
	}

	namespace Detail
	{
		using namespace DirectX;
		inline void XM_CALLCONV centralize(_Inout_ Curve & curve, FXMVECTOR center, FXMVECTOR rotation, bool rotateRespectPolarAngle)
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

		inline RigidTransform XM_CALLCONV centralize(Curve& curve, FXMVECTOR identityAxis = DirectX::g_XMIdentityR1.v, bool rotateRespectPolarAngle = true);

		/// <summary>
		///	return the transform from centralized curve to original curve
		/// </summary>
		/// <param name="curve">curve to transform</param>
		/// <param name="identityAxis">the axis that rotate the patch normal into</param>
		/// <param name="rotateRespectPolarAngle">should the function zerolize the polar angle</param>
		/// <returns>return the transform from centralized curve to original curve r[0] = translation, r[1] = rotation quaternion</returns>
		inline RigidTransform XM_CALLCONV centralize(_Inout_ Curve & curve, FXMVECTOR identityAxis, bool rotateRespectPolarAngle)
		{
			using namespace DirectX;

			auto n = curve.size();

			// caculate geo center
			XMVECTOR center = XMVectorZero();
			for (int i = 0; i < n; i++)
			{
				center += curve.position(i);
			}
			center /= (float)n;

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
		inline std::enable_if_t<std::is_integral<IndexType>::value> pushTriangle(vector<IndexType>& indices, size_t a, size_t b, size_t c)
		{
			indices.push_back(static_cast<IndexType>(a));
			indices.push_back(static_cast<IndexType>(b));
			indices.push_back(static_cast<IndexType>(c));
		}
	}

	template <class _MeshType>
	_MeshType & GeneralizedExtrusion<_MeshType>::triangulate(int axisSubdiv, int polarSubdiv)
	{
		using namespace Detail;
		using namespace DirectX;
		using namespace DirectX::VertexTraits;
		XMVECTOR stdAxis = g_XMIdentityR1.v;

		bool useTop = m_top != nullptr;

		m_mesh.clear();
		assert(m_bottom != nullptr);

		auto path = *m_path;

		path.resample(axisSubdiv + 1);
		//path.smooth(0.8f, 8);

		XMVECTOR p0 = path.position(0);;
		XMVECTOR t0 = path.tangent(0);
		if (XMVector4Less(t0, g_XMEpsilon.v))
			t0 = stdAxis;

		XMVECTOR r0 = XMQuaternionRotationVectorToVector(stdAxis, t0);

		// inverse r0
		r0 = XMQuaternionConjugate(r0);

		Curve top, bottom = m_bottom->boundry();

		bottom.resample(polarSubdiv);
		centralize(bottom, p0, r0, true);

		if (useTop)
		{
			XMVECTOR p1 = path.tangent((int)(path.size() - 1));
			XMVECTOR t1 = path.tangent((int)(path.size() - 1));
			XMVECTOR r1 = XMQuaternionRotationVectorToVector(t0, t1);
			r1 = XMQuaternionMultiply(r0, r1);
			r1 = XMQuaternionConjugate(r1);

			top.resample(polarSubdiv);
			centralize(top, p1, r1, true);
		}

		// put r0 back , r1 are useless after
		r0 = XMQuaternionConjugate(r0);

		auto  length = path.length();
		auto  interval = length / axisSubdiv;
		auto& vertices = m_mesh.vertices;
		auto& indices = m_mesh.indices;

		vertices.reserve((axisSubdiv + 1) * (polarSubdiv + 1));
		indices.reserve(3 * axisSubdiv * polarSubdiv * 2);

		int idxBase = vertices.size();

		XMVECTOR r0t = XMQuaternionIdentity();
		XMVECTOR tprev = t0;
		for (int axisIdx = 0; axisIdx <= axisSubdiv; axisIdx++)
		{
			float t = axisIdx * interval;
			XMVECTOR tv = XMVectorReplicate(t);

			// axis posion
			XMVECTOR pt = path.position(axisIdx);
			// axis tanget
			XMVECTOR tt = path.tangent(axisIdx);

			// update rotation only if the tangent are not zero
			if (XMVector4Greater(tt, g_XMEpsilon.v))
			{
				XMVECTOR rtprevt = XMQuaternionRotationVectorToVector(tprev, tt);
				r0t = XMQuaternionMultiply(r0t, rtprevt);
				tprev = tt;
			}

			// axis rotation
			XMVECTOR rt = XMQuaternionMultiply(r0, r0t);

			// [0,polarSubdiv], 0 and polarSubdiv should be overlaped
			for (int i = 0; i <= polarSubdiv; ++i)
			{
				XMVECTOR v, vt;
				float polart = (float)i / (float)(polarSubdiv + 1);

				v = bottom.position(i);
				vt = bottom.tangent(i);
				if (useTop)
				{
					// Lerp between bottom curve and top curve
					v = XMVectorLerpV(v, top.position(i), tv);
					vt = XMVectorLerpV(vt, top.tangent(i), tv);
				}

				v = XMVector3Rotate(v, rt);
				v += pt;
				vt = XMVector3Rotate(vt, rt);

				DefaultVertex vtx;
				set_position(vtx, v);
				set_normal(vtx, XMVector3Cross(tt, vt));
				set_uv(vtx, polart, t);

				vertices.push_back(vtx);
			}
		}

		int cols = polarSubdiv + 1;
		for (int axisIdx = 0; axisIdx < axisSubdiv; axisIdx++)
		{
			for (int i = 0; i < polarSubdiv; ++i)
			{
				IndexType idx = axisIdx * cols + i;
				IndexType upIdx = idx + cols;

				IndexType idx_1 = axisIdx * cols + (i + 1)/* % polarSubdiv*/;
				IndexType upIdx_1 = idx_1 + cols;

				pushTriangle(indices, idx, idx_1, upIdx);
				pushTriangle(indices, idx_1, upIdx_1, upIdx);
			}
		}

		//m_mesh.build();

		m_dirty = false;
		return m_mesh;
	}
}