#include <rawrbox/render/stencil.hpp>
#include <rawrbox/render/text/font.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/ui/elements/graph.hpp>

#include <string>
#include <utility>

namespace rawrbox {
	// CATEGORY --
	rawrbox::UIGraphCategory& UIGraph::getCategory(size_t id) { return *this->_categories[id].get(); }
	rawrbox::UIGraphCategory& UIGraph::addCategory(const std::string& name, const rawrbox::Color& color) {
		auto cat = std::make_unique<rawrbox::UIGraphCategory>();
		cat->color = color;
		cat->name = name;
		cat->smoothSize = this->_smoothSize;

		this->_categories.push_back(std::move(cat));
		return *this->_categories.back().get();
	}
	// ----------

	// UTILS ----
	void UIGraph::setStyle(rawrbox::UIGraphStyle style) { this->_style = style; }
	void UIGraph::setSmoothing(size_t frames) { this->_smoothSize = frames; }
	void UIGraph::setAutoScale(bool val) { this->_autoScale = val; }
	void UIGraph::setHighest(float val) { this->_highestValue = val; }
	// ---------

	// TEXT ----
	void UIGraph::addText(const std::string& text, float val) {
		texts.emplace_back(text, val);
	}

	void UIGraph::setTextLineColor(const rawrbox::Color& col) { this->_textLineColor = col; }
	const rawrbox::Color& UIGraph::getTextLineColor() const { return this->_textLineColor; }

	void UIGraph::setTextColor(const rawrbox::Color& col) { this->_textColor = col; }
	const rawrbox::Color& UIGraph::getTextColor() const { return this->_textColor; }

	void UIGraph::setTextShadowPos(const rawrbox::Vector2f& pos) { this->_textShadow = pos; }
	const rawrbox::Vector2f& UIGraph::getTextShadowPos() const { return this->_textShadow; }

	void UIGraph::setTextShadowColor(const rawrbox::Color& col) { this->_textShadowColor = col; }
	const rawrbox::Color& UIGraph::getTextShadowColor() const { return this->_textShadowColor; }

	void UIGraph::setFont(const std::filesystem::path& font, int size) {
		this->_font = rawrbox::RESOURCES::getFile<rawrbox::ResourceFont>(font)->getSize(size);
	}

	void UIGraph::setFont(rawrbox::Font* font) {
		if (font == nullptr) throw std::runtime_error("[RawrBox-UI] Invalid font");
		this->_font = font;
	}

	rawrbox::Font* UIGraph::getFont() const { return this->_font; }

	void UIGraph::setFontLegend(const std::filesystem::path& font, int size) {
		this->_fontLegend = rawrbox::RESOURCES::getFile<rawrbox::ResourceFont>(font)->getSize(size);
	}

	void UIGraph::setFontLegend(rawrbox::Font* font) {
		if (font == nullptr) throw std::runtime_error("[RawrBox-UI] Invalid font");
		this->_fontLegend = font;
	}

	rawrbox::Font* UIGraph::getFontLegend() const { return this->_fontLegend; }
	// ---------

	// FOCUS HANDLE ---
	bool UIGraph::hitTest(const rawrbox::Vector2f& /*point*/) const { return false; }
	// -----

