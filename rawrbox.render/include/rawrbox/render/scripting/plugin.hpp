#pragma once

#include <rawrbox/render/scripting/wrappers/camera.hpp>
#include <rawrbox/render/scripting/wrappers/textures/base.hpp>
#include <rawrbox/render/scripting/wrappers/window.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/scripting/plugin.hpp>

namespace rawrbox {
	class RendererPlugin : public rawrbox::ScriptingPlugin {
	protected:
		rawrbox::Window* _window = nullptr;

	public:
		RendererPlugin(rawrbox::Window* window) : _window(window){};

		void registerTypes(lua_State* L) override {
			if (L == nullptr) throw std::runtime_error("Tried to register plugin on invalid mod!");

			// GAME --
			rawrbox::CameraWrapper::registerLua(L);
			rawrbox::WindowWrapper::registerLua(L);
			// -------

			// TEXTURES ----
			rawrbox::TextureWrapper::registerLua(L);
			// ------

			/*// GAME
			rawrbox::WindowWrapper::registerLua(lua);
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
#endif*/
		}

		void registerGlobal(lua_State* L) override {
			if (L == nullptr) throw std::runtime_error("Tried to register plugin on invalid mod!");

			luabridge::getGlobalNamespace(L)
			    .beginNamespace("renderer")
			    .addVariable("MAX_BONES_PER_VERTEX", &rawrbox::MAX_BONES_PER_VERTEX)
			    .addVariable("MAX_BONES_PER_MODEL", &rawrbox::MAX_BONES_PER_MODEL)
			    .addVariable("MAX_POST_DATA", &rawrbox::MAX_POST_DATA)

			    .addFunction("frame", []() { return rawrbox::FRAME; })
			    .endNamespace();

			luabridge::getGlobalNamespace(L)
			    .beginNamespace("window")
			    .addFunction("get", [](uint32_t index) {
				    return rawrbox::WindowWrapper(rawrbox::Window::getWindow(index));
			    })
			    .endNamespace();
			/*if (mod == nullptr) throw std::runtime_error("[RawrBox-RenderPlugin] Tried to register plugin on invalid mod!");

			auto& env = mod->getEnvironment();

			// Game singletons
			env["camera"] = rawrbox::CameraWrapper();
			// ---------------

			// Renderer statics ---
			env["MAX_BONES_PER_VERTEX"] = rawrbox::MAX_BONES_PER_VERTEX;
			env["MAX_BONES_PER_MODEL"] = rawrbox::MAX_BONES_PER_MODEL;
			env["MAX_POST_DATA"] = rawrbox::MAX_POST_DATA;

			env["FRAME"] = []() { return rawrbox::FRAME; };
			// -----

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
			env["texture"] = rawrbox::TextureLoaderWrapper();
			env["font"] = rawrbox::FontLoaderWrapper();
			//-----
#endif

			// Renderer statics ---
			env["MAX_BONES_PER_VERTEX"] = rawrbox::MAX_BONES_PER_VERTEX;
			env["MAX_BONES_PER_MODEL"] = rawrbox::MAX_BONES_PER_MODEL;
			env["CURRENT_VIEW_ID"] = []() { return rawrbox::CURRENT_VIEW_ID; };
			env["BGFX_FRAME"] = []() { return rawrbox::BGFX_FRAME; };
			// -----*/
		}

		void loadLibraries(lua_State* L) override {
			if (L == nullptr) throw std::runtime_error("Tried to register plugin on invalid mod!");
			rawrbox::LuaUtils::compileAndLoadFile(L, "RawrBox::Enums::RENDER", "./lua/enums/render.lua");
		}
	};
} // namespace rawrbox
