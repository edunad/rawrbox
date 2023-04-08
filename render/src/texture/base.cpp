#include <rawrbox/render/texture/base.h>

namespace rawrBox {
	TextureBase::~TextureBase() { bgfx::destroy(this->_handle); }
	bgfx::TextureHandle& TextureBase::getHandle() { return this->_handle; }
}
