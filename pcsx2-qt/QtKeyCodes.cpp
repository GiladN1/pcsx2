// SPDX-FileCopyrightText: 2002-2025 PCSX2 Dev Team
// SPDX-License-Identifier: GPL-3.0+

#include "QtUtils.h"

#include "pcsx2/Input/InputManager.h"

#include "common/Pcsx2Defs.h"
#include "common/StringUtil.h"

#include "fmt/format.h"
#include "IconsPromptFont.h"

#include <QtGui/QKeyEvent>

u8 map_text_to_keycode(const QString& text)
{
	if (text == "!")
		return Qt::Key_1;
	if (text == "@")
		return Qt::Key_2;
	if (text == "#")
		return Qt::Key_3;
	if (text == "$")
		return Qt::Key_4;
	if (text == "%")
		return Qt::Key_5;
	if (text == "^")
		return Qt::Key_6;
	if (text == "&")
		return Qt::Key_7;
	if (text == "*")
		return Qt::Key_8;
	if (text == "(")
		return Qt::Key_9;
	if (text == ")")
		return Qt::Key_0;
	if (text == "_")
		return Qt::Key_Minus;
	if (text == "+")
		return Qt::Key_Equal;
	if (text == "?")
		return Qt::Key_Slash;
	if (text == ":")
		return Qt::Key_Semicolon;
	if (text == "\"")
		return Qt::Key_Apostrophe;
	if (text == "~")
		return Qt::Key_QuoteLeft;
	if (text == "<")
		return Qt::Key_Comma;
	if (text == ">")
		return Qt::Key_Period;
	if (text == "|")
		return Qt::Key_Backslash;
	if (text == "{")
		return Qt::Key_BracketLeft;
	if (text == "}")
		return Qt::Key_BracketRight;
	return 0; // No remapping
}

struct KeyCodeName
{
	int code;
	const char* name;
	const char* icon_name;
};

