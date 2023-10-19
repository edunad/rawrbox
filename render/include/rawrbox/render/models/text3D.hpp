#pragma once

#include <rawrbox/render/materials/text.hpp>
#include <rawrbox/render/models/model.hpp>
#include <rawrbox/render/text/font.hpp>

namespace rawrbox {
	class Text3D : public rawrbox::Model {
	protected:
		float _scaleMul = 0.25F;

	public:
		Text3D();
		Text3D(const Text3D&) = delete;
		Text3D(Text3D&&) = delete;
		Text3D& operator=(const Text3D&) = delete;
		Text3D& operator=(Text3D&&) = delete;
		~Text3D() override = default;

		static uint32_t ID;

		template <typename M = rawrbox::MaterialBase>
		void setMaterial() {
			this->_material = std::make_unique<M>();
			if ((this->_material->supports() & rawrbox::MaterialFlags::TEXT) == 0) throw std::runtime_error("[RawrBox-Text3D] Invalid material! Text3D only supports `text` materials!");
		}

		// UTILS ----
		void setScaleMul(float mul);
		[[nodiscard]] float getScaleMul() const;

		uint32_t addText(const rawrbox::Font& font, const std::string& text, const rawrbox::Vector3f& pos, const rawrbox::Colorf& cl = rawrbox::Colors::White(), rawrbox::Alignment alignX = rawrbox::Alignment::Center, rawrbox::Alignment alignY = rawrbox::Alignment::Center, bool billboard = false);
		void removeText(uint32_t indx);
		// ----------

		void upload(bool dynamic = false) override;
	};
} // namespace rawrbox
