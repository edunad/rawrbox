
#include <rawrbox/ui/scripting/wrappers/elements/graph_wrapper.hpp>

namespace rawrbox {
	GraphWrapper::GraphWrapper(const std::shared_ptr<rawrbox::UIContainer>& element, rawrbox::Mod* mod) : rawrbox::UIContainerWrapper(element), _mod(mod) {}

	// CATEGORY --
	rawrbox::UIGraphCategory& GraphWrapper::getCategory(size_t id) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIGraph> ptr = std::dynamic_pointer_cast<rawrbox::UIGraph>(this->_ref.lock());

		return ptr.lock()->getCategory(id);
	}

	rawrbox::UIGraphCategory& GraphWrapper::addCategory(const std::string& name, const rawrbox::Colori& color) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIGraph> ptr = std::dynamic_pointer_cast<rawrbox::UIGraph>(this->_ref.lock());

		return ptr.lock()->addCategory(name, color.cast<float>());
	}
	// ----------

	// UTILS -----
	void GraphWrapper::setStyle(rawrbox::UIGraphStyle style) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIGraph> ptr = std::dynamic_pointer_cast<rawrbox::UIGraph>(this->_ref.lock());

		ptr.lock()->setStyle(style);
	}

	void GraphWrapper::setSmoothing(size_t frames) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIGraph> ptr = std::dynamic_pointer_cast<rawrbox::UIGraph>(this->_ref.lock());

		ptr.lock()->setSmoothing(frames);
	}

	void GraphWrapper::setAutoScale(bool val) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIGraph> ptr = std::dynamic_pointer_cast<rawrbox::UIGraph>(this->_ref.lock());

		ptr.lock()->setAutoScale(val);
	}

	void GraphWrapper::setHighest(float val) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIGraph> ptr = std::dynamic_pointer_cast<rawrbox::UIGraph>(this->_ref.lock());

		ptr.lock()->setHighest(val);
	}
	// ----

	// TEXT ----
	void GraphWrapper::setShowLegend(bool mode) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIGraph> ptr = std::dynamic_pointer_cast<rawrbox::UIGraph>(this->_ref.lock());

		ptr.lock()->setShowLegend(mode);
	}

	bool GraphWrapper::getShowLegend() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIGraph> ptr = std::dynamic_pointer_cast<rawrbox::UIGraph>(this->_ref.lock());

		return ptr.lock()->getShowLegend();
	}

	void GraphWrapper::addText(const std::string& text, float val) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIGraph> ptr = std::dynamic_pointer_cast<rawrbox::UIGraph>(this->_ref.lock());

		ptr.lock()->addText(text, val);
	}

	void GraphWrapper::setTextLineColor(const rawrbox::Colori& col) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIGraph> ptr = std::dynamic_pointer_cast<rawrbox::UIGraph>(this->_ref.lock());

		ptr.lock()->setTextLineColor(col.cast<float>());
	}

	rawrbox::Colori GraphWrapper::getTextLineColor() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIGraph> ptr = std::dynamic_pointer_cast<rawrbox::UIGraph>(this->_ref.lock());

		return ptr.lock()->getTextLineColor().cast<int>();
	}

	void GraphWrapper::setTextColor(const rawrbox::Colori& col) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIGraph> ptr = std::dynamic_pointer_cast<rawrbox::UIGraph>(this->_ref.lock());

		ptr.lock()->setTextColor(col.cast<float>());
	}

	rawrbox::Colori GraphWrapper::getTextColor() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIGraph> ptr = std::dynamic_pointer_cast<rawrbox::UIGraph>(this->_ref.lock());

		return ptr.lock()->getTextColor().cast<int>();
	}

	void GraphWrapper::setTextShadowPos(const rawrbox::Vector2f& pos) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIGraph> ptr = std::dynamic_pointer_cast<rawrbox::UIGraph>(this->_ref.lock());

		ptr.lock()->setTextShadowPos(pos);
	}

	const rawrbox::Vector2f& GraphWrapper::getTextShadowPos() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIGraph> ptr = std::dynamic_pointer_cast<rawrbox::UIGraph>(this->_ref.lock());

		return ptr.lock()->getTextShadowPos();
	}

	void GraphWrapper::setTextShadowColor(const rawrbox::Colori& col) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIGraph> ptr = std::dynamic_pointer_cast<rawrbox::UIGraph>(this->_ref.lock());

		ptr.lock()->setTextShadowColor(col.cast<float>());
	}

	rawrbox::Colori GraphWrapper::getTextShadowColor() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIGraph> ptr = std::dynamic_pointer_cast<rawrbox::UIGraph>(this->_ref.lock());

		return ptr.lock()->getTextShadowColor().cast<int>();
	}

	void GraphWrapper::setFont(const rawrbox::FontWrapper& font) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIGraph> ptr = std::dynamic_pointer_cast<rawrbox::UIGraph>(this->_ref.lock());

		ptr.lock()->setFont(font.getRef());
	}

	void GraphWrapper::setFont(const std::string& font, sol::optional<int> size) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIGraph> ptr = std::dynamic_pointer_cast<rawrbox::UIGraph>(this->_ref.lock());

		ptr.lock()->setFont(rawrbox::LuaUtils::getContent(font, this->_mod->getFolder()), size.value_or(11));
	}

	rawrbox::FontWrapper GraphWrapper::getFont() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIGraph> ptr = std::dynamic_pointer_cast<rawrbox::UIGraph>(this->_ref.lock());

		return {ptr.lock()->getFont()};
	}

	void GraphWrapper::setFontLegend(const rawrbox::FontWrapper& font) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIGraph> ptr = std::dynamic_pointer_cast<rawrbox::UIGraph>(this->_ref.lock());

		ptr.lock()->setFontLegend(font.getRef());
	}

	void GraphWrapper::setFontLegend(const std::string& font, sol::optional<int> size) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIGraph> ptr = std::dynamic_pointer_cast<rawrbox::UIGraph>(this->_ref.lock());

		ptr.lock()->setFontLegend(rawrbox::LuaUtils::getContent(font, this->_mod->getFolder()), size.value_or(11));
	}

	rawrbox::FontWrapper GraphWrapper::getFontLegend() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GraphWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIGraph> ptr = std::dynamic_pointer_cast<rawrbox::UIGraph>(this->_ref.lock());

		return {ptr.lock()->getFontLegend()};
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

		    "setFont", sol::overload(sol::resolve<void(const std::string&, sol::optional<int>)>(&GraphWrapper::setFont), sol::resolve<void(const rawrbox::FontWrapper&)>(&GraphWrapper::setFont)),
		    "getFont", &GraphWrapper::getFont,

		    "setFontLegend", sol::overload(sol::resolve<void(const std::string&, sol::optional<int>)>(&GraphWrapper::setFontLegend), sol::resolve<void(const rawrbox::FontWrapper&)>(&GraphWrapper::setFontLegend)),
		    "getFontLegend", &GraphWrapper::getFontLegend,
		    // ----

		    sol::base_classes, sol::bases<rawrbox::UIContainerWrapper>());
	}
} // namespace rawrbox
