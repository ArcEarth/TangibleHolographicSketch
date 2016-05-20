#pragma once
#include "Primitives.h"
#include <map>

namespace Geometrics
{
	namespace Primitives
	{
		using DirectX::XMVECTOR;

		template <typename IndexCollection>
		inline void push_index(IndexCollection& indices, size_t value)
		{
			using idx_t = typename IndexCollection::value_type;
			if (value > std::numeric_limits<idx_t>::max())
				throw std::exception("Index value out of range: cannot tesselate primitive so finely");
			indices.push_back((idx_t)value);
		}

		template <typename VertexCollection>
		inline void push_vertex(VertexCollection& vertices, DirectX::FXMVECTOR pos, DirectX::FXMVECTOR normal, DirectX::FXMVECTOR uv)
		{
			using vex_t = typename VertexCollection::value_type;
			vertices.emplacce_back();
			DirectX::VertexTraits::set_vertex(vertices.back(), pos, normal, uv);
		}

		// Helper for flipping winding of geometric primitives for LH vs. RH coords
		template <typename MeshType>
		static void reverse_winding(MeshType& mesh, const submesh_data metric)
		{
			using namespace DirectX;
			using namespace DirectX::VertexTraits;
			auto& vertices = mesh.vertices;
			auto& indices = mesh.indices;

			auto vb = vertices.begin() + metric.vertex_offset;
			auto ve = vertices.begin() + metric.vertex_offset + metric.vertex_count;

			XMVECTOR uvscl = XMVectorSet(1.0f,-1.0f,1.0f,1.0f);
			XMVECTOR uvoff = g_XMIdentityR1.v;
			for (auto it = vb; it != ve; ++it)
				set_uv(*it, XMVectorMultiplyAdd(get_uv(*it), uvscl, uvoff));

			auto ib = indices.begin() + metric.index_offset;
			auto ie = indices.begin() + metric.index_offset + metric.index_count;

			assert((metric.index_count % 3) == 0);
			for (auto it = ib; it != ie; it += 3)
			{
				std::swap(*it, *(it + 2));
			}
		}


		// Helper for inverting normals of geometric primitives for 'inside' vs. 'outside' viewing
		template <typename MeshType>
		static void invert_normal(MeshType& mesh, const submesh_data metric)
		{
			using namespace DirectX;
			using namespace DirectX::VertexTraits;
			auto& vertices = mesh.vertices;
			auto& indices = mesh.indices;

			auto b = vertices.begin() + metric.vertex_offset;
			auto e = vertices.begin() + metric.vertex_offset + metric.vertex_count;
			for (auto it = b; it != e; ++it)
			{
				set_normal(*it,-get_normal(*it));
			}
		}

		template <typename MeshType>
		static void reverse_winding(MeshType& mesh)
		{
			submesh_data metric;
			metric.index_offset = 0;
			metric.vertex_offset = 0;
			metric.index_count = mesh.indices.size();
			metric.vertex_count = mesh.vertices.size();

			reverse_winding(mesh, metric);
		}

		template <typename MeshType>
		static void invert_normal(MeshType& mesh)
		{
			submesh_data metric;
			metric.index_offset = 0;
			metric.vertex_offset = 0;
			metric.index_count = mesh.indices.size();
			metric.vertex_count = mesh.vertices.size();

			invert_normal(mesh, metric);
		}

		template <typename MeshType>
		inline submesh_data create_cube(MeshType& mesh, float size , bool rhcoords)
		{
			create_box(mesh,DirectX::XMFLOAT3(size, size, size), rhcoords);
		}

		template <typename MeshType>
		inline submesh_data create_box(MeshType& mesh, const DirectX::XMFLOAT3& size, bool rhcoords, bool invertn)
		{
			static_assert(concept_mesh_type<MeshType>::value, "MeshType must have vertices and indices collection");
			using namespace DirectX;
			auto& vertices = mesh.vertices;
			auto& indices = mesh.indices;
			submesh_data metric;
			metric.index_offset = indices.size();
			metric.vertex_offset = vertices.size();

			// A box has six faces, each one pointing in a different direction.
			const int FaceCount = 6;

			static const XMVECTORF32 faceNormals[FaceCount] =
			{
				{ 0,  0,  1 },
				{ 0,  0, -1 },
				{ 1,  0,  0 },
				{ -1,  0,  0 },
				{ 0,  1,  0 },
				{ 0, -1,  0 },
			};

			static const XMVECTORF32 textureCoordinates[4] =
			{
				{ 1, 0 },
				{ 1, 1 },
				{ 0, 1 },
				{ 0, 0 },
			};

			XMVECTOR tsize = XMLoadFloat3(&size);
			tsize = XMVectorDivide(tsize, g_XMTwo);

			// Create each face in turn.
			for (int i = 0; i < FaceCount; i++)
			{
				XMVECTOR normal = faceNormals[i];

				// Get two vectors perpendicular both to the face normal and to each other.
				XMVECTOR basis = (i >= 4) ? g_XMIdentityR2 : g_XMIdentityR1;

				XMVECTOR side1 = XMVector3Cross(normal, basis);
				XMVECTOR side2 = XMVector3Cross(normal, side1);

				// Six indices (two triangles) per face.
				size_t vbase = vertices.size();
				push_index(indices, vbase + 0);
				push_index(indices, vbase + 1);
				push_index(indices, vbase + 2);

				push_index(indices, vbase + 0);
				push_index(indices, vbase + 2);
				push_index(indices, vbase + 3);

				// Four vertices per face.
				push_vertex(vertices,(normal - side1 - side2) * tsize, normal, textureCoordinates[0]);
				push_vertex(vertices,(normal - side1 + side2) * tsize, normal, textureCoordinates[1]);
				push_vertex(vertices,(normal + side1 + side2) * tsize, normal, textureCoordinates[2]);
				push_vertex(vertices,(normal + side1 - side2) * tsize, normal, textureCoordinates[3]);
			}

			metric.index_count = indices.size() - metric.index_offset;
			metric.vertex_count = vertices.size() - metric.vertex_offset;

			// Build RH above
			if (!rhcoords)
				reverse_winding(mesh, metric);

			if (invertn)
				invert_normal(vertices, metric);
		}


