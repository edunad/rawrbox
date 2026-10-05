#include <rawrbox/render/decals/decal.hpp>
#include <rawrbox/render/static.hpp>

#include <algorithm>

namespace rawrbox {
	// PRIVATE ---
	std::unique_ptr<rawrbox::Logger> Decal::_logger = std::make_unique<rawrbox::Logger>("RawrBox-DECAL");

	void Decal::updateBounds(const rawrbox::Matrix4x4& localToWorld) {
		const rawrbox::Vector3f center = localToWorld.mulVec(rawrbox::Vector3f{0.F, 0.F, 0.F});

		float radius = 0.F;
		for (int i = 0; i < 8; i++) {
			const rawrbox::Vector3f corner = {(i & 1) != 0 ? 1.F : -1.F, (i & 2) != 0 ? 1.F : -1.F, (i & 4) != 0 ? 1.F : -1.F};
			radius = std::max(radius, (localToWorld.mulVec(corner) - center).length());
		}

		this->bounds = {center.x, center.y, center.z, radius};
	}
	// ------

	Decal::Decal(const rawrbox::Matrix4x4& _mtx, const rawrbox::TextureBase& _texture, const rawrbox::Colorf& _color, uint32_t _atlas) : worldToLocal(_mtx), color(_color) {
		auto localToWorld = this->worldToLocal; // Stored transposed, same as setMatrix
		this->updateBounds(rawrbox::Matrix4x4::mtxInverse(localToWorld.transpose()));

		this->setTexture(_texture, _atlas);
	}

	void Decal::setTexture(const rawrbox::TextureBase& texture, uint32_t id) {
		if (!texture.isValid()) {
			this->_logger->error("Invalid texture, not uploaded?");
			return;
		}

		this->data.x = texture.getTextureID();
		this->data.y = id;

		rawrbox::__DECALS_DIRTY__ = true;
	}

	void Decal::setMatrix(const rawrbox::Matrix4x4& mtx) {
		this->worldToLocal = rawrbox::Matrix4x4::mtxInverse(mtx).transpose();
		this->updateBounds(mtx);

		rawrbox::__DECALS_DIRTY__ = true;
	}

	void Decal::setColor(const rawrbox::Color& cl) {
		this->color = cl;
		rawrbox::__DECALS_DIRTY__ = true;
	}
}; // namespace rawrbox
