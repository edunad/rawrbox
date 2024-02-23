#include <rawrbox/ui/elements/graph.hpp>
#include <rawrbox/ui/scripting/wrappers/graph.hpp>

namespace rawrbox {
	void UIGraphWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .deriveClass<rawrbox::UIGraph, rawrbox::UIContainer>("UIGraph")
		    .addFunction("getCategory", &rawrbox::UIGraph::getCategory)
		    .addFunction("addCategory", &rawrbox::UIGraph::addCategory)

		    .addFunction("setStyle", [](rawrbox::UIGraph& self, uint32_t style) {
			    self.setStyle(static_cast<rawrbox::UIGraphStyle>(style));
		    })
		    .addFunction("setSmoothing", &rawrbox::UIGraph::setSmoothing)
		    .addFunction("setAutoScale", &rawrbox::UIGraph::setAutoScale)
		    .addFunction("setHighest", &rawrbox::UIGraph::setHighest)

		    .addFunction("setShowLegend", &rawrbox::UIGraph::setShowLegend)
		    .addFunction("getShowLegend", &rawrbox::UIGraph::getShowLegend)

		    .addFunction("addText", &rawrbox::UIGraph::addText)

		    .addFunction("setTextLineColor", &rawrbox::UIGraph::setTextLineColor)
		    .addFunction("getTextLineColor", &rawrbox::UIGraph::getTextLineColor)

		    .addFunction("setTextColor", &rawrbox::UIGraph::setTextColor)
		    .addFunction("getTextColor", &rawrbox::UIGraph::getTextColor)

		    .addFunction("setTextShadowPos", &rawrbox::UIGraph::setTextShadowPos)
		    .addFunction("getTextShadowPos", &rawrbox::UIGraph::getTextShadowPos)

		    .addFunction("setTextShadowColor", &rawrbox::UIGraph::setTextShadowColor)
		    .addFunction("getTextShadowColor", &rawrbox::UIGraph::getTextShadowColor)

		    .addFunction("setFontPTR", [](rawrbox::UIGraph& self, rawrbox::Font* fnt) {
			    self.setFont(fnt);
		    })
		    .addFunction("setFontPath", [](rawrbox::UIGraph& self, const std::string& path, std::optional<uint16_t> size, lua_State* L) {
			    auto modFolder = rawrbox::LuaUtils::getLuaENVVar(L, "__mod_folder");
			    auto fixedPath = rawrbox::LuaUtils::getContent(path, modFolder);

			    self.setFont(fixedPath, size.value_or(11));
		    })

		    .endClass();
	}
} // namespace rawrbox