		//--------------------------------------------------------------------------------------
		// Sphere
		//--------------------------------------------------------------------------------------
		template <typename MeshType>
		inline submesh_data create_sphere(MeshType& mesh, float diameter , size_t tessellation , bool rhcoords , bool invertn )
		{
			static_assert(concept_mesh_type<MeshType>::value, "MeshType must have vertices and indices collection");
			using namespace DirectX;
			auto& vertices = mesh.vertices;
			auto& indices = mesh.indices;
			submesh_data metric;
			metric.index_offset = indices.size();
			metric.vertex_offset = vertices.size();

			if (tessellation < 3)
				throw std::out_of_range("tesselation parameter out of range");

			size_t verticalSegments = tessellation;
			size_t horizontalSegments = tessellation * 2;

			float radius = diameter / 2;

			// Create rings of vertices at progressively higher latitudes.
			for (size_t i = 0; i <= verticalSegments; i++)
			{
				float v = 1 - (float)i / verticalSegments;

				float latitude = (i * XM_PI / verticalSegments) - XM_PIDIV2;
				float dy, dxz;

				XMScalarSinCos(&dy, &dxz, latitude);

				// Create a single ring of vertices at this latitude.
				for (size_t j = 0; j <= horizontalSegments; j++)
				{
					float u = (float)j / horizontalSegments;

					float longitude = j * XM_2PI / horizontalSegments;
					float dx, dz;

					XMScalarSinCos(&dx, &dz, longitude);

					dx *= dxz;
					dz *= dxz;

					XMVECTOR normal = XMVectorSet(dx, dy, dz, 0);
					XMVECTOR textureCoordinate = XMVectorSet(u, v, 0, 0);

					push_vertex(vertices,normal * radius, normal, textureCoordinate);
				}
			}

			// Fill the index buffer with triangles joining each pair of latitude rings.
			size_t stride = horizontalSegments + 1;
			size_t ioffset = metric.index_offset;

			for (size_t i = 0; i < verticalSegments; i++)
			{
				for (size_t j = 0; j <= horizontalSegments; j++)
				{
					size_t nextI = i + 1;
					size_t nextJ = (j + 1) % stride;

					push_index(indices, ioffset + i * stride + j);
					push_index(indices, ioffset + nextI * stride + j);
					push_index(indices, ioffset + i * stride + nextJ);

					push_index(indices, ioffset + i * stride + nextJ);
					push_index(indices, ioffset + nextI * stride + j);
					push_index(indices, ioffset + nextI * stride + nextJ);
				}
			}

			metric.index_count = indices.size() - metric.index_offset;
			metric.vertex_count = vertices.size() - metric.vertex_offset;

			// Build RH above
			if (!rhcoords)
				reverse_winding(mesh, metric);

			if (invertn)
				invert_normal(vertices, metric);
		}


