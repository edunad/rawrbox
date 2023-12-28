
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/render/stencil.hpp>
#include <rawrbox/render/textures/gif.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/ui/elements/image.hpp>

#ifdef RAWRBOX_SCRIPTING
	#include <rawrbox/ui/scripting/wrappers/elements/image_wrapper.hpp>
#endif

namespace rawrbox {
#ifdef RAWRBOX_SCRIPTING
	void UIImage::initializeLua() {
		if (!SCRIPTING::initialized) return;
		this->_luaWrapper = sol::make_object(rawrbox::SCRIPTING::getLUA(), rawrbox::ImageWrapper(this->shared_from_this()));
	}
#endif

	// UTILS ----
	rawrbox::TextureBase* UIImage::getTexture() const { return this->_texture; }
	void UIImage::setTexture(rawrbox::TextureBase* texture) { this->_texture = texture; }
	void UIImage::setTexture(const std::filesystem::path& path) {
		this->_texture = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>(path)->get();
	}

	const rawrbox::Color& UIImage::getColor() const { return this->_color; }
	void UIImage::setColor(const rawrbox::Color& cl) { this->_color = cl; }

	void UIImage::sizeToContents() {
		if (this->_texture == nullptr) return;
		this->setSize(this->_texture->getSize().cast<float>());
	}
	// ---------------

	// FOCUS HANDLE ---
	bool UIImage::hitTest(const rawrbox::Vector2f& /*point*/) const { return false; }
	// -----

	// DRAW ----
	void UIImage::update() {
		if (this->_texture == nullptr) return;
		this->_texture->update();
	}

	void UIImage::draw(rawrbox::Stencil& stencil) {
		if (this->_texture == nullptr) return;
		stencil.drawTexture({0, 0}, this->getSize(), *this->_texture, this->_color);
	}
	// -------
} // namespace rawrbox
