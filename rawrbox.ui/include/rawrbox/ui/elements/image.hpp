
#pragma once
#include <rawrbox/render/textures/base.hpp>
#include <rawrbox/ui/container.hpp>

#include <filesystem>

namespace rawrbox {
	class UIImage : public rawrbox::UIContainer {
	protected:
		rawrbox::TextureBase* _texture = nullptr;
		rawrbox::Color _color = rawrbox::Colors::White();

	public:
		UIImage(rawrbox::UIRoot* root);
		UIImage(const UIImage&) = default;
		UIImage(UIImage&&) = delete;
		UIImage& operator=(const UIImage&) = default;
		UIImage& operator=(UIImage&&) = delete;
		~UIImage() override;

		// UTILS ----
		[[nodiscard]] virtual rawrbox::TextureBase* getTexture() const;
		virtual void setTexture(rawrbox::TextureBase* texture);
		virtual void setTexture(const std::filesystem::path& path);

		[[nodiscard]] virtual const rawrbox::Color& getColor() const;
		virtual void setColor(const rawrbox::Color& col);

		virtual void sizeToContents();
		// ----------

		// FOCUS HANDLE ---
		[[nodiscard]] bool hitTest(const rawrbox::Vector2f& point) const override;
		// -----

		// DRAW ----
		void draw(rawrbox::Stencil& stencil) override;
		// -------
	};
} // namespace rawrbox