	// DRAW ----
	void UIGraph::update() {
		auto size = this->getSize();

		float stepWidth = size.x / static_cast<float>(this->_categories.front()->smoothed.size());
		float stepHight = size.y / this->_highestValue;

		float newHighest = 0;

		// Generate categories
		this->_vertCats.clear();
		this->_totalTimes = {0};

		for (auto& cat : this->_categories) {
			// get every category and create a tmp buffer

			std::array<rawrbox::Vector2f, rawrbox::UIGraphCategory::ENTRY_COUNT> verts = {};
			for (size_t indexEntry = 0; indexEntry < rawrbox::UIGraphCategory::ENTRY_COUNT; indexEntry++) {
				// add the time to the total
				this->_totalTimes[indexEntry] += cat->smoothed[indexEntry];
				auto timeTotal = cat->smoothed[indexEntry];

				// clamp it to make sure it doesn't render funny at peaks
				if (timeTotal > this->_highestValue) {
					if (this->_autoScale) {
						this->_highestValue = timeTotal;
					} else {
						timeTotal = this->_highestValue;
					}
				}

				if (timeTotal > newHighest) newHighest = timeTotal;

				// generate the vertice
				verts[indexEntry] = {
				    static_cast<float>(indexEntry) * stepWidth,
				    size.y - (timeTotal * stepHight)};
			}

			this->_vertCats.push_back(verts);
		}

		// smooth out to whatever value is the upper limit currently
		if (newHighest < this->_highestValue && this->_autoScale) {
			this->_highestValue = std::lerp(this->_highestValue, newHighest, 0.2F);
		}

		// force new frame, even if they didn't push times
		for (auto& cat : this->_categories) {
			if (cat->smoothSize > 0) {
				// use double to allow for bigger numbers
				double total = 0;
				for (size_t i = 0; i < this->_smoothSize; i++) {
					total += *(cat->entries.end() - 1 - i);
				}

				cat->smoothed.back() = static_cast<float>(total / (this->_smoothSize + 1));
			} else {
				cat->smoothed.back() = 0;
			}
		}
	}

	bool UIGraph::clipOverflow() const { return true; };

	void UIGraph::setShowLegend(bool mode) {
		_showLegend = mode;
	}
	bool UIGraph::getShowLegend() const {
		return _showLegend;
	}

	void UIGraph::draw(rawrbox::Stencil& stencil) {
		if (this->_categories.empty()) return;

		auto size = this->getSize();

		// BG ---
		stencil.drawBox({}, size, rawrbox::Colors::DarkGray * 0.2F);
		//---

		if (_showLegend) {
			float y = 5;
			for (auto& cat : _categories) {
				Vector2f tpos = {size.x - _fontLegend->getStringSize(cat->name).x - 5 - 4 - 5, y};
				stencil.drawText(*_fontLegend, cat->name, tpos + _textShadow, _textShadowColor);
				stencil.drawText(*_fontLegend, cat->name, tpos, _textColor);
				stencil.drawBox({size.x - 5 - 4, y + 1}, {4, _font->getLineHeight() - 2}, cat->color);
				y += _fontLegend->getLineHeight();
			}
		}

		for (auto& pair : texts) {
			float stepHeight = size.y / this->_highestValue;
			float y = size.y - (pair.second * stepHeight);
			stencil.drawText(*_font, pair.first, _textShadow + Vector2f(0, y), _textShadowColor);
			stencil.drawText(*_font, pair.first, {0, y}, _textColor);
			stencil.drawBox({0, y + _font->getLineHeight()}, {size.x, 2});
		}

		// Draw lines ---
		for (size_t indexEntry = 0; indexEntry < rawrbox::UIGraphCategory::ENTRY_COUNT - 1; indexEntry++) {
			std::vector<rawrbox::UIGraphData> lines = {};

			for (size_t indexCat = 0; indexCat < this->_categories.size(); indexCat++) {
				auto& cat = this->_categories[indexCat];
				if (this->_vertCats.empty()) continue;

				rawrbox::UIGraphData l;
				l.a = this->_vertCats[indexCat][indexEntry];
				l.b = this->_vertCats[indexCat][indexEntry + 1];
				l.col = cat->color;
				l.height = cat->smoothed[indexEntry + 1];
				lines.push_back(l);
			}

			std::sort(lines.begin(),
			    lines.end(),
			    [](rawrbox::UIGraphData& a, rawrbox::UIGraphData& b) {
				    return a.height > b.height;
			    });

			for (auto& l : lines) {
				if (this->_style == rawrbox::UIGraphStyle::BLOCK) {
					rawrbox::Polygon poly;
					auto col = l.col * 0.25F;
					col.a = 1.F;

					poly.verts = {
					    {{l.a.x, size.y}, {0, 0}, col},
					    {{l.b.x, size.y}, {1, 0}, col},
					    {l.a, {0, 1}, col},
					    {l.b, {1, 1}, col}};
					poly.indices = {0, 1, 2, 1, 3, 2};

					stencil.drawPolygon(poly);
				}

				stencil.pushOutline({1.01F}); // Force non-line, so it merges
				stencil.drawLine(l.a, l.b, l.col);
				stencil.popOutline();
			}
		}
		// ---------
	}
	// -------
} // namespace rawrbox
