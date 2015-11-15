#pragma once
#include "SceneObject.h"
#include "SmartPointers.h"
namespace Causality
{

	class TrackedObjectControl : public SceneObject
	{
	public:
		TrackedObjectControl(int objectIdx = 0);
		~TrackedObjectControl();

		void Update(time_seconds const& time_delta) override;

	private:
		class Impl;
		uptr<Impl> m_pImpl;
	};
}