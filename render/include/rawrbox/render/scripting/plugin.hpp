#pragma once
#include <rawrbox/render/scripting/wrapper/camera_wrapper.hpp>
#include <rawrbox/render/scripting/wrapper/stencil_wrapper.hpp>
#include <rawrbox/render/scripting/wrapper/window_wrapper.hpp>
#include <rawrbox/render/window.hpp>
#include <rawrbox/scripting/plugin.hpp>

namespace rawrbox {
	class RenderPlugin : public rawrbox::Plugin {
	protected:
		rawrbox::Window* _window = nullptr;

	public:
		RenderPlugin(rawrbox::Window* window) : rawrbox::Plugin(), _window(window){};

		void registerTypes(sol::state& lua) override {
			rawrbox::WindowWrapper::registerLua(lua);
			rawrbox::CameraWrapper::registerLua(lua);
			rawrbox::StencilWrapper::registerLua(lua);
		}

		void registerGlobal(sol::environment& env) override {
			if (this->_window == nullptr) throw std::runtime_error("[RawrBox-Plugin] Window not set!");

			env["camera"] = rawrbox::CameraWrapper();
			env["window"] = rawrbox::WindowWrapper(this->_window);
			env["stencil"] = rawrbox::StencilWrapper(&this->_window->getStencil());
		}
	};
} // namespace rawrbox
