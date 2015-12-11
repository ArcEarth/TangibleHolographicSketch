#pragma once
#include "VisualObject.h"
#include "SmartPointers.h"
#include "Geometrics\SpaceCurve.h"
#include "Geometrics\Extrusion.h"

namespace Causality
{
	using Geometrics::Curve;
	using Geometrics::Patch;
	using Geometrics::Extrusion;
	using Geometrics::MeshType;

	class PenModeler : public VisualObject
	{
	public:
		enum PenModelerStateEnum
		{
			None = 0,
			Inking = 1,
			Dragging = 2,
			Erasing = 3,
		};

	public:
		PenModeler(int objectIdx = 0);
		~PenModeler();

		void SurfaceSketchBegin(MeshType* surface);
		void SrufaceSketchUpdate(FXMVECTOR pos);
		void SurfaceSketchEnd();

		void OnAirDragBegin();
		void OnAirDragUpdate(FXMVECTOR pos);
		void OnAirDragEnd();

		void Update(time_seconds const& time_delta) override;

		// Camera culling
		virtual RenderFlags GetRenderFlags() const;
		virtual bool IsVisible(const BoundingGeometry& viewFrustum) const;
		virtual void Render(IRenderContext *context, IEffect* pEffect = nullptr);
		virtual void XM_CALLCONV UpdateViewMatrix(FXMMATRIX view, CXMMATRIX projection);

	private:
		// the curves we sketched on the surface
		PenModelerStateEnum			m_state;

		MeshType*					m_target;
		vector<Patch>				m_patches;
		vector<Extrusion>			m_extrusions;

		uptr<DynamicMeshBuffer>		m_meshBuffer;

		class TrackedPen;
		uptr<TrackedPen>			m_pTracker;
	};
}