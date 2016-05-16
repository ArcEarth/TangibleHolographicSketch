#pragma once
#include "DirectXMathExtend.h"

#if defined(_XBOX_ONE) && defined(_TITLE)
#include <d3d11_x.h>
#else
#include <d3d11_1.h>
#endif

namespace DirectX
{
	namespace SimpleMath
	{
		//------------------------------------------------------------------------------
		// 2D rectangle
		struct Rectangle
		{
			long x;
			long y;
			long width;
			long height;

			// Creators
			Rectangle() : x(0), y(0), width(0), height(0) {}
			Rectangle(long ix, long iy, long iw, long ih) : x(ix), y(iy), width(iw), height(ih) {}
			explicit Rectangle(const RECT& rct) : x(rct.left), y(rct.top), width(rct.right - rct.left), height(rct.bottom - rct.top) {}

			operator RECT() { RECT rct; rct.left = x; rct.top = y; rct.right = (x + width); rct.bottom = (y + height); return rct; }
#if (defined(_XBOX_ONE) && defined(_TITLE)) || (defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP))
			operator ABI::Windows::Foundation::Rect() { ABI::Windows::Foundation::Rect r; r.X = float(x); r.Y = float(y); r.Width = float(width); r.Height = float(height); return r; }
#ifdef __cplusplus_winrt
			operator Windows::Foundation::Rect() { return Windows::Foundation::Rect(float(x), float(y), float(width), float(height)); }
#endif
#endif

			// Comparison operators
			bool operator == (const Rectangle& r) const { return (x == r.x) && (y == r.y) && (width == r.width) && (height == r.height); }
			bool operator == (const RECT& rct) const { return (x == rct.left) && (y == rct.top) && (width == (rct.right - rct.left)) && (height == (rct.bottom - rct.top)); }

			bool operator != (const Rectangle& r) const { return (x != r.x) || (y != r.y) || (width != r.width) || (height != r.height); }
			bool operator != (const RECT& rct) const { return (x != rct.left) || (y != rct.top) || (width != (rct.right - rct.left)) || (height != (rct.bottom - rct.top)); }

			// Assignment operators
			Rectangle& operator=(_In_ const Rectangle& r) { x = r.x; y = r.y; width = r.width; height = r.height; return *this; }
			Rectangle& operator=(_In_ const RECT& rct) { x = rct.left; y = rct.top; width = (rct.right - rct.left); height = (rct.bottom - rct.top); return *this; }

			// Rectangle operations
			Vector2 Location() const;
			Vector2 Center() const;

			bool IsEmpty() const { return (width == 0 && height == 0 && x == 0 && y == 0); }

			bool Contains(long ix, long iy) const { return (x <= ix) && (ix < (x + width)) && (y <= iy) && (iy < (y + height)); }
			bool Contains(const Vector2& point) const;
			bool Contains(const Rectangle& r) const { return (x <= r.x) && ((r.x + r.width) <= (x + width)) && (y <= r.y) && ((r.y + r.height) <= (y + height)); }
			bool Contains(const RECT& rct) const { return (x <= rct.left) && (rct.right <= (x + width)) && (y <= rct.top) && (rct.bottom <= (y + height)); }

			void Inflate(long horizAmount, long vertAmount);

			bool Intersects(const Rectangle& r) const { return (r.x < (x + width)) && (x < (r.x + r.width)) && (r.y < (y + height)) && (y < (r.y + r.height)); }
			bool Intersects(const RECT& rct) const { return (rct.left < (x + width)) && (x < rct.right) && (rct.top < (y + height)) && (y < rct.bottom); }

			void Offset(long ox, long oy) { x += ox; y += oy; }

			// Static functions
			static Rectangle Intersect(const Rectangle& ra, const Rectangle& rb);
			static RECT Intersect(const RECT& rcta, const RECT& rctb);

			static Rectangle Union(const Rectangle& ra, const Rectangle& rb);
			static RECT Union(const RECT& rcta, const RECT& rctb);
		};

		//------------------------------------------------------------------------------
		// Viewport
		class Viewport
		{
		public:
			float x;
			float y;
			float width;
			float height;
			float minDepth;
			float maxDepth;

			Viewport() :
				x(0.f), y(0.f), width(0.f), height(0.f), minDepth(0.f), maxDepth(1.f) {}
			Viewport(float ix, float iy, float iw, float ih, float iminz = 0.f, float imaxz = 1.f) :
				x(ix), y(iy), width(iw), height(ih), minDepth(iminz), maxDepth(imaxz) {}
			explicit Viewport(const RECT& rct) :
				x(float(rct.left)), y(float(rct.top)),
				width(float(rct.right - rct.left)),
				height(float(rct.bottom - rct.top)),
				minDepth(0.f), maxDepth(1.f) {}
			explicit Viewport(const D3D11_VIEWPORT& vp) :
				x(vp.TopLeftX), y(vp.TopLeftY),
				width(vp.Width), height(vp.Height),
				minDepth(vp.MinDepth), maxDepth(vp.MaxDepth) {}

