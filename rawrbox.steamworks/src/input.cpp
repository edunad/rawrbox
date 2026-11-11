#include <rawrbox/steamworks/input.hpp>
// --
#include <magic_enum/magic_enum.hpp>

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
		if (SteamInput() == nullptr) RAWRBOX_CRITICAL("SteamInput is null");
		SteamInput()->Init(false);
	}

	void SteamINPUT::update() {
		int controllers = SteamINPUT::getConnectedControllers();

		if (!SteamINPUT::_usingController && controllers > 0) {
			SteamINPUT::_usingController = true;
			SteamINPUT::onControllerConnected();
		} else if (SteamINPUT::_usingController && controllers <= 0) {
			SteamINPUT::_usingController = false;
			SteamINPUT::onControllerDisconnected();
		}
	}

	void SteamINPUT::shutdown() {
		if (SteamInput() == nullptr) return;
		SteamInput()->Shutdown();

		SteamINPUT::_logger.reset();
	}

	// CONTROLLER -----
	bool SteamINPUT::showGamepadInput(int x, int y, int width, int height) {
		return SteamUtils()->ShowFloatingGamepadTextInput(k_EFloatingGamepadTextInputModeModeSingleLine, x, y, width, height);
	}

	int SteamINPUT::getConnectedControllers() {
		return SteamInput()->GetConnectedControllers(SteamINPUT::_controllerHandle.data());
	}

	Controller SteamINPUT::getControllerType(int slot) {
		if (slot >= SteamINPUT::getConnectedControllers()) return Controller::Unknown;

		const InputHandle_t inputHandle = SteamInput()->GetControllerForGamepadIndex(slot);
		if (inputHandle == 0) return Controller::Unknown;

		const ESteamInputType inputType = SteamInput()->GetInputTypeForHandle(inputHandle);
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

		SteamINPUT::_inputID = inputHandle;
		SteamINPUT::_actionID = actionHandle;

		SteamINPUT::_logger->debug("Set controller action to {}", actionSTR);
	}

	ControllerJoystickData SteamINPUT::getAnalogData(ControllerActionAnalogHandle handle) {
		if (SteamINPUT::_inputID == 0 || SteamINPUT::_actionID == 0) return {0, 0};

		const std::string actionSTR = std::string{magic_enum::enum_name(handle)};
		const InputAnalogActionHandle_t actionHandle = SteamInput()->GetAnalogActionHandle(actionSTR.c_str());
		if (actionHandle == 0) return {0, 0};

		InputAnalogActionData_t data = SteamInput()->GetAnalogActionData(SteamINPUT::_inputID, actionHandle);
		if (!data.bActive) return {0, 0};

		return {
		    data.x,
		    data.y};
	}

	bool SteamINPUT::getButtonData(ControllerActionHandle handle) {
		if (SteamINPUT::_inputID == 0 || SteamINPUT::_actionID == 0) return false;

		const std::string actionSTR = std::string{magic_enum::enum_name(handle)};
		const InputDigitalActionHandle_t actionHandle = SteamInput()->GetDigitalActionHandle(actionSTR.c_str());
		if (actionHandle == 0) return false;

		const InputDigitalActionData_t data = SteamInput()->GetDigitalActionData(SteamINPUT::_inputID, actionHandle);
		if (!data.bActive) return false;
		
		return data.bState;
	}

	bool SteamINPUT::isUsingController() {
		return SteamINPUT::_usingController;
	}
	// ------------

} // namespace rawrbox