static constexpr const KeyCodeName s_qt_key_names[] = {
	{Qt::Key_Escape, "Escape", ICON_PF_ESC},
	{Qt::Key_Tab, "Tab", ICON_PF_TAB},
	{Qt::Key_Backtab, "Backtab", nullptr},
	{Qt::Key_Backspace, "Backspace", ICON_PF_BACKSPACE},
	{Qt::Key_Return, "Return", ICON_PF_ENTER},
	{Qt::Key_Enter, "Enter", ICON_PF_ENTER},
	{Qt::Key_Insert, "Insert", ICON_PF_INSERT},
	{Qt::Key_Delete, "Delete", ICON_PF_DELETE},
	{Qt::Key_Pause, "Pause", ICON_PF_PAUSE},
	{Qt::Key_Print, "Print", ICON_PF_PRTSC},
	{Qt::Key_SysReq, "SysReq", ICON_PF_PAUSE},
	{Qt::Key_Clear, "Clear", nullptr},
	{Qt::Key_Home, "Home", ICON_PF_HOME},
	{Qt::Key_End, "End", ICON_PF_END},
	{Qt::Key_Left, "Left", ICON_PF_ARROW_LEFT},
	{Qt::Key_Up, "Up", ICON_PF_ARROW_UP},
	{Qt::Key_Right, "Right", ICON_PF_ARROW_RIGHT},
	{Qt::Key_Down, "Down", ICON_PF_ARROW_DOWN},
	{Qt::Key_PageUp, "PageUp", ICON_PF_PAGE_UP},
	{Qt::Key_PageDown, "PageDown", ICON_PF_PAGE_DOWN},
	{Qt::Key_Shift, "Shift", ICON_PF_SHIFT},
	{Qt::Key_Control, "Control", ICON_PF_CTRL},
	{Qt::Key_Meta, "Meta", ICON_PF_SUPER},
	{Qt::Key_Alt, "Alt", ICON_PF_ALT},
	{Qt::Key_CapsLock, "CapsLock", ICON_PF_CAPS},
	{Qt::Key_NumLock, "NumLock", ICON_PF_NUMLOCK},
	{Qt::Key_ScrollLock, "ScrollLock", ICON_PF_SCRLK},
	{Qt::Key_F1, "F1", ICON_PF_F1},
	{Qt::Key_F2, "F2", ICON_PF_F2},
	{Qt::Key_F3, "F3", ICON_PF_F3},
	{Qt::Key_F4, "F4", ICON_PF_F4},
	{Qt::Key_F5, "F5", ICON_PF_F5},
	{Qt::Key_F6, "F6", ICON_PF_F6},
	{Qt::Key_F7, "F7", ICON_PF_F7},
	{Qt::Key_F8, "F8", ICON_PF_F8},
	{Qt::Key_F9, "F9", ICON_PF_F9},
	{Qt::Key_F10, "F10", ICON_PF_F10},
	{Qt::Key_F11, "F11", ICON_PF_F11},
	{Qt::Key_F12, "F12", ICON_PF_F12},
	{Qt::Key_F13, "F13", nullptr},
	{Qt::Key_F14, "F14", nullptr},
	{Qt::Key_F15, "F15", nullptr},
	{Qt::Key_F16, "F16", nullptr},
	{Qt::Key_F17, "F17", nullptr},
	{Qt::Key_F18, "F18", nullptr},
	{Qt::Key_F19, "F19", nullptr},
	{Qt::Key_F20, "F20", nullptr},
	{Qt::Key_F21, "F21", nullptr},
	{Qt::Key_F22, "F22", nullptr},
	{Qt::Key_F23, "F23", nullptr},
	{Qt::Key_F24, "F24", nullptr},
	{Qt::Key_F25, "F25", nullptr},
	{Qt::Key_F26, "F26", nullptr},
	{Qt::Key_F27, "F27", nullptr},
	{Qt::Key_F28, "F28", nullptr},
	{Qt::Key_F29, "F29", nullptr},
	{Qt::Key_F30, "F30", nullptr},
	{Qt::Key_F31, "F31", nullptr},
	{Qt::Key_F32, "F32", nullptr},
	{Qt::Key_F33, "F33", nullptr},
	{Qt::Key_F34, "F34", nullptr},
	{Qt::Key_F35, "F35", nullptr},
	{Qt::Key_Super_L, "Super_L", nullptr},
	{Qt::Key_Super_R, "Super_R", nullptr},
	{Qt::Key_Menu, "Menu", nullptr},
	{Qt::Key_Hyper_L, "Hyper_L", nullptr},
	{Qt::Key_Hyper_R, "Hyper_R", nullptr},
	{Qt::Key_Help, "Help", nullptr},
	{Qt::Key_Direction_L, "Direction_L", nullptr},
	{Qt::Key_Direction_R, "Direction_R", nullptr},
	{Qt::Key_Space, "Space", ICON_PF_SPACE},
	{Qt::Key_Any, "Any", nullptr},
	{Qt::Key_Exclam, "Exclam", nullptr},
	{Qt::Key_QuoteDbl, "QuoteDbl", nullptr},
	{Qt::Key_NumberSign, "NumberSign", nullptr},
	{Qt::Key_Dollar, "Dollar", nullptr},
	{Qt::Key_Percent, "Percent", nullptr},
	{Qt::Key_Ampersand, "Ampersand", nullptr},
	{Qt::Key_Apostrophe, "Apostrophe", nullptr},
	{Qt::Key_ParenLeft, "ParenLeft", nullptr},
	{Qt::Key_ParenRight, "ParenRight", nullptr},
	{Qt::Key_Asterisk, "Asterisk", nullptr},
	{Qt::Key_Plus, "Plus", nullptr},
	{Qt::Key_Comma, "Comma", nullptr},
	{Qt::Key_Minus, "Minus", nullptr},
	{Qt::Key_Period, "Period", nullptr},
	{Qt::Key_Slash, "Slash", nullptr},
	{Qt::Key_0, "0", ICON_PF_0},
	{Qt::Key_1, "1", ICON_PF_1},
	{Qt::Key_2, "2", ICON_PF_2},
	{Qt::Key_3, "3", ICON_PF_3},
	{Qt::Key_4, "4", ICON_PF_4},
	{Qt::Key_5, "5", ICON_PF_5},
	{Qt::Key_6, "6", ICON_PF_6},
	{Qt::Key_7, "7", ICON_PF_7},
	{Qt::Key_8, "8", ICON_PF_8},
	{Qt::Key_9, "9", ICON_PF_9},
	{Qt::Key_Colon, "Colon", nullptr},
	{Qt::Key_Semicolon, "Semicolon", nullptr},
	{Qt::Key_Less, "Less", nullptr},
	{Qt::Key_Equal, "Equal", nullptr},
	{Qt::Key_Greater, "Greater", nullptr},
	{Qt::Key_Question, "Question", nullptr},
	{Qt::Key_At, "At", nullptr},
	{Qt::Key_A, "A", ICON_PF_KEY_A},
	{Qt::Key_B, "B", ICON_PF_KEY_B},
	{Qt::Key_C, "C", ICON_PF_KEY_C},
	{Qt::Key_D, "D", ICON_PF_KEY_D},
	{Qt::Key_E, "E", ICON_PF_KEY_E},
	{Qt::Key_F, "F", ICON_PF_KEY_F},
	{Qt::Key_G, "G", ICON_PF_KEY_G},
	{Qt::Key_H, "H", ICON_PF_KEY_H},
	{Qt::Key_I, "I", ICON_PF_KEY_I},
	{Qt::Key_J, "J", ICON_PF_KEY_J},
	{Qt::Key_K, "K", ICON_PF_KEY_K},
	{Qt::Key_L, "L", ICON_PF_KEY_L},
	{Qt::Key_M, "M", ICON_PF_KEY_M},
	{Qt::Key_N, "N", ICON_PF_KEY_N},
	{Qt::Key_O, "O", ICON_PF_KEY_O},
	{Qt::Key_P, "P", ICON_PF_KEY_P},
	{Qt::Key_Q, "Q", ICON_PF_KEY_Q},
	{Qt::Key_R, "R", ICON_PF_KEY_R},
	{Qt::Key_S, "S", ICON_PF_KEY_S},
	{Qt::Key_T, "T", ICON_PF_KEY_T},
	{Qt::Key_U, "U", ICON_PF_KEY_U},
	{Qt::Key_V, "V", ICON_PF_KEY_V},
	{Qt::Key_W, "W", ICON_PF_KEY_W},
	{Qt::Key_X, "X", ICON_PF_KEY_X},
	{Qt::Key_Y, "Y", ICON_PF_KEY_Y},
	{Qt::Key_Z, "Z", ICON_PF_KEY_Z},
	{Qt::Key_BracketLeft, "BracketLeft", nullptr},
	{Qt::Key_Backslash, "Backslash", nullptr},
	{Qt::Key_BracketRight, "BracketRight", nullptr},
	{Qt::Key_AsciiCircum, "AsciiCircum", nullptr},
	{Qt::Key_Underscore, "Underscore", nullptr},
	{Qt::Key_QuoteLeft, "QuoteLeft", nullptr},
	{Qt::Key_BraceLeft, "BraceLeft", nullptr},
	{Qt::Key_Bar, "Bar", nullptr},
	{Qt::Key_BraceRight, "BraceRight", nullptr},
	{Qt::Key_AsciiTilde, "AsciiTilde", nullptr},
	{Qt::Key_nobreakspace, "nobreakspace", nullptr},
	{Qt::Key_exclamdown, "exclamdown", nullptr},
	{Qt::Key_cent, "cent", nullptr},
	{Qt::Key_sterling, "sterling", nullptr},
	{Qt::Key_currency, "currency", nullptr},
	{Qt::Key_yen, "yen", nullptr},
	{Qt::Key_brokenbar, "brokenbar", nullptr},
	{Qt::Key_section, "section", nullptr},
	{Qt::Key_diaeresis, "diaeresis", nullptr},
	{Qt::Key_copyright, "copyright", nullptr},
	{Qt::Key_ordfeminine, "ordfeminine", nullptr},
	{Qt::Key_guillemotleft, "guillemotleft", nullptr},
	{Qt::Key_notsign, "notsign", nullptr},
	{Qt::Key_hyphen, "hyphen", nullptr},
	{Qt::Key_registered, "registered", nullptr},
	{Qt::Key_macron, "macron", nullptr},
	{Qt::Key_degree, "degree", nullptr},
	{Qt::Key_plusminus, "plusminus", nullptr},
	{Qt::Key_twosuperior, "twosuperior", nullptr},
	{Qt::Key_threesuperior, "threesuperior", nullptr},
	{Qt::Key_acute, "acute", nullptr},
	{Qt::Key_micro, "micro", nullptr},
	{Qt::Key_paragraph, "paragraph", nullptr},
	{Qt::Key_periodcentered, "periodcentered", nullptr},
	{Qt::Key_cedilla, "cedilla", nullptr},
	{Qt::Key_onesuperior, "onesuperior", nullptr},
	{Qt::Key_masculine, "masculine", nullptr},
	{Qt::Key_guillemotright, "guillemotright", nullptr},
	{Qt::Key_onequarter, "onequarter", nullptr},
	{Qt::Key_onehalf, "onehalf", nullptr},
	{Qt::Key_threequarters, "threequarters", nullptr},
	{Qt::Key_questiondown, "questiondown", nullptr},
	{Qt::Key_Agrave, "Agrave", nullptr},
	{Qt::Key_Aacute, "Aacute", nullptr},
	{Qt::Key_Acircumflex, "Acircumflex", nullptr},
	{Qt::Key_Atilde, "Atilde", nullptr},
	{Qt::Key_Adiaeresis, "Adiaeresis", nullptr},
	{Qt::Key_Aring, "Aring", nullptr},
	{Qt::Key_AE, "AE", nullptr},
	{Qt::Key_Ccedilla, "Ccedilla", nullptr},
	{Qt::Key_Egrave, "Egrave", nullptr},
	{Qt::Key_Eacute, "Eacute", nullptr},
	{Qt::Key_Ecircumflex, "Ecircumflex", nullptr},
	{Qt::Key_Ediaeresis, "Ediaeresis", nullptr},
	{Qt::Key_Igrave, "Igrave", nullptr},
	{Qt::Key_Iacute, "Iacute", nullptr},
	{Qt::Key_Icircumflex, "Icircumflex", nullptr},
	{Qt::Key_Idiaeresis, "Idiaeresis", nullptr},
	{Qt::Key_ETH, "ETH", nullptr},
	{Qt::Key_Ntilde, "Ntilde", nullptr},
	{Qt::Key_Ograve, "Ograve", nullptr},
	{Qt::Key_Oacute, "Oacute", nullptr},
	{Qt::Key_Ocircumflex, "Ocircumflex", nullptr},
	{Qt::Key_Otilde, "Otilde", nullptr},
	{Qt::Key_Odiaeresis, "Odiaeresis", nullptr},
	{Qt::Key_multiply, "multiply", nullptr},
	{Qt::Key_Ooblique, "Ooblique", nullptr},
	{Qt::Key_Ugrave, "Ugrave", nullptr},
	{Qt::Key_Uacute, "Uacute", nullptr},
	{Qt::Key_Ucircumflex, "Ucircumflex", nullptr},
	{Qt::Key_Udiaeresis, "Udiaeresis", nullptr},
	{Qt::Key_Yacute, "Yacute", nullptr},
	{Qt::Key_THORN, "THORN", nullptr},
	{Qt::Key_ssharp, "ssharp", nullptr},
	{Qt::Key_division, "division", nullptr},
	{Qt::Key_ydiaeresis, "ydiaeresis", nullptr},
	{Qt::Key_AltGr, "AltGr", nullptr},
	{Qt::Key_Multi_key, "Multi_key", nullptr},
	{Qt::Key_Codeinput, "Codeinput", nullptr},
	{Qt::Key_SingleCandidate, "SingleCandidate", nullptr},
	{Qt::Key_MultipleCandidate, "MultipleCandidate", nullptr},
	{Qt::Key_PreviousCandidate, "PreviousCandidate", nullptr},
	{Qt::Key_Mode_switch, "Mode_switch", nullptr},
	{Qt::Key_Kanji, "Kanji", nullptr},
	{Qt::Key_Muhenkan, "Muhenkan", nullptr},
	{Qt::Key_Henkan, "Henkan", nullptr},
	{Qt::Key_Romaji, "Romaji", nullptr},
	{Qt::Key_Hiragana, "Hiragana", nullptr},
	{Qt::Key_Katakana, "Katakana", nullptr},
	{Qt::Key_Hiragana_Katakana, "Hiragana_Katakana", nullptr},
	{Qt::Key_Zenkaku, "Zenkaku", nullptr},
	{Qt::Key_Hankaku, "Hankaku", nullptr},
	{Qt::Key_Zenkaku_Hankaku, "Zenkaku_Hankaku", nullptr},
	{Qt::Key_Touroku, "Touroku", nullptr},
	{Qt::Key_Massyo, "Massyo", nullptr},
	{Qt::Key_Kana_Lock, "Kana_Lock", nullptr},
	{Qt::Key_Kana_Shift, "Kana_Shift", nullptr},
	{Qt::Key_Eisu_Shift, "Eisu_Shift", nullptr},
	{Qt::Key_Eisu_toggle, "Eisu_toggle", nullptr},
	{Qt::Key_Hangul, "Hangul", nullptr},
	{Qt::Key_Hangul_Start, "Hangul_Start", nullptr},
	{Qt::Key_Hangul_End, "Hangul_End", nullptr},
	{Qt::Key_Hangul_Hanja, "Hangul_Hanja", nullptr},
	{Qt::Key_Hangul_Jamo, "Hangul_Jamo", nullptr},
	{Qt::Key_Hangul_Romaja, "Hangul_Romaja", nullptr},
	{Qt::Key_Hangul_Jeonja, "Hangul_Jeonja", nullptr},
	{Qt::Key_Hangul_Banja, "Hangul_Banja", nullptr},
	{Qt::Key_Hangul_PreHanja, "Hangul_PreHanja", nullptr},
	{Qt::Key_Hangul_PostHanja, "Hangul_PostHanja", nullptr},
	{Qt::Key_Hangul_Special, "Hangul_Special", nullptr},
	{Qt::Key_Dead_Grave, "Dead_Grave", nullptr},
	{Qt::Key_Dead_Acute, "Dead_Acute", nullptr},
	{Qt::Key_Dead_Circumflex, "Dead_Circumflex", nullptr},
	{Qt::Key_Dead_Tilde, "Dead_Tilde", nullptr},
	{Qt::Key_Dead_Macron, "Dead_Macron", nullptr},
	{Qt::Key_Dead_Breve, "Dead_Breve", nullptr},
	{Qt::Key_Dead_Abovedot, "Dead_Abovedot", nullptr},
	{Qt::Key_Dead_Diaeresis, "Dead_Diaeresis", nullptr},
	{Qt::Key_Dead_Abovering, "Dead_Abovering", nullptr},
	{Qt::Key_Dead_Doubleacute, "Dead_Doubleacute", nullptr},
	{Qt::Key_Dead_Caron, "Dead_Caron", nullptr},
	{Qt::Key_Dead_Cedilla, "Dead_Cedilla", nullptr},
	{Qt::Key_Dead_Ogonek, "Dead_Ogonek", nullptr},
	{Qt::Key_Dead_Iota, "Dead_Iota", nullptr},
	{Qt::Key_Dead_Voiced_Sound, "Dead_Voiced_Sound", nullptr},
	{Qt::Key_Dead_Semivoiced_Sound, "Dead_Semivoiced_Sound", nullptr},
	{Qt::Key_Dead_Belowdot, "Dead_Belowdot", nullptr},
	{Qt::Key_Dead_Hook, "Dead_Hook", nullptr},
	{Qt::Key_Dead_Horn, "Dead_Horn", nullptr},
	{Qt::Key_Back, "Back", nullptr},
	{Qt::Key_Forward, "Forward", nullptr},
	{Qt::Key_Stop, "Stop", nullptr},
	{Qt::Key_Refresh, "Refresh", nullptr},
	{Qt::Key_VolumeDown, "VolumeDown", nullptr},
	{Qt::Key_VolumeMute, "VolumeMute", nullptr},
	{Qt::Key_VolumeUp, "VolumeUp", nullptr},
	{Qt::Key_BassBoost, "BassBoost", nullptr},
	{Qt::Key_BassUp, "BassUp", nullptr},
	{Qt::Key_BassDown, "BassDown", nullptr},
	{Qt::Key_TrebleUp, "TrebleUp", nullptr},
	{Qt::Key_TrebleDown, "TrebleDown", nullptr},
	{Qt::Key_MediaPlay, "MediaPlay", nullptr},
	{Qt::Key_MediaStop, "MediaStop", nullptr},
	{Qt::Key_MediaPrevious, "MediaPrevious", nullptr},
	{Qt::Key_MediaNext, "MediaNext", nullptr},
	{Qt::Key_MediaRecord, "MediaRecord", nullptr},
	{Qt::Key_MediaPause, "MediaPause", nullptr},
	{Qt::Key_MediaTogglePlayPause, "MediaTogglePlayPause", nullptr},
	{Qt::Key_HomePage, "HomePage", nullptr},
	{Qt::Key_Favorites, "Favorites", nullptr},
	{Qt::Key_Search, "Search", nullptr},
	{Qt::Key_Standby, "Standby", nullptr},
	{Qt::Key_OpenUrl, "OpenUrl", nullptr},
	{Qt::Key_LaunchMail, "LaunchMail", nullptr},
	{Qt::Key_LaunchMedia, "LaunchMedia", nullptr},
	{Qt::Key_Launch0, "Launch0", nullptr},
	{Qt::Key_Launch1, "Launch1", nullptr},
	{Qt::Key_Launch2, "Launch2", nullptr},
	{Qt::Key_Launch3, "Launch3", nullptr},
	{Qt::Key_Launch4, "Launch4", nullptr},
	{Qt::Key_Launch5, "Launch5", nullptr},
	{Qt::Key_Launch6, "Launch6", nullptr},
	{Qt::Key_Launch7, "Launch7", nullptr},
	{Qt::Key_Launch8, "Launch8", nullptr},
	{Qt::Key_Launch9, "Launch9", nullptr},
	{Qt::Key_LaunchA, "LaunchA", nullptr},
	{Qt::Key_LaunchB, "LaunchB", nullptr},
	{Qt::Key_LaunchC, "LaunchC", nullptr},
	{Qt::Key_LaunchD, "LaunchD", nullptr},
	{Qt::Key_LaunchE, "LaunchE", nullptr},
	{Qt::Key_LaunchF, "LaunchF", nullptr},
	{Qt::Key_MonBrightnessUp, "MonBrightnessUp", nullptr},
	{Qt::Key_MonBrightnessDown, "MonBrightnessDown", nullptr},
	{Qt::Key_KeyboardLightOnOff, "KeyboardLightOnOff", nullptr},
	{Qt::Key_KeyboardBrightnessUp, "KeyboardBrightnessUp", nullptr},
	{Qt::Key_KeyboardBrightnessDown, "KeyboardBrightnessDown", nullptr},
	{Qt::Key_PowerOff, "PowerOff", nullptr},
	{Qt::Key_WakeUp, "WakeUp", nullptr},
	{Qt::Key_Eject, "Eject", nullptr},
	{Qt::Key_ScreenSaver, "ScreenSaver", nullptr},
	{Qt::Key_WWW, "WWW", nullptr},
	{Qt::Key_Memo, "Memo", nullptr},
	{Qt::Key_LightBulb, "LightBulb", nullptr},
	{Qt::Key_Shop, "Shop", nullptr},
	{Qt::Key_History, "History", nullptr},
	{Qt::Key_AddFavorite, "AddFavorite", nullptr},
	{Qt::Key_HotLinks, "HotLinks", nullptr},
	{Qt::Key_BrightnessAdjust, "BrightnessAdjust", nullptr},
	{Qt::Key_Finance, "Finance", nullptr},
	{Qt::Key_Community, "Community", nullptr},
	{Qt::Key_AudioRewind, "AudioRewind", nullptr},
	{Qt::Key_BackForward, "BackForward", nullptr},
	{Qt::Key_ApplicationLeft, "ApplicationLeft", nullptr},
	{Qt::Key_ApplicationRight, "ApplicationRight", nullptr},
	{Qt::Key_Book, "Book", nullptr},
	{Qt::Key_CD, "CD", nullptr},
	{Qt::Key_Calculator, "Calculator", nullptr},
	{Qt::Key_ToDoList, "ToDoList", nullptr},
	{Qt::Key_ClearGrab, "ClearGrab", nullptr},
	{Qt::Key_Close, "Close", nullptr},
	{Qt::Key_Copy, "Copy", nullptr},
	{Qt::Key_Cut, "Cut", nullptr},
	{Qt::Key_Display, "Display", nullptr},
	{Qt::Key_DOS, "DOS", nullptr},
	{Qt::Key_Documents, "Documents", nullptr},
	{Qt::Key_Excel, "Excel", nullptr},
	{Qt::Key_Explorer, "Explorer", nullptr},
	{Qt::Key_Game, "Game", nullptr},
	{Qt::Key_Go, "Go", nullptr},
	{Qt::Key_iTouch, "iTouch", nullptr},
	{Qt::Key_LogOff, "LogOff", nullptr},
	{Qt::Key_Market, "Market", nullptr},
	{Qt::Key_Meeting, "Meeting", nullptr},
	{Qt::Key_MenuKB, "MenuKB", nullptr},
	{Qt::Key_MenuPB, "MenuPB", nullptr},
	{Qt::Key_MySites, "MySites", nullptr},
	{Qt::Key_News, "News", nullptr},
	{Qt::Key_OfficeHome, "OfficeHome", nullptr},
	{Qt::Key_Option, "Option", nullptr},
	{Qt::Key_Paste, "Paste", nullptr},
	{Qt::Key_Phone, "Phone", nullptr},
	{Qt::Key_Calendar, "Calendar", nullptr},
	{Qt::Key_Reply, "Reply", nullptr},
	{Qt::Key_Reload, "Reload", nullptr},
	{Qt::Key_RotateWindows, "RotateWindows", nullptr},
	{Qt::Key_RotationPB, "RotationPB", nullptr},
	{Qt::Key_RotationKB, "RotationKB", nullptr},
	{Qt::Key_Save, "Save", nullptr},
	{Qt::Key_Send, "Send", nullptr},
	{Qt::Key_Spell, "Spell", nullptr},
	{Qt::Key_SplitScreen, "SplitScreen", nullptr},
	{Qt::Key_Support, "Support", nullptr},
	{Qt::Key_TaskPane, "TaskPane", nullptr},
	{Qt::Key_Terminal, "Terminal", nullptr},
	{Qt::Key_Tools, "Tools", nullptr},
	{Qt::Key_Travel, "Travel", nullptr},
	{Qt::Key_Video, "Video", nullptr},
	{Qt::Key_Word, "Word", nullptr},
	{Qt::Key_Xfer, "Xfer", nullptr},
	{Qt::Key_ZoomIn, "ZoomIn", nullptr},
	{Qt::Key_ZoomOut, "ZoomOut", nullptr},
	{Qt::Key_Away, "Away", nullptr},
	{Qt::Key_Messenger, "Messenger", nullptr},
	{Qt::Key_WebCam, "WebCam", nullptr},
	{Qt::Key_MailForward, "MailForward", nullptr},
	{Qt::Key_Pictures, "Pictures", nullptr},
	{Qt::Key_Music, "Music", nullptr},
	{Qt::Key_Battery, "Battery", nullptr},
	{Qt::Key_Bluetooth, "Bluetooth", nullptr},
	{Qt::Key_WLAN, "WLAN", nullptr},
	{Qt::Key_UWB, "UWB", nullptr},
	{Qt::Key_AudioForward, "AudioForward", nullptr},
	{Qt::Key_AudioRepeat, "AudioRepeat", nullptr},
	{Qt::Key_AudioRandomPlay, "AudioRandomPlay", nullptr},
	{Qt::Key_Subtitle, "Subtitle", nullptr},
	{Qt::Key_AudioCycleTrack, "AudioCycleTrack", nullptr},
	{Qt::Key_Time, "Time", nullptr},
	{Qt::Key_Hibernate, "Hibernate", nullptr},
	{Qt::Key_View, "View", nullptr},
	{Qt::Key_TopMenu, "TopMenu", nullptr},
	{Qt::Key_PowerDown, "PowerDown", nullptr},
	{Qt::Key_Suspend, "Suspend", nullptr},
	{Qt::Key_ContrastAdjust, "ContrastAdjust", nullptr},
	{Qt::Key_LaunchG, "LaunchG", nullptr},
	{Qt::Key_LaunchH, "LaunchH", nullptr},
	{Qt::Key_TouchpadToggle, "TouchpadToggle", nullptr},
	{Qt::Key_TouchpadOn, "TouchpadOn", nullptr},
	{Qt::Key_TouchpadOff, "TouchpadOff", nullptr},
	{Qt::Key_MicMute, "MicMute", nullptr},
	{Qt::Key_Red, "Red", nullptr},
	{Qt::Key_Green, "Green", nullptr},
	{Qt::Key_Yellow, "Yellow", nullptr},
	{Qt::Key_Blue, "Blue", nullptr},
	{Qt::Key_ChannelUp, "ChannelUp", nullptr},
	{Qt::Key_ChannelDown, "ChannelDown", nullptr},
	{Qt::Key_Guide, "Guide", nullptr},
	{Qt::Key_Info, "Info", nullptr},
	{Qt::Key_Settings, "Settings", nullptr},
	{Qt::Key_MicVolumeUp, "MicVolumeUp", nullptr},
	{Qt::Key_MicVolumeDown, "MicVolumeDown", nullptr},
	{Qt::Key_New, "New", nullptr},
	{Qt::Key_Open, "Open", nullptr},
	{Qt::Key_Find, "Find", nullptr},
	{Qt::Key_Undo, "Undo", nullptr},
	{Qt::Key_Redo, "Redo", nullptr},
	{Qt::Key_MediaLast, "MediaLast", nullptr},
	{Qt::Key_Select, "Select", nullptr},
	{Qt::Key_Yes, "Yes", nullptr},
	{Qt::Key_No, "No", nullptr},
	{Qt::Key_Cancel, "Cancel", nullptr},
	{Qt::Key_Printer, "Printer", nullptr},
	{Qt::Key_Execute, "Execute", nullptr},
	{Qt::Key_Sleep, "Sleep", nullptr},
	{Qt::Key_Play, "Play", nullptr},
	{Qt::Key_Zoom, "Zoom", nullptr},
	{Qt::Key_Exit, "Exit", nullptr},
	{Qt::Key_Context1, "Context1", nullptr},
	{Qt::Key_Context2, "Context2", nullptr},
	{Qt::Key_Context3, "Context3", nullptr},
	{Qt::Key_Context4, "Context4", nullptr},
	{Qt::Key_Call, "Call", nullptr},
	{Qt::Key_Hangup, "Hangup", nullptr},
	{Qt::Key_Flip, "Flip", nullptr},
	{Qt::Key_ToggleCallHangup, "ToggleCallHangup", nullptr},
	{Qt::Key_VoiceDial, "VoiceDial", nullptr},
	{Qt::Key_LastNumberRedial, "LastNumberRedial", nullptr},
	{Qt::Key_Camera, "Camera", nullptr},
	{Qt::Key_CameraFocus, "CameraFocus", nullptr}};

