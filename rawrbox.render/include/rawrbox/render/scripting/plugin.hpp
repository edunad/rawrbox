#pragma once

#include <rawrbox/render/scripting/wrappers/camera.hpp>
#include <rawrbox/render/scripting/wrappers/decals/decal.hpp>
#include <rawrbox/render/scripting/wrappers/decals/manager.hpp>
#include <rawrbox/render/scripting/wrappers/light/base.hpp>
#include <rawrbox/render/scripting/wrappers/light/manager.hpp>
#include <rawrbox/render/scripting/wrappers/models/base.hpp>
#include <rawrbox/render/scripting/wrappers/models/instance.hpp>
#include <rawrbox/render/scripting/wrappers/models/instanced.hpp>
#include <rawrbox/render/scripting/wrappers/models/mesh.hpp>
#include <rawrbox/render/scripting/wrappers/models/model.hpp>
#include <rawrbox/render/scripting/wrappers/resources/font_loader.hpp>
#include <rawrbox/render/scripting/wrappers/resources/texture_loader.hpp>
#include <rawrbox/render/scripting/wrappers/stencil.hpp>
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
			rawrbox::CameraWrapper::registerLua(L);
			rawrbox::WindowWrapper::registerLua(L);
			rawrbox::StencilWrapper::registerLua(L);
			//  -------

			// DECALS ---
			rawrbox::DecalWrapper::registerLua(L);
			rawrbox::DecalsWrapper::registerLua(L);
			//  -------

			// TEXTURES ----
			rawrbox::TextureWrapper::registerLua(L);
			// ------

			// LIGHT ----
			rawrbox::LightBaseWrapper::registerLua(L);
			rawrbox::LightsWrapper::registerLua(L);
			// ------

			// MODEL --
			rawrbox::MeshWrapper::registerLua(L);
			rawrbox::ModelBaseWrapper::registerLua(L);
			rawrbox::ModelWrapper::registerLua(L);
			rawrbox::InstanceWrapper::registerLua(L);
			rawrbox::InstancedModelWrapper::registerLua(L);
			// ----

			// RESOURCES ---
#ifdef RAWRBOX_RESOURCES
			rawrbox::TextureLoaderWrapper::registerLua(L);
			rawrbox::FontLoaderWrapper::registerLua(L);
#endif
			// ------
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
		}

		void loadLibraries(lua_State* L) override {
			if (L == nullptr) throw std::runtime_error("Tried to register plugin on invalid mod!");
			rawrbox::LuaUtils::compileAndLoadFile(L, "RawrBox::Enums::RENDER", "./lua/enums/render.lua");
		}
	};
} // namespace rawrbox
