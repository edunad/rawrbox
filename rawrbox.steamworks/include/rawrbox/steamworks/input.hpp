#pragma once

#include <rawrbox/utils/event.hpp>
#include <rawrbox/utils/logger.hpp>

#include <steam/steam_api.h>

#include <memory>

namespace rawrbox {
	enum class Controller {
		Unknown,
		Steam,
		SteamDeck,
		XBox360,
		XBoxOne,
		PS4,
		PS5,
		Generic
	};

	enum class ControllerAction {
		INGAME,
		EDITOR
	};

	enum class ControllerActionAnalogHandle {
		Move,
		Look
	};

	enum class ControllerActionHandle {
		Use
	};

	class ControllerJoystickData {
	public:
		float x;
		float y;

		ControllerJoystickData(float x_, float y_) : x(x_), y(y_) {}
	};

	class SteamINPUT {
	protected:
		// CONTROLLER ----
		static InputHandle_t _inputID;
		static InputActionSetHandle_t _actionID;
		static std::array<InputHandle_t, STEAM_INPUT_MAX_COUNT> _controllerHandle;

		static bool _usingController;
		// ------------

		static std::unique_ptr<rawrbox::Logger> _logger;

	public:
		static rawrbox::Event<> onControllerConnected;
		static rawrbox::Event<> onControllerDisconnected;

		static void init();
		static void update();
		static void shutdown();

		// CONTROLLER ---
		static void setControllerAction(rawrbox::ControllerAction action, int index = 0);
		static rawrbox::ControllerJoystickData getAnalogData(rawrbox::ControllerActionAnalogHandle handle);
		static bool getButtonData(rawrbox::ControllerActionHandle handle);
		// ---------------

		// UTILS -----
		static bool showGamepadInput(int x, int y, int width, int height);

		static bool isUsingController();
		static int getConnectedControllers();
		static rawrbox::Controller getControllerType(int slot = 0);
		// ------------
	};
} // namespace rawrbox