		//--------------------------------------------------------------------------------------
		// Geodesic sphere
		//--------------------------------------------------------------------------------------
		template <typename MeshType>
		inline submesh_data create_geo_sphere(MeshType& mesh, float diameter , size_t tessellation , bool rhcoords )
		{
			static_assert(concept_mesh_type<MeshType>::value, "MeshType must have vertices and indices collection");
			using namespace DirectX;
			using namespace DirectX::VertexTraits;
			auto& vertices = mesh.vertices;
			auto& indices = mesh.indices;

			// this methods to is too complicated to midify and support non-empty buffer
			assert(indices.size() == 0 && vertices.size() == 0);
			indices.clear();
			vertices.clear();

			submesh_data metric;
			metric.index_offset = indices.size();
			metric.vertex_offset = vertices.size();
			size_t base = metric.vertex_offset;

			using IndexCollection = decltype(mesh.indices);
			using index_t = typename decltype(mesh.indices)::value_type;
			using vertex_t = typename decltype(mesh.vertices)::value_type;


			// An undirected edge between two vertices, represented by a pair of indexes into a vertex array.
			// Becuse this edge is undirected, (a,b) is the same as (b,a).
			typedef std::pair<index_t, index_t> UndirectedEdge;

			// Makes an undirected edge. Rather than overloading comparison operators to give us the (a,b)==(b,a) property,
			// we'll just ensure that the larger of the two goes first. This'll simplify things greatly.
			auto makeUndirectedEdge = [](index_t a, index_t b)
			{
				return std::make_pair(std::max(a, b), std::min(a, b));
			};

			// Key: an edge
			// Value: the index of the vertex which lies midway between the two vertices pointed to by the key value
			// This map is used to avoid duplicating vertices when subdividing triangles along edges.
			typedef std::map<UndirectedEdge, index_t> EdgeSubdivisionMap;


			static const XMFLOAT3A OctahedronVertices[] =
			{
				// when looking down the negative z-axis (into the screen)
				XMFLOAT3A(0,  1,  0), // 0 top
				XMFLOAT3A(0,  0, -1), // 1 front
				XMFLOAT3A(1,  0,  0), // 2 right
				XMFLOAT3A(0,  0,  1), // 3 back
				XMFLOAT3A(-1,  0,  0), // 4 left
				XMFLOAT3A(0, -1,  0), // 5 bottom
			};
			static const index_t OctahedronIndices[] =
			{
				0, 1, 2, // top front-right face
				0, 2, 3, // top back-right face
				0, 3, 4, // top back-left face
				0, 4, 1, // top front-left face
				5, 1, 4, // bottom front-left face
				5, 4, 3, // bottom back-left face
				5, 3, 2, // bottom back-right face
				5, 2, 1, // bottom front-right face
			};

			const float radius = diameter / 2.0f;

			// Start with an octahedron; copy the data into the vertex/index collection.

			std::vector<XMFLOAT3A,XMAllocator> vertexPositions(std::begin(OctahedronVertices), std::end(OctahedronVertices));

			indices.insert(indices.begin(), std::begin(OctahedronIndices), std::end(OctahedronIndices));

			auto CheckIndexOverflow = [](size_t value)
			{
				if (value > std::numeric_limits<index_t>::max())
					throw std::exception("Index value out of range: cannot tesselate primitive so finely");
			};

			// We know these values by looking at the above index list for the octahedron. Despite the subdivisions that are
			// about to go on, these values aren't ever going to change because the vertices don't move around in the array.
			// We'll need these values later on to fix the singularities that show up at the poles.
			const index_t northPoleIndex = 0;
			const index_t southPoleIndex = 5;

			for (size_t iSubdivision = 0; iSubdivision < tessellation; ++iSubdivision)
			{
				assert(indices.size() % 3 == 0); // sanity

												 // We use this to keep track of which edges have already been subdivided.
				EdgeSubdivisionMap subdividedEdges;

				// The new index collection after subdivision.
				IndexCollection newIndices;

				const size_t triangleCount = indices.size() / 3;
				for (size_t iTriangle = 0; iTriangle < triangleCount; ++iTriangle)
				{
					// For each edge on this triangle, create a new vertex in the middle of that edge.
					// The winding order of the triangles we output are the same as the winding order of the inputs.

					// Indices of the vertices making up this triangle
					index_t iv0 = indices[iTriangle * 3 + 0];
					index_t iv1 = indices[iTriangle * 3 + 1];
					index_t iv2 = indices[iTriangle * 3 + 2];

					// Get the new vertices
					XMFLOAT3 v01; // vertex on the midpoint of v0 and v1
					XMFLOAT3 v12; // ditto v1 and v2
					XMFLOAT3 v20; // ditto v2 and v0
					index_t iv01; // index of v01
					index_t iv12; // index of v12
					index_t iv20; // index of v20

								   // Function that, when given the index of two vertices, creates a new vertex at the midpoint of those vertices.
					auto divideEdge = [&](index_t i0, index_t i1, XMFLOAT3& outVertex, index_t& outIndex)
					{
						const UndirectedEdge edge = makeUndirectedEdge(i0, i1);

						// Check to see if we've already generated this vertex
						auto it = subdividedEdges.find(edge);
						if (it != subdividedEdges.end())
						{
							// We've already generated this vertex before
							outIndex = it->second; // the index of this vertex
							outVertex = vertexPositions[outIndex]; // and the vertex itself
						}
						else
						{
							// Haven't generated this vertex before: so add it now

							// outVertex = (vertices[i0] + vertices[i1]) / 2
							XMStoreFloat3(
								&outVertex,
								XMVectorScale(
									XMVectorAdd(XMLoadFloat3(&vertexPositions[i0]), XMLoadFloat3(&vertexPositions[i1])),
									0.5f
								)
							);

							CheckIndexOverflow(vertexPositions.size());

							outIndex = static_cast<index_t>(vertexPositions.size());
							vertexPositions.push_back(outVertex);

							// Now add it to the map.
							subdividedEdges.insert(std::make_pair(edge, outIndex));
						}
					};

					// Add/get new vertices and their indices
					divideEdge(iv0, iv1, v01, iv01);
					divideEdge(iv1, iv2, v12, iv12);
					divideEdge(iv0, iv2, v20, iv20);

					// Add the new indices. We have four new triangles from our original one:
					//        v0
					//        o
					//       /a\
					//  v20 o---o v01
					//     /b\c/d\
					// v2 o---o---o v1
					//       v12
					const index_t indicesToAdd[] =
					{
						iv0, iv01, iv20, // a
						iv20, iv12,  iv2, // b
						iv20, iv01, iv12, // c
						iv01,  iv1, iv12, // d
					};
					newIndices.insert(newIndices.end(), std::begin(indicesToAdd), std::end(indicesToAdd));
				}

				indices = std::move(newIndices);
			}

			// Now that we've completed subdivision, fill in the final vertex collection
			vertices.reserve(vertexPositions.size());
			for (auto it = vertexPositions.begin(); it != vertexPositions.end(); ++it)
			{
				auto vertexValue = *it;

				auto normal = XMVector3Normalize(XMLoadFloat3(&vertexValue));
				auto pos = XMVectorScale(normal, radius);

				XMFLOAT3 normalFloat3;
				XMStoreFloat3(&normalFloat3, normal);

				// calculate texture coordinates for this vertex
				float longitude = atan2(normalFloat3.x, -normalFloat3.z);
				float latitude = acos(normalFloat3.y);

				float u = longitude / XM_2PI + 0.5f;
				float v = latitude / XM_PI;

				auto texcoord = XMVectorSet(1.0f - u, v, 0.0f, 0.0f);
				push_vertex(vertices,pos, normal, texcoord);
			}

			// There are a couple of fixes to do. One is a texture coordinate wraparound fixup. At some point, there will be
			// a set of triangles somewhere in the mesh with texture coordinates such that the wraparound across 0.0/1.0
			// occurs across that triangle. Eg. when the left hand side of the triangle has a U coordinate of 0.98 and the
			// right hand side has a U coordinate of 0.0. The intent is that such a triangle should render with a U of 0.98 to
			// 1.0, not 0.98 to 0.0. If we don't do this fixup, there will be a visible seam across one side of the sphere.
			// 
			// Luckily this is relatively easy to fix. There is a straight edge which runs down the prime meridian of the
			// completed sphere. If you imagine the vertices along that edge, they circumscribe a semicircular arc starting at
			// y=1 and ending at y=-1, and sweeping across the range of z=0 to z=1. x stays zero. It's along this edge that we
			// need to duplicate our vertices - and provide the correct texture coordinates.
			size_t preFixupVertexCount = vertices.size();
			for (size_t i = 0; i < preFixupVertexCount; ++i)
			{
				// This vertex is on the prime meridian if position.x and texcoord.u are both zero (allowing for small epsilon).
				bool isOnPrimeMeridian = XMVector2NearEqual(
					XMVectorSet(vertices[i].position.x, vertices[i].textureCoordinate.x, 0.0f, 0.0f),
					XMVectorZero(),
					XMVectorSplatEpsilon());

				if (isOnPrimeMeridian)
				{
					size_t newIndex = vertices.size(); // the index of this vertex that we're about to add
					CheckIndexOverflow(newIndex);

					// copy this vertex, correct the texture coordinate, and add the vertex
					vertex_t v = vertices[i];
					set_uv(v,XMVectorSetX(get_uv(v)1.0)); //v.textureCoordinate.x = 1.0f;
					vertices.push_back(v);

					// Now find all the triangles which contain this vertex and update them if necessary
					for (size_t j = 0; j < indices.size(); j += 3)
					{
						auto* triIndex0 = &indices[j + 0];
						auto* triIndex1 = &indices[j + 1];
						auto* triIndex2 = &indices[j + 2];

						if (*triIndex0 == i)
						{
							// nothing; just keep going
						}
						else if (*triIndex1 == i)
						{
							std::swap(triIndex0, triIndex1); // swap the pointers (not the values)
						}
						else if (*triIndex2 == i)
						{
							std::swap(triIndex0, triIndex2); // swap the pointers (not the values)
						}
						else
						{
							// this triangle doesn't use the vertex we're interested in
							continue;
						}

						// If we got to this point then triIndex0 is the pointer to the index to the vertex we're looking at
						assert(*triIndex0 == i);
						assert(*triIndex1 != i && *triIndex2 != i); // assume no degenerate triangles

						const auto& v0 = vertices[*triIndex0];
						const auto& v1 = vertices[*triIndex1];
						const auto& v2 = vertices[*triIndex2];

						// check the other two vertices to see if we might need to fix this triangle

						if (abs(v0.textureCoordinate.x - v1.textureCoordinate.x) > 0.5f ||
							abs(v0.textureCoordinate.x - v2.textureCoordinate.x) > 0.5f)
						{
							// yep; replace the specified index to point to the new, corrected vertex
							*triIndex0 = static_cast<index_t>(newIndex);
						}
					}
				}
			}

			// And one last fix we need to do: the poles. A common use-case of a sphere mesh is to map a rectangular texture onto
			// it. If that happens, then the poles become singularities which map the entire top and bottom rows of the texture
			// onto a single point. In general there's no real way to do that right. But to match the behavior of non-geodesic
			// spheres, we need to duplicate the pole vertex for every triangle that uses it. This will introduce seams near the
			// poles, but reduce stretching.
			auto fixPole = [&](size_t poleIndex)
			{
				auto poleVertex = vertices[poleIndex];
				bool overwrittenPoleVertex = false; // overwriting the original pole vertex saves us one vertex

				for (size_t i = 0; i < indices.size(); i += 3)
				{
					// These pointers point to the three indices which make up this triangle. pPoleIndex is the pointer to the
					// entry in the index array which represents the pole index, and the other two pointers point to the other
					// two indices making up this triangle.
					index_t* pPoleIndex;
					index_t* pOtherIndex0;
					index_t* pOtherIndex1;
					if (indices[i + 0] == poleIndex)
					{
						pPoleIndex = &indices[i + 0];
						pOtherIndex0 = &indices[i + 1];
						pOtherIndex1 = &indices[i + 2];
					}
					else if (indices[i + 1] == poleIndex)
					{
						pPoleIndex = &indices[i + 1];
						pOtherIndex0 = &indices[i + 2];
						pOtherIndex1 = &indices[i + 0];
					}
					else if (indices[i + 2] == poleIndex)
					{
						pPoleIndex = &indices[i + 2];
						pOtherIndex0 = &indices[i + 0];
						pOtherIndex1 = &indices[i + 1];
					}
					else
					{
						continue;
					}

					const auto& otherVertex0 = vertices[*pOtherIndex0];
					const auto& otherVertex1 = vertices[*pOtherIndex1];

					// Calculate the texcoords for the new pole vertex, add it to the vertices and update the index
					VertexPositionNormalTexture newPoleVertex = poleVertex;
					newPoleVertex.textureCoordinate.x = (otherVertex0.textureCoordinate.x + otherVertex1.textureCoordinate.x) / 2;
					newPoleVertex.textureCoordinate.y = poleVertex.textureCoordinate.y;

					if (!overwrittenPoleVertex)
					{
						vertices[poleIndex] = newPoleVertex;
						overwrittenPoleVertex = true;
					}
					else
					{
						CheckIndexOverflow(vertices.size());

						*pPoleIndex = static_cast<index_t>(vertices.size());
						vertices.push_back(newPoleVertex);
					}
				}
			};

			fixPole(northPoleIndex);
			fixPole(southPoleIndex);

			metric.index_count = indices.size() - metric.index_offset;
			metric.vertex_count = vertices.size() - metric.vertex_offset;

			// Build RH above
			if (!rhcoords)
				reverse_winding(mesh, metric);
		}


