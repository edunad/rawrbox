
#include <rawrbox/engine/static.hpp>
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/ui/elements/console.hpp>
#include <rawrbox/ui/static.hpp>
#include <rawrbox/utils/keys.hpp>
#include <rawrbox/utils/string.hpp>

namespace rawrbox {
	UIConsole::UIConsole(rawrbox::Console& console) : _console(&console) {}
	UIConsole::~UIConsole() {
		if (this->_console == nullptr) return;
		this->_console->onPrint.clear();
	}

	void UIConsole::initialize() {
		if (this->_console == nullptr) throw std::runtime_error("[RawrBox-UIConsole] Invalid console reference");

		this->_consolas = rawrbox::RESOURCES::getFile<rawrbox::ResourceFont>("consola.ttf")->getSize(11);
		this->_consolasb = rawrbox::RESOURCES::getFile<rawrbox::ResourceFont>("consolab.ttf")->getSize(11);

		this->_overlay = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("assets/textures/ui/overlay/overlay.png")->get();

		this->_md = std::make_unique<rawrbox::Markdown>();
		this->_md->fontRegular = this->_consolas;
		this->_md->fontBold = this->_consolasb;
		this->_md->fontItalic = rawrbox::RESOURCES::getFile<rawrbox::ResourceFont>("consolai.ttf")->getSize(11);

		// SETUP UI ---
		auto& size = this->getSize();
		this->_input = this->createChild<rawrbox::UIInput>();
		this->_input->setFont(this->_consolas);
		this->_input->setSize({size.x, 18});
		this->_input->setPos({0, size.y - 18});
		this->_input->setBackgroundColor(rawrbox::Colors::White());
		this->_input->setColor(rawrbox::Colors::Black());
		this->_input->setPadding(2);

		this->_input->onKey += [this](uint32_t key) {
			// TAB TO CYCLE HELP
			if (key == KEY_TAB) {
				if (this->_commandHelp.empty()) return;
				this->_input->setText(this->_commandHelp[this->_helpOffset], true, true);

				if (this->_helpOffset + 1 >= this->_commandHelp.size()) {
					this->_helpOffset = 0;
				} else {
					this->_helpOffset++;
				}

			} else {
				// Command history
				if (this->_commandHistory.empty()) return;

				if (key == KEY_UP) {
					this->_input->setText(this->_commandHistory[this->_lastCmdIndex], true);
					if (this->_lastCmdIndex > 0) this->_lastCmdIndex--;
				} else if (key == KEY_DOWN) {
					if (this->_lastCmdIndex < this->_commandHistory.size() - 1) {
						this->_lastCmdIndex++;
						this->_input->setText(this->_commandHistory[this->_lastCmdIndex], true);
					} else {
						this->_input->clear();
					}
				} else {
					this->_lastCmdIndex = this->_commandHistory.size() - 1;
				}
			}
		};

		this->_input->onTextUpdate += [this]() {
			auto& text = this->_input->getText();
			this->_commandHelp.clear();

			if (!text.empty()) {
				auto commands = this->_console->getCommandStr();
				for (auto& cmd : commands) {
					if (cmd.find(text) == std::string::npos) continue;
					if (this->_commandHelp.size() > 6) break;

					this->_commandHelp.push_back(cmd);
				}
			}

			this->_helpOffset = 0;
			this->_input->setHints(this->_commandHelp);
		};

		this->_input->onEnter += [this]() {
			auto& text = this->_input->getText();

			if (!text.empty()) {
				// Print the executed command
				this->_console->printExecuteCommand(rawrbox::StrUtils::split(text, ' '));

				// Only add to the history if different from last command
				if (this->_commandHistory.empty() || this->_commandHistory.back() != text) {
					this->_commandHistory.push_back(text);
				}

				this->_lastCmdIndex = this->_commandHistory.size() - 1;
			}

			this->_input->clear();
		};

		this->_console->onPrint += [this]() {
			// on print re-create all entries due appending / resizing
			rawrbox::runOnRenderThread([this]() {
				this->_isDirty = true;
			});
		};

		this->updateEntries();
	}

	void UIConsole::setVisible(bool visible) {
		rawrbox::UIContainer::setVisible(visible);
		if (visible && this->_input != nullptr) {
			this->_input->focus();
		}
	}

	void UIConsole::setSize(const rawrbox::Vector2& size) {
		rawrbox::UIContainer::setSize(size);

		if (this->_input != nullptr) {
			this->_input->setSize({size.x, 18});
			this->_input->setPos({0, size.y - 18});
		}
	}

	bool UIConsole::lockKeyboard() const {
		return this->visible();
	}

	bool UIConsole::lockScroll() const {
		return this->visible();
	}

	void UIConsole::mouseScroll(const Vector2i& /*_mousePos*/, const Vector2i& offset) {
		if (offset.y == 0) return;

		float newVal = this->_mouseScrollY + static_cast<float>(offset.y) * 2;
		if (newVal > 0) newVal = 0;
		if (newVal < this->_maxYOffset) newVal = this->_maxYOffset;

		this->_mouseScrollY = newVal;
		this->_autoScroll = this->_mouseScrollY == this->_maxYOffset;
	}

