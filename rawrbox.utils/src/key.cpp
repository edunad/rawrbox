
#include <rawrbox/utils/key.hpp>
#include <rawrbox/utils/string.hpp>

namespace rawrbox {

	std::string KeyUtils::keyToStr(uint32_t key) {
		switch (key) {
			case KEY_SPACE:
				return " ";
			case KEY_APOSTROPHE:
				return "'";
			case KEY_COMMA:
				return ",";
			case KEY_MINUS:
				return "-";
			case KEY_PERIOD:
				return ".";
			case KEY_SLASH:
				return "/";

			case KEY_0:
				return "0";
			case KEY_1:
				return "1";
			case KEY_2:
				return "2";
			case KEY_3:
				return "3";
			case KEY_4:
				return "4";
			case KEY_5:
				return "5";
			case KEY_6:
				return "6";
			case KEY_7:
				return "7";
			case KEY_8:
				return "8";
			case KEY_9:
				return "9";

			case KEY_SEMICOLON:
				return ";";
			case KEY_EQUAL:
				return "=";

			case KEY_A:
				return "A";
			case KEY_B:
				return "B";
			case KEY_C:
				return "C";
			case KEY_D:
				return "D";
			case KEY_E:
				return "E";
			case KEY_F:
				return "F";
			case KEY_G:
				return "G";
			case KEY_H:
				return "H";
			case KEY_I:
				return "I";
			case KEY_J:
				return "J";
			case KEY_K:
				return "K";
			case KEY_L:
				return "L";
			case KEY_M:
				return "M";
			case KEY_N:
				return "N";
			case KEY_O:
				return "O";
			case KEY_P:
				return "P";
			case KEY_Q:
				return "Q";
			case KEY_R:
				return "R";
			case KEY_S:
				return "S";
			case KEY_T:
				return "T";
			case KEY_U:
				return "U";
			case KEY_V:
				return "V";
			case KEY_W:
				return "W";
			case KEY_X:
				return "X";
			case KEY_Y:
				return "Y";
			case KEY_Z:
				return "Z";

			case KEY_LEFT_BRACKET:
				return "[";
			case KEY_BACKSLASH:
				return "\\";
			case KEY_RIGHT_BRACKET:
				return "]";
			case KEY_GRAVE_ACCENT:
				return "`";

			case KEY_ESCAPE:
				return "ESCAPE";
			case KEY_ENTER:
				return "ENTER";
			case KEY_TAB:
				return "TAB";
			case KEY_BACKSPACE:
				return "BACKSPACE";
			case KEY_INSERT:
				return "INSERT";
			case KEY_DELETE:
				return "DELETE";
			case KEY_RIGHT:
				return "RIGHT";
			case KEY_LEFT:
				return "LEFT";
			case KEY_DOWN:
				return "DOWN";
			case KEY_UP:
				return "UP";
			case KEY_PAGE_UP:
				return "PAGE_UP";
			case KEY_PAGE_DOWN:
				return "PAGE_DOWN";
			case KEY_HOME:
				return "HOME";
			case KEY_END:
				return "END";
			case KEY_CAPS_LOCK:
				return "CAPS_LOCK";
			case KEY_SCROLL_LOCK:
				return "SCROLL_LOCK";
			case KEY_NUM_LOCK:
				return "NUM_LOCK";
			case KEY_PRINT_SCREEN:
				return "PRINT_SCREEN";
			case KEY_PAUSE:
				return "PAUSE";

			case KEY_F1:
				return "F1";
			case KEY_F2:
				return "F2";
			case KEY_F3:
				return "F3";
			case KEY_F4:
				return "F4";
			case KEY_F5:
				return "F5";
			case KEY_F6:
				return "F6";
			case KEY_F7:
				return "F7";
			case KEY_F8:
				return "F8";
			case KEY_F9:
				return "F9";
			case KEY_F10:
				return "F10";
			case KEY_F11:
				return "F11";
			case KEY_F12:
				return "F12"; // we don't use > F12

			case KEY_KP_0:
				return "KP_0";
			case KEY_KP_1:
				return "KP_1";
			case KEY_KP_2:
				return "KP_2";
			case KEY_KP_3:
				return "KP_3";
			case KEY_KP_4:
				return "KP_4";
			case KEY_KP_5:
				return "KP_5";
			case KEY_KP_6:
				return "KP_6";
			case KEY_KP_7:
				return "KP_7";
			case KEY_KP_8:
				return "KP_8";
			case KEY_KP_9:
				return "KP_9";

			case KEY_KP_DECIMAL:
				return "KP_DECIMAL";
			case KEY_KP_DIVIDE:
				return "KP_DIVIDE";
			case KEY_KP_MULTIPLY:
				return "KP_MULTIPLY";
			case KEY_KP_SUBTRACT:
				return "KP_SUBTRACT";
			case KEY_KP_ADD:
				return "KP_ADD";
			case KEY_KP_ENTER:
				return "KP_ENTER";
			case KEY_KP_EQUAL:
				return "KP_EQUAL";

			case KEY_LEFT_SHIFT:
				return "LEFT_SHIFT";
			case KEY_LEFT_CONTROL:
				return "LEFT_CONTROL";
			case KEY_LEFT_ALT:
				return "LEFT_ALT";
			case KEY_LEFT_SUPER:
				return "LEFT_SUPER";

			case KEY_RIGHT_SHIFT:
				return "RIGHT_SHIFT";
			case KEY_RIGHT_CONTROL:
				return "RIGHT_CONTROL";
			case KEY_RIGHT_ALT:
				return "RIGHT_ALT";
			case KEY_RIGHT_SUPER:
				return "RIGHT_SUPER";
			case KEY_MENU:
				return "MENU";
			default:
				return "?";
		}
	}

