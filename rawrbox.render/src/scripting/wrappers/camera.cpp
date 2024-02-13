#include <rawrbox/render/scripting/wrappers/camera.hpp>
#include <rawrbox/render/static.hpp>

namespace rawrbox {

	// UTILS -----
	void CameraWrapper::setPos(const rawrbox::Vector3f& pos) {
		if (rawrbox::MAIN_CAMERA == nullptr) throw std::runtime_error("No MAIN_CAMERA found");
		rawrbox::MAIN_CAMERA->setPos(pos);
	}

	rawrbox::Vector3f CameraWrapper::getPos() {
		if (rawrbox::MAIN_CAMERA == nullptr) throw std::runtime_error("No MAIN_CAMERA found");
		return rawrbox::MAIN_CAMERA->getPos();
	}

	void CameraWrapper::setAngle(const rawrbox::Vector4f& ang) {
		if (rawrbox::MAIN_CAMERA == nullptr) throw std::runtime_error("No MAIN_CAMERA found");
		rawrbox::MAIN_CAMERA->setAngle(ang);
	}

	rawrbox::Vector4f CameraWrapper::getAngle() {
		if (rawrbox::MAIN_CAMERA == nullptr) throw std::runtime_error("No MAIN_CAMERA found");
		return rawrbox::MAIN_CAMERA->getAngle();
	}

	rawrbox::Vector3f CameraWrapper::getForward() {
		if (rawrbox::MAIN_CAMERA == nullptr) throw std::runtime_error("No MAIN_CAMERA found");
		return rawrbox::MAIN_CAMERA->getForward();
	}

	rawrbox::Vector3f CameraWrapper::getRight() {
		if (rawrbox::MAIN_CAMERA == nullptr) throw std::runtime_error("No MAIN_CAMERA found");
		return rawrbox::MAIN_CAMERA->getRight();
	}

	rawrbox::Vector3f CameraWrapper::getUp() {
		if (rawrbox::MAIN_CAMERA == nullptr) throw std::runtime_error("No MAIN_CAMERA found");
		return rawrbox::MAIN_CAMERA->getUp();
	}

	float CameraWrapper::getZFar() {
		if (rawrbox::MAIN_CAMERA == nullptr) throw std::runtime_error("No MAIN_CAMERA found");
		return rawrbox::MAIN_CAMERA->getZFar();
	}

	float CameraWrapper::getZNear() {
		if (rawrbox::MAIN_CAMERA == nullptr) throw std::runtime_error("No MAIN_CAMERA found");
		return rawrbox::MAIN_CAMERA->getZNear();
	}

	rawrbox::Matrix4x4 CameraWrapper::getViewMtx() {
		if (rawrbox::MAIN_CAMERA == nullptr) throw std::runtime_error("No MAIN_CAMERA found");
		return rawrbox::MAIN_CAMERA->getViewMtx();
	}

	rawrbox::Matrix4x4 CameraWrapper::getProjMtx() {
		if (rawrbox::MAIN_CAMERA == nullptr) throw std::runtime_error("No MAIN_CAMERA found");
		return rawrbox::MAIN_CAMERA->getProjMtx();
	}

	rawrbox::Matrix4x4 CameraWrapper::getViewProjMtx() {
		if (rawrbox::MAIN_CAMERA == nullptr) throw std::runtime_error("No MAIN_CAMERA found");
		return rawrbox::MAIN_CAMERA->getViewProjMtx();
	}

	rawrbox::Vector3f CameraWrapper::worldToScreen(const rawrbox::Vector3f& pos) {
		if (rawrbox::MAIN_CAMERA == nullptr) throw std::runtime_error("No MAIN_CAMERA found");
		return rawrbox::MAIN_CAMERA->worldToScreen(pos);
	}

	rawrbox::Vector3f CameraWrapper::screenToWorld(const rawrbox::Vector2f& screen_pos) {
		if (rawrbox::MAIN_CAMERA == nullptr) throw std::runtime_error("No MAIN_CAMERA found");
		return rawrbox::MAIN_CAMERA->screenToWorld(screen_pos);
	}
	// ----------------

	void CameraWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginNamespace("camera", {})

		    .addFunction("setPos", &CameraWrapper::setPos)
		    .addFunction("getPos", &CameraWrapper::getPos)
		    .addFunction("setAngle", &CameraWrapper::setAngle)
		    .addFunction("getAngle", &CameraWrapper::getAngle)

		    // UTILS ----
		    .addFunction("getForward", &CameraWrapper::getForward)
		    .addFunction("getRight", &CameraWrapper::getRight)
		    .addFunction("getUp", &CameraWrapper::getUp)

		    .addFunction("getZFar", &CameraWrapper::getZFar)
		    .addFunction("getZNear", &CameraWrapper::getZNear)

		    .addFunction("getViewMtx", &CameraWrapper::getViewMtx)
		    .addFunction("getProjMtx", &CameraWrapper::getProjMtx)
		    .addFunction("getViewProjMtx", &CameraWrapper::getViewProjMtx)

		    .addFunction("worldToScreen", &CameraWrapper::worldToScreen)
		    .addFunction("screenToWorld", &CameraWrapper::screenToWorld)

		    .endNamespace();
	}
} // namespace rawrbox
