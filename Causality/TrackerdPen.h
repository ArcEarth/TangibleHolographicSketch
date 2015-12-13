#pragma once
#include "TrackedObjectControl.h"

namespace Causality
{
	class TrackedPen : public TrackedObjectControl
	{
		void Update(const time_seconds& time_delta) override;
	};
}