		//--------------------------------------------------------------------------------------
		// Cylinder / Cone
		//--------------------------------------------------------------------------------------

		// Helper computes a point on a unit circle, aligned to the x/z plane and centered on the origin.
		static inline XMVECTOR GetCircleVector(size_t i, size_t tessellation)
		{
			using namespace DirectX;
			float angle = i * XM_2PI / tessellation;
			float dx, dz;

			XMScalarSinCos(&dx, &dz, angle);

			XMVECTORF32 v = { dx, 0, dz, 0 };
			return v;
		}

		static inline XMVECTOR GetCircleTangent(size_t i, size_t tessellation)
		{
			using namespace DirectX;
			float angle = (i * XM_2PI / tessellation) + XM_PIDIV2;
			float dx, dz;

			XMScalarSinCos(&dx, &dz, angle);

			XMVECTORF32 v = { dx, 0, dz, 0 };
			return v;
		}


		// Helper creates a triangle fan to close the end of a cylinder / cone
		template <typename MeshType>
		inline submesh_data create_cylinder_cap(MeshType& mesh, size_t tessellation, float height, float radius, bool isTop)
		{
			// Create cap indices.
			for (size_t i = 0; i < tessellation - 2; i++)
			{
				size_t i1 = (i + 1) % tessellation;
				size_t i2 = (i + 2) % tessellation;

				if (isTop)
				{
					std::swap(i1, i2);
				}

				size_t vbase = vertices.size();
				push_index(indices, vbase);
				push_index(indices, vbase + i1);
				push_index(indices, vbase + i2);
			}

			// Which end of the cylinder is this?
			XMVECTOR normal = g_XMIdentityR1;
			XMVECTOR textureScale = g_XMNegativeOneHalf;

			if (!isTop)
			{
				normal = -normal;
				textureScale *= g_XMNegateX;
			}

			// Create cap vertices.
			for (size_t i = 0; i < tessellation; i++)
			{
				XMVECTOR circleVector = GetCircleVector(i, tessellation);

				XMVECTOR position = (circleVector * radius) + (normal * height);

				XMVECTOR textureCoordinate = XMVectorMultiplyAdd(XMVectorSwizzle<0, 2, 3, 3>(circleVector), textureScale, g_XMOneHalf);

				push_vertex(vertices,position, normal, textureCoordinate);
			}
		}


