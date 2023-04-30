#include <rawrbox/render/static.hpp>
#include <rawrbox/render/texture/base.hpp>

namespace rawrBox {
	TextureBase::~TextureBase() { RAWRBOX_DESTROY(this->_handle); }
	bgfx::TextureHandle& TextureBase::getHandle() { return this->_handle; }
	bool TextureBase::valid() { return bgfx::isValid(this->_handle); }
} // namespace rawrBox
