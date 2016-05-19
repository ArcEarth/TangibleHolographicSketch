#pragma once
#include "TrackedObjectControl.h"

namespace Causality
{
	class IPointer;

	class TrackedPen : public TrackedObjectControl
	{
	public:
		TrackedPen();

		~TrackedPen();

		void Parse(const ParamArchive * archive) override;

		void Update(const time_seconds& time_delta) override;

		bool UpdateFromLeapFinger(double dt);

		bool IsInking() const;
		bool IsErasing() const;
		bool IsDraging() const;

		bool IsVisible() const;

		XMVECTOR XM_CALLCONV GetTipPosition() const;
		XMVECTOR XM_CALLCONV GetTipDirection() const;
		XMDUALVECTOR XM_CALLCONV GetTipRay() const;
		// = 0.0145 m
		static const float TipLength;
	protected:
		float m_inkingStr;
		float m_erasingStr;
		float m_dragingStr;

		Vector3 m_tipDirbase;
		scoped_connection m_con_pc;
	};
}