	void UIConsole::updateEntries() {
		if (this->_console == nullptr) return;

		auto logs = this->_console->getLogs();
		this->_entries.clear();

		for (auto& log : logs) {
			ConsoleEntry entry;

			entry.msg = log.log;
			entry.timestamp = log.timestamp;
			entry.type = log.type;
			entry.colorTag = Color::RGBAHex(0x000000A4);
			entry.color = Color::RGBHex(0x8465ec);
			entry.height = this->_consolas->getStringSize(log.log).y;

			switch (log.type) {
				case rawrbox::PrintType::EXECUTE:
					entry.color = Color::RGBHex(0xffd5c8);
					break;
				case rawrbox::PrintType::WARNING:
					entry.tag = "WARNING";
					entry.color = Color::RGBHex(0xffb879);
					break;
				case rawrbox::PrintType::ERR:
					entry.tag = "ERROR";
					entry.color = Color::RGBHex(0xcc425e);
					break;
				case rawrbox::PrintType::ECHO:
					entry.tag = "ECHO";
					entry.color = Color::RGBHex(0xB33771);
					break;
				default:
					entry.tag = "LOG";
					break;
			};

			entry.tagWidth = this->_consolasb->getStringSize(entry.tag).x;

			// Merge logs
			if (this->_entries.size() > 0 && this->_entries.back().type == log.type) {
				this->_entries.back().msg += "\n" + log.log;
				this->_entries.back().height += this->_consolas->getStringSize(log.log).y;
			} else {
				this->_entries.push_back(entry);
			}
		}
	}

	void UIConsole::drawPrimitives(rawrbox::Stencil& stencil, const std::vector<ConsoleEntry>& entries) {
		auto size = this->getSize();
		float yPos = 0;

		// Draw top background
		stencil.drawBox({}, {size.x, size.y - this->_input->getSize().y}, Color::RGBHex(0x0c0c0c));

		for (auto& log : entries) {
			float sizeY = log.height + this->_textPadding * 2;

			// skip out of view ones
			if (yPos + sizeY <= 0) {
				yPos += sizeY + 1; // + offset
				continue;
			}

			// Tag drawing
			if (!log.tag.empty()) {
				stencil.drawBox({0, this->_mouseScrollY + yPos}, {size.x, sizeY}, log.color * 0.5F);     // bg
				stencil.drawBox({0, this->_mouseScrollY + yPos}, {log.tagWidth + 10, sizeY}, log.color); // tag
			}

			yPos += sizeY + 1; // + offset
		}

		// Set the max Y
		this->_maxYOffset = -(yPos - size.y + 18); // + input
		if (this->_autoScroll) this->_mouseScrollY = this->_maxYOffset;
	}

	void UIConsole::drawText(rawrbox::Stencil& stencil, const std::vector<ConsoleEntry>& entries) {
		auto size = this->getSize();
		float yPos = 0;

		for (auto& log : entries) {
			float sizeY = log.height + this->_textPadding * 2;

			// skip out of view ones
			if (yPos + sizeY <= 0) {
				yPos += sizeY + 1; // + offset
				continue;
			}

			float offsetX = 0;

			// Tag drawing
			if (!log.tag.empty()) {
				stencil.drawText(*this->_consolasb, log.tag, {this->_textPadding + 2.5F, this->_mouseScrollY + yPos + this->_textPadding}, log.colorTag);
				offsetX = log.tagWidth + 10 + this->_textPadding;
			}

			// Setup markdown
			this->_md->clear();
			this->_md->parse(log.msg, log.color);
			// --

			this->_md->render(stencil, {offsetX + this->_textPadding, this->_mouseScrollY + yPos + this->_textPadding});

			// Draw timestamp (+ padding)
			stencil.drawText(*this->_consolas, log.timestamp, {size.x - 5, this->_mouseScrollY + yPos + this->_textPadding}, Color::RGBAHex(0xFFFFFF64), rawrbox::Alignment::Right);

			yPos += sizeY + 1; // + offset
		}
	}

	void UIConsole::draw(Stencil& stencil) {
		// if we had new entries, recalculate things
		if (this->_isDirty) {
			this->_isDirty = false;
			this->updateEntries();
		}

		// split draw calls up in 2, as text is a seperate draw call from drawing primitives
		this->drawPrimitives(stencil, this->_entries);
		this->drawText(stencil, this->_entries);
	}

	void UIConsole::afterDraw(rawrbox::Stencil& stencil) {
		auto& size = this->getSize();
		auto overlaySize = this->_overlay->getSize().cast<float>() / 2.F;

		stencil.drawTexture({}, size, *this->_overlay, Color::RGBAHex(0xFFFFFF0A), {}, {size.x / overlaySize.x, size.y / overlaySize.y});
	}
} // namespace rawrbox
