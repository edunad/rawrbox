#include <rawrbox/steamworks/input.hpp>
// --
#include <magic_enum.hpp>

namespace rawrbox {
	// PRIVATE ----------
	// CONTROLLER ----
	InputHandle_t SteamINPUT::_inputID = 0;
	InputActionSetHandle_t SteamINPUT::_actionID = 0;
	std::array<InputHandle_t, STEAM_INPUT_MAX_COUNT> SteamINPUT::_controllerHandle = {};

	bool SteamINPUT::_usingController = false;
	// ------------

	std::unique_ptr<rawrbox::Logger> SteamINPUT::_logger = std::make_unique<rawrbox::Logger>("RawrBox-SteamINPUT");
	// ----------------

	// PUBLIC ------
	rawrbox::Event<> SteamINPUT::onControllerConnected = {};
	rawrbox::Event<> SteamINPUT::onControllerDisconnected = {};
	// ------------

	void SteamINPUT::init() {
		if (SteamInput() == nullptr) CRITICAL_RAWRBOX("SteamInput is null");
		SteamInput()->Init(false);
	}

	void SteamINPUT::update() {
		int controllers = getConnectedControllers();

		if (!_usingController && controllers > 0) {
			_usingController = true;
			onControllerConnected();
		} else if (_usingController && controllers <= 0) {
			_usingController = false;
			onControllerDisconnected();
		}
	}

	void SteamINPUT::shutdown() {
		if (SteamInput() == nullptr) return;
		SteamInput()->Shutdown();

		_logger.reset();
	}

	// CONTROLLER -----
	bool SteamINPUT::showGamepadInput(int x, int y, int width, int height) {
		return SteamUtils()->ShowFloatingGamepadTextInput(k_EFloatingGamepadTextInputModeModeSingleLine, x, y, width, height);
	}

	int SteamINPUT::getConnectedControllers() {
		return SteamInput()->GetConnectedControllers(_controllerHandle.data());
	}

	Controller SteamINPUT::getControllerType(int slot) {
		if (slot >= getConnectedControllers()) return Controller::Unknown;
		InputHandle_t inputHandle = SteamInput()->GetControllerForGamepadIndex(slot);
		if (inputHandle == 0) return Controller::Unknown;

		ESteamInputType inputType = SteamInput()->GetInputTypeForHandle(inputHandle);
		switch (inputType) {
			case k_ESteamInputType_SteamController:
				return Controller::Steam;
			case k_ESteamInputType_SteamDeckController:
				return Controller::SteamDeck;
			case k_ESteamInputType_XBox360Controller:
				return Controller::XBox360;
			case k_ESteamInputType_XBoxOneController:
				return Controller::XBoxOne;
			case k_ESteamInputType_PS4Controller:
				return Controller::PS4;
			case k_ESteamInputType_PS5Controller:
				return Controller::PS5;
			case k_ESteamInputType_GenericGamepad:
				return Controller::Generic;
			case k_ESteamInputType_Unknown:
			default:
				return Controller::Unknown;
		}
	}

	void SteamINPUT::setControllerAction(rawrbox::ControllerAction action, int index) {
		InputHandle_t inputHandle = SteamInput()->GetControllerForGamepadIndex(index);
		if (inputHandle == 0) return;

		std::string actionSTR = std::string{magic_enum::enum_name(action)};
		InputActionSetHandle_t actionHandle = SteamInput()->GetActionSetHandle(actionSTR.c_str());
		if (actionHandle == 0) return;

		SteamInput()->ActivateActionSet(inputHandle, actionHandle);

		_inputID = inputHandle;
		_actionID = actionHandle;

		_logger->debug("Set controller action to {}", actionSTR);
	}

	ControllerJoystickData SteamINPUT::getAnalogData(ControllerActionAnalogHandle handle) {
		if (_inputID == 0 || _actionID == 0) return {0, 0};

		std::string actionSTR = std::string{magic_enum::enum_name(handle)};
		InputAnalogActionHandle_t actionHandle = SteamInput()->GetAnalogActionHandle(actionSTR.c_str());
		if (actionHandle == 0) return {0, 0};

		InputAnalogActionData_t data = SteamInput()->GetAnalogActionData(_inputID, actionHandle);
		if (!data.bActive) return {0, 0};

		return {
		    data.x,
		    data.y};
	}

	bool SteamINPUT::getButtonData(ControllerActionHandle handle) {
		if (_inputID == 0 || _actionID == 0) return false;

		std::string actionSTR = std::string{magic_enum::enum_name(handle)};
		InputDigitalActionHandle_t actionHandle = SteamInput()->GetDigitalActionHandle(actionSTR.c_str());
		if (actionHandle == 0) return false;

		InputDigitalActionData_t data = SteamInput()->GetDigitalActionData(_inputID, actionHandle);
		if (!data.bActive) return false;
		return data.bState;
	}

	bool SteamINPUT::isUsingController() {
		return _usingController;
	}
	// ------------

} // namespace rawrbox
