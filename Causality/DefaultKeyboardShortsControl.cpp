#include "pch_bcl.h"
#include "Scene.h"
#include "SceneObject.h"
#include "NativeWindow.h"

namespace Causality
{
	class DefaultKeyboardShortChuts;

	class DefaultKeyboardShortChuts : public SceneObject
	{
	public:
		scoped_connection con;
		DefaultKeyboardShortChuts()
		{
			con = CoreInputs::PrimaryKeyboard()->KeyUp += [this](const auto& args) { this->OnKeyUp(args); };
		}

		void OnKeyUp(const KeyboardEventArgs & e)
		{
			auto scene = this->Scene;

			if (e.Key == 'K')
				g_DebugView = !g_DebugView;
			if (e.Key == 'T')
				g_ShowCharacterMesh = !g_ShowCharacterMesh;

			if (e.Key == '-' || e.Key == '_' || e.Key == VK_SUBTRACT || e.Key == VK_OEM_MINUS)
				scene->SetTimeScale(scene->GetTimeScale() - 0.1);

			if (e.Key == '=' || e.Key == '+' || e.Key == VK_ADD || e.Key == VK_OEM_PLUS)
				scene->SetTimeScale(scene->GetTimeScale() + 0.1);

			if (e.Key == '0' || e.Key == ')')
				scene->SetTimeScale(1.0);

			if (e.Key == VK_SPACE)
			{
				if (!scene->IsPaused())
					scene->Pause();
				else
					scene->Resume();
			}

			if (e.Key == VK_ESCAPE)
				Application::Current->Exit();

			if (e.Key == VK_OEM_4) // [{
			{
				g_DebugArmatureThinkness = std::max(0.001f, g_DebugArmatureThinkness - 0.002f);
			}
			if (e.Key == VK_OEM_6) // ]}
			{
				g_DebugArmatureThinkness = std::min(0.015f, g_DebugArmatureThinkness + 0.002f);
			}
		}
	};

	REGISTER_SCENE_OBJECT_IN_PARSER(default_keyboard_shorcuts, DefaultKeyboardShortChuts);
}