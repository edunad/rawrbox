#pragma once
#include <rawrbox/render/scripting/wrappers/camera_wrapper.hpp>
#include <rawrbox/render/scripting/wrappers/decals_wrapper.hpp>
#include <rawrbox/render/scripting/wrappers/light/base_wrapper.hpp>
#include <rawrbox/render/scripting/wrappers/light/manager_wrapper.hpp>
#include <rawrbox/render/scripting/wrappers/model/base_wrapper.hpp>
#include <rawrbox/render/scripting/wrappers/model/instance_wrapper.hpp>
#include <rawrbox/render/scripting/wrappers/model/instanced_wrapper.hpp>
#include <rawrbox/render/scripting/wrappers/model/mesh_wrapper.hpp>
#include <rawrbox/render/scripting/wrappers/model/model_wrapper.hpp>
#ifdef RAWRBOX_RESOURCES
	#include <rawrbox/render/scripting/wrappers/resources/texture_loader_wrapper.hpp>
	#include <rawrbox/render/scripting/wrappers/resources/font_loader_wrapper.hpp>
#endif
#include <rawrbox/render/scripting/wrappers/stencil_wrapper.hpp>
#include <rawrbox/render/scripting/wrappers/text/font_wrapper.hpp>
#include <rawrbox/render/scripting/wrappers/textures/base_wrapper.hpp>
#include <rawrbox/render/scripting/wrappers/window_wrapper.hpp>
#include <rawrbox/render/window.hpp>
#include <rawrbox/scripting/scripting.hpp>

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
			rawrbox::DecalsWrapper::registerLua(lua);
			// ----

			// MODEL --
			rawrbox::ModelBaseWrapper::registerLua(lua);
			rawrbox::ModelWrapper::registerLua(lua);
			rawrbox::InstanceWrapper::registerLua(lua);
			rawrbox::InstancedModelWrapper::registerLua(lua);
			rawrbox::MeshWrapper::registerLua(lua);
			// ----

			// TEXTURES ----
			rawrbox::TextureWrapper::registerLua(lua);
			// ------

			// LIGHT ----
			rawrbox::LightBaseWrapper::registerLua(lua);
			rawrbox::LightsWrapper::registerLua(lua);
			// ------

			// FONT ----
			rawrbox::FontWrapper::registerLua(lua);
			// ------

#ifdef RAWRBOX_RESOURCES
			// RESOURCES ---
			rawrbox::TextureLoaderWrapper::registerLua(lua);
			rawrbox::FontLoaderWrapper::registerLua(lua);
// ------
#endif
		}

		void registerGlobal(rawrbox::Mod* mod) override {
			if (mod == nullptr) throw std::runtime_error("[RawrBox-RenderPlugin] Tried to register plugin on invalid mod!");
			if (this->_window == nullptr) throw std::runtime_error("[RawrBox-RenderPlugin] Window not set!");

			auto& env = mod->getEnvironment();

			// Game singletons
			env["camera"] = rawrbox::CameraWrapper();
			env["window"] = rawrbox::WindowWrapper(this->_window);
			env["stencil"] = rawrbox::StencilWrapper(&this->_window->getStencil());

			env["decals"] = rawrbox::DecalsWrapper();
			env["lights"] = rawrbox::LightsWrapper();

#ifdef RAWRBOX_RESOURCES
			// Loaders ----
			env["texture"] = rawrbox::TextureLoaderWrapper(mod);
			env["font"] = rawrbox::FontLoaderWrapper(mod);
			//-----
#endif

			// Renderer statics ---
			env["MAX_BONES_PER_VERTEX"] = rawrbox::MAX_BONES_PER_VERTEX;
			env["MAX_BONES_PER_MODEL"] = rawrbox::MAX_BONES_PER_MODEL;
			env["CURRENT_VIEW_ID"] = []() { return rawrbox::CURRENT_VIEW_ID; };
			env["BGFX_FRAME"] = []() { return rawrbox::BGFX_FRAME; };
			// -----
		}

		void loadLuaExtensions(rawrbox::Mod* mod) override {
			if (mod == nullptr) throw std::runtime_error("[RawrBox-RenderPlugin] Tried to register plugin on invalid mod!");
			rawrbox::SCRIPTING::loadLuaFile("./lua/render_enums.lua", mod->getEnvironment());
		}
	};
} // namespace rawrbox
