#pragma once

#include <rawrbox/render/model/base.hpp>
#include <rawrbox/render/model/material/text_unlit.hpp>
#include <rawrbox/render/text/font.hpp>

namespace rawrbox {
	class Text3D : public rawrbox::ModelBase<rawrbox::MaterialText3DUnlit> {
	public:
		using ModelBase<rawrbox::MaterialText3DUnlit>::ModelBase;

		// UTILS ----
		void addText(std::shared_ptr<rawrbox::Font> font, const std::string& text, const rawrbox::Vector3f& pos, const rawrbox::Colorf& cl = rawrbox::Colors::White, rawrbox::Alignment alignX = rawrbox::Alignment::Center, rawrbox::Alignment alignY = rawrbox::Alignment::Center, bool billboard = true);
		// ----------

		void draw(const rawrbox::Vector3f& camPos) override;
	};
} // namespace rawrbox
