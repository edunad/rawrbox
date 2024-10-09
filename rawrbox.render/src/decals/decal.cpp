#include <rawrbox/render/decals/decal.hpp>
#include <rawrbox/render/static.hpp>

namespace rawrbox {
	// PRIVATE ---
	std::unique_ptr<rawrbox::Logger> Decal::_logger = std::make_unique<rawrbox::Logger>("RawrBox-DECAL");
	// ------

	Decal::Decal(const rawrbox::Matrix4x4& _mtx, const rawrbox::TextureBase& _texture, const rawrbox::Colorf& _color, uint32_t _atlas) : worldToLocal(_mtx), color(_color) {
		this->setTexture(_texture, _atlas);
	}

	void Decal::setTexture(const rawrbox::TextureBase& texture, uint32_t id) {
		if (!texture.isValid()) {
			ERROR_RAWRBOX("Invalid texture, not uploaded?");
			return;
		}

		this->data.x = texture.getTextureID();
		this->data.y = id;

		rawrbox::__DECALS_DIRTY__ = true;
	}

	void Decal::setMatrix(const rawrbox::Matrix4x4& mtx) {
		this->worldToLocal = rawrbox::Matrix4x4::mtxInverse(mtx).transpose();
		rawrbox::__DECALS_DIRTY__ = true;
	}

	void Decal::setColor(const rawrbox::Color& cl) {
		this->color = cl;
		rawrbox::__DECALS_DIRTY__ = true;
	}
}; // namespace rawrbox