std::optional<u32> InputManager::ConvertHostKeyboardStringToCode(const std::string_view str)
{
	std::string_view compare_name = str;
	u32 modifier_bits = 0;
	if (compare_name.starts_with("Numpad"))
	{
		compare_name = compare_name.substr(6);
		modifier_bits |= Qt::KeypadModifier;
	}

	for (const KeyCodeName& name : s_qt_key_names)
	{
		if (compare_name == name.name)
			return static_cast<u32>(name.code) | modifier_bits;
	}

	return std::nullopt;
}

std::optional<std::string> InputManager::ConvertHostKeyboardCodeToString(u32 code)
{
	std::optional<std::string> ret;

	const u32 modifier_bits = (code & Qt::KeyboardModifierMask);
	const u32 masked_code = (code & ~Qt::KeyboardModifierMask);

	for (const KeyCodeName& name : s_qt_key_names)
	{
		if (static_cast<int>(masked_code) == name.code)
		{
			if (modifier_bits & Qt::KeypadModifier)
				ret = fmt::format("Numpad{}", name.name);
			else
				ret = std::string(name.name);

			break;
		}
	}

	return ret;
}

const char* InputManager::ConvertHostKeyboardCodeToIcon(u32 code)
{
	if (code & Qt::KeyboardModifierMask)
		return nullptr;

	const u32 masked_code = (code & ~Qt::KeyboardModifierMask);
	for (const KeyCodeName& name : s_qt_key_names)
	{
		if (static_cast<int>(masked_code) == name.code)
			return name.icon_name;
	}

	return nullptr;
}