			// Direct3D 11 interop
			operator D3D11_VIEWPORT() { return *reinterpret_cast<D3D11_VIEWPORT*>(this); }
			const D3D11_VIEWPORT* Get11() const { return reinterpret_cast<const D3D11_VIEWPORT*>(this); }

			// Comparison operators
			bool operator == (const Viewport& vp) const;
			bool operator != (const Viewport& vp) const;

			// Assignment operators
			Viewport& operator= (const Viewport& vp);
			Viewport& operator= (const RECT& rct);
			Viewport& operator= (const D3D11_VIEWPORT& vp);

			// Viewport operations
			float AspectRatio() const;

			Vector3 Project(const Vector3& p, const Matrix& proj, const Matrix& view, const Matrix& world) const;
			void Project(const Vector3& p, const Matrix& proj, const Matrix& view, const Matrix& world, Vector3& result) const;

			Vector3 Unproject(const Vector3& p, const Matrix& proj, const Matrix& view, const Matrix& world) const;
			void Unproject(const Vector3& p, const Matrix& proj, const Matrix& view, const Matrix& world, Vector3& result) const;

			// Static methods
			static RECT __cdecl ComputeDisplayArea(DXGI_SCALING scaling, UINT backBufferWidth, UINT backBufferHeight, int outputWidth, int outputHeight);
			static RECT __cdecl ComputeTitleSafeArea(UINT backBufferWidth, UINT backBufferHeight);
		};

		//------------------------------------------------------------------------------
		// Rectangle operations
		//------------------------------------------------------------------------------
		inline Vector2 Rectangle::Location() const
		{
			return Vector2(float(x), float(y));
		}

		inline Vector2 Rectangle::Center() const
		{
			return Vector2(float(x) + float(width / 2.f), float(y) + float(height / 2.f));
		}

		inline bool Rectangle::Contains(const Vector2& point) const
		{
			return (float(x) <= point.x) && (point.x < float(x + width)) && (float(y) <= point.y) && (point.y < float(y + height));
		}

		inline void Rectangle::Inflate(long horizAmount, long vertAmount)
		{
			x -= horizAmount;
			y -= vertAmount;
			width += horizAmount;
			height += vertAmount;
		}

		//------------------------------------------------------------------------------
		// Static functions
		//------------------------------------------------------------------------------

		inline Rectangle Rectangle::Intersect(const Rectangle& ra, const Rectangle& rb)
		{
			long righta = ra.x + ra.width;
			long rightb = rb.x + rb.width;

			long bottoma = ra.y + ra.height;
			long bottomb = rb.y + rb.height;

			long maxX = ra.x > rb.x ? ra.x : rb.x;
			long maxY = ra.y > rb.y ? ra.y : rb.y;

			long minRight = righta < rightb ? righta : rightb;
			long minBottom = bottoma < bottomb ? bottoma : bottomb;

			Rectangle result;

			if ((minRight > maxX) && (minBottom > maxY))
			{
				result.x = maxX;
				result.y = maxY;
				result.width = minRight - maxX;
				result.height = minBottom - maxY;
			}
			else
			{
				result.x = 0;
				result.y = 0;
				result.width = 0;
				result.height = 0;
			}

			return result;
		}

		inline RECT Rectangle::Intersect(const RECT& rcta, const RECT& rctb)
		{
			long maxX = rcta.left > rctb.left ? rcta.left : rctb.left;
			long maxY = rcta.top > rctb.top ? rcta.top : rctb.top;

			long minRight = rcta.right < rctb.right ? rcta.right : rctb.right;
			long minBottom = rcta.bottom < rctb.bottom ? rcta.bottom : rctb.bottom;

			RECT result;

			if ((minRight > maxX) && (minBottom > maxY))
			{
				result.left = maxX;
				result.top = maxY;
				result.right = minRight;
				result.bottom = minBottom;
			}
			else
			{
				result.left = 0;
				result.top = 0;
				result.right = 0;
				result.bottom = 0;
			}

			return result;
		}

