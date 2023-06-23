
#include <rawrbox/render/decals/manager.hpp>
#include <rawrbox/render/model/utils/mesh.hpp>

#include <fmt/format.h>

namespace rawrbox {
	// PRIVATE ----
	std::unique_ptr<rawrbox::InstancedModel<>> DECALS::_model = std::make_unique<rawrbox::InstancedModel<>>();
	std::vector<rawrbox::Decal> DECALS::_decals = {};
	// -----

	// PUBLIC ----
	void DECALS::setTexture(rawrbox::TextureBase* atlas) {
		if (_model == nullptr) return;

		auto tmp = rawrbox::MeshUtils::generateCube({0, 0, 0}, {1.0F, 1.0F, 1.0F});
		tmp.setTexture(atlas);

		_model->setTemplate(tmp);
	}

	void DECALS::addInstance(const rawrbox::Vector3f& pos, uint16_t atlasId) {
		if (_model == nullptr) return;

		rawrbox::Matrix4x4 m;
		m.mtxSRT({1.F, 1.F, 1.F}, rawrbox::Vector4f::toQuat({bx::toRad(90), 0, 0}), pos);
		_model->addInstance({m, rawrbox::Colors::White, {static_cast<float>(atlasId), 0, 0, 0}});
	}

	void DECALS::upload() {
		if (_model == nullptr) return;
		_model->upload();
	}

	void DECALS::draw() {
		if (_model == nullptr) return;
		_model->draw();
	}
	// ---------

} // namespace rawrbox
