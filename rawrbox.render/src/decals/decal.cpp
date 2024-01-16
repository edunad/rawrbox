#include <rawrbox/render/decals/decal.hpp>
#include <rawrbox/render/static.hpp>

namespace rawrbox {
	Decal::Decal(const rawrbox::Matrix4x4& _mtx, const rawrbox::TextureBase& _texture, const rawrbox::Colorf& _color, uint32_t _atlas) : localToWorld(_mtx), color(_color), textureAtlasIndex(_atlas) {
		this->setTexture(_texture);
	}

	void Decal::setTexture(const rawrbox::TextureBase& texture, uint32_t id) {
		if (!texture.isValid()) throw rawrbox::Logger::err("RawrBox-DECAL", "Invalid texture, not uploaded?");
		this->textureID = texture.getTextureID();
		this->textureAtlasIndex = id;

		rawrbox::__DECALS_DIRTY__ = true;
	}

	void Decal::setScale(const rawrbox::Vector3f& pos) {
		this->localToWorld.scale(pos);
		rawrbox::__DECALS_DIRTY__ = true;
	}

	void Decal::setPos(const rawrbox::Vector3f& pos) {
		this->localToWorld.translate(pos);
		rawrbox::__DECALS_DIRTY__ = true;
	}

	void Decal::setColor(const rawrbox::Color& cl) {
		this->color = cl;
		rawrbox::__DECALS_DIRTY__ = true;
	}
}; // namespace rawrbox
