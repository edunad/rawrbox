
#include <rawrbox/ui/scripting/wrappers/elements/graph_wrapper.hpp>
#include <rawrbox/utils/memory.hpp>

namespace rawrbox {

	// CATEGORY --
	rawrbox::UIGraphCategory& GraphWrapper::getCategory(size_t id) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		return rawrbox::cast<rawrbox::UIGraph>(this->_ref).lock()->getCategory(id);
	}

	rawrbox::UIGraphCategory& GraphWrapper::addCategory(const std::string& name, const rawrbox::Colori& color) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		return rawrbox::cast<rawrbox::UIGraph>(this->_ref).lock()->addCategory(name, color.cast<float>());
	}
	// ----------

	// UTILS -----
	void GraphWrapper::setStyle(rawrbox::UIGraphStyle style) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIGraph>(this->_ref).lock()->setStyle(style);
	}

	void GraphWrapper::setSmoothing(size_t frames) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIGraph>(this->_ref).lock()->setSmoothing(frames);
	}

	void GraphWrapper::setAutoScale(bool val) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIGraph>(this->_ref).lock()->setAutoScale(val);
	}

	void GraphWrapper::setHighest(float val) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIGraph>(this->_ref).lock()->setHighest(val);
	}
	// ----

	// TEXT ----
	void GraphWrapper::setShowLegend(bool mode) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIGraph>(this->_ref).lock()->setShowLegend(mode);
	}

	bool GraphWrapper::getShowLegend() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		return rawrbox::cast<rawrbox::UIGraph>(this->_ref).lock()->getShowLegend();
	}

	void GraphWrapper::addText(const std::string& text, float val) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIGraph>(this->_ref).lock()->addText(text, val);
	}

	void GraphWrapper::setTextLineColor(const rawrbox::Colori& col) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIGraph>(this->_ref).lock()->setTextLineColor(col.cast<float>());
	}

	rawrbox::Colori GraphWrapper::getTextLineColor() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		return rawrbox::cast<rawrbox::UIGraph>(this->_ref).lock()->getTextLineColor().cast<int>();
	}

	void GraphWrapper::setTextColor(const rawrbox::Colori& col) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIGraph>(this->_ref).lock()->setTextColor(col.cast<float>());
	}

	rawrbox::Colori GraphWrapper::getTextColor() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		return rawrbox::cast<rawrbox::UIGraph>(this->_ref).lock()->getTextColor().cast<int>();
	}

	void GraphWrapper::setTextShadowPos(const rawrbox::Vector2f& pos) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIGraph>(this->_ref).lock()->setTextShadowPos(pos);
	}

	const rawrbox::Vector2f& GraphWrapper::getTextShadowPos() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		return rawrbox::cast<rawrbox::UIGraph>(this->_ref).lock()->getTextShadowPos();
	}

	void GraphWrapper::setTextShadowColor(const rawrbox::Colori& col) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIGraph>(this->_ref).lock()->setTextShadowColor(col.cast<float>());
	}

	rawrbox::Colori GraphWrapper::getTextShadowColor() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		return rawrbox::cast<rawrbox::UIGraph>(this->_ref).lock()->getTextShadowColor().cast<int>();
	}

	void GraphWrapper::setFont(const rawrbox::FontWrapper& font) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIGraph>(this->_ref).lock()->setFont(font.getRef());
	}

	void GraphWrapper::setFont(const std::string& font, sol::optional<uint16_t> size, sol::this_environment modEnv) {
		if (!modEnv.env.has_value()) throw std::runtime_error("[RawrBox-GraphWrapper] MOD not set!");
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");

		std::string modFolder = modEnv.env.value()["__mod_folder"];
		rawrbox::cast<rawrbox::UIGraph>(this->_ref).lock()->setFont(rawrbox::LuaUtils::getContent(font, modFolder), size.value_or(11));
	}

	rawrbox::FontWrapper GraphWrapper::getFont() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		return {rawrbox::cast<rawrbox::UIGraph>(this->_ref).lock()->getFont()};
	}

	void GraphWrapper::setFontLegend(const rawrbox::FontWrapper& font) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIGraph>(this->_ref).lock()->setFontLegend(font.getRef());
	}

	void GraphWrapper::setFontLegend(const std::string& font, sol::optional<uint16_t> size, sol::this_environment modEnv) {
		if (!modEnv.env.has_value()) throw std::runtime_error("[RawrBox-GraphWrapper] MOD not set!");
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");

		std::string modFolder = modEnv.env.value()["__mod_folder"];
		rawrbox::cast<rawrbox::UIGraph>(this->_ref).lock()->setFontLegend(rawrbox::LuaUtils::getContent(font, modFolder), size.value_or(11));
	}

	rawrbox::FontWrapper GraphWrapper::getFontLegend() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		return {rawrbox::cast<rawrbox::UIGraph>(this->_ref).lock()->getFontLegend()};
	}
	// ----

	void GraphWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<GraphWrapper>("UIGraph",
		    sol::no_constructor,

		    // CATEGORY --
		    "getCategory", &GraphWrapper::getCategory,
		    "addCategory", &GraphWrapper::addCategory,
		    // ----

		    // UTILS -----
		    "setStyle", &GraphWrapper::setStyle,
		    "setSmoothing", &GraphWrapper::setSmoothing,

		    "setAutoScale", &GraphWrapper::setAutoScale,
		    "setHighest", &GraphWrapper::setHighest,
		    // ----

		    // TEXT -----
		    "setShowLegend", &GraphWrapper::setShowLegend,
		    "getShowLegend", &GraphWrapper::getShowLegend,

		    "addText", &GraphWrapper::addText,

		    "setTextLineColor", &GraphWrapper::setTextLineColor,
		    "getTextLineColor", &GraphWrapper::getTextLineColor,

		    "setTextColor", &GraphWrapper::setTextColor,
		    "getTextColor", &GraphWrapper::getTextColor,

		    "setTextShadowPos", &GraphWrapper::setTextShadowPos,
		    "getTextShadowPos", &GraphWrapper::getTextShadowPos,

		    "setTextShadowColor", &GraphWrapper::setTextShadowColor,
		    "getTextShadowColor", &GraphWrapper::getTextShadowColor,

		    "setFont", sol::overload(sol::resolve<void(const std::string&, sol::optional<uint16_t>, sol::this_environment)>(&GraphWrapper::setFont), sol::resolve<void(const rawrbox::FontWrapper&)>(&GraphWrapper::setFont)),
		    "getFont", &GraphWrapper::getFont,

		    "setFontLegend", sol::overload(sol::resolve<void(const std::string&, sol::optional<uint16_t>, sol::this_environment)>(&GraphWrapper::setFontLegend), sol::resolve<void(const rawrbox::FontWrapper&)>(&GraphWrapper::setFontLegend)),
		    "getFontLegend", &GraphWrapper::getFontLegend,
		    // ----

		    sol::base_classes, sol::bases<rawrbox::UIContainerWrapper>());
	}
} // namespace rawrbox
