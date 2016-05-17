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

		void Update(const time_seconds& time_delta) override;

		bool UpdateFromLeapFinger(double dt);

		bool IsInking() const;
		bool IsErasing() const;
		bool IsDraging() const;

		bool IsVisible() const;

		// = 0.0145 m
		static const float TipLength;
	protected:
		bool  m_visible;
		float m_inkingStr;
		float m_erasingStr;
		float m_dragingStr;

		scoped_connection m_con_pc;
	};
}