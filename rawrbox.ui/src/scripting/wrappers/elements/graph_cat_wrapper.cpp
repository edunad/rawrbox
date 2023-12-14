#include <rawrbox/ui/elements/graph.hpp>
#include <rawrbox/ui/scripting/wrappers/elements/graph_cat_wrapper.hpp>

namespace rawrbox {
	void GraphCatWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<UIGraphCategory>("UIGraphCategory",
		    sol::constructors<rawrbox::UIGraphCategory(), rawrbox::UIGraphCategory(rawrbox::UIGraphCategory), rawrbox::UIGraphCategory(const std::string&, const rawrbox::Colori&)>(),

		    "getName", &UIGraphCategory::getName,
		    "setName", &UIGraphCategory::setName,

		    "getColor", &UIGraphCategory::getColor,
		    "setColor", &UIGraphCategory::setColor,

		    "addEntry", &UIGraphCategory::addEntry,

		    "startTimer", &UIGraphCategory::startTimer,
		    "stopTimer", &UIGraphCategory::stopTimer);
	}
	// -------------------------
} // namespace rawrbox