		// Creates a cylinder primitive.
		template <typename MeshType>
		inline submesh_data create_cylinder(MeshType& mesh, float height, float diameter, size_t tessellation, bool rhcoords)
		{
			static_assert(concept_mesh_type<MeshType>::value, "MeshType must have vertices and indices collection");
			using namespace DirectX;
			auto& vertices = mesh.vertices;
			auto& indices = mesh.indices;
			submesh_data metric;
			metric.index_offset = indices.size();
			metric.vertex_offset = vertices.size();
			size_t base = metric.vertex_offset;

			if (tessellation < 3)
				throw std::out_of_range("tesselation parameter out of range");

			height /= 2;

			XMVECTOR topOffset = g_XMIdentityR1 * height;

			float radius = diameter / 2;
			size_t stride = tessellation + 1;

			// Create a ring of triangles around the outside of the cylinder.
			for (size_t i = 0; i <= tessellation; i++)
			{
				XMVECTOR normal = GetCircleVector(i, tessellation);

				XMVECTOR sideOffset = normal * radius;

				float u = (float)i / tessellation;

				XMVECTOR textureCoordinate = XMLoadFloat(&u);

				push_vertex(vertices,sideOffset + topOffset, normal, textureCoordinate);
				push_vertex(vertices,sideOffset - topOffset, normal, textureCoordinate + g_XMIdentityR1);

				push_index(indices, base + i * 2);
				push_index(indices, base + (i * 2 + 2) % (stride * 2));
				push_index(indices, base + i * 2 + 1);

				push_index(indices, base + i * 2 + 1);
				push_index(indices, base + (i * 2 + 2) % (stride * 2));
				push_index(indices, base + (i * 2 + 3) % (stride * 2));
			}

			// Create flat triangle fan caps to seal the top and bottom.
			create_cylinder_cap(vertices, indices, tessellation, height, radius, true);
			create_cylinder_cap(vertices, indices, tessellation, height, radius, false);


			metric.index_count = indices.size() - metric.index_offset;
			metric.vertex_count = vertices.size() - metric.vertex_offset;

			// Build RH above
			if (!rhcoords)
				reverse_winding(mesh, metric);
		}


