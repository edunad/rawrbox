#include <rawrbox/render/cameras/base.hpp>
#include <rawrbox/render/scripting/wrappers/camera.hpp>

namespace rawrbox {
	void CameraWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginClass<rawrbox::CameraBase>("Camera")

		    // UTILS ----
		    .addFunction("setPos", &CameraBase::setPos)
		    .addFunction("getPos", &CameraBase::getPos)

		    .addFunction("setAngle", &CameraBase::setAngle)
		    .addFunction("getAngle", &CameraBase::getAngle)

		    .addFunction("getForward", &CameraBase::getForward)
		    .addFunction("getRight", &CameraBase::getRight)
		    .addFunction("getUp", &CameraBase::getUp)

		    .addFunction("getZFar", &CameraBase::getZFar)
		    .addFunction("getZNear", &CameraBase::getZNear)

		    .addFunction("getViewMtx", &CameraBase::getViewMtx)
		    .addFunction("getProjMtx", &CameraBase::getProjMtx)
		    .addFunction("getViewProjMtx", &CameraBase::getViewProjMtx)

		    .addFunction("worldToScreen", &CameraBase::worldToScreen)
		    .addFunction("screenToWorld", [](const rawrbox::CameraBase* camera, const rawrbox::Vector2f& screenPos) { return camera->screenToWorld(screenPos); })
		    .addFunction("screenRayDir", &CameraBase::screenRayDir)
		    // ----------------

		    // STATE ----
		    .addFunction("isEnabled", &CameraBase::isEnabled)
		    .addFunction("setEnabled", &CameraBase::setEnabled)

		    .addFunction("getLayers", &CameraBase::getLayers)
		    .addFunction("setLayers", &CameraBase::setLayers)
		    .addFunction("shouldRenderLayer", &CameraBase::shouldRenderLayer)
		    // ----------------

		    .endClass();
	}
} // namespace rawrbox
