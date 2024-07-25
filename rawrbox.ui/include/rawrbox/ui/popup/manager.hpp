#pragma once

#include <rawrbox/utils/logger.hpp>

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

namespace rawrbox {
	enum class PopupType {
		GENERIC = 0,
		WARNING,
		ERR,
		LOADING,
		QUESTION,
	};

	class UIFrame;
	class UIRoot;

	class POPUP {
		static rawrbox::UIRoot* _root;

		static std::unordered_map<std::string, rawrbox::UIFrame*> _popups;
		static std::unique_ptr<rawrbox::Logger> _logger;

		static rawrbox::UIFrame* spawn(const std::string& id, const std::string& title, const std::string& message, PopupType type, const std::function<void(bool)>& callback = nullptr);

	public:
		static void init(rawrbox::UIRoot* root);
		static void shutdown();

		// CREATE ----
		static rawrbox::UIFrame* create(const std::string& title, const std::string& message, PopupType type = PopupType::GENERIC, const std::function<void(bool)>& callback = nullptr);
		static rawrbox::UIFrame* create(const std::string& id, const std::string& title, const std::string& message, PopupType type = PopupType::GENERIC, const std::function<void(bool)>& callback = nullptr);

		static bool destroy();
		static bool destroy(const std::string& id);
		// -------------

		// UTILS ----
		static rawrbox::UIFrame* get(const std::string& id);
		static bool exists(const std::string& id);

		static void clear();
		// ------
	};
} // namespace rawrbox
