#pragma once

// Original CSG.JS library by Evan Wallace (http://madebyevan.com), under the MIT license.
// GitHub: https://github.com/evanw/csg.js/
// 
// C++ port by Tomasz Dabrowski (http://28byteslater.com), under the MIT license.
// GitHub: https://github.com/dabroz/csgjs-cpp/
// 
// Constructive Solid Geometry (CSG) is a modeling technique that uses Boolean
// operations like union and intersection to combine 3D solids. This library
// implements CSG operations on meshes elegantly and concisely using BSP trees,
// and is meant to serve as an easily understandable implementation of the
// algorithm. All edge cases involving overlapping coplanar polygons in both
// solids are correctly handled.
//
// To use this as a header file, define CSGJS_HEADER_ONLY before including this file.
//

#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <memory>
#include "TriangleMesh.h"
#include <hlslm/hlsl.hpp>

namespace Geometrics
{
	namespace csg
	{
		using Vector3 = DirectX::hlsl::xmvector3f;
		using Vector4 = DirectX::hlsl::xmvector4f;
		using FloatV = DirectX::hlsl::xmfloat;

		struct XM_ALIGNATTR Vertex
		{
			Vector3 position;
			Vector3 normal;
		};

		typedef std::vector<Vertex, DirectX::XMAllocator> VertexCollection;

		struct Plane;
		struct ConvexPolygon;
		struct BSPNode;

		typedef std::vector<ConvexPolygon, DirectX::XMAllocator> ConvexPolygonCollection;

		// Represents a plane in 3D space.
		struct XM_ALIGNATTR Plane
		{
			Vector3 normal;
			FloatV  w;

			Plane();
			Plane(const Vector3 & a, const Vector3 & b, const Vector3 & c);
			bool ok() const;
			void flip();
			void splitPolygon(const ConvexPolygon & polygon, ConvexPolygonCollection & coplanarFront, ConvexPolygonCollection & coplanarBack, ConvexPolygonCollection & front, ConvexPolygonCollection & back) const;
		};


		// Represents a convex polygon. The vertices used to initialize a polygon must
		// be coplanar and form a convex loop. They do not have to be `CSG.Vertex`
		// instances but they must behave similarly (duck typing can be used for
		// customization).
		// 
		// Each convex polygon has a `shared` property, which is shared between all
		// polygons that are clones of each other or were split from the same polygon.
		// This can be used to define per-polygon properties (such as surface color).
		struct ConvexPolygon
		{
			VertexCollection vertices;
			Plane plane;
			void flip();

			ConvexPolygon();
			ConvexPolygon(const VertexCollection & list);
			ConvexPolygon(VertexCollection && list);
		};

		// Holds a node in a BSP tree. A BSP tree is built from a collection of polygons
		// by picking a polygon to split along. That polygon (and all other coplanar
		// polygons) are added directly to that node and the other polygons are added to
		// the front and/or back subtrees. This is not a leafy BSP tree since there is
		// no distinction between internal and leaf nodes.
		struct XM_ALIGNATTR BSPNode : public DirectX::AlignedNew<Plane>
		{
			Plane plane;
			ConvexPolygonCollection polygons;
			BSPNode * front;
			BSPNode * back;

			BSPNode();
			BSPNode(const ConvexPolygonCollection & list);
			~BSPNode();

			BSPNode * clone() const;
			void clipTo(const BSPNode * other);
			void invert();

			void build(const ConvexPolygonCollection & polygon);

			ConvexPolygonCollection clipPolygons(const ConvexPolygonCollection & list) const;
			ConvexPolygonCollection allPolygons() const;

			// interaction with mesh
			template <typename _VertexType, typename _IndexType>
			inline void convertToMesh(TriangleMesh<_VertexType, _IndexType> & mesh) const
			{
				mesh = ModelFromPolygons<_VertexType, _IndexType>(this->allPolygons());
			}

			template <typename _VertexType, typename _IndexType = uint16_t, typename _FaceType>
			BSPNode(const PolygonSoup<_VertexType, _IndexType, _FaceType> & model)
				:BSPNode(ModelToPolygons(model))
			{}

			static std::unique_ptr<BSPNode> create(const ConvexPolygonCollection & list)
			{
				std::unique_ptr<BSPNode> ptr(new BSPNode(list));
				return ptr;
			}

			template <typename _VertexType, typename _IndexType = uint16_t, typename _FaceType>
			static std::unique_ptr<BSPNode> create(const PolygonSoup<_VertexType, _IndexType, _FaceType> & model)
			{
				std::unique_ptr<BSPNode> ptr(new BSPNode(model));
				return ptr;
			}
		};

