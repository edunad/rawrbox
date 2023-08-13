#pragma once
#include <rawrbox/render/scripting/wrappers/camera_wrapper.hpp>
#include <rawrbox/render/scripting/wrappers/model/base_wrapper.hpp>
#include <rawrbox/render/scripting/wrappers/model/mesh_wrapper.hpp>
#include <rawrbox/render/scripting/wrappers/model/model_wrapper.hpp>
#include <rawrbox/render/scripting/wrappers/stencil_wrapper.hpp>
#include <rawrbox/render/scripting/wrappers/textures/base_wrapper.hpp>
#include <rawrbox/render/scripting/wrappers/window_wrapper.hpp>
#include <rawrbox/render/window.hpp>
#include <rawrbox/scripting/plugin.hpp>

namespace rawrbox {
	class RenderPlugin : public rawrbox::Plugin {
	protected:
		rawrbox::Window* _window = nullptr;

	public:
		RenderPlugin(rawrbox::Window* window) : rawrbox::Plugin(), _window(window){};

		void registerTypes(sol::state& lua) override {
			// GAME
			rawrbox::WindowWrapper::registerLua(lua);
			rawrbox::CameraWrapper::registerLua(lua);
			rawrbox::StencilWrapper::registerLua(lua);
			// ----

			// MODEL --
			rawrbox::ModelBaseWrapper::registerLua(lua);
			rawrbox::ModelWrapper::registerLua(lua);
			rawrbox::MeshWrapper::registerLua(lua);
			// ----

			// TEXTURES ----
			rawrbox::TextureWrapper::registerLua(lua);
			// ------
		}

		void registerGlobal(sol::environment& env) override {
			if (this->_window == nullptr) throw std::runtime_error("[RawrBox-Plugin] Window not set!");

			// Game singletons
			env["camera"] = rawrbox::CameraWrapper();
			env["window"] = rawrbox::WindowWrapper(this->_window);
			env["stencil"] = rawrbox::StencilWrapper(&this->_window->getStencil());
			//-----

			// Renderer statics ---
			env["MAX_BONES_PER_VERTEX"] = rawrbox::MAX_BONES_PER_VERTEX;
			env["MAX_BONES_PER_MODEL"] = rawrbox::MAX_BONES_PER_MODEL;
			env["CURRENT_VIEW_ID"] = rawrbox::CURRENT_VIEW_ID;
			env["BGFX_FRAME"] = rawrbox::BGFX_FRAME;
			// -----
		}
	};
} // namespace rawrbox
