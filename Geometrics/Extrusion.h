#pragma once
#include <vector>
#include <cassert>
#include "csg.h"
#include "SpaceCurve.h"

namespace Geometrics
{
	using DirectX::Vector3;
	using DirectX::Ray;
	using DirectX::FXMVECTOR;

	using Curve = SpaceCurve;

	// A Patch is a closed curve on a surface
	// And the region within
	class Patch
	{
		MeshType*	m_surface;
		Curve		m_boundry;
		MeshType	m_mesh;
		int			m_dirty;

	public:
		MeshType& surface() { return *m_surface; }
		const MeshType& surface() const { return *m_surface; }

		Curve& boundry() { return m_boundry; }
		const Curve& boundry() const { return m_boundry; }

		MeshType& mesh() { return m_mesh; }
		const MeshType& mesh() const { return m_mesh; }

		// seperate and re-triangluate the patch from exist surface
		MeshType& triangulate(size_t subdiv);
	};


	using std::vector;
	class Extrusion
	{
		Patch *m_top, *m_bottom;
		Curve *m_path;
		MeshType m_mesh;
		int		m_dirty;

	public:
		Extrusion();

		Extrusion(Patch *top, Patch *bottom, Curve *axis);

		~Extrusion();

		int intersect(const Ray &ray, std::vector<Vector3>& intersections);

		bool valiad() const { return m_top != nullptr && m_bottom != nullptr && m_path && !m_dirty; }

		MeshType& mesh();
		const MeshType& mesh() const;

		Patch& top() { return *m_top; }
		const Patch& top() const { return *m_top; }
		void setTop(Patch *path) { m_dirty = (m_top == path); m_top = path; }

		Patch& bottom() { return *m_bottom; }
		const Patch& bottom() const { return *m_bottom; }
		void setBottom(Patch *path) { m_dirty = (m_bottom == path); m_bottom = path; }

		Curve& axis() { return *m_path; }
		const Curve& axis() const { return *m_path; }
		void setAxis(Curve *axis) { m_dirty = (m_path == axis); m_path = axis; }

		// triangulate top and bottom with path into m_mesh
		MeshType& triangulate(int axisSubdiv, int polarSubdiv);

	};
}