
#include <rawrbox/render/decals/manager.hpp>
#include <rawrbox/render/model/utils/mesh.hpp>

#include <fmt/format.h>

namespace rawrbox {
	// PRIVATE ----
	std::unique_ptr<rawrbox::InstancedModel<rawrbox::MaterialDecal>> DECALS::_model = nullptr;
	std::vector<rawrbox::Decal> DECALS::_decals = {};
	// -----

	// PUBLIC ----
	void DECALS::setAtlasTexture(rawrbox::TextureBase* atlas) {
		if (_model == nullptr) return;
		_model->getTemplate().setTexture(atlas);
	}

	void DECALS::addInstance(const rawrbox::Vector3f& pos, float direction, const rawrbox::Colorf& color, uint16_t atlasId) {
		if (_model == nullptr) return;

		rawrbox::Matrix4x4 m;
		m.mtxSRT({1.F, 1.F, 1.F}, rawrbox::Vector4f::toQuat({bx::toRad(direction), 0, 0}), pos);
		_model->addInstance({m, color, {static_cast<float>(atlasId), 0, 0, 0}});
	}

	void DECALS::init() {
		if (_model != nullptr) return;
		_model = std::make_unique<rawrbox::InstancedModel<rawrbox::MaterialDecal>>();

		auto mdlTemp = rawrbox::MeshUtils::generateCube({0, 0, 0}, {1.0F, 1.0F, 0.10F});
		mdlTemp.setBlend(BGFX_STATE_BLEND_ALPHA);
		mdlTemp.setDepthTest(0);

		_model->setTemplate(mdlTemp);
		_model->upload();
	}

	void DECALS::shutdown() {
		if (_model == nullptr) return;
		_model.reset();
	}

	void DECALS::draw() {
		if (_model == nullptr) return;
		_model->draw();
	}
	// ---------

} // namespace rawrbox
