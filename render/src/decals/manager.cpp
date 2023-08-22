
#include <rawrbox/render/decals/manager.hpp>
#include <rawrbox/render/materials/decal.hpp>
#include <rawrbox/render/model/utils/mesh.hpp>

#include <fmt/format.h>

namespace rawrbox {
	// PRIVATE ----
	std::unique_ptr<rawrbox::InstancedModel> DECALS::_model = nullptr;
	// -----

	// PUBLIC ----
	void DECALS::setAtlasTexture(rawrbox::TextureBase* atlas) {
		if (_model == nullptr) return;
		_model->getTemplate().setTexture(atlas);
	}

	bool DECALS::add(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& scale, float direction, const rawrbox::Colorf& color, uint16_t atlasId) {
		if (_model == nullptr) return false;

		rawrbox::Matrix4x4 m;
		m.mtxSRT(scale, rawrbox::Vector4f::toQuat({bx::toRad(direction), 0, 0}), pos);

		_model->addInstance({m, color, {static_cast<float>(atlasId), 0, 0, 0}});
		return true;
	}

	bool DECALS::remove(size_t i) {
		if (_model == nullptr) return false;
		_model->removeInstance(i);
		return true;
	}

	const rawrbox::Instance& DECALS::get(size_t i) {
		if (_model == nullptr) throw std::runtime_error("[RawrBox-DECALS] Model template not initialized! Did you call 'init'");
		return _model->getInstance(i);
	}

	const rawrbox::Mesh& DECALS::getTemplate() { return _model->getTemplate(); }
	size_t DECALS::count() { return _model->count(); }

	void DECALS::init() {
		if (_model != nullptr) throw std::runtime_error("[RawrBox-DECALS] Already initialized");

		_model = std::make_unique<rawrbox::InstancedModel>();
		_model->setMaterial<rawrbox::MaterialDecal>();

		auto mdlTemp = rawrbox::MeshUtils::generateCube({0, 0, 0}, {1.0F, 1.0F, 0.25F});
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
