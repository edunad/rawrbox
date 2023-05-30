
#pragma once
#include <rawrbox/render/texture/base.hpp>
#include <rawrbox/ui/base.hpp>

#include <filesystem>

namespace rawrbox {
	class UIImage : public rawrbox::UIBase {
	protected:
		std::shared_ptr<rawrbox::TextureBase> _texture = nullptr;

		rawrbox::Color _color = rawrbox::Colors::White;
		bool _isAnimated = false;

	public:
		~UIImage() override;

		// UTILS ----
		virtual void setTexture(std::shared_ptr<rawrbox::TextureBase>& texture);
		virtual void setTexture(const std::filesystem::path& path);

		[[nodiscard]] virtual const rawrbox::Color& getColor() const;
		virtual void setColor(const rawrbox::Color& col);

		virtual void sizeToContents();
		// ----------

		// FOCUS HANDLE ---
		[[nodiscard]] bool hitTest(const rawrbox::Vector2f& point) const override;
		// -----

		// DRAW ----
		void update() override;
		void draw(rawrbox::Stencil& stencil) override;
		// -------
	};
} // namespace rawrbox
