#pragma once

#include <rawrbox/render/materials/text.hpp>
#include <rawrbox/render/model/model.hpp>
#include <rawrbox/render/text/font.hpp>

namespace rawrbox {
	class Text3D : public rawrbox::Model<rawrbox::MaterialText3D> {
	protected:
		float _scaleMul = 0.25F;

	public:
		Text3D() = default;
		Text3D(const Text3D&) = delete;
		Text3D(Text3D&&) = delete;
		Text3D& operator=(const Text3D&) = delete;
		Text3D& operator=(Text3D&&) = delete;
		~Text3D() override = default;

		static uint32_t ID;

		// UTILS ----
		void setScaleMul(float mul);
		[[nodiscard]] float getScaleMul() const;

		uint32_t addText(const rawrbox::Font& font, const std::string& text, const rawrbox::Vector3f& pos, const rawrbox::Colorf& cl = rawrbox::Colors::White, rawrbox::Alignment alignX = rawrbox::Alignment::Center, rawrbox::Alignment alignY = rawrbox::Alignment::Center, bool billboard = false);
		void removeText(uint32_t indx);
		// ----------

		void upload(bool dynamic = false) override;
		void draw() override;
	};
} // namespace rawrbox
