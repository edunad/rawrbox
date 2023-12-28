
#pragma once
#include <rawrbox/math/color.hpp>
#include <rawrbox/render/resources/font.hpp>
#include <rawrbox/ui/container.hpp>

#include <chrono>
#include <string>
#include <utility>

namespace rawrbox {
	enum class UIGraphStyle {
		BLOCK = 0,
		LINE
	};

	struct UIGraphData {
		float height = 0.F;

		rawrbox::Vector2f a = {};
		rawrbox::Vector2f b = {};
		rawrbox::Color col = {};
	};

	struct UIGraphCategory {
	protected:
		std::chrono::high_resolution_clock::time_point _timer;
		std::string _name = "";
		rawrbox::Color _color = rawrbox::Colors::White();

	public:
		static constexpr size_t ENTRY_COUNT = 128;

		UIGraphCategory() = default;
		UIGraphCategory(std::string name, const rawrbox::Colorf& cl) : _name(std::move(name)), _color(cl) {}
		UIGraphCategory(std::string name, const rawrbox::Colori& cl) : _name(std::move(name)), _color(cl.cast<float>()) {}

		std::array<float, ENTRY_COUNT> entries = {0};
		std::array<float, ENTRY_COUNT> smoothed = {0};

		const std::string& getName() { return this->_name; }
		void setName(const std::string& name) { this->_name = name; }

		const rawrbox::Colorf& getColor() { return this->_color; }
		void setColor(const rawrbox::Colorf& color) { this->_color = color; }

		void addEntry(float data) {
			// move times one to the right
			for (size_t i = 0; i < entries.size() - 1; i++) {
				this->entries[i] = entries[i + 1];
				this->smoothed[i] = smoothed[i + 1];
			}

			// overwrite current
			this->entries.back() = data;
		}

		void startTimer() {
			this->_timer = std::chrono::high_resolution_clock::now();
		}

		void stopTimer() {
			auto time = std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - this->_timer).count() * 1000;
			this->addEntry(time);
		}
	};

	class UIGraph : public rawrbox::UIContainer {
	protected:
		std::vector<std::unique_ptr<rawrbox::UIGraphCategory>> _categories = {};

		float _highestValue = 60.0F;
		size_t _smoothSize = 10;

		bool _autoScale = false;
		bool _showLegend = false;
		rawrbox::UIGraphStyle _style = rawrbox::UIGraphStyle::BLOCK;

		rawrbox::Color _textLineColor = rawrbox::Colors::Gray();
		rawrbox::Color _textColor = rawrbox::Colors::White();
		rawrbox::Color _textShadowColor = rawrbox::Colors::Black();
		rawrbox::Vector2f _textShadow = {1, 1};

		// RESOURCES ---
		rawrbox::Font* _font = nullptr;
		rawrbox::Font* _fontLegend = nullptr;
		// -----------------

		std::array<float, rawrbox::UIGraphCategory::ENTRY_COUNT> _totalTimes = {};
		std::vector<std::array<rawrbox::Vector2f, rawrbox::UIGraphCategory::ENTRY_COUNT>> _vertCats = {};
		std::vector<std::pair<std::string, float>> texts = {};

#ifdef RAWRBOX_SCRIPTING
		void initializeLua() override;
#endif

	public:
		~UIGraph() override = default;
		UIGraph() = default;
		UIGraph(const UIGraph&) = default;
		UIGraph(UIGraph&&) = delete;
		UIGraph& operator=(const UIGraph&) = default;
		UIGraph& operator=(UIGraph&&) = delete;

		// CATEGORY --
		virtual rawrbox::UIGraphCategory& getCategory(size_t id);
		virtual rawrbox::UIGraphCategory& addCategory(const std::string& name, const rawrbox::Color& color);
		// ----------

		// UTILS ----
		virtual void setStyle(rawrbox::UIGraphStyle style);
		virtual void setSmoothing(size_t frames);
		virtual void setAutoScale(bool val);
		virtual void setHighest(float val);
		// ---------

		// TEXT ----
		virtual void setShowLegend(bool mode);
		[[nodiscard]] virtual bool getShowLegend() const;

		virtual void addText(const std::string& text, float val);

		virtual void setTextLineColor(const rawrbox::Color& col);
		[[nodiscard]] virtual const rawrbox::Color& getTextLineColor() const;

		virtual void setTextColor(const rawrbox::Color& col);
		[[nodiscard]] virtual const rawrbox::Color& getTextColor() const;

		virtual void setTextShadowPos(const rawrbox::Vector2f& pos);
		[[nodiscard]] virtual const rawrbox::Vector2f& getTextShadowPos() const;

		virtual void setTextShadowColor(const rawrbox::Color& col);
		[[nodiscard]] virtual const rawrbox::Color& getTextShadowColor() const;

		virtual void setFont(rawrbox::Font* font);
		virtual void setFont(const std::filesystem::path& font, uint16_t size = 11);
		[[nodiscard]] virtual rawrbox::Font* getFont() const;

		virtual void setFontLegend(rawrbox::Font* font);
		virtual void setFontLegend(const std::filesystem::path& font, uint16_t size = 11);
		[[nodiscard]] virtual rawrbox::Font* getFontLegend() const;
		// ---------

		// FOCUS HANDLE ---
		[[nodiscard]] bool hitTest(const rawrbox::Vector2f& point) const override;
		// -----

		// DRAW ----
		void update() override;
		[[nodiscard]] bool clipOverflow() const override;
		void draw(rawrbox::Stencil& stencil) override;
		// -------
	};
} // namespace rawrbox
