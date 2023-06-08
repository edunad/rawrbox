#include <rawrbox/render/resources/gif.hpp>
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/render/stencil.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/ui/elements/image.hpp>

namespace rawrbox {
	UIImage::~UIImage() {
		this->_texture = nullptr;
	}

	// UTILS ----
	void UIImage::setTexture(rawrbox::TextureBase& texture) { this->_texture = &texture; }
	void UIImage::setTexture(const std::filesystem::path& path) {
		this->_isAnimated = path.extension() == ".gif";

		if (!this->_isAnimated) {
			this->_texture = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>(path)->get();
		} else {
			this->_texture = rawrbox::RESOURCES::getFile<rawrbox::ResourceGIF>(path)->get();
		}
	}

	const rawrbox::Color& UIImage::getColor() const { return this->_color; }
	void UIImage::setColor(const rawrbox::Color& cl) { this->_color = cl; }

	void UIImage::sizeToContents() {
		if (this->_texture == nullptr) return;
		this->setSize(this->_texture->getSize().cast<float>());
	}
	// ---------------

	// FOCUS HANDLE ---
	bool UIImage::hitTest(const rawrbox::Vector2f& point) const { return false; }
	// -----

	// DRAW ----
	void UIImage::update() {
		if (!this->_isAnimated || this->_texture == nullptr) return;
		auto t = dynamic_cast<rawrbox::TextureGIF*>(this->_texture);
		if (t == nullptr) return;

		t->step();
	}

	void UIImage::draw(rawrbox::Stencil& stencil) {
		if (this->_texture == nullptr) return;
		stencil.drawTexture({0, 0}, this->getSize(), *this->_texture, this->_color);
	}
	// -------
} // namespace rawrbox
