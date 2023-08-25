
#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/render/text/font.hpp>
#include <rawrbox/render/text/markdown.hpp>
#include <rawrbox/ui/elements/input.hpp>
#include <rawrbox/utils/console.hpp>

namespace rawrbox {
	struct ConsoleEntry {
		float height = 0;
		float tagWidth = 0;

		std::string msg;
		std::string tag;
		std::string timestamp;

		rawrbox::Colorf color;
		rawrbox::Colorf colorTag;

		rawrbox::PrintType type = rawrbox::PrintType::LOG;
	};

	class UIConsole : public rawrbox::UIContainer {
		rawrbox::Font* _consolas = nullptr;
		rawrbox::Font* _consolasb = nullptr;
		rawrbox::TextureBase* _overlay = nullptr;
		rawrbox::UIInput* _input = nullptr;
		rawrbox::Console* _console = nullptr;

		std::unique_ptr<rawrbox::Markdown> _md = nullptr;

		std::vector<std::string> _commandHistory = {};
		std::vector<std::string> _commandHelp = {};
		std::vector<rawrbox::ConsoleEntry> _entries = {};

		size_t _lastCmdIndex = 0;
		size_t _helpOffset = 0;

		float _textPadding = 2;
		float _mouseScrollY = 0;
		float _maxYOffset = 0;

		bool _autoScroll = true;
		bool _isDirty = false;

		virtual void updateEntries();
		virtual void drawPrimitives(rawrbox::Stencil& stencil, const std::vector<ConsoleEntry>& entries);
		virtual void drawText(rawrbox::Stencil& stencil, const std::vector<ConsoleEntry>& entries);

	public:
		UIConsole(rawrbox::Console* console);
		UIConsole(const UIConsole&) = delete;
		UIConsole(UIConsole&&) = delete;
		UIConsole& operator=(const UIConsole&) = delete;
		UIConsole& operator=(UIConsole&&) = delete;
		~UIConsole() override;

		void initialize() override;
		void draw(rawrbox::Stencil& stencil) override;
		void afterDraw(rawrbox::Stencil& stencil) override;

		void setSize(const rawrbox::Vector2& size) override;
		void setVisible(bool visible) override;

		// FOCUS HANDLING ------
		[[nodiscard]] bool lockKeyboard() const override;
		[[nodiscard]] bool lockScroll() const override;
		// --

		void mouseScroll(const rawrbox::Vector2i& mousePos, const rawrbox::Vector2i& offset) override;
	};
} // namespace rawrbox
