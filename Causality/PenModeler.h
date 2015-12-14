#pragma once
#include "VisualObject.h"
#include "SmartPointers.h"
#include "Geometrics\SpaceCurve.h"
#include "Geometrics\Extrusion.h"
#include "TrackerdPen.h"

namespace Causality
{
	using Geometrics::Curve;
	using Geometrics::SurfacePatch;
	using Geometrics::Extrusion;
	using Geometrics::MeshType;
	
	class KeyboardMouseFirstPersonControl;

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
		PenModeler(int objectIdx = 1);
		~PenModeler();

		void OnParentChanged(SceneObject* oldParent) override;
		void ExtractMeshFromVisual(Causality::VisualObject * pVisual);
		void AddChild(SceneObject* child) override;
		virtual void Parse(const ParamArchive* store) override;

		void SurfaceSketchBegin();
		void SrufaceSketchUpdate(XMVECTOR pos, XMVECTOR dir);
		void SurfaceSketchEnd();

		void OnAirDragBegin();
		void OnAirDragUpdate(FXMVECTOR pos);
		void OnAirDragEnd();

		void UpdateMeshBuffer(Geometrics::Extrusion & extruder);
		void Update(time_seconds const& time_delta) override;

		// Camera culling
		virtual RenderFlags GetRenderFlags() const;
		virtual bool IsVisible(const BoundingGeometry& viewFrustum) const;
		virtual void Render(IRenderContext *context, IEffect* pEffect = nullptr);
		virtual void XM_CALLCONV UpdateViewMatrix(FXMMATRIX view, CXMMATRIX projection);

	private:
		// the curves we sketched on the surface
		PenModelerStateEnum			m_state;
		vector<SurfacePatch>		m_patches;
		vector<Extrusion>			m_extrusions;

		IRenderDevice*				m_pDevice;
		I2DContext*					m_p2DContex;

		vector<uptr<DynamicMeshBuffer>>
									m_meshBuffers;

		// Decal texture for rendering highlights in target model
		sptr<IMaterial>				m_decalMat;
		uptr<RenderableTexture2D>	m_decal;
		// material for extrusion
		sptr<IMaterial>				m_extruMat;

		TrackedPen*					m_pTracker;
		MeshType*					m_target;
	};
}