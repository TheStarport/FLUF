#include "PCH.hpp"

#include <FLUF.UI.hpp>
#include <RmlUi/Core.h>
#include <RmlUi/Debugger/Debugger.h>

#include "Rml/RmlWin32.hpp"

namespace RmlWin32
{
    int GetKeyModifierState()
    {
        int key_modifier_state = 0;

        if (GetKeyState(VK_CAPITAL) & 1)
        {
            key_modifier_state |= Rml::Input::KM_CAPSLOCK;
        }

        if (GetKeyState(VK_NUMLOCK) & 1)
        {
            key_modifier_state |= Rml::Input::KM_NUMLOCK;
        }

        if (HIWORD(GetKeyState(VK_SHIFT)) & 1)
        {
            key_modifier_state |= Rml::Input::KM_SHIFT;
        }

        if (HIWORD(GetKeyState(VK_CONTROL)) & 1)
        {
            key_modifier_state |= Rml::Input::KM_CTRL;
        }

        if (HIWORD(GetKeyState(VK_MENU)) & 1)
        {
            key_modifier_state |= Rml::Input::KM_ALT;
        }

        return key_modifier_state;
    }

    Rml::Input::KeyIdentifier ConvertKey(int win32_key_code)
    {
        // clang-format off
	switch (win32_key_code)
	{
		case 'A':                    return Rml::Input::KI_A;
		case 'B':                    return Rml::Input::KI_B;
		case 'C':                    return Rml::Input::KI_C;
		case 'D':                    return Rml::Input::KI_D;
		case 'E':                    return Rml::Input::KI_E;
		case 'F':                    return Rml::Input::KI_F;
		case 'G':                    return Rml::Input::KI_G;
		case 'H':                    return Rml::Input::KI_H;
		case 'I':                    return Rml::Input::KI_I;
		case 'J':                    return Rml::Input::KI_J;
		case 'K':                    return Rml::Input::KI_K;
		case 'L':                    return Rml::Input::KI_L;
		case 'M':                    return Rml::Input::KI_M;
		case 'N':                    return Rml::Input::KI_N;
		case 'O':                    return Rml::Input::KI_O;
		case 'P':                    return Rml::Input::KI_P;
		case 'Q':                    return Rml::Input::KI_Q;
		case 'R':                    return Rml::Input::KI_R;
		case 'S':                    return Rml::Input::KI_S;
		case 'T':                    return Rml::Input::KI_T;
		case 'U':                    return Rml::Input::KI_U;
		case 'V':                    return Rml::Input::KI_V;
		case 'W':                    return Rml::Input::KI_W;
		case 'X':                    return Rml::Input::KI_X;
		case 'Y':                    return Rml::Input::KI_Y;
		case 'Z':                    return Rml::Input::KI_Z;

		case '0':                    return Rml::Input::KI_0;
		case '1':                    return Rml::Input::KI_1;
		case '2':                    return Rml::Input::KI_2;
		case '3':                    return Rml::Input::KI_3;
		case '4':                    return Rml::Input::KI_4;
		case '5':                    return Rml::Input::KI_5;
		case '6':                    return Rml::Input::KI_6;
		case '7':                    return Rml::Input::KI_7;
		case '8':                    return Rml::Input::KI_8;
		case '9':                    return Rml::Input::KI_9;

		case VK_BACK:                return Rml::Input::KI_BACK;
		case VK_TAB:                 return Rml::Input::KI_TAB;

		case VK_CLEAR:               return Rml::Input::KI_CLEAR;
		case VK_RETURN:              return Rml::Input::KI_RETURN;

		case VK_PAUSE:               return Rml::Input::KI_PAUSE;
		case VK_CAPITAL:             return Rml::Input::KI_CAPITAL;

		case VK_KANA:                return Rml::Input::KI_KANA;
		//case VK_HANGUL:              return Rml::Input::KI_HANGUL; /* overlaps with VK_KANA */
		case VK_JUNJA:               return Rml::Input::KI_JUNJA;
		case VK_FINAL:               return Rml::Input::KI_FINAL;
		case VK_HANJA:               return Rml::Input::KI_HANJA;
		//case VK_KANJI:               return Rml::Input::KI_KANJI; /* overlaps with VK_HANJA */

		case VK_ESCAPE:              return Rml::Input::KI_ESCAPE;

		case VK_CONVERT:             return Rml::Input::KI_CONVERT;
		case VK_NONCONVERT:          return Rml::Input::KI_NONCONVERT;
		case VK_ACCEPT:              return Rml::Input::KI_ACCEPT;
		case VK_MODECHANGE:          return Rml::Input::KI_MODECHANGE;

		case VK_SPACE:               return Rml::Input::KI_SPACE;
		case VK_PRIOR:               return Rml::Input::KI_PRIOR;
		case VK_NEXT:                return Rml::Input::KI_NEXT;
		case VK_END:                 return Rml::Input::KI_END;
		case VK_HOME:                return Rml::Input::KI_HOME;
		case VK_LEFT:                return Rml::Input::KI_LEFT;
		case VK_UP:                  return Rml::Input::KI_UP;
		case VK_RIGHT:               return Rml::Input::KI_RIGHT;
		case VK_DOWN:                return Rml::Input::KI_DOWN;
		case VK_SELECT:              return Rml::Input::KI_SELECT;
		case VK_PRINT:               return Rml::Input::KI_PRINT;
		case VK_EXECUTE:             return Rml::Input::KI_EXECUTE;
		case VK_SNAPSHOT:            return Rml::Input::KI_SNAPSHOT;
		case VK_INSERT:              return Rml::Input::KI_INSERT;
		case VK_DELETE:              return Rml::Input::KI_DELETE;
		case VK_HELP:                return Rml::Input::KI_HELP;

		case VK_LWIN:                return Rml::Input::KI_LWIN;
		case VK_RWIN:                return Rml::Input::KI_RWIN;
		case VK_APPS:                return Rml::Input::KI_APPS;

		case VK_SLEEP:               return Rml::Input::KI_SLEEP;

		case VK_NUMPAD0:             return Rml::Input::KI_NUMPAD0;
		case VK_NUMPAD1:             return Rml::Input::KI_NUMPAD1;
		case VK_NUMPAD2:             return Rml::Input::KI_NUMPAD2;
		case VK_NUMPAD3:             return Rml::Input::KI_NUMPAD3;
		case VK_NUMPAD4:             return Rml::Input::KI_NUMPAD4;
		case VK_NUMPAD5:             return Rml::Input::KI_NUMPAD5;
		case VK_NUMPAD6:             return Rml::Input::KI_NUMPAD6;
		case VK_NUMPAD7:             return Rml::Input::KI_NUMPAD7;
		case VK_NUMPAD8:             return Rml::Input::KI_NUMPAD8;
		case VK_NUMPAD9:             return Rml::Input::KI_NUMPAD9;
		case VK_MULTIPLY:            return Rml::Input::KI_MULTIPLY;
		case VK_ADD:                 return Rml::Input::KI_ADD;
		case VK_SEPARATOR:           return Rml::Input::KI_SEPARATOR;
		case VK_SUBTRACT:            return Rml::Input::KI_SUBTRACT;
		case VK_DECIMAL:             return Rml::Input::KI_DECIMAL;
		case VK_DIVIDE:              return Rml::Input::KI_DIVIDE;
		case VK_F1:                  return Rml::Input::KI_F1;
		case VK_F2:                  return Rml::Input::KI_F2;
		case VK_F3:                  return Rml::Input::KI_F3;
		case VK_F4:                  return Rml::Input::KI_F4;
		case VK_F5:                  return Rml::Input::KI_F5;
		case VK_F6:                  return Rml::Input::KI_F6;
		case VK_F7:                  return Rml::Input::KI_F7;
		case VK_F8:                  return Rml::Input::KI_F8;
		case VK_F9:                  return Rml::Input::KI_F9;
		case VK_F10:                 return Rml::Input::KI_F10;
		case VK_F11:                 return Rml::Input::KI_F11;
		case VK_F12:                 return Rml::Input::KI_F12;
		case VK_F13:                 return Rml::Input::KI_F13;
		case VK_F14:                 return Rml::Input::KI_F14;
		case VK_F15:                 return Rml::Input::KI_F15;
		case VK_F16:                 return Rml::Input::KI_F16;
		case VK_F17:                 return Rml::Input::KI_F17;
		case VK_F18:                 return Rml::Input::KI_F18;
		case VK_F19:                 return Rml::Input::KI_F19;
		case VK_F20:                 return Rml::Input::KI_F20;
		case VK_F21:                 return Rml::Input::KI_F21;
		case VK_F22:                 return Rml::Input::KI_F22;
		case VK_F23:                 return Rml::Input::KI_F23;
		case VK_F24:                 return Rml::Input::KI_F24;

		case VK_NUMLOCK:             return Rml::Input::KI_NUMLOCK;
		case VK_SCROLL:              return Rml::Input::KI_SCROLL;

		case VK_OEM_NEC_EQUAL:       return Rml::Input::KI_OEM_NEC_EQUAL;

		//case VK_OEM_FJ_JISHO:        return Rml::Input::KI_OEM_FJ_JISHO; /* overlaps with VK_OEM_NEC_EQUAL */
		case VK_OEM_FJ_MASSHOU:      return Rml::Input::KI_OEM_FJ_MASSHOU;
		case VK_OEM_FJ_TOUROKU:      return Rml::Input::KI_OEM_FJ_TOUROKU;
		case VK_OEM_FJ_LOYA:         return Rml::Input::KI_OEM_FJ_LOYA;
		case VK_OEM_FJ_ROYA:         return Rml::Input::KI_OEM_FJ_ROYA;

		case VK_SHIFT:               return Rml::Input::KI_LSHIFT;
		case VK_CONTROL:             return Rml::Input::KI_LCONTROL;
		case VK_MENU:                return Rml::Input::KI_LMENU;

		case VK_BROWSER_BACK:        return Rml::Input::KI_BROWSER_BACK;
		case VK_BROWSER_FORWARD:     return Rml::Input::KI_BROWSER_FORWARD;
		case VK_BROWSER_REFRESH:     return Rml::Input::KI_BROWSER_REFRESH;
		case VK_BROWSER_STOP:        return Rml::Input::KI_BROWSER_STOP;
		case VK_BROWSER_SEARCH:      return Rml::Input::KI_BROWSER_SEARCH;
		case VK_BROWSER_FAVORITES:   return Rml::Input::KI_BROWSER_FAVORITES;
		case VK_BROWSER_HOME:        return Rml::Input::KI_BROWSER_HOME;

		case VK_VOLUME_MUTE:         return Rml::Input::KI_VOLUME_MUTE;
		case VK_VOLUME_DOWN:         return Rml::Input::KI_VOLUME_DOWN;
		case VK_VOLUME_UP:           return Rml::Input::KI_VOLUME_UP;
		case VK_MEDIA_NEXT_TRACK:    return Rml::Input::KI_MEDIA_NEXT_TRACK;
		case VK_MEDIA_PREV_TRACK:    return Rml::Input::KI_MEDIA_PREV_TRACK;
		case VK_MEDIA_STOP:          return Rml::Input::KI_MEDIA_STOP;
		case VK_MEDIA_PLAY_PAUSE:    return Rml::Input::KI_MEDIA_PLAY_PAUSE;
		case VK_LAUNCH_MAIL:         return Rml::Input::KI_LAUNCH_MAIL;
		case VK_LAUNCH_MEDIA_SELECT: return Rml::Input::KI_LAUNCH_MEDIA_SELECT;
		case VK_LAUNCH_APP1:         return Rml::Input::KI_LAUNCH_APP1;
		case VK_LAUNCH_APP2:         return Rml::Input::KI_LAUNCH_APP2;

		case VK_OEM_1:               return Rml::Input::KI_OEM_1;
		case VK_OEM_PLUS:            return Rml::Input::KI_OEM_PLUS;
		case VK_OEM_COMMA:           return Rml::Input::KI_OEM_COMMA;
		case VK_OEM_MINUS:           return Rml::Input::KI_OEM_MINUS;
		case VK_OEM_PERIOD:          return Rml::Input::KI_OEM_PERIOD;
		case VK_OEM_2:               return Rml::Input::KI_OEM_2;
		case VK_OEM_3:               return Rml::Input::KI_OEM_3;

		case VK_OEM_4:               return Rml::Input::KI_OEM_4;
		case VK_OEM_5:               return Rml::Input::KI_OEM_5;
		case VK_OEM_6:               return Rml::Input::KI_OEM_6;
		case VK_OEM_7:               return Rml::Input::KI_OEM_7;
		case VK_OEM_8:               return Rml::Input::KI_OEM_8;

		case VK_OEM_AX:              return Rml::Input::KI_OEM_AX;
		case VK_OEM_102:             return Rml::Input::KI_OEM_102;
		case VK_ICO_HELP:            return Rml::Input::KI_ICO_HELP;
		case VK_ICO_00:              return Rml::Input::KI_ICO_00;

		case VK_PROCESSKEY:          return Rml::Input::KI_PROCESSKEY;

		case VK_ICO_CLEAR:           return Rml::Input::KI_ICO_CLEAR;

		case VK_ATTN:                return Rml::Input::KI_ATTN;
		case VK_CRSEL:               return Rml::Input::KI_CRSEL;
		case VK_EXSEL:               return Rml::Input::KI_EXSEL;
		case VK_EREOF:               return Rml::Input::KI_EREOF;
		case VK_PLAY:                return Rml::Input::KI_PLAY;
		case VK_ZOOM:                return Rml::Input::KI_ZOOM;
		case VK_PA1:                 return Rml::Input::KI_PA1;
		case VK_OEM_CLEAR:           return Rml::Input::KI_OEM_CLEAR;
	}
        // clang-format on

        return Rml::Input::KI_UNKNOWN;
    }

