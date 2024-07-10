
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/render/stencil.hpp>
#include <rawrbox/render/textures/gif.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/ui/elements/image.hpp>

namespace rawrbox {
	UIImage::UIImage(rawrbox::UIRoot* root) : rawrbox::UIContainer(root) {}
	UIImage::~UIImage() {
		this->_texture = nullptr;
	}

	// UTILS ----
	rawrbox::TextureBase* UIImage::getTexture() const { return this->_texture; }
	void UIImage::setTexture(rawrbox::TextureBase* texture) { this->_texture = texture; }
	void UIImage::setTexture(const std::filesystem::path& path) {
		this->_texture = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>(path)->get();
	}

	const rawrbox::Color& UIImage::getColor() const { return this->_color; }
	void UIImage::setColor(const rawrbox::Color& col) { this->_color = col; }

	void UIImage::sizeToContents() {
		if (this->_texture == nullptr) return;
		this->setSize(this->_texture->getSize().cast<float>());
	}
	// ---------------

	// FOCUS HANDLE ---
	bool UIImage::hitTest(const rawrbox::Vector2f& /*point*/) const { return false; }
	// -----

	// DRAW ----
	void UIImage::draw(rawrbox::Stencil& stencil) {
		if (this->_texture == nullptr) return;
		stencil.drawTexture({0, 0}, this->getSize(), *this->_texture, this->_color);
	}
	// -------
} // namespace rawrbox
