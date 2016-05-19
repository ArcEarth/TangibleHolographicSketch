#pragma once
#include "TriangleMesh.h"

namespace Geometrics
{
	namespace Primitives
	{
		namespace Detail
		{
			XM_HAS_MEMBER(vertices, has_vertices);
			XM_HAS_MEMBER(indices, has_indices);
		}

		template <typename MeshType>
		struct concept_mesh_type : public std::integral_constant<bool, Detail::has_vertices<MeshType>::value && Detail::has_indices<MeshType>::value> {};
		
		struct primitive_metric
		{
			uint32_t vertex_offset; // start vertices index of this geometry
			uint32_t vertex_count;
			uint32_t index_offset; // start index position of this geometry
			uint32_t index_count;
		};

		template <typename MeshType>
		inline primitive_metric create_cube(MeshType& mesh, float size = 1, bool rhcoords = true);
		template <typename MeshType>
		inline primitive_metric create_box(MeshType& mesh, const DirectX::XMFLOAT3& size, bool rhcoords = true, bool invertn = false);
		template <typename MeshType>
		inline primitive_metric create_sphere(MeshType& mesh, float diameter = 1, size_t tessellation = 16, bool rhcoords = true, bool invertn = false);
		template <typename MeshType> 
		inline primitive_metric create_geo_sphere(MeshType& mesh, float diameter = 1, size_t tessellation = 3, bool rhcoords = true);

		template <typename MeshType> 
		inline primitive_metric create_cylinder(MeshType& mesh, float height = 1, float diameter = 1, size_t tessellation = 32, bool rhcoords = true);

		template <typename MeshType> 
		inline primitive_metric create_cone(MeshType& mesh, float diameter = 1, float height = 1, size_t tessellation = 32, bool rhcoords = true);

		template <typename MeshType> 
		inline primitive_metric create_torus(MeshType& mesh, float diameter = 1, float thickness = 0.333f, size_t tessellation = 32, bool rhcoords = true);

		template <typename MeshType> 
		inline primitive_metric create_tetrahedron(MeshType& mesh, float size = 1, bool rhcoords = true);

		template <typename MeshType> 
		inline primitive_metric create_octahedron(MeshType& mesh, float size = 1, bool rhcoords = true);

		template <typename MeshType> 
		inline primitive_metric create_dodecahedron(MeshType& mesh, float size = 1, bool rhcoords = true);

		template <typename MeshType> 
		inline primitive_metric create_icosahedron(MeshType& mesh, float size = 1, bool rhcoords = true);

	}
}