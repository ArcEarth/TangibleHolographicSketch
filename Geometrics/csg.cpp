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

#include "csg.h"
using namespace DirectX;
using namespace DirectX::hlsl;

namespace Geometrics
{
	namespace csg
	{
		// IMPLEMENTATION BELOW ---------------------------------------------------------------------------

#ifndef CSGJS_HEADER_ONLY

		// Vertex implementation
		// Invert all orientation-specific data (e.g. vertex normal). Called when the
		// orientation of a polygon is flipped.
		inline static Vertex XM_CALLCONV flip(Vertex v)
		{
			XMVECTOR p = XMVectorNegate(XMLoadA(v.position));
			XMStoreA(v.normal, p);
			return v;
		}

		// Create a new vertex between this vertex and `other` by linearly
		// interpolating all properties using a parameter of `t`. Subclasses should
		// override this to interpolate additional properties.
		inline static Vertex XM_CALLCONV interpolate(const Vertex & a, const Vertex & b, float t)
		{
			Vertex ret;
			xmfloat vt = t;
			ret.position = lerp(a.position, b.position, vt);
			ret.normal = lerp(a.normal, b.normal, vt);
			//ret.color = Color::Lerp(a.color, b.color, t);
			//ret.uv = lerp(a.uv, b.uv, t);
			//ret.uv = Vector2::Lerp(a.uv, b.uv, t);
			return ret;
		}

		// `CSG.Plane.EPSILON` is the tolerance used by `splitPolygon()` to decide if a
		// point is on the plane.
		static const float csgjs_EPSILON = 0.00001f;

		// Plane implementation

		Plane::Plane() : normal(), w(0.0f)
		{
		}

		bool Plane::ok() const
		{
			return length(this->normal) > 0.0f;
		}

		void Plane::flip()
		{
			this->normal = -normal;
			this->w = -this->w;
		}

		Plane::Plane(const Vector3 & a, const Vector3 & b, const Vector3 & c)
		{
			this->normal = normalize(cross(b - a, c - a));
			this->w = dot(this->normal, a);
		}

		// Split `polygon` by this plane if needed, then put the polygon or polygon
		// fragments in the appropriate lists. Coplanar polygons go into either
		// `coplanarFront` or `coplanarBack` depending on their orientation with
		// respect to this plane. Polygons in front or in back of this plane go into
		// either `front` or `back`.
		void Plane::splitPolygon(const ConvexPolygon & polygon, ConvexPolygonCollection & coplanarFront, ConvexPolygonCollection & coplanarBack, ConvexPolygonCollection & front, ConvexPolygonCollection & back) const
		{
			enum
			{
				COPLANAR = 0,
				FRONT = 1,
				BACK = 2,
				SPANNING = 3
			};

			// Classify each point as well as the entire polygon into one of the above
			// four classes.
			int polygonType = 0;
			std::vector<int> types;

			for (size_t i = 0; i < polygon.vertices.size(); i++)
			{
				float t = dot(this->normal, polygon.vertices[i].position) - this->w;
				int type = (t < -csgjs_EPSILON) ? BACK : ((t > csgjs_EPSILON) ? FRONT : COPLANAR);
				polygonType |= type;
				types.push_back(type);
			}

			// Put the polygon in the correct list, splitting it when necessary.
			switch (polygonType)
			{
			case COPLANAR:
			{
				if (dot(this->normal, polygon.plane.normal) > 0)
					coplanarFront.push_back(polygon);
				else
					coplanarBack.push_back(polygon);
				break;
			}
			case FRONT:
			{
				front.push_back(polygon);
				break;
			}
			case BACK:
			{
				back.push_back(polygon);
				break;
			}
			case SPANNING:
			{
				VertexCollection f, b;
				for (size_t i = 0; i < polygon.vertices.size(); i++)
				{
					int j = (i + 1) % polygon.vertices.size();
					int ti = types[i], tj = types[j];
					Vertex vi = polygon.vertices[i], vj = polygon.vertices[j];
					if (ti != BACK) f.push_back(vi);
					if (ti != FRONT) b.push_back(vi);
					if ((ti | tj) == SPANNING)
					{
						float t = (this->w - dot(this->normal, vi.position)) / dot(this->normal, vj.position - vi.position);
						Vertex v = interpolate(vi, vj, t);
						f.push_back(v);
						b.push_back(v);
					}
				}
				if (f.size() >= 3) front.push_back(ConvexPolygon(f));
				if (b.size() >= 3) back.push_back(ConvexPolygon(b));
				break;
			}
			}
		}

		// Polygon implementation

		void ConvexPolygon::flip()
		{
			std::reverse(vertices.begin(), vertices.end());
			for (size_t i = 0; i < vertices.size(); i++)
				vertices[i].normal = -vertices[i].normal;
			plane.flip();
		}

		ConvexPolygon::ConvexPolygon()
		{
		}

		ConvexPolygon::ConvexPolygon(const VertexCollection & list) : vertices(list), plane(vertices[0].position, vertices[1].position, vertices[2].position)
		{
		}

		ConvexPolygon::ConvexPolygon(VertexCollection && list) : vertices(list), plane(vertices[0].position, vertices[1].position, vertices[2].position)
		{
		}

		// Node implementation

		// Return a new CSG solid representing space in either this solid or in the
		// solid `csg`. Neither this solid nor the solid `csg` are modified.
		BSPNode * nodeUnion(const BSPNode * a1, const BSPNode * b1)
		{
			BSPNode * a = a1->clone();
			BSPNode * b = b1->clone();
			a->clipTo(b);
			b->clipTo(a);
			b->invert();
			b->clipTo(a);
			b->invert();
			a->build(b->allPolygons());
			BSPNode * ret = new BSPNode(a->allPolygons());
			delete a; a = 0;
			delete b; b = 0;
			return ret;
		}

		// Return a new CSG solid representing space in this solid but not in the
		// solid `csg`. Neither this solid nor the solid `csg` are modified.
		BSPNode * nodeSubtract(const BSPNode * a1, const BSPNode * b1)
		{
			BSPNode * a = a1->clone();
			BSPNode * b = b1->clone();
			a->invert();
			a->clipTo(b);
			b->clipTo(a);
			b->invert();
			b->clipTo(a);
			b->invert();
			a->build(b->allPolygons());
			a->invert();
			BSPNode * ret = new BSPNode(a->allPolygons());
			delete a; a = 0;
			delete b; b = 0;
			return ret;
		}

		// Return a new CSG solid representing space both this solid and in the
		// solid `csg`. Neither this solid nor the solid `csg` are modified.
		BSPNode * nodeIntersect(const BSPNode * a1, const BSPNode * b1)
		{
			BSPNode * a = a1->clone();
			BSPNode * b = b1->clone();
			a->invert();
			b->clipTo(a);
			b->invert();
			a->clipTo(b);
			b->clipTo(a);
			a->build(b->allPolygons());
			a->invert();
			BSPNode * ret = new BSPNode(a->allPolygons());
			delete a; a = 0;
			delete b; b = 0;
			return ret;
		}

		// Convert solid space to empty space and empty space to solid space.
		void BSPNode::invert()
		{
			for (size_t i = 0; i < this->polygons.size(); i++)
				this->polygons[i].flip();
			this->plane.flip();
			if (this->front) this->front->invert();
			if (this->back) this->back->invert();
			std::swap(this->front, this->back);
		}

		// Recursively remove all polygons in `polygons` that are inside this BSP
		// tree.
		ConvexPolygonCollection BSPNode::clipPolygons(const ConvexPolygonCollection & list) const
		{
			if (!this->plane.ok()) return list;
			ConvexPolygonCollection list_front, list_back;
			for (size_t i = 0; i < list.size(); i++)
			{
				this->plane.splitPolygon(list[i], list_front, list_back, list_front, list_back);
			}
			if (this->front) list_front = this->front->clipPolygons(list_front);
			if (this->back) list_back = this->back->clipPolygons(list_back);
			else list_back.clear();

			list_front.insert(list_front.end(), list_back.begin(), list_back.end());
			return list_front;
		}

		// Remove all polygons in this BSP tree that are inside the other BSP tree
		// `bsp`.
		void BSPNode::clipTo(const BSPNode * other)
		{
			this->polygons = other->clipPolygons(this->polygons);
			if (this->front) this->front->clipTo(other);
			if (this->back) this->back->clipTo(other);
		}

		// Return a list of all polygons in this BSP tree.
		ConvexPolygonCollection BSPNode::allPolygons() const
		{
			ConvexPolygonCollection list = this->polygons;
			ConvexPolygonCollection list_front, list_back;
			if (this->front) list_front = this->front->allPolygons();
			if (this->back) list_back = this->back->allPolygons();
			list.insert(list.end(), list_front.begin(), list_front.end());
			list.insert(list.end(), list_back.begin(), list_back.end());
			return list;
		}

		BSPNode * BSPNode::clone() const
		{
			BSPNode * ret = new BSPNode();
			ret->polygons = this->polygons;
			ret->plane = this->plane;
			if (this->front) ret->front = this->front->clone();
			if (this->back) ret->back = this->back->clone();
			return ret;
		}

		// Build a BSP tree out of `polygons`. When called on an existing tree, the
		// new polygons are filtered down to the bottom of the tree and become new
		// nodes there. Each set of polygons is partitioned using the first polygon
		// (no heuristic is used to pick a good split).
		void BSPNode::build(const ConvexPolygonCollection & list)
		{
			if (!list.size()) return;

			int k = -1; // the splitter polygon

			ConvexPolygonCollection list_front, list_back;

			if (!this->plane.ok())
			{
				k = 0;
				this->plane = list[k].plane;
				this->plane.splitPolygon(list[k], this->polygons, this->polygons, list_front, list_back);

				// When the polygon doesnot split itself properly, force it into Coplanner list
				if (this->polygons.empty() && (list_front.empty() ^ list_back.empty()))
				{
					if (!list_front.empty())
						polygons.swap(list_front);
					else
						polygons.swap(list_back);
				}
			}

			for (size_t i = 0; i < list.size(); i++)
			{
				if (i == k) continue;
				this->plane.splitPolygon(list[i], this->polygons, this->polygons, list_front, list_back);
			}

			if (list_front.size())
			{
				if (!this->front) this->front = new BSPNode;
				this->front->build(list_front);
			}
			if (list_back.size())
			{
				if (!this->back) this->back = new BSPNode;
				this->back->build(list_back);
			}
		}

		BSPNode::BSPNode() : front(0), back(0)
		{
		}

		BSPNode::BSPNode(const ConvexPolygonCollection & list) : front(0), back(0)
		{
			build(list);
		}

		BSPNode::~BSPNode()
		{
			delete front;
			delete back;
		}
#endif
		BSPNode * build_node(const ConvexPolygonCollection & list)
		{
			return new BSPNode(list);
		}
		void destroy_node(BSPNode * node)
		{
			delete node;
		}
}
}