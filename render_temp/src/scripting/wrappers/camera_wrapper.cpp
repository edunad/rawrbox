#include <rawrbox/render_temp/scripting/wrappers/camera_wrapper.hpp>
#include <rawrbox/render_temp/static.hpp>

namespace rawrbox {

	// UTILS -----
	void CameraWrapper::setPos(const rawrbox::Vector3f& pos) {
		if (rawrbox::MAIN_CAMERA == nullptr) return;
		rawrbox::MAIN_CAMERA->setPos(pos);
	}

	const rawrbox::Vector3f CameraWrapper::getPos() const {
		if (rawrbox::MAIN_CAMERA == nullptr) return {};
		return rawrbox::MAIN_CAMERA->getPos();
	}

	void CameraWrapper::setAngle(const rawrbox::Vector4f& ang) {
		if (rawrbox::MAIN_CAMERA == nullptr) return;
		rawrbox::MAIN_CAMERA->setAngle(ang);
	}

	const rawrbox::Vector4f CameraWrapper::getAngle() const {
		if (rawrbox::MAIN_CAMERA == nullptr) return {};
		return rawrbox::MAIN_CAMERA->getAngle();
	}

	rawrbox::Vector3f CameraWrapper::getForward() const {
		if (rawrbox::MAIN_CAMERA == nullptr) return {};
		return rawrbox::MAIN_CAMERA->getForward();
	}

	rawrbox::Vector3f CameraWrapper::getRight() const {
		if (rawrbox::MAIN_CAMERA == nullptr) return {};
		return rawrbox::MAIN_CAMERA->getRight();
	}

	rawrbox::Vector3f CameraWrapper::getUp() const {
		if (rawrbox::MAIN_CAMERA == nullptr) return {};
		return rawrbox::MAIN_CAMERA->getUp();
	}

	float CameraWrapper::getZFar() const {
		if (rawrbox::MAIN_CAMERA == nullptr) return 0;
		return rawrbox::MAIN_CAMERA->getZFar();
	}

	float CameraWrapper::getZNear() const {
		if (rawrbox::MAIN_CAMERA == nullptr) return 0;
		return rawrbox::MAIN_CAMERA->getZNear();
	}

	const rawrbox::Matrix4x4 CameraWrapper::getViewMtx() const {
		if (rawrbox::MAIN_CAMERA == nullptr) return {};
		return rawrbox::MAIN_CAMERA->getViewMtx();
	}

	const rawrbox::Matrix4x4 CameraWrapper::getProjMtx() const {
		if (rawrbox::MAIN_CAMERA == nullptr) return {};
		return rawrbox::MAIN_CAMERA->getProjMtx();
	}

	const rawrbox::Matrix4x4 CameraWrapper::getProjViewMtx() const {
		if (rawrbox::MAIN_CAMERA == nullptr) return {};
		return rawrbox::MAIN_CAMERA->getProjViewMtx();
	}

	const rawrbox::Vector3f CameraWrapper::worldToScreen(const rawrbox::Vector3f& pos) const {
		if (rawrbox::MAIN_CAMERA == nullptr) return {};
		return rawrbox::MAIN_CAMERA->worldToScreen(pos);
	}

	const rawrbox::Vector3f CameraWrapper::screenToWorld(const rawrbox::Vector2f& screen_pos) const {
		if (rawrbox::MAIN_CAMERA == nullptr) return {};
		return rawrbox::MAIN_CAMERA->screenToWorld(screen_pos);
	}
	// ----------------

	void CameraWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<CameraWrapper>("Camera",
		    sol::no_constructor,

		    "setPos", &CameraWrapper::setPos,
		    "getPos", &CameraWrapper::getPos,
		    "setAngle", &CameraWrapper::setAngle,
		    "getAngle", &CameraWrapper::getAngle,

		    // UTILS ----
		    "getForward", &CameraWrapper::getForward,
		    "getRight", &CameraWrapper::getRight,
		    "getUp", &CameraWrapper::getUp,
		    "getZFar", &CameraWrapper::getZFar,
		    "getZNear", &CameraWrapper::getZNear,
		    "getViewMtx", &CameraWrapper::getViewMtx,
		    "getProjMtx", &CameraWrapper::getProjMtx,
		    "getProjViewMtx", &CameraWrapper::getProjViewMtx,
		    "worldToScreen", &CameraWrapper::worldToScreen,
		    "screenToWorld", &CameraWrapper::screenToWorld
		    // ---------
		);
	}
} // namespace rawrbox
