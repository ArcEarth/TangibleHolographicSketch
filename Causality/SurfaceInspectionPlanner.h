#pragma once
#include <Causality\VisualObject.h>
#include <Geometrics\BezierMesh.h>
#include <VertexTypes.h>
#include <atomic>
#include <Textures.h>
#include <Geometrics\Extrusion.h>

namespace Causality
{
	typedef Geometrics::Bezier::BezierPatch<Vector3, 3U> CubicBezierPatch;
	typedef CubicBezierPatch::ClippingType CubicBezierCurve;
	class IPointer;
	class TrackedPen;

	namespace SurfaceInspection
	{
		using TriangleMeshType = Geometrics::TriangleMesh<DirectX::VertexPositionNormalTangentColorTexture, uint32_t>;

		using MeshType = TriangleMeshType;

		struct InspectionPatch : public Geometrics::SurfacePatch
		{
			InspectionPatch()
			{
				UVRotation = .0f;
				Mesh = nullptr;
			}

			Vector2					UVCenter;
			Vector2					UVExtent;
			float					UVRotation;
			const MeshType	*		Mesh;
			bool					Valiad;
			BoundingFrustum			CameraFrustum;
			bool					RequireRedraw;
			cptr<ID2D1PathGeometry> DeaclGeometry;

			bool CaculateCameraFrustum();
			void UpdateGeometry(I2DFactory* pFactory, I2DContext* pContext);
		};

		typedef std::vector<InspectionPatch> InspectionPath;

		class SurfaceInspectionPlanner : public VisualObject
		{
		public:
			void Parse(const ParamArchive* archive) override;

			SurfaceInspectionPlanner();
			~SurfaceInspectionPlanner();
			virtual void AddChild(SceneObject* child) override;
			virtual void Update(time_seconds const& time_delta) override;
			virtual void Render(IRenderContext * pContext, IEffect* pEffect = nullptr) override;
			void RenderPen(IRenderContext *pContext, IEffect*pEffect);

		private:
			void DrawDecal(I2DContext* pContext);
			
			void UpdateDecalGeometry(I2DFactory* pFactory);
			void ExtractMeshFromModel(TriangleMeshType& mesh, const IModelNode* pNode);
			void BuildTriangleMesh(int tessellation, DirectX::SimpleMath::Color &color);

			void AddPatch();
			void RemovePatch();

			TrackedPen*						m_pen;

			CubicBezierPatch				m_patch;
			TriangleMeshType				m_mesh;

			// Decal texture for rendering highlights in target model
			int								m_declDirtyFalg;
			int								m_requestCancelLoading;
			RenderableTexture2D				m_decal;
			cptr<ID2D1PathGeometry>			m_patchGeos;
			cptr<ID2D1SolidColorBrush>		m_brush;

			Color							m_decalBackground;
			Color							m_decalStroke;
			Color							m_decalFill;

			const IPointer*					m_cursor;
			sptr<PhongMaterial>				m_cursorMaterial;

			std::vector<TriangleMeshType>		m_fracorizedMeshes;
			uptr<DirectX::Scene::IModelNode>	m_decalModel;

			uptr<DirectX::Scene::IModelNode>	m_factorizeModel;

			std::atomic_bool				m_isReady;
			concurrency::task<void>			m_loadingTask;
		};
	}
}