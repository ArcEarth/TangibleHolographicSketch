#pragma once
#include "VisualObject.h"
#include "SmartPointers.h"
#include "Geometrics\SpaceCurve.h"

namespace Causality
{
	using Geometrics::SpaceCurve;
	struct ExtrutedGeometry
	{
		SpaceCurve curveFrom;
		SpaceCurve curveTo;
		SpaceCurve extrusionPath;
	};

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

		void SurfaceSketchBegin();
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
		vector<SpaceCurve> m_curves;
		vector<ExtrutedGeometry> m_extrusions;

		PenModelerStateEnum m_state;

		class TrackedPen;
		uptr<TrackedPen> m_pTracker;
	};
}