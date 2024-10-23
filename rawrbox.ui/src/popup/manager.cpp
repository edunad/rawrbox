
#include <rawrbox/render/static.hpp>
#include <rawrbox/ui/elements/button.hpp>
#include <rawrbox/ui/elements/frame.hpp>
#include <rawrbox/ui/elements/label.hpp>
#include <rawrbox/ui/elements/loading.hpp>
#include <rawrbox/ui/popup/manager.hpp>
#include <rawrbox/ui/root.hpp>

namespace rawrbox {
	// PRIVATE ---
	rawrbox::UIRoot* POPUP::_root = {};
	std::unordered_map<std::string, rawrbox::UIFrame*> POPUP::_popups = {};
	std::unique_ptr<rawrbox::Logger> POPUP::_logger = std::make_unique<rawrbox::Logger>("RawrBox-POPUP");
	// ----------

	void POPUP::init(rawrbox::UIRoot* root) {
		if (root == nullptr) RAWRBOX_CRITICAL("Invalid UIRoot!");
		_root = root;
	}

	void POPUP::shutdown() {
		_popups.clear();
		_root = nullptr;
	}

	rawrbox::UIFrame* POPUP::spawn(const std::string& id, const std::string& title, const std::string& message, rawrbox::PopupType type, const std::function<void(bool)>& callback) {
		if (_root == nullptr) RAWRBOX_CRITICAL("UIRoot is not initialized!");

		auto titleColor = rawrbox::Color::RGBHex(0xFFFFFF);
		auto btnColor = rawrbox::Color::RGBHex(0x282a2e);

		switch (type) {
			case PopupType::WARNING:
				titleColor = Color::RGBHex(0xffb142);
				btnColor = Color::RGBHex(0x3F3528);
				break;
			case PopupType::ERR:
				titleColor = Color::RGBHex(0xff5252);
				btnColor = Color::RGBHex(0x402929);
				break;
			case PopupType::LOADING:
				titleColor = Color::RGBHex(0x706fd3);
				break;
			default:
				break;
		}

		const auto& windowSize = rawrbox::RENDERER->getSize().cast<float>();

		rawrbox::Vector2f size = {350, 60};
		rawrbox::Vector2f pos = (windowSize - size) / 2;

		const bool loading = type == rawrbox::PopupType::LOADING;

		// Destroy old one, if it exists
		auto* old = get(id);
		if (old != nullptr) {
			pos = old->getPos();
			old->remove();

			destroy(id);
		}
		// -----

		auto* popupFrame = _root->createChild<rawrbox::UIFrame>();
		popupFrame->setTitle(title);
		popupFrame->setTitleColor(titleColor);
		popupFrame->setSize(size);
		popupFrame->setPos(pos);
		popupFrame->setClosable(!loading);
		popupFrame->bringToFront();
		popupFrame->onClose += [id, callback, popupFrame]() {
			if (callback != nullptr) callback(false);
			destroy(id);
			popupFrame->remove();
		};

		if (loading) {
			auto* loadFrame = popupFrame->createChild<rawrbox::UILoading>();
			loadFrame->setSize({size.x, size.x});
		}

		auto* messageLabel = popupFrame->createChild<rawrbox::UILabel>();
		messageLabel->setText(message);
		messageLabel->setFont(rawrbox::DEBUG_FONT_REGULAR);
		messageLabel->sizeToContents();

		const auto& frameSize = popupFrame->getContentSize();
		if (!loading) {
			messageLabel->setPos({2, 3});

			if (type == rawrbox::PopupType::QUESTION) {
				auto halfWidth = frameSize.x / 2;
				auto offset = 40;

				auto* yesButton = popupFrame->createChild<rawrbox::UIButton>();
				yesButton->setSize({halfWidth + offset - 4, 20}); // - Padding
				yesButton->setPos({2, frameSize.y - 22});         // - Padding
				yesButton->setText("YES", 11);
				yesButton->setEnabled(true);
				yesButton->setTextColor(rawrbox::Colors::White());
				yesButton->setBorder(false);
				yesButton->setBackgroundColor(btnColor);
				yesButton->onClick += [id, callback, popupFrame]() {
					if (callback != nullptr) callback(true);
					if (destroy(id)) popupFrame->remove();
				};

				auto* noButton = popupFrame->createChild<rawrbox::UIButton>();
				noButton->setSize({halfWidth - offset - 4, 20});              // - Padding
				noButton->setPos({2 + halfWidth + offset, frameSize.y - 22}); // - Padding
				noButton->setText("NO", 11);
				noButton->setEnabled(true);
				noButton->setTextColor(rawrbox::Colors::White());
				noButton->setBorder(false);
				noButton->setBackgroundColor(Color::RGBHex(0x402929));
				noButton->onClick += [id, callback, popupFrame]() {
					if (callback != nullptr) callback(false);
					if (destroy(id)) popupFrame->remove();
				};

			} else {
				auto* okButton = popupFrame->createChild<rawrbox::UIButton>();
				okButton->setSize({frameSize.x - 4, 20}); // - Padding
				okButton->setPos({2, frameSize.y - 22});  // - Padding
				okButton->setText("OK", 11);
				okButton->setEnabled(true);
				okButton->setTextColor(rawrbox::Colors::White());
				okButton->setBorder(false);
				okButton->setBackgroundColor(btnColor);
				okButton->onClick += [id, callback, popupFrame]() {
					if (callback != nullptr) callback(true);
					if (destroy(id)) popupFrame->remove();
				};
			}
		} else {
			messageLabel->setPos((frameSize - messageLabel->getSize()) / 2);
		}

		_popups[id] = popupFrame;
		return popupFrame;
	}

	// CREATE ----
	rawrbox::UIFrame* POPUP::create(const std::string& title, const std::string& message, rawrbox::PopupType type, const std::function<void(bool)>& callback) {
		return spawn("default", title, message, type, callback);
	}

	rawrbox::UIFrame* POPUP::create(const std::string& id, const std::string& title, const std::string& message, rawrbox::PopupType type, const std::function<void(bool)>& callback) {
		return spawn(id, title, message, type, callback);
	}

	bool POPUP::destroy() { return destroy("default"); }
	bool POPUP::destroy(const std::string& id) {
		auto fnd = _popups.find(id);
		if (fnd == _popups.end()) return false;

		_popups.erase(id);
		return true;
	}
	// ---------------

	// UTILS ----
	bool POPUP::exists(const std::string& id) { return _popups.find(id) != _popups.end(); }
	rawrbox::UIFrame* POPUP::get(const std::string& id) {
		auto fnd = _popups.find(id);
		if (fnd == _popups.end()) return nullptr;

		return fnd->second;
	}

	void POPUP::clear() {
		for (const auto& popup : _popups) {
			popup.second->remove();
		}

		_popups.clear();
	}
	// ---------------
} // namespace rawrbox