		inline Rectangle Rectangle::Union(const Rectangle& ra, const Rectangle& rb)
		{
			long righta = ra.x + ra.width;
			long rightb = rb.x + rb.width;

			long bottoma = ra.y + ra.height;
			long bottomb = rb.y + rb.height;

			int minX = ra.x < rb.x ? ra.x : rb.x;
			int minY = ra.y < rb.y ? ra.y : rb.y;

			int maxRight = righta > rightb ? righta : rightb;
			int maxBottom = bottoma > bottomb ? bottoma : bottomb;

			Rectangle result;
			result.x = minX;
			result.y = minY;
			result.width = maxRight - minX;
			result.height = maxBottom - minY;
			return result;
		}

		inline RECT Rectangle::Union(const RECT& rcta, const RECT& rctb)
		{
			RECT result;
			result.left = rcta.left < rctb.left ? rcta.left : rctb.left;
			result.top = rcta.top < rctb.top ? rcta.top : rctb.top;
			result.right = rcta.right > rctb.right ? rcta.right : rctb.right;
			result.bottom = rcta.bottom > rctb.bottom ? rcta.bottom : rctb.bottom;
			return result;
		}

		/****************************************************************************
		*
		* Viewport
		*
		****************************************************************************/

		//------------------------------------------------------------------------------
		// Comparision operators
		//------------------------------------------------------------------------------

		inline bool Viewport::operator == (const Viewport& vp) const
		{
			return (x == vp.x && y == vp.y
				&& width == vp.width && height == vp.height
				&& minDepth == vp.minDepth && maxDepth == vp.maxDepth);
		}

		inline bool Viewport::operator != (const Viewport& vp) const
		{
			return (x != vp.x || y != vp.y
				|| width != vp.width || height != vp.height
				|| minDepth != vp.minDepth || maxDepth != vp.maxDepth);
		}

		//------------------------------------------------------------------------------
		// Assignment operators
		//------------------------------------------------------------------------------

		inline Viewport& Viewport::operator= (const Viewport& vp)
		{
			x = vp.x; y = vp.y;
			width = vp.width; height = vp.height;
			minDepth = vp.minDepth; maxDepth = vp.maxDepth;
			return *this;
		}

		inline Viewport& Viewport::operator= (const RECT& rct)
		{
			x = float(rct.left); y = float(rct.top);
			width = float(rct.right - rct.left);
			height = float(rct.bottom - rct.top);
			minDepth = 0.f; maxDepth = 1.f;
			return *this;
		}

		inline Viewport& Viewport::operator= (const D3D11_VIEWPORT& vp)
		{
			x = vp.TopLeftX; y = vp.TopLeftY;
			width = vp.Width; height = vp.Height;
			minDepth = vp.MinDepth; maxDepth = vp.MaxDepth;
			return *this;
		}

		//------------------------------------------------------------------------------
		// Viewport operations
		//------------------------------------------------------------------------------

		inline float Viewport::AspectRatio() const
		{
			if (width == 0.f || height == 0.f)
				return 0.f;

			return (width / height);
		}

		inline Vector3 Viewport::Project(const Vector3& p, const Matrix& proj, const Matrix& view, const Matrix& world) const
		{
			using namespace DirectX;
			XMVECTOR v = XMLoadFloat3(&p);
			XMMATRIX projection = XMLoadFloat4x4(&proj);
			v = XMVector3Project(v, x, y, width, height, minDepth, maxDepth, projection, view, world);
			Vector3 result;
			XMStoreFloat3(&result, v);
			return result;
		}

		inline void Viewport::Project(const Vector3& p, const Matrix& proj, const Matrix& view, const Matrix& world, Vector3& result) const
		{
			using namespace DirectX;
			XMVECTOR v = XMLoadFloat3(&p);
			XMMATRIX projection = XMLoadFloat4x4(&proj);
			v = XMVector3Project(v, x, y, width, height, minDepth, maxDepth, projection, view, world);
			XMStoreFloat3(&result, v);
		}

		inline Vector3 Viewport::Unproject(const Vector3& p, const Matrix& proj, const Matrix& view, const Matrix& world) const
		{
			using namespace DirectX;
			XMVECTOR v = XMLoadFloat3(&p);
			XMMATRIX projection = XMLoadFloat4x4(&proj);
			v = XMVector3Unproject(v, x, y, width, height, minDepth, maxDepth, projection, view, world);
			Vector3 result;
			XMStoreFloat3(&result, v);
			return result;
		}

		inline void Viewport::Unproject(const Vector3& p, const Matrix& proj, const Matrix& view, const Matrix& world, Vector3& result) const
		{
			using namespace DirectX;
			XMVECTOR v = XMLoadFloat3(&p);
			XMMATRIX projection = XMLoadFloat4x4(&proj);
			v = XMVector3Unproject(v, x, y, width, height, minDepth, maxDepth, projection, view, world);
			XMStoreFloat3(&result, v);
		}

	}
}
