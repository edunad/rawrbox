#pragma once

#include <rawrbox/scripting/mod.hpp>
#include <rawrbox/ui/scripting/wrappers/elements/button_wrapper.hpp>
#include <rawrbox/ui/scripting/wrappers/elements/frame_wrapper.hpp>
#include <rawrbox/ui/scripting/wrappers/elements/graph_cat_wrapper.hpp>
#include <rawrbox/ui/scripting/wrappers/elements/graph_wrapper.hpp>
#include <rawrbox/ui/scripting/wrappers/elements/group_wrapper.hpp>
#include <rawrbox/ui/scripting/wrappers/elements/image_wrapper.hpp>
#include <rawrbox/ui/scripting/wrappers/elements/input_wrapper.hpp>
#include <rawrbox/ui/scripting/wrappers/elements/label_wrapper.hpp>
#include <rawrbox/ui/scripting/wrappers/elements/progress_bar_wrapper.hpp>

namespace rawrbox {
	class UIRoot;
	class UIWrapper {
	protected:
		rawrbox::Mod* _mod;
		rawrbox::UIRoot* _root = nullptr;

		sol::function create;

	public:
		UIWrapper(rawrbox::Mod* mod, rawrbox::UIRoot* root);
		UIWrapper(const UIWrapper&) = default;
		UIWrapper(UIWrapper&&) = default;
		UIWrapper& operator=(const UIWrapper&) = default;
		UIWrapper& operator=(UIWrapper&&) = default;
		virtual ~UIWrapper() = default;

		// CREATE ------
		sol::object createFrame(sol::optional<rawrbox::UIContainerWrapper> parent);
		sol::object createGroup(sol::optional<rawrbox::UIContainerWrapper> parent);
		sol::object createButton(sol::optional<rawrbox::UIContainerWrapper> parent);
		sol::object createProgressBar(sol::optional<rawrbox::UIContainerWrapper> parent);
		sol::object createLabel(sol::optional<rawrbox::UIContainerWrapper> parent);
		sol::object createInput(sol::optional<rawrbox::UIContainerWrapper> parent);
		sol::object createImage(sol::optional<rawrbox::UIContainerWrapper> parent);
		sol::object createGraph(sol::optional<rawrbox::UIContainerWrapper> parent);
		// ----------------

		// UTILS -----------
		[[nodiscard]] virtual const rawrbox::Vector2f size() const;
		// ---------------

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