		template <typename MeshType>
		inline submesh_data create_cone(MeshType& mesh, float diameter , float height , size_t tessellation , bool rhcoords)
		{
			static_assert(concept_mesh_type<MeshType>::value, "MeshType must have vertices and indices collection");
			using namespace DirectX;
			auto& vertices = mesh.vertices;
			auto& indices = mesh.indices;
			submesh_data metric;
			metric.index_offset = indices.size();
			metric.vertex_offset = vertices.size();
			size_t base = metric.vertex_offset;

			if (tessellation < 3)
				throw std::out_of_range("tesselation parameter out of range");

			height /= 2;

			XMVECTOR topOffset = g_XMIdentityR1 * height;

			float radius = diameter / 2;
			size_t stride = tessellation + 1;

			// Create a ring of triangles around the outside of the cone.
			for (size_t i = 0; i <= tessellation; i++)
			{
				XMVECTOR circlevec = GetCircleVector(i, tessellation);

				XMVECTOR sideOffset = circlevec * radius;

				float u = (float)i / tessellation;

				XMVECTOR textureCoordinate = XMLoadFloat(&u);

				XMVECTOR pt = sideOffset - topOffset;

				XMVECTOR normal = XMVector3Cross(GetCircleTangent(i, tessellation), topOffset - pt);
				normal = XMVector3Normalize(normal);

				// Duplicate the top vertex for distinct normals
				push_vertex(vertices,topOffset, normal, g_XMZero);
				push_vertex(vertices,pt, normal, textureCoordinate + g_XMIdentityR1);

				push_index(indices, base + i * 2);
				push_index(indices, base + (i * 2 + 3) % (stride * 2));
				push_index(indices, base + (i * 2 + 1) % (stride * 2));
			}

			// Create flat triangle fan caps to seal the bottom.
			CreateCylinderCap(vertices, indices, tessellation, height, radius, false);

			metric.index_count = indices.size() - metric.index_offset;
			metric.vertex_count = vertices.size() - metric.vertex_offset;

			// Build RH above
			if (!rhcoords)
				reverse_winding(mesh, metric);
			return metric;
		}


		//--------------------------------------------------------------------------------------
		// Torus
		//--------------------------------------------------------------------------------------

		template <typename MeshType>
		inline submesh_data create_torus(MeshType& mesh, float diameter , float thickness , size_t tessellation , bool rhcoords )
		{
			static_assert(concept_mesh_type<MeshType>::value, "MeshType must have vertices and indices collection");
			using namespace DirectX;
			auto& vertices = mesh.vertices;
			auto& indices = mesh.indices;
			submesh_data metric;
			metric.index_offset = indices.size();
			metric.vertex_offset = vertices.size();
			size_t base = metric.vertex_offset;

			if (tessellation < 3)
				throw std::out_of_range("tesselation parameter out of range");

			size_t stride = tessellation + 1;

			// First we loop around the main ring of the torus.
			for (size_t i = 0; i <= tessellation; i++)
			{
				float u = (float)i / tessellation;

				float outerAngle = i * XM_2PI / tessellation - XM_PIDIV2;

				// Create a transform matrix that will align geometry to
				// slice perpendicularly though the current ring position.
				XMMATRIX transform = XMMatrixTranslation(diameter / 2, 0, 0) * XMMatrixRotationY(outerAngle);

				// Now we loop along the other axis, around the side of the tube.
				for (size_t j = 0; j <= tessellation; j++)
				{
					float v = 1 - (float)j / tessellation;

					float innerAngle = j * XM_2PI / tessellation + XM_PI;
					float dx, dy;

					XMScalarSinCos(&dy, &dx, innerAngle);

					// Create a vertex.
					XMVECTOR normal = XMVectorSet(dx, dy, 0, 0);
					XMVECTOR position = normal * thickness / 2;
					XMVECTOR textureCoordinate = XMVectorSet(u, v, 0, 0);

					position = XMVector3Transform(position, transform);
					normal = XMVector3TransformNormal(normal, transform);

					push_vertex(vertices,position, normal, textureCoordinate);

					// And create indices for two triangles.
					size_t nextI = (i + 1) % stride;
					size_t nextJ = (j + 1) % stride;

					push_index(indices, base + i * stride + j);
					push_index(indices, base + i * stride + nextJ);
					push_index(indices, base + nextI * stride + j);

					push_index(indices, base + i * stride + nextJ);
					push_index(indices, base + nextI * stride + nextJ);
					push_index(indices, base + nextI * stride + j);
				}
			}

			// Build RH above
			metric.index_count = indices.size() - metric.index_offset;
			metric.vertex_count = vertices.size() - metric.vertex_offset;

			// Build RH above
			if (!rhcoords)
				reverse_winding(mesh, metric);

			return metric;
		}


		//--------------------------------------------------------------------------------------
		// Tetrahedron
		//--------------------------------------------------------------------------------------
		template <typename MeshType>
		inline submesh_data create_tetrahedron(MeshType& mesh, float size , bool rhcoords )
		{
			static_assert(concept_mesh_type<MeshType>::value, "MeshType must have vertices and indices collection");
			using namespace DirectX;
			auto& vertices = mesh.vertices;
			auto& indices = mesh.indices;
			submesh_data metric;
			metric.index_offset = indices.size();
			metric.vertex_offset = vertices.size();

			static const XMVECTORF32 verts[4] =
			{
				{ 0.f,        0.f,      1.f },
				{ 2.f*SQRT2 / 3.f,        0.f, -1.f / 3.f },
				{ -SQRT2 / 3.f,  SQRT6 / 3.f, -1.f / 3.f },
				{ -SQRT2 / 3.f, -SQRT6 / 3.f, -1.f / 3.f }
			};

			static const uint32_t faces[4 * 3] =
			{
				0, 1, 2,
				0, 2, 3,
				0, 3, 1,
				1, 3, 2,
			};

			for (size_t j = 0; j < _countof(faces); j += 3)
			{
				uint32_t v0 = faces[j];
				uint32_t v1 = faces[j + 1];
				uint32_t v2 = faces[j + 2];

				XMVECTOR normal = XMVector3Cross(verts[v1].v - verts[v0].v,
					verts[v2].v - verts[v0].v);
				normal = XMVector3Normalize(normal);

				size_t base = vertices.size();
				push_index(indices, base);
				push_index(indices, base + 1);
				push_index(indices, base + 2);

				// Duplicate vertices to use face normals
				XMVECTOR position = XMVectorScale(verts[v0], size);
				push_vertex(vertices,position, normal, g_XMZero /* 0, 0 */);

				position = XMVectorScale(verts[v1], size);
				push_vertex(vertices,position, normal, g_XMIdentityR0 /* 1, 0 */);

				position = XMVectorScale(verts[v2], size);
				push_vertex(vertices,position, normal, g_XMIdentityR1 /* 0, 1 */);
			}

			metric.index_count = indices.size() - metric.index_offset;
			metric.vertex_count = vertices.size() - metric.vertex_offset;

			// Built LH above
			if (rhcoords)
				reverse_winding(mesh, metric);


			assert(vertices.size() == 4 * 3);
			assert(indices.size() == 4 * 3);
			return metric;
		}