	uint32_t KeyUtils::strToKey(const std::string& rawKey) {
		auto key = rawrbox::StrUtils::toUpper(rawKey);

		if (key == " ") return KEY_SPACE;
		if (key == "'") return KEY_APOSTROPHE;
		if (key == ",") return KEY_COMMA;
		if (key == "-") return KEY_MINUS;
		if (key == ".") return KEY_PERIOD;
		if (key == "/") return KEY_SLASH;

		if (key == "0") return KEY_0;
		if (key == "1") return KEY_1;
		if (key == "2") return KEY_2;
		if (key == "3") return KEY_3;
		if (key == "4") return KEY_4;
		if (key == "5") return KEY_5;
		if (key == "6") return KEY_6;
		if (key == "7") return KEY_7;
		if (key == "8") return KEY_8;
		if (key == "9") return KEY_9;

		if (key == ";") return KEY_SEMICOLON;
		if (key == "=") return KEY_EQUAL;

		if (key == "A") return KEY_A;
		if (key == "B") return KEY_B;
		if (key == "C") return KEY_C;
		if (key == "D") return KEY_D;
		if (key == "E") return KEY_E;
		if (key == "F") return KEY_F;
		if (key == "G") return KEY_G;
		if (key == "H") return KEY_H;
		if (key == "I") return KEY_I;
		if (key == "J") return KEY_J;
		if (key == "K") return KEY_K;
		if (key == "L") return KEY_L;
		if (key == "M") return KEY_M;
		if (key == "N") return KEY_N;
		if (key == "O") return KEY_O;
		if (key == "P") return KEY_P;
		if (key == "Q") return KEY_Q;
		if (key == "R") return KEY_R;
		if (key == "S") return KEY_S;
		if (key == "T") return KEY_T;
		if (key == "U") return KEY_U;
		if (key == "V") return KEY_V;
		if (key == "W") return KEY_W;
		if (key == "X") return KEY_X;
		if (key == "Y") return KEY_Y;
		if (key == "Z") return KEY_Z;

		if (key == "[") return KEY_LEFT_BRACKET;
		if (key == "\\") return KEY_BACKSLASH;
		if (key == "]") return KEY_RIGHT_BRACKET;
		if (key == "`") return KEY_GRAVE_ACCENT;

		if (key == "ESCAPE") return KEY_ESCAPE;
		if (key == "ENTER") return KEY_ENTER;
		if (key == "TAB") return KEY_TAB;
		if (key == "BACKSPACE") return KEY_BACKSPACE;
		if (key == "INSERT") return KEY_INSERT;
		if (key == "DELETE") return KEY_DELETE;

		if (key == "RIGHT") return KEY_RIGHT;
		if (key == "LEFT") return KEY_LEFT;
		if (key == "DOWN") return KEY_DOWN;
		if (key == "UP") return KEY_UP;

		if (key == "PAGE_UP") return KEY_PAGE_UP;
		if (key == "PAGE_DOWN") return KEY_PAGE_DOWN;
		if (key == "HOME") return KEY_HOME;
		if (key == "END") return KEY_END;
		if (key == "CAPS_LOCK") return KEY_CAPS_LOCK;
		if (key == "SCROLL_LOCK") return KEY_SCROLL_LOCK;
		if (key == "NUM_LOCK") return KEY_NUM_LOCK;
		if (key == "PRINT_SCREEN") return KEY_PRINT_SCREEN;
		if (key == "PAUSE") return KEY_PAUSE;

		if (key == "F1") return KEY_F1;
		if (key == "F2") return KEY_F2;
		if (key == "F3") return KEY_F3;
		if (key == "F4") return KEY_F4;
		if (key == "F5") return KEY_F5;
		if (key == "F6") return KEY_F6;
		if (key == "F7") return KEY_F7;
		if (key == "F8") return KEY_F8;
		if (key == "F9") return KEY_F9;
		if (key == "F10") return KEY_F10;
		if (key == "F11") return KEY_F11;
		if (key == "F12") return KEY_F12; // we don't use > F12

		if (key == "KP_0") return KEY_KP_0;
		if (key == "KP_1") return KEY_KP_1;
		if (key == "KP_2") return KEY_KP_2;
		if (key == "KP_3") return KEY_KP_3;
		if (key == "KP_4") return KEY_KP_4;
		if (key == "KP_5") return KEY_KP_5;
		if (key == "KP_6") return KEY_KP_6;
		if (key == "KP_7") return KEY_KP_7;
		if (key == "KP_8") return KEY_KP_8;
		if (key == "KP_9") return KEY_KP_9;

		if (key == "KP_DECIMAL") return KEY_KP_DECIMAL;
		if (key == "KP_DIVIDE") return KEY_KP_DIVIDE;
		if (key == "*") return KEY_KP_MULTIPLY;
		if (key == "KP_SUBTRACT") return KEY_KP_SUBTRACT;
		if (key == "+") return KEY_KP_ADD;
		if (key == "KP_ENTER") return KEY_KP_ENTER;
		if (key == "=") return KEY_KP_EQUAL;

		if (key == "LEFT_SHIFT") return KEY_LEFT_SHIFT;
		if (key == "LEFT_CONTROL") return KEY_LEFT_CONTROL;
		if (key == "LEFT_ALT") return KEY_LEFT_ALT;
		if (key == "LEFT_SUPER") return KEY_LEFT_SUPER;

		if (key == "RIGHT_SHIFT") return KEY_RIGHT_SHIFT;
		if (key == "RIGHT_CONTROL") return KEY_RIGHT_CONTROL;
		if (key == "RIGHT_ALT") return KEY_RIGHT_ALT;
		if (key == "RIGHT_SUPER") return KEY_RIGHT_SUPER;
		if (key == "MENU") return KEY_MENU;

		return 0;
	}
} // namespace rawrbox
