#pragma once
#include "TriangleMesh.h"

namespace Geometrics
{
	namespace Primitives
	{
		template <typename MeshType>
		inline submesh_data create_cube(MeshType& mesh, float size = 1, bool rhcoords = true);
		template <typename MeshType>
		inline submesh_data create_box(MeshType& mesh, const DirectX::XMFLOAT3& size, bool rhcoords = true, bool invertn = false);
		template <typename MeshType>
		inline submesh_data create_sphere(MeshType& mesh, float diameter = 1, size_t tessellation = 16, bool rhcoords = true, bool invertn = false);
		template <typename MeshType> 
		inline submesh_data create_geo_sphere(MeshType& mesh, float diameter = 1, size_t tessellation = 3, bool rhcoords = true);

		template <typename MeshType> 
		inline submesh_data create_cylinder(MeshType& mesh, float height = 1, float diameter = 1, size_t tessellation = 32, bool rhcoords = true);

		template <typename MeshType> 
		inline submesh_data create_cone(MeshType& mesh, float diameter = 1, float height = 1, size_t tessellation = 32, bool rhcoords = true);

		template <typename MeshType> 
		inline submesh_data create_torus(MeshType& mesh, float diameter = 1, float thickness = 0.333f, size_t tessellation = 32, bool rhcoords = true);

		template <typename MeshType> 
		inline submesh_data create_tetrahedron(MeshType& mesh, float size = 1, bool rhcoords = true);

		template <typename MeshType> 
		inline submesh_data create_octahedron(MeshType& mesh, float size = 1, bool rhcoords = true);

		template <typename MeshType> 
		inline submesh_data create_dodecahedron(MeshType& mesh, float size = 1, bool rhcoords = true);

		template <typename MeshType> 
		inline submesh_data create_icosahedron(MeshType& mesh, float size = 1, bool rhcoords = true);

	}
}

#include "Primitives.inl"