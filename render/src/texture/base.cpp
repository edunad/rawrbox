#include <rawrbox/render/static.hpp>
#include <rawrbox/render/texture/base.hpp>

namespace rawrbox {
	TextureBase::~TextureBase() { RAWRBOX_DESTROY(this->_handle); }

	bgfx::TextureHandle& TextureBase::getHandle() { return this->_handle; }

	bool TextureBase::valid() { return bgfx::isValid(this->_handle); }
	void TextureBase::setFlags(uint32_t flags) { this->_flags = flags; }
} // namespace rawrbox