		//--------------------------------------------------------------------------------------
		// Octahedron
		//--------------------------------------------------------------------------------------
		template <typename MeshType>
		inline submesh_data create_octahedron(MeshType& mesh, float size , bool rhcoords )
		{
			static_assert(concept_mesh_type<MeshType>::value, "MeshType must have vertices and indices collection");
			using namespace DirectX;
			auto& vertices = mesh.vertices;
			auto& indices = mesh.indices;
			submesh_data metric;
			metric.index_offset = indices.size();
			metric.vertex_offset = vertices.size();

			static const XMVECTORF32 verts[6] =
			{
				{ 1,  0,  0 },
				{ -1,  0,  0 },
				{ 0,  1,  0 },
				{ 0, -1,  0 },
				{ 0,  0,  1 },
				{ 0,  0, -1 }
			};

			static const uint32_t faces[8 * 3] =
			{
				4, 0, 2,
				4, 2, 1,
				4, 1, 3,
				4, 3, 0,
				5, 2, 0,
				5, 1, 2,
				5, 3, 1,
				5, 0, 3
			};

			for (size_t j = 0; j < _countof(faces); j += 3)
			{
				uint32_t v0 = faces[j];
				uint32_t v1 = faces[j + 1];
				uint32_t v2 = faces[j + 2];

				XMVECTOR normal = XMVector3Cross(verts[v1].v - verts[v0].v,
					verts[v2].v - verts[v0].v);
				normal = XMVector3Normalize(normal);

				size_t base = vertices.size();
				push_index(indices, base);
				push_index(indices, base + 1);
				push_index(indices, base + 2);

				// Duplicate vertices to use face normals
				XMVECTOR position = XMVectorScale(verts[v0], size);
				push_vertex(vertices,position, normal, g_XMZero /* 0, 0 */);

				position = XMVectorScale(verts[v1], size);
				push_vertex(vertices,position, normal, g_XMIdentityR0 /* 1, 0 */);

				position = XMVectorScale(verts[v2], size);
				push_vertex(vertices,position, normal, g_XMIdentityR1 /* 0, 1*/);
			}

			metric.index_count = indices.size() - metric.index_offset;
			metric.vertex_count = vertices.size() - metric.vertex_offset;

			// Built LH above
			if (rhcoords)
				reverse_winding(mesh, metric);

			assert(vertices.size() == 8 * 3);
			assert(indices.size() == 8 * 3);
			return metric;
		}


		//--------------------------------------------------------------------------------------
		// Dodecahedron
		//--------------------------------------------------------------------------------------
		template <typename MeshType>
		inline submesh_data create_dodecahedron(MeshType& mesh, float size , bool rhcoords )
		{
			static_assert(concept_mesh_type<MeshType>::value, "MeshType must have vertices and indices collection");
			using namespace DirectX;
			auto& vertices = mesh.vertices;
			auto& indices = mesh.indices;
			submesh_data metric;
			metric.index_offset = indices.size();
			metric.vertex_offset = vertices.size();

			static const float a = 1.f / SQRT3;
			static const float b = 0.356822089773089931942f; // sqrt( ( 3 - sqrt(5) ) / 6 )
			static const float c = 0.934172358962715696451f; // sqrt( ( 3 + sqrt(5) ) / 6 );

			static const XMVECTORF32 verts[20] =
			{
				{ a,  a,  a },
				{ a,  a, -a },
				{ a, -a,  a },
				{ a, -a, -a },
				{ -a,  a,  a },
				{ -a,  a, -a },
				{ -a, -a,  a },
				{ -a, -a, -a },
				{ b,  c,  0 },
				{ -b,  c,  0 },
				{ b, -c,  0 },
				{ -b, -c,  0 },
				{ c,  0,  b },
				{ c,  0, -b },
				{ -c,  0,  b },
				{ -c,  0, -b },
				{ 0,  b,  c },
				{ 0, -b,  c },
				{ 0,  b, -c },
				{ 0, -b, -c }
			};

			static const uint32_t faces[12 * 5] =
			{
				0, 8, 9, 4, 16,
				0, 16, 17, 2, 12,
				12, 2, 10, 3, 13,
				9, 5, 15, 14, 4,
				3, 19, 18, 1, 13,
				7, 11, 6, 14, 15,
				0, 12, 13, 1, 8,
				8, 1, 18, 5, 9,
				16, 4, 14, 6, 17,
				6, 11, 10, 2, 17,
				7, 15, 5, 18, 19,
				7, 19, 3, 10, 11,
			};

			static const XMVECTORF32 textureCoordinates[5] =
			{
				{ 0.654508f, 0.0244717f },
				{ 0.0954915f,  0.206107f },
				{ 0.0954915f,  0.793893f },
				{ 0.654508f,  0.975528f },
				{ 1.f,       0.5f }
			};

			static const uint32_t textureIndex[12][5] =
			{
				{ 0, 1, 2, 3, 4 },
				{ 2, 3, 4, 0, 1 },
				{ 4, 0, 1, 2, 3 },
				{ 1, 2, 3, 4, 0 },
				{ 2, 3, 4, 0, 1 },
				{ 0, 1, 2, 3, 4 },
				{ 1, 2, 3, 4, 0 },
				{ 4, 0, 1, 2, 3 },
				{ 4, 0, 1, 2, 3 },
				{ 1, 2, 3, 4, 0 },
				{ 0, 1, 2, 3, 4 },
				{ 2, 3, 4, 0, 1 },
			};

			size_t t = 0;
			for (size_t j = 0; j < _countof(faces); j += 5, ++t)
			{
				uint32_t v0 = faces[j];
				uint32_t v1 = faces[j + 1];
				uint32_t v2 = faces[j + 2];
				uint32_t v3 = faces[j + 3];
				uint32_t v4 = faces[j + 4];

				XMVECTOR normal = XMVector3Cross(verts[v1].v - verts[v0].v,
					verts[v2].v - verts[v0].v);
				normal = XMVector3Normalize(normal);

				size_t base = vertices.size();

				push_index(indices, base);
				push_index(indices, base + 1);
				push_index(indices, base + 2);

				push_index(indices, base);
				push_index(indices, base + 2);
				push_index(indices, base + 3);

				push_index(indices, base);
				push_index(indices, base + 3);
				push_index(indices, base + 4);

				// Duplicate vertices to use face normals
				XMVECTOR position = XMVectorScale(verts[v0], size);
				push_vertex(vertices,position, normal, textureCoordinates[textureIndex[t][0]]);

				position = XMVectorScale(verts[v1], size);
				push_vertex(vertices,position, normal, textureCoordinates[textureIndex[t][1]]);

				position = XMVectorScale(verts[v2], size);
				push_vertex(vertices,position, normal, textureCoordinates[textureIndex[t][2]]);

				position = XMVectorScale(verts[v3], size);
				push_vertex(vertices,position, normal, textureCoordinates[textureIndex[t][3]]);

				position = XMVectorScale(verts[v4], size);
				push_vertex(vertices,position, normal, textureCoordinates[textureIndex[t][4]]);
			}

			metric.index_count = indices.size() - metric.index_offset;
			metric.vertex_count = vertices.size() - metric.vertex_offset;

			// Built LH above
			if (rhcoords)
				reverse_winding(mesh, metric);

			assert(vertices.size() == 12 * 5);
			assert(indices.size() == 12 * 3 * 3);
			return metric;
		}


