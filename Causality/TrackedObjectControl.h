#pragma once
#include "SceneObject.h"
#include "SmartPointers.h"
#include <Common\Filter.h>

namespace Causality
{
	namespace Devices
	{
		class LeapSensor;
		class IViconClient;
	}

	class KeyboardMouseFirstPersonControl;

	class RigidObjectTracker;
	class IPointer;

	class TrackedObjectControl : public SceneObject
	{
	public:
		TrackedObjectControl(); 

		~TrackedObjectControl();

		void Parse(const ParamArchive* archive) override;

		void Update(time_seconds const& time_delta) override;
		
		bool UpdateFromVicon(double time_delta);
		bool UpdateFromLeapHand(double time_delta);
		bool UpdateFromCursor(double time_delta);

		// Using mouse cursor and touch to emulate tracking
		void SetCursorEmulation(const IPointer* _2dpointer);

	protected:
		LowPassFilter<Vector3,float>	m_posFilter;
		LowPassFilter<Quaternion,float>	m_rotFilter;
		float				m_freq;

		SceneObject*		m_pRigid;
		string				m_internalName;
		IsometricTransform&	m_intrinsic;
		sptr<Devices::LeapSensor>	m_pLeap;
		sptr<Devices::IViconClient>	m_pVicon;
		const IPointer*		m_cursor;
		int					m_idx;

		scoped_connection	m_parentChangedConnection;

	};
}