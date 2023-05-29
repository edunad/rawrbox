#include <rawrbox/render/static.hpp>
#include <rawrbox/render/texture/base.hpp>

namespace rawrbox {
	TextureBase::~TextureBase() { RAWRBOX_DESTROY(this->_handle); }

	// UTILS ---

	const bool TextureBase::hasTransparency() const { return false; }
	const rawrbox::Vector2i& TextureBase::getSize() const { return this->_size; }
	const bool TextureBase::valid() const { return bgfx::isValid(this->_handle); }
	const bgfx::TextureHandle TextureBase::getHandle() const { return this->_handle; }
	// ----

	void TextureBase::setFlags(uint32_t flags) { this->_flags = flags; }

} // namespace rawrbox
