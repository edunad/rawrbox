#pragma once

#include <rawrbox/render/scripting/global/camera.hpp>
#include <rawrbox/render/scripting/global/decals/manager.hpp>
#include <rawrbox/render/scripting/global/lights/manager.hpp>
#include <rawrbox/render/scripting/global/resources/font_loader.hpp>
#include <rawrbox/render/scripting/global/resources/texture_loader.hpp>
#include <rawrbox/render/scripting/global/stencil.hpp>
#include <rawrbox/render/scripting/wrappers/decals/decal.hpp>
#include <rawrbox/render/scripting/wrappers/light/base.hpp>
#include <rawrbox/render/scripting/wrappers/models/base.hpp>
#include <rawrbox/render/scripting/wrappers/models/instance.hpp>
#include <rawrbox/render/scripting/wrappers/models/instanced.hpp>
#include <rawrbox/render/scripting/wrappers/models/mesh.hpp>
#include <rawrbox/render/scripting/wrappers/models/model.hpp>
#include <rawrbox/render/scripting/wrappers/text/font.hpp>
#include <rawrbox/render/scripting/wrappers/textures/base.hpp>
#include <rawrbox/render/scripting/wrappers/window.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/scripting/plugin.hpp>

namespace rawrbox {
	class RendererScripting : public rawrbox::ScriptingPlugin {
	protected:
		rawrbox::Window* _window = nullptr;

	public:
		RendererScripting(rawrbox::Window* window) : _window(window){};

		void registerTypes(lua_State* L) override {
			if (L == nullptr) throw std::runtime_error("Tried to register plugin on invalid mod!");

			// FONT ----
			rawrbox::FontWrapper::registerLua(L);
			// ------

			// GAME --
			rawrbox::WindowWrapper::registerLua(L);
			//  -------

			// DECALS ---
			rawrbox::DecalWrapper::registerLua(L);
			//  -------

			// TEXTURES ----
			rawrbox::TextureWrapper::registerLua(L);
			// ------

			// LIGHT ----
			rawrbox::LightBaseWrapper::registerLua(L);
			// ------

			// MODEL --
			rawrbox::MeshWrapper::registerLua(L);
			rawrbox::ModelBaseWrapper::registerLua(L);
			rawrbox::ModelWrapper::registerLua(L);
			rawrbox::InstanceWrapper::registerLua(L);
			rawrbox::InstancedModelWrapper::registerLua(L);
			// ----
		}

		void registerGlobal(lua_State* L) override {
			if (L == nullptr) throw std::runtime_error("Tried to register plugin on invalid mod!");

			// GAME --
			rawrbox::CameraGlobal::registerLua(L);
			rawrbox::StencilGlobal::registerLua(L);
			//  -------

			// DECALS ---
			rawrbox::DecalsGlobal::registerLua(L);
			//  -------

			// LIGHT ----
			rawrbox::LightsGlobal::registerLua(L);
			// ------

			// RESOURCES ---
#ifdef RAWRBOX_RESOURCES
			rawrbox::TextureLoaderGlobal::registerLua(L);
			rawrbox::FontLoaderGlobal::registerLua(L);
#endif
			// ------

			luabridge::getGlobalNamespace(L)
			    .beginNamespace("renderer", {})
			    .addVariable("MAX_BONES_PER_VERTEX", RB_MAX_BONES_PER_VERTEX)
			    .addVariable("MAX_BONES_PER_MODEL", RB_RENDER_MAX_BONES_PER_MODEL)
			    .addVariable("MAX_POST_DATA", RB_RENDER_MAX_POST_DATA)
			    .addFunction("frame", []() { return rawrbox::FRAME; })
			    .endNamespace();

			luabridge::getGlobalNamespace(L)
			    .beginNamespace("window", {})
			    .addFunction("get", [](uint32_t index) {
				    return rawrbox::WindowWrapper(rawrbox::Window::getWindow(index));
			    })
			    .endNamespace();
		}

		void loadLibraries(lua_State* L) override {
			if (L == nullptr) throw std::runtime_error("Tried to register plugin on invalid mod!");
			rawrbox::LuaUtils::compileAndLoadFile(L, "RawrBox::Enums::RENDER", "./lua/enums/render.lua");
		}
	};
} // namespace rawrbox
