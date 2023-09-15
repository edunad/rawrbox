#include <rawrbox/render/static.hpp>
#include <rawrbox/render/texture/base.hpp>

namespace rawrbox {
	TextureBase::~TextureBase() {
		if (this->_failedToLoad) return; // Don't delete the fallback
		RAWRBOX_DESTROY(this->_handle);
	}

	void TextureBase::loadFallback() {
		this->_failedToLoad = true;
		this->_handle = rawrbox::MISSING_TEXTURE->getHandle();
	}

	// UTILS ---
	bool TextureBase::hasTransparency() const { return false; }
	const rawrbox::Vector2i& TextureBase::getSize() const { return this->_size; }
	bool TextureBase::isValid() const { return bgfx::isValid(this->_handle); }
	const bgfx::TextureHandle& TextureBase::getHandle() const { return this->_handle; }
	void TextureBase::setTextureUV(rawrbox::TEXTURE_UV mode) { this->_textureUV = mode; }
	rawrbox::TEXTURE_UV TextureBase::getTextureUV() const { return this->_textureUV; }
	// ----

	void TextureBase::setFlags(uint64_t flags) { this->_flags = flags; }
	void TextureBase::update() {}

	std::array<float, 4> TextureBase::getData() const { return {static_cast<float>(this->_textureUV)}; }

} // namespace rawrbox
