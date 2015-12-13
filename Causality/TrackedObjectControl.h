#pragma once
#include "SceneObject.h"
#include "SmartPointers.h"

namespace Causality
{
	namespace Devices
	{
		class LeapMotion;
		class IViconClient;
	}

	class RigidObjectTracker;

	class TrackedObjectControl : public SceneObject
	{
	public:
		TrackedObjectControl(); 

		~TrackedObjectControl();

		void Parse(const ParamArchive* archive) override;

		void OnParentChanged(SceneObject* oldParent) override;

		void Update(time_seconds const& time_delta) override;
		
		bool UpdateFromVicon(double time_delta);
		bool UpdateFromLeapHand(double time_delta);

	protected:
		SceneObject*		m_pRigid;
		string				m_internalName;
		IsometricTransform&	m_intrinsic;
		sptr<Devices::LeapMotion>	m_pLeap;
		sptr<Devices::IViconClient>	m_pVicon;
		int					m_idx;
	};
}