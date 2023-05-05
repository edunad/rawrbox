#include <rawrbox/render/static.hpp>

namespace rawrBox {
	bgfx::ViewId CURRENT_VIEW_ID = 0;

	bool BGFX_INITIALIZED = false;

	std::shared_ptr<rawrBox::TextureMissing> MISSING_TEXTURE = nullptr;
	std::shared_ptr<rawrBox::TextureFlat> MISSING_SPECULAR_TEXTURE = nullptr;
	std::shared_ptr<rawrBox::TextureFlat> WHITE_TEXTURE = nullptr;

} // namespace rawrBox