    bool ProcessKeyDownShortcuts(FlufUi* fluf, Rml::Context* context, Rml::Input::KeyIdentifier key, int key_modifier, float native_dp_ratio, bool priority)
    {
        if (!context)
        {
            return true;
        }

        // Result should return true to allow the event to propagate to the next handler.
        bool result = false;

        // This function is intended to be called twice by the backend, before and after submitting the key event to the context. This way we can
        // intercept shortcuts that should take priority over the context, and then handle any shortcuts of lower priority if the context did not
        // intercept it.
        if (priority)
        {
            // Priority shortcuts are handled before submitting the key to the context.

            // Toggle debugger and set dp-ratio using Ctrl +/-/0 keys.
            if (key == Rml::Input::KI_F8)
            {
                Rml::Debugger::SetVisible(!Rml::Debugger::IsVisible());
            }
            else if (key == Rml::Input::KI_0 && key_modifier & Rml::Input::KM_CTRL)
            {
                context->SetDensityIndependentPixelRatio(native_dp_ratio);
            }
            else if (key == Rml::Input::KI_1 && key_modifier & Rml::Input::KM_CTRL)
            {
                context->SetDensityIndependentPixelRatio(1.f);
            }
            else if ((key == Rml::Input::KI_OEM_MINUS || key == Rml::Input::KI_SUBTRACT) && key_modifier & Rml::Input::KM_CTRL)
            {
                // TODO: Save scale to config file and restore on load
                const float newDpi = Rml::Math::Max(context->GetDensityIndependentPixelRatio() / 1.1f, 0.5f);
                context->SetDensityIndependentPixelRatio(newDpi);

                const auto config = fluf->GetConfig();
                config->dpi = newDpi;
                ConfigHelper<FlufUiConfig, FlufUiConfig::configPathOverrides>::Save(*config);
            }
            else if ((key == Rml::Input::KI_OEM_PLUS || key == Rml::Input::KI_ADD) && key_modifier & Rml::Input::KM_CTRL)
            {
                const float newDpi = Rml::Math::Min(context->GetDensityIndependentPixelRatio() * 1.1f, 2.5f);
                context->SetDensityIndependentPixelRatio(newDpi);

                const auto config = fluf->GetConfig();
                config->dpi = newDpi;
                ConfigHelper<FlufUiConfig, FlufUiConfig::configPathOverrides>::Save(*config);
            }
            else
            {
                // Propagate the key down event to the context.
                result = true;
            }
        }
        else
        {
            // We arrive here when no priority keys are detected and the key was not consumed by the context. Check for shortcuts of lower priority.
            if (key == Rml::Input::KI_R && key_modifier & Rml::Input::KM_CTRL)
            {
                for (int i = 0; i < context->GetNumDocuments(); i++)
                {
                    Rml::ElementDocument* document = context->GetDocument(i);
                    const Rml::String& src = document->GetSourceURL();
                    if (src.size() > 4 && src.substr(src.size() - 4) == ".rml")
                    {
                        document->ReloadStyleSheet();
                    }
                }
            }
            else
            {
                result = true;
            }
        }

        return result;
    }
} // namespace RmlWin32
