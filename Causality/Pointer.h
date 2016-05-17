#pragma once
#include "Math3D.h"
#include "Events.h"
#ifndef NOMINMAX 
#define NOMINMAX
#endif
#include <minwindef.h>

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
		RButton = 1,
		MButton = 2,
		NoButton = -1,
	};

	enum PointerButtonStateEnum
	{
		PointerButton_Up = 0,
		PointerButton_Down = 1,
		PointerButton_Releasing = 2,
		PointerButton_Pressing = 3,
	};

	enum PointerType
	{
		PointerType_Unknown = 0,
		PointerType_Mouse = 1,
		PointerType_Touch = 2,
		PointerType_Tracker = 3,
	};

	class IPointer;

	struct PointerMoveEventArgs
	{
		const IPointer* Pointer;
		Vector4	Position; 		// Relative position to window's top left corner
		Vector4 PositionDelta;
	};

	// Bitset that encodes button states
	struct PointerButtonStates
	{
		size_t _bitcode;

		PointerButtonStates() = default;
		explicit PointerButtonStates(size_t state) : _bitcode(state) {}
		operator size_t () const { return _bitcode; }
		PointerButtonStates& operator=(size_t state) { _bitcode = state; return *this; }

		PointerButtonStateEnum GetState(int button = 0) const
		{
			return PointerButtonStateEnum((_bitcode >> (button * 2)) & 0x3);
		};

		bool IsButtonDown(unsigned short key) const { return (bool)(GetState(key) & 1); }
		bool IsButtonChanged(unsigned short key) const { return (bool)(GetState(key) & 2); }


		void SetState(int button, PointerButtonStateEnum _state)
		{
			size_t curState = (_bitcode >> (button * 2)) & 0x4;

			size_t state = _state ^ ((-((curState ^ _state) & 0x1) ^ _state) & 0x2);

			_bitcode |= 0x3 << (button * 2);
			_bitcode &= (state & 0x3) << (button * 2);
		}
	};

	struct PointerButtonEventArgs
	{
		const IPointer* Pointer;
		PointerButtonStates	States;
	};


	class IPointer abstract
	{
	protected:
		PointerType		m_type;
		size_t			m_index;
		Vector4			m_pos;
		Vector4			m_delta;
		size_t			m_btnCount;
		PointerButtonStates
						m_btnState;
		bool			m_isPresent;
		IWindow*		m_parent;

	public:
		Event<PointerMoveEventArgs>		Move;
		Event<PointerButtonEventArgs>	ButtonStateChanged;

	public:
		IPointer();
		virtual ~IPointer();

		void		SetWindow(IWindow* pWindow);

		virtual void Update(Vector4 pos, PointerButtonStates state) = 0;
		inline void Update(PointerButtonStates state) { Update(m_pos, state); }
		inline void Update(Vector4 pos) { Update(pos, m_btnState); }

		IWindow*	Owner() const { return m_parent; }
		size_t		Index() const { return m_index; }

		PointerType Type() const { return m_type; }

		bool		IsPresent() const { return m_isPresent; }

		Vector3		Position() const { return reinterpret_cast<const Vector3&>(m_pos); };
		Vector3		PositionDelta() const { return reinterpret_cast<const Vector3&>(m_delta); }

		size_t		ButtonCount() const { return m_btnCount; }

		PointerButtonStateEnum ButtonState(int button = 0) const
		{ return PointerButtonStateEnum((m_btnState >> (button * 2)) & 0x3); };

		bool		IsButtonDown(int button = 0) const { return (m_btnState >> (button * 2)) & 0x1; }
		bool		IsButtonChanging(int button = 0) const { return (m_btnState >> (button * 2 + 1)) & 0x1; }

		PointerButtonStates ButtonStates() const { return m_btnState; }

	protected:
		void		SignalMoved();
		void		SignalButton();
	};

	class CursorHandler;

	class MousePointer : public IPointer
	{
		friend CursorHandler;
	protected:
		unsigned short	m_eventFlag;
		float			m_deltaEpsilon;
		std::string		m_device_name;
		void*			m_device_handle;

	public:
		MousePointer();
		~MousePointer();

		virtual void Update(Vector4 pos, PointerButtonStates state);

		const char* DeviceName() const { m_device_name.c_str(); }
		void* DeviceHandle() const { return m_device_handle; }

		float X() const { return m_pos.x; }
		float Y() const { return m_pos.y; }

		float XDelta() const { return m_delta.x; }
		float YDelta() const { return m_delta.y; }

		void  ResetWheel() { m_pos.z = .0f; }
		float WheelAccumlated() const { m_pos.z; }
		float WheelDelta() const { return m_delta.z; }

		Vector2	XY() const { return reinterpret_cast<const Vector2&>(m_pos); };
		Vector2	XYDelta() const { return reinterpret_cast<const Vector2&>(m_delta); };
	};

	class CursorHandler
	{
	protected:
		MousePointer			m_primaryPtr;	// The primary mouse pointer 
		std::vector<IPointer*>	m_pointers;		// 
		bool					m_initialized;	// 
		bool					m_useRdp;	// if use the remote device pointer
		std::vector<BYTE>		m_lbp;		// Buffer to raw input data

	public:
		CursorHandler();

		struct PointerAccquiredEvent
		{
			int				PointerIndex;
			const IPointer*	Pointer;
		};

		// Event Interface
		Event<const PointerButtonEventArgs&>	PointerReleased;
		Event<const PointerButtonEventArgs&>	PointerAccquired;

		bool SetWindow(IWindow *window);

		int	 GetPointerCount() const { m_pointers.size(); }
		const IPointer* GetPointer(int index) const { return m_pointers[index]; }
		const IPointer* GetPointer(const char* name) const;

		const IPointer* GetPrimaryPointer() const { return &m_primaryPtr; }
		IPointer* GetPrimaryPointer() { return &m_primaryPtr; }

		void	ProcessMessage(UINT, WPARAM, LPARAM);

		private:
		int		MouseInputDispicher(void* input);
	};

	namespace CoreInputs
	{
		IPointer*	PrimaryPointer();
	}
}