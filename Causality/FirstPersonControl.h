#pragma once
#include "SceneObject.h"
#include "Pointer.h"
#include "Keyboard.h"

namespace Causality
{
	class KeyboardMouseFirstPersonControl : public SceneObject
	{
	public:
		KeyboardMouseFirstPersonControl();

		virtual void Parse(const ParamArchive* store) override;

		// Inherited via ITimeAnimatable
		virtual void Update(time_seconds const& time_delta) override;

		void ResetTarget(IRigid* pTarget);

		void OnPointerMove(const PointerMoveEventArgs & e);

	public:
		float											Speed;
		float											TuringSpeed;

		bool											Flip_X; // flip Roll
		bool											Flip_Y; // flip Pitch

		DirectX::Quaternion								InitialOrientation;

		float											AddationalYaw = 0;
		float											AddationalPitch = 0;
		float											AddationalRoll = 0;

	private:
		KeyboardHandler*								m_keyboard;
		IPointer*										m_cursor;

		std::array<char,4>								m_directionalKeys;
		int												m_turingBtn;

		scoped_connection								m_conParChg;
		scoped_connection								m_conkeyUp;
		scoped_connection								m_cursorMove;

		IRigid*											m_pTarget = nullptr;

		bool											IsTrackingCursor = false;
		bool											leftButtonDown = false;
		bool											rightButtonDown = false;
		DirectX::Vector3								CameraVeclocity;
		DirectX::Vector3								CameraAngularVeclocity;
		DirectX::Vector3								AngularDisplacement; // Eular Angle
	};
}