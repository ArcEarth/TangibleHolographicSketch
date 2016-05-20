#pragma once
#include "BezierClip.h"
#include <vector>
#include <list>
#include "csg.h"

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

	template <typename _Ty, int _Order>
	bool is_curve_degreed(const Bezier::BezierClipping<_Ty, _Order>& latitude, float epsilon)
	{
		return ((latitude[0] - latitude[1]).Length() < epsilon) && ((latitude[1] - latitude[2]).Length() < epsilon) && ((latitude[2] - latitude[3]).Length() < epsilon);
	}

	template <typename _Ty, int _Order, typename MeshType>
	bool triangluate(const Bezier::BezierPatch<_Ty, _Order>& patch, MeshType& mesh, int tessellation, bool flip = false, const DirectX::Vector4 &uvRect = DirectX::Vector4(.0f,.0f,1.f,1.f))
	{
		using namespace DirectX::VertexTraits;
		if (tessellation == 0)
			return false;
		size_t offset = mesh.vertices.size();
		size_t stride = tessellation + 1;

		using VertexType = typename decltype(mesh.vertices)::value_type;

		VertexType d_vertex;
		using DirectX::XMVECTOR;

		float delta = 1.0f / (float)tessellation;

		static constexpr float epsilon = std::numeric_limits<float>::epsilon() * 100;
		for (size_t i = 0; i <= tessellation; i++)
		{
			float u = (float)i * delta;

			auto latitude = patch.row_clipping(u);

			if (is_curve_degreed(latitude,epsilon))
				latitude = patch.row_clipping(u + copysignf(0.01f * delta, 0.5f - u));

			for (size_t j = 0; j <= tessellation; j++)
			{
				float v = (float)j  * delta;
				XMVECTOR position = latitude.eval(v);
				XMVECTOR tangent = latitude.tangent(v);
				//tangent = DirectX::XMVector3Normalize(tangent);
				// storage the tangent info in normal field

				set_position(d_vertex, position);
				set_uv(d_vertex, u * uvRect.z + uvRect.x, v * uvRect.w + uvRect.y);
				set_normal(d_vertex, tangent);
				set_tangent(d_vertex, tangent);

				mesh.vertices.push_back(d_vertex);
			}
		}

		// Fix normals
		if (has_normal<VertexType>::value)
		{
			for (size_t j = 0; j <= tessellation; j++)
			{
				float v = (float)j * delta;

				auto longitude = patch.col_clipping(v);

				if (is_curve_degreed(longitude, epsilon))
					longitude = patch.col_clipping(v + copysignf(0.01f * delta, 0.5f - v));


				for (size_t i = 0; i <= tessellation; i++)
				{
					float u = (float)i  * delta;
					XMVECTOR binormal = longitude.tangent(u);

					int idx = offset + i * stride + j;
					auto& vertex = mesh.vertices[idx];

					XMVECTOR tangent = get_normal(vertex);

					XMVECTOR normal = DirectX::XMVector3Cross(tangent, binormal);
					normal = DirectX::XMVector3Normalize(normal);

					if (flip)
						normal = -normal;

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

				if (flip)
				{
					auto pIdices = &mesh.indices.back() - 5;
					std::swap(pIdices[0], pIdices[2]);
					std::swap(pIdices[3], pIdices[5]);
				}

			}
		}

		return true;
	}
}