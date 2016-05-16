#pragma once
#include "Math3D.h"
#include "Events.h"

namespace Causality
{
	class IWindow;

	enum PointerPositionMode
	{
		MODE_ABSOLUTE = 0,
		MODE_RELATIVE,
	};

	enum PointerButtonEnum
	{
		LButton = 0,
		MButton = 2,
		RButton = 1,
		NoButton = -1,
	};

	enum PointerButtonStateEnum
	{
		PointerButton_Down = 0,
		PointerButton_Up = 1,
		PointerButton_Hold = 2,
		PointerButton_Release = 3,
	};

	enum PointerType
	{
		PointerType_Unknown = 0,
		PointerType_Mouse = 1,
		PointerType_Touch = 2,
		PointerType_Tracker = 3,
	};

	struct PointerButtonEventArgs
	{
		const Pointer* Pointer;
		int						ButtonIndex;
		PointerButtonStateEnum	OldState;
		PointerButtonStateEnum	NewState;
	};

	struct PointerMoveEventArgs
	{
		const Pointer* Pointer;
		Vector3	Position; 		// Relative position to window's top left corner
		Vector3 PositionDelta;
	};

	class Pointer abstract
	{
	protected:
		PointerType		m_type;
		Vector4			m_pos;
		Vector4			m_delta;
		size_t			m_buttonCount;
		size_t			m_buttonState;
		IWindow*		m_parent;

	public:
		Event<PointerMoveEventArgs>		Move;
		Event<PointerButtonEventArgs>	ButtonStateChanged;

	public:
		Pointer();
		~Pointer();

		void		SetWindow(IWindow* pWindow);

		virtual void Update(Vector4 pos, size_t state) = 0;

		IWindow*	Owner() const { return m_parent; }

		PointerType Type() const { return m_type; }

		Vector3		Position() const { return reinterpret_cast<const Vector3&>(m_pos); };
		Vector3		PositionDelta() const { return reinterpret_cast<const Vector3&>(m_delta); }

		size_t		ButtonCount() const { return m_buttonCount; }

		PointerButtonStateEnum
			ButtonState(int button = 0) const
		{
			return PointerButtonStateEnum((m_buttonState >> (button * 4)) & 0x4);
		};

	protected:
		void		SignalMoved();
		void		SignalButton();
	};

	class MousePointer : public Pointer
	{
	protected:
		float m_deltaEpsilon;

	public:
		MousePointer(){
			m_deltaEpsilon = 1.0f;
			m_parent = nullptr;
			m_type = PointerType_Mouse;
		}

		virtual void Update(Vector4 pos, size_t state) {
			m_delta = pos - m_pos;
			m_pos = pos;

			if (state != m_buttonState)
				SignalButton();

			if (m_delta.LengthSquared() > m_deltaEpsilon)
				SignalMoved();
		};


		float X() const { return m_pos.x; }
		float Y() const { return m_pos.y; }

		float XDelta() const { return m_delta.x; }
		float YDelta() const { return m_delta.y; }

		float WheelDelta() const { return m_delta.w; }

		Vector2	XY() const { return reinterpret_cast<const Vector2&>(m_pos); };
		Vector2	XYDelta() const { return reinterpret_cast<const Vector2&>(m_delta); };
	};
}