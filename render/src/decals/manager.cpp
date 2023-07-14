
#include <rawrbox/render/decals/manager.hpp>
#include <rawrbox/render/model/utils/mesh.hpp>

#include <fmt/format.h>

namespace rawrbox {
	// PRIVATE ----
	std::unique_ptr<rawrbox::Model<rawrbox::MaterialDecal>> DECALS::_model = std::make_unique<rawrbox::Model<rawrbox::MaterialDecal>>();
	std::vector<rawrbox::Decal> DECALS::_decals = {};
	// -----

	// PUBLIC ----
	void DECALS::addInstance(rawrbox::TextureBase* atlas, const rawrbox::Vector3f& pos, const rawrbox::Vector3f& ang, uint16_t atlasId) {
		if (_model == nullptr) return;

		auto tmp = rawrbox::MeshUtils::generateCube(pos, {1.0F, 1.0F, 1.0F});
		tmp.setEulerAngle(ang);
		tmp.setTexture(atlas);

		_model->addMesh(tmp);
	}

	void DECALS::upload() {
		if (_model == nullptr) return;
		_model->upload(true);
	}

	void DECALS::draw() {
		if (_model == nullptr) return;
		_model->draw();
	}
	// ---------

} // namespace rawrbox
