#pragma once
#include "Pointer.h"
#include "Keyboard.h"
#include "Events.h"
#include <string>
#include "Application.h"

namespace Causality
{
	using DirectX::Vector2;

	struct Rect
	{
		Vector2 Position;
		Vector2 Size;

		float Top() const { return Position.y; }
		float Left() const { return Position.x; }
		float Bottom() const { return Position.y + Size.y; }
		float Right() const { return Position.x + Size.x; }
		float Width() const { return Size.x; }
		float Height() const { return Size.y; }
	};

	//ref class Window;
	class NativeWindow;

	class IWindow;

	class IWindow abstract
	{
	public:
		~IWindow() {}

		virtual HWND Handle() const = 0;
		virtual void Initialize(const std::string& title, unsigned int width, unsigned int height, bool fullScreen = false) = 0;
		virtual void Show() = 0;
		virtual void Hide() = 0;
		virtual void Focus() = 0;
		virtual void Close() = 0;
		virtual void Minimize() = 0;
		virtual void Maximize() = 0;
		virtual bool IsFullScreen() const = 0;
		virtual void EnterFullScreen() = 0;
		virtual void ExitFullScreen() = 0;
		virtual bool Move(int x, int y) = 0;

		virtual void OnResize(size_t width, size_t height) = 0;
		virtual void ProcessMessage(UINT, WPARAM, LPARAM) = 0;
	};

	class DebugConsole : public std::enable_shared_from_this<DebugConsole>, public IWindow
	{
	public:
		HWND Handle() const { return hWnd; }

		DebugConsole()
			: hWnd(NULL)
		{}
		// Inherited via IWindow
		virtual void Initialize(const std::string& title, unsigned int width, unsigned int height, bool fullScreen = false) override;
		virtual void Show() override;
		virtual void Hide() override;
		virtual void Focus() override;
		virtual void Close() override;
		virtual void Minimize() override;
		virtual void Maximize() override;
		virtual bool IsFullScreen() const override;
		virtual void EnterFullScreen() override;
		virtual void ExitFullScreen() override;
		virtual void OnResize(size_t width, size_t height) override;
		virtual bool Move(int x, int y) override;
		virtual void ProcessMessage(UINT, WPARAM, LPARAM) override {}

	private:
		HWND hWnd;
	};

	// Design to work with std::shared_ptr<NativeWindow>
	class NativeWindow :public std::enable_shared_from_this<NativeWindow>,  public IWindow
	{
	public:
		NativeWindow();
		~NativeWindow();

		static shared_ptr<NativeWindow> GetForCurrentView();

		virtual void Initialize(const std::string& title, unsigned int width, unsigned int height, bool fullScreen = false) override;

		void Show();
		void Hide();
		void Focus();

		void Close();

		void Minimize();
		void Maximize();

		bool IsFullScreen() const;

		void EnterFullScreen();
		void ExitFullScreen();
		bool Move(int x, int y) override;

		virtual void OnResize(size_t width, size_t height) override;
		virtual void ProcessMessage(UINT, WPARAM, LPARAM) override;

		HWND Handle() const
		{
			return m_hWnd;
		}

		HINSTANCE ApplicationInstance() const
		{
			return m_hInstance;
		}

		Rect Boundary() const
		{
			return m_Boundary;
		}

		Event<Vector2> SizeChanged;

		const CursorHandler&	Cursors() const { return m_cursor; }
		const KeyboardHandler&	Keyboard() const { return m_keyboard; }
		CursorHandler&	Cursors() { return m_cursor; }
		KeyboardHandler&	Keyboard() { return m_keyboard; }
	private:
		std::wstring		m_Title;
		HWND				m_hWnd;
		HINSTANCE			m_hInstance;
		bool				m_FullScreen;
		Rect				m_Boundary;

		CursorHandler		m_cursor;
		KeyboardHandler		m_keyboard;
	};

	//ref class Window sealed
	//{
	//public:
	//	Window();
	//	//Window(std::unique_ptr<NativeWindow> &&native);
	//	virtual ~Window();
	//	void Initialize(Platform::String^ title, unsigned int screenWidth, unsigned int screenHeight, bool fullScreen = false);
	//	void Show();
	//	void Focus();
	//	void Close();

	//	event Windows::Foundation::TypedEventHandler<Window^, Windows::UI::Core::WindowActivatedEventArgs^> ^Activated{
	//		Windows::Foundation::EventRegistrationToken add(Windows::Foundation::TypedEventHandler<Window^, Windows::UI::Core::WindowActivatedEventArgs^>^ value);
	//		void remove(Windows::Foundation::EventRegistrationToken token);
	//		void raise(Window^, Windows::UI::Core::WindowActivatedEventArgs^);
	//	}

	//private:
	//	std::unique_ptr<NativeWindow>	m_Native;
	//};

}
