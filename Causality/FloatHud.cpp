#include "pch_bcl.h"
#include "FloatHud.h"
#include <PrimitiveVisualizer.h>
#include <Textures.h>
#include <HUD.h>

using namespace Causality;
using namespace DirectX;
using namespace DirectX::Visualizers;

FloatingHud::FloatingHud(HUDElement * hudElem)
{

}

void FloatingHud::Update(time_seconds const & time_delta)
{
	SceneObject::Update(time_delta);
}

bool FloatingHud::IsVisible(const BoundingGeometry & viewFrustum) const
{
	auto pVisual = FirstAncesterOfType<IVisual>();
	return pVisual && pVisual->IsVisible(viewFrustum);
}

RenderFlags Causality::FloatingHud::GetRenderFlags() const
{
	return RenderFlags::SpecialEffects;
}

void FloatingHud::Render(IRenderContext * pContext, IEffect * pEffect)
{
	auto sprites = g_PrimitiveDrawer.GetSpriteBatch();
	sprites->Begin(SpriteSortMode_Immediate);
	sprites->Draw(m_pTexture->ShaderResourceView(),m_position,m_size);
	sprites->End();
}

void XM_CALLCONV FloatingHud::UpdateViewMatrix(FXMMATRIX view, CXMMATRIX projection)
{
	XMVECTOR viewport = XMLoad(m_viewportSize);
	XMVector3ConvertToTextureCoord(GetPosition(), viewport, view*projection);
}
