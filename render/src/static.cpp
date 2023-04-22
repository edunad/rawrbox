#include <rawrbox/render/static.h>
namespace rawrBox {
	bgfx::ViewId CURRENT_VIEW_ID = 0;

	std::shared_ptr<rawrBox::TextureMissing> MISSING_TEXTURE = nullptr;
	std::shared_ptr<rawrBox::TextureFlat> MISSING_SPECULAR_TEXTURE = nullptr;
} // namespace rawrBox