u32 QtUtils::KeyEventToCode(const QKeyEvent* ev)
{
	Qt::KeyboardModifiers modifiers = ev->modifiers();
	const QString text = ev->text();
	// Map special text symbols to keycodes if we're using Shift modifier.
	// Also check that we're not using Keypad modifier otherwise "NumpadAsterisk" would return "8" keycode
	// and "NumpadPlus" would return "Equal" keycode.
	const bool set_keycode = (modifiers & Qt::ShiftModifier) && !(modifiers & Qt::KeypadModifier);
	const u8 keycode = set_keycode ? map_text_to_keycode(text) : 0;
	int key = ev->key();

	if (keycode != 0)
		key = keycode; // Override key if mapped

#ifdef __APPLE__
	// On macOS, Qt applies the Keypad modifier regardless of whether the arrow keys, or numpad was pressed.
	// The only way to differentiate between the keypad and the arrow keys is by the text.
	// Hopefully some keyboard layouts don't change the numpad positioning...
	if (modifiers & Qt::KeypadModifier && key >= Qt::Key_Insert && key <= Qt::Key_PageDown)
	{
		if (ev->text().isEmpty())
		{
			// Drop the modifier, because it's probably not actually a numpad push.
			modifiers &= ~Qt::KeypadModifier;
		}
	}
#endif

	return static_cast<u32>(key) | (static_cast<u32>(modifiers) & static_cast<u32>(Qt::KeypadModifier));
}