		typedef uint16_t Index;

		typedef BSPNode * nodeFunction(const BSPNode * a1, const BSPNode * b1);

		//struct CsgNode
		//{
		//private:
		//	BSPNode * node; // internal representation

		//public:
		//	CsgNode() : node(nullptr), ref_count(0) {}
		//	CsgNode(BSPNode* _node) : node(_node), ref_count(1){}
		//	~CsgNode() { destroy_node(node); }

		//	operator BSPNode*() { return node; }
		//	operator const BSPNode*() const { return node; }

		//	static CsgNode Intesect(const CsgNode a, const CsgNode b)
		//	{}
		//	static CsgNode Union(const CsgNode a, const CsgNode b)
		//	{}
		//	static CsgNode Subtract(const CsgNode a, const CsgNode b)
		//	{}
		//};

		// Public interface implementation
		template <typename _VertexType, typename _IndexType = uint16_t, typename _FaceType>
		inline ConvexPolygonCollection ModelToPolygons(const PolygonSoup<_VertexType, _IndexType, _FaceType> & model)
		{
			using namespace DirectX::VertexTraits;
			ConvexPolygonCollection list;
			Vertex v;
			for (size_t i = 0; i < model.indices.size(); i += 3)
			{
				VertexCollection triangle;
				for (int j = 0; j < _FaceType::VertexCount; j++)
				{
					convert_vertex(model.vertices[model.indices[i + j]], v); // v = model.vertices[model.indices[i + j]]
					triangle.push_back(v);
				}
				list.push_back(ConvexPolygon(std::move(triangle)));
			}
			return list;
		}

		template <typename _VertexType, typename _IndexType>

		inline TriangleMesh<_VertexType, _IndexType> ModelFromPolygons(const ConvexPolygonCollection & polygons)
		{
			using namespace DirectX::VertexTraits;
			typedef TriangleMesh<_VertexType, _IndexType> MeshType;
			MeshType model;
			int p = 0;
			_VertexType v;
			for (size_t i = 0; i < polygons.size(); i++)
			{
				const ConvexPolygon & poly = polygons[i];
				for (size_t j = 2; j < poly.vertices.size(); j++)
				{
					convert_vertex(poly.vertices[0], v);
					model.vertices.push_back(v);
					model.indices.push_back(p++);
					convert_vertex(poly.vertices[j - 1], v);
					model.vertices.push_back(v);
					model.indices.push_back(p++);
					convert_vertex(poly.vertices[j], v);
					model.vertices.push_back(v);
					model.indices.push_back(p++);
				}
			}
			return model;
		}

		BSPNode * nodeIntersect(const BSPNode * a1, const BSPNode * b1);
		BSPNode * nodeUnion(const BSPNode * a1, const BSPNode * b1);
		BSPNode * nodeSubtract(const BSPNode * a1, const BSPNode * b1);

		template <typename _VertexType, typename _IndexType>
		inline static TriangleMesh<_VertexType, _IndexType> meshOperation(const TriangleMesh<_VertexType, _IndexType> & a, const TriangleMesh<_VertexType, _IndexType> & b, nodeFunction fun)
		{
			using node_ptr = std::unique_ptr<BSPNode>;
			node_ptr A(a);
			node_ptr B(b);
			node_ptr AB(fun(A, B));
			return AB->convertToMesh();
		}
	}

	// public interface - not super efficient, if you use multiple CSG operations you should
	// use BSP trees and convert them into model only once. Another optimization trick is
	// replacing MeshType with your own class.

	template <typename _VertexType, typename _IndexType>
	TriangleMesh<_VertexType, _IndexType> mesh_union(const TriangleMesh<_VertexType, _IndexType> & a, const TriangleMesh<_VertexType, _IndexType> & b)
	{
		return csg::meshOperation(a, b, csg::nodeUnion);
	}

	template <typename _VertexType, typename _IndexType>
	TriangleMesh<_VertexType, _IndexType> mesh_intersection(const TriangleMesh<_VertexType, _IndexType> & a, const TriangleMesh<_VertexType, _IndexType> & b)
	{
		return csg::meshOperation(a, b, csg::nodeIntersect);
	}

	template <typename _VertexType, typename _IndexType>
	TriangleMesh<_VertexType, _IndexType> mesh_difference(const TriangleMesh<_VertexType, _IndexType> & a, const TriangleMesh<_VertexType, _IndexType> & b)
	{
		return csg::meshOperation(a, b, csg::nodeSubtract);
	}
}