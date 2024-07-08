#include <rawrbox/render/scripting/global/camera.hpp>
#include <rawrbox/render/static.hpp>

namespace rawrbox {

	// UTILS -----
	void CameraGlobal::setPos(const rawrbox::Vector3f& pos) {
		if (rawrbox::MAIN_CAMERA == nullptr) throw std::runtime_error("No MAIN_CAMERA found");
		rawrbox::MAIN_CAMERA->setPos(pos);
	}

	rawrbox::Vector3f CameraGlobal::getPos() {
		if (rawrbox::MAIN_CAMERA == nullptr) throw std::runtime_error("No MAIN_CAMERA found");
		return rawrbox::MAIN_CAMERA->getPos();
	}

	void CameraGlobal::setAngle(const rawrbox::Vector4f& ang) {
		if (rawrbox::MAIN_CAMERA == nullptr) throw std::runtime_error("No MAIN_CAMERA found");
		rawrbox::MAIN_CAMERA->setAngle(ang);
	}

	rawrbox::Vector4f CameraGlobal::getAngle() {
		if (rawrbox::MAIN_CAMERA == nullptr) throw std::runtime_error("No MAIN_CAMERA found");
		return rawrbox::MAIN_CAMERA->getAngle();
	}

	rawrbox::Vector3f CameraGlobal::getForward() {
		if (rawrbox::MAIN_CAMERA == nullptr) throw std::runtime_error("No MAIN_CAMERA found");
		return rawrbox::MAIN_CAMERA->getForward();
	}

	rawrbox::Vector3f CameraGlobal::getRight() {
		if (rawrbox::MAIN_CAMERA == nullptr) throw std::runtime_error("No MAIN_CAMERA found");
		return rawrbox::MAIN_CAMERA->getRight();
	}

	rawrbox::Vector3f CameraGlobal::getUp() {
		if (rawrbox::MAIN_CAMERA == nullptr) throw std::runtime_error("No MAIN_CAMERA found");
		return rawrbox::MAIN_CAMERA->getUp();
	}

	float CameraGlobal::getZFar() {
		if (rawrbox::MAIN_CAMERA == nullptr) throw std::runtime_error("No MAIN_CAMERA found");
		return rawrbox::MAIN_CAMERA->getZFar();
	}

	float CameraGlobal::getZNear() {
		if (rawrbox::MAIN_CAMERA == nullptr) throw std::runtime_error("No MAIN_CAMERA found");
		return rawrbox::MAIN_CAMERA->getZNear();
	}

	rawrbox::Matrix4x4 CameraGlobal::getViewMtx() {
		if (rawrbox::MAIN_CAMERA == nullptr) throw std::runtime_error("No MAIN_CAMERA found");
		return rawrbox::MAIN_CAMERA->getViewMtx();
	}

	rawrbox::Matrix4x4 CameraGlobal::getProjMtx() {
		if (rawrbox::MAIN_CAMERA == nullptr) throw std::runtime_error("No MAIN_CAMERA found");
		return rawrbox::MAIN_CAMERA->getProjMtx();
	}

	rawrbox::Matrix4x4 CameraGlobal::getViewProjMtx() {
		if (rawrbox::MAIN_CAMERA == nullptr) throw std::runtime_error("No MAIN_CAMERA found");
		return rawrbox::MAIN_CAMERA->getViewProjMtx();
	}

	rawrbox::Vector3f CameraGlobal::worldToScreen(const rawrbox::Vector3f& pos) {
		if (rawrbox::MAIN_CAMERA == nullptr) throw std::runtime_error("No MAIN_CAMERA found");
		return rawrbox::MAIN_CAMERA->worldToScreen(pos);
	}

	rawrbox::Vector3f CameraGlobal::screenToWorld(const rawrbox::Vector2f& screen_pos) {
		if (rawrbox::MAIN_CAMERA == nullptr) throw std::runtime_error("No MAIN_CAMERA found");
		return rawrbox::MAIN_CAMERA->screenToWorld(screen_pos);
	}
	// ----------------

	void CameraGlobal::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginNamespace("camera", {})

		    .addFunction("setPos", &CameraGlobal::setPos)
		    .addFunction("getPos", &CameraGlobal::getPos)
		    .addFunction("setAngle", &CameraGlobal::setAngle)
		    .addFunction("getAngle", &CameraGlobal::getAngle)

		    // UTILS ----
		    .addFunction("getForward", &CameraGlobal::getForward)
		    .addFunction("getRight", &CameraGlobal::getRight)
		    .addFunction("getUp", &CameraGlobal::getUp)

		    .addFunction("getZFar", &CameraGlobal::getZFar)
		    .addFunction("getZNear", &CameraGlobal::getZNear)

		    .addFunction("getViewMtx", &CameraGlobal::getViewMtx)
		    .addFunction("getProjMtx", &CameraGlobal::getProjMtx)
		    .addFunction("getViewProjMtx", &CameraGlobal::getViewProjMtx)

		    .addFunction("worldToScreen", &CameraGlobal::worldToScreen)
		    .addFunction("screenToWorld", &CameraGlobal::screenToWorld)

		    .endNamespace();
	}
} // namespace rawrbox