		//--------------------------------------------------------------------------------------
		// Icosahedron
		//--------------------------------------------------------------------------------------
		template <typename MeshType>
		inline submesh_data create_icosahedron(MeshType& mesh, float size , bool rhcoords )
		{
			static_assert(concept_mesh_type<MeshType>::value, "MeshType must have vertices and indices collection");
			using namespace DirectX;
			auto& vertices = mesh.vertices;
			auto& indices = mesh.indices;
			submesh_data metric;
			metric.index_offset = indices.size();
			metric.vertex_offset = vertices.size();

			static const float  t = 1.618033988749894848205f; // (1 + sqrt(5)) / 2
			static const float t2 = 1.519544995837552493271f; // sqrt( 1 + sqr( (1 + sqrt(5)) / 2 ) )

			static const XMVECTORF32 verts[12] =
			{
				{ t / t2,  1.f / t2,       0 },
				{ -t / t2,  1.f / t2,       0 },
				{ t / t2, -1.f / t2,       0 },
				{ -t / t2, -1.f / t2,       0 },
				{ 1.f / t2,       0,    t / t2 },
				{ 1.f / t2,       0,   -t / t2 },
				{ -1.f / t2,       0,    t / t2 },
				{ -1.f / t2,       0,   -t / t2 },
				{ 0,    t / t2,  1.f / t2 },
				{ 0,   -t / t2,  1.f / t2 },
				{ 0,    t / t2, -1.f / t2 },
				{ 0,   -t / t2, -1.f / t2 }
			};

			static const uint32_t faces[20 * 3] =
			{
				0, 8, 4,
				0, 5, 10,
				2, 4, 9,
				2, 11, 5,
				1, 6, 8,
				1, 10, 7,
				3, 9, 6,
				3, 7, 11,
				0, 10, 8,
				1, 8, 10,
				2, 9, 11,
				3, 11, 9,
				4, 2, 0,
				5, 0, 2,
				6, 1, 3,
				7, 3, 1,
				8, 6, 4,
				9, 4, 6,
				10, 5, 7,
				11, 7, 5
			};

			for (size_t j = 0; j < _countof(faces); j += 3)
			{
				uint32_t v0 = faces[j];
				uint32_t v1 = faces[j + 1];
				uint32_t v2 = faces[j + 2];

				XMVECTOR normal = XMVector3Cross(verts[v1].v - verts[v0].v,
					verts[v2].v - verts[v0].v);
				normal = XMVector3Normalize(normal);

				size_t base = vertices.size();
				push_index(indices, base);
				push_index(indices, base + 1);
				push_index(indices, base + 2);

				// Duplicate vertices to use face normals
				XMVECTOR position = XMVectorScale(verts[v0], size);
				push_vertex(vertices,position, normal, g_XMZero /* 0, 0 */);

				position = XMVectorScale(verts[v1], size);
				push_vertex(vertices,position, normal, g_XMIdentityR0 /* 1, 0 */);

				position = XMVectorScale(verts[v2], size);
				push_vertex(vertices,position, normal, g_XMIdentityR1 /* 0, 1 */);
			}

			// Built LH above
			metric.index_count = indices.size() - metric.index_offset;
			metric.vertex_count = vertices.size() - metric.vertex_offset;

			if (rhcoords)
				reverse_winding(mesh, metric);

			assert(vertices.size() == 20 * 3);
			assert(indices.size() == 20 * 3);

			return metric;
		}

	}
}