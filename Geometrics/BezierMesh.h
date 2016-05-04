#pragma once
#include "BezierClip.h"
#include <vector>
#include <list>
#include "csg.h"
//#include <CGAL\Polyhedron_3.h>

namespace Geometrics
{
	namespace Bezier
	{
		namespace Mesh
		{
			typedef float PointType;

			// Use Cubic-bezier patch as patch type
			typedef BezierClipping<PointType, 3>  CurveType;
			typedef BezierPatch<PointType, 3>	  PatchType;

			class BezierMesh;

			class HalfEdge;

			class Face;

			struct Vertex
			{
				PointType position;

				BezierMesh* mesh;

				std::list<HalfEdge*> in_edges;
				std::list<HalfEdge*> out_edges;
			};

			struct HalfEdge
			{
				CurveType curve;

				// owner mesh
				BezierMesh* mesh;

				// incident vertex
				Vertex* vertex;
				// incident face
				Face*	face;

				// opposite half edge
				HalfEdge* opposite;
				// previous half edge
				HalfEdge* prev;
				// next half edge
				HalfEdge* next;

				const Vertex* incident_vertex() const { return nullptr; }
				Vertex* incident_vertex() { return nullptr; }
			};

			struct Face
			{
				PatchType patch;

				HalfEdge* boundry_edge;
			};

			class BezierMesh
			{
			public:
				std::vector<Vertex> Vertices;
			};
		}
	}

	typedef Bezier::BezierPatch<Vector3, 3U> CubicBezierPatch;
	typedef CubicBezierPatch::ClippingType CubicBezierCurve;


	template <typename _Ty, int _Order, typename MeshType>
	bool triangluate(const Bezier::BezierPatch<_Ty, _Order>& patch, MeshType& mesh, int tessellation, const Vector4 &uvRect = Vector4(.0f,.0f,1.f,1.f))
	{
		using namespace DirectX::VertexTraits;
		if (tessellation == 0)
			return false;
		size_t offset = mesh.vertices.size();
		size_t stride = tessellation + 1;

		MeshType::VertexType d_vertex;
		using DirectX::XMVECTOR;

		for (size_t i = 0; i <= tessellation; i++)
		{
			float u = (float)i / tessellation;

			auto latitude = patch.row_clipping(u);

			for (size_t j = 0; j <= tessellation; j++)
			{
				float v = (float)j / tessellation;
				XMVECTOR position = latitude.eval(v);
				XMVECTOR tangent = latitude.tangent(v);
				tangent = DirectX::XMVector3Normalize(tangent);
				// storage the tangent info in normal field

				set_position(d_vertex, position);
				set_uv(d_vertex, u * uvRect.z + uvRect.x, v * uvRect.w + uvRect.y);
				set_normal(d_vertex, tangent);
				set_tangent(d_vertex, tangent);

				mesh.vertices.push_back(d_vertex);
			}
		}

		// Fix normals
		if (has_normal<MeshType::VertexType>::value)
		{
			for (size_t j = 0; j <= tessellation; j++)
			{
				float v = (float)j / (float)tessellation;

				auto longitude = patch.col_clipping(v);

				for (size_t i = 0; i <= tessellation; i++)
				{
					float u = (float)i / (float)tessellation;
					XMVECTOR binormal = longitude.tangent(u);

					int idx = offset + i * stride + j;
					auto& vertex = mesh.vertices[idx];

					XMVECTOR tangent = get_normal(vertex);

					XMVECTOR normal = DirectX::XMVector3Cross(tangent, binormal);
					normal = DirectX::XMVector3Normalize(normal);

					set_normal(vertex, normal);
				}
			}
		}

		for (size_t i = 0; i < tessellation; i++)
		{
			for (size_t j = 0; j < tessellation; j++)
			{
				// Make a list of six index values (two triangles).
				mesh.add_facet(
					offset + i * stride + j,
					offset + (i + 1) * stride + j,
					offset + (i + 1) * stride + j + 1);
				mesh.add_facet(
					offset + i * stride + j,
					offset + (i + 1) * stride + j + 1,
					offset + i * stride + j + 1);
			}
		}

		return true;
	}
}