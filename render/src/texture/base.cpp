#include <rawrbox/render/static.h>
#include <rawrbox/render/texture/base.h>

namespace rawrBox {
	TextureBase::~TextureBase() { RAWRBOX_DESTROY(this->_handle); }
	bgfx::TextureHandle& TextureBase::getHandle() { return this->_handle; }
} // namespace rawrBox
