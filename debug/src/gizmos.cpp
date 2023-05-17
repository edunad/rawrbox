
#include <rawrbox/debug/gizmos.hpp>
#include <rawrbox/render/model/light/base.hpp>
#include <rawrbox/render/particles/emitter.hpp>

#ifdef RAWRBOX_BASS
	#include <rawrbox/bass/sound/instance.hpp>
#endif

#include <memory>
#include <stdexcept>

namespace rawrbox {
	std::shared_ptr<rawrbox::Sprite<>> GIZMOS::_gizmo_lights = std::make_shared<rawrbox::Sprite<>>();
	std::shared_ptr<rawrbox::Sprite<>> GIZMOS::_gizmo_sounds = std::make_shared<rawrbox::Sprite<>>();
	std::shared_ptr<rawrbox::Sprite<>> GIZMOS::_gizmo_emitters = std::make_shared<rawrbox::Sprite<>>();

	std::unordered_map<std::string, std::shared_ptr<rawrbox::TextureImage>> GIZMOS::_textures = {};

	void GIZMOS::shutdown() {
		_gizmo_lights = nullptr;
		_gizmo_sounds = nullptr;
		_gizmo_emitters = nullptr;

		_textures.clear();
	}

	void GIZMOS::upload() {
		if (!rawrbox::BGFX_INITIALIZED) return;
		if (!_textures.empty()) throw std::runtime_error(fmt::format("[RawrBox-Debug] GIZMOS already initialized!"));

		// Lights
		_textures["light_point"] = std::make_shared<rawrbox::TextureImage>("./content/textures/debug/gizmo_lights/point.png");
		_textures["light_dir"] = std::make_shared<rawrbox::TextureImage>("./content/textures/debug/gizmo_lights/dir.png");
		_textures["light_spot"] = std::make_shared<rawrbox::TextureImage>("./content/textures/debug/gizmo_lights/spot.png");

		// Sound
		_textures["sound_emitter"] = std::make_shared<rawrbox::TextureImage>("./content/textures/debug/gizmo_sounds/emitter.png");

		// Particle
		_textures["particle_emitter"] = std::make_shared<rawrbox::TextureImage>("./content/textures/debug/gizmo_emitter/emitter.png");

		for (auto& t : _textures) {
			t.second->upload();
		}

		_gizmo_lights->upload(true);   // Dynamic
		_gizmo_sounds->upload(true);   // Dynamic
		_gizmo_emitters->upload(true); // Dynamic
	}

	// UTILS ----
	void GIZMOS::addLight(rawrbox::LightBase* l) {
		std::shared_ptr<rawrbox::Mesh<typename MaterialBase::vertexBufferType>> mesh = _gizmo_lights->generatePlane({}, {0.25F, 0.25F});
		auto p = l->getPosMatrix();
		mesh->setPos({p[0], p[1], p[2]});
		mesh->setName(fmt::format("Light-{}", l->id()));
		mesh->setCulling(BGFX_STATE_CULL_CW);
		mesh->setBlend(BGFX_STATE_BLEND_NORMAL);

		switch (l->getType()) {
			case LightType::LIGHT_SPOT:
				mesh->setTexture(_textures["light_spot"]);
				break;
			case LightType::LIGHT_DIR:
				mesh->setTexture(_textures["light_dir"]);
				break;
			default:
			case LightType::LIGHT_POINT:
				mesh->setTexture(_textures["light_point"]);
				break;
		}

		_gizmo_lights->addMesh(mesh);
	}

	void GIZMOS::removeLight(rawrbox::LightBase* l) {
		if (_gizmo_lights == nullptr) return;

		auto& m = _gizmo_lights->meshes();
		for (size_t i = 0; i < m.size(); i++) {
			if (m[i]->getName() != fmt::format("Light-{}", l->id())) continue;
			_gizmo_lights->removeMesh(i);
			return;
		}
	}

	void GIZMOS::addEmitter(rawrbox::Emitter* l) {
		std::shared_ptr<rawrbox::Mesh<typename MaterialBase::vertexBufferType>> mesh = _gizmo_emitters->generatePlane({}, {0.25F, 0.25F});
		mesh->setPos(l->getPos());
		mesh->setName(fmt::format("Emitter-{}", l->id()));
		mesh->setCulling(BGFX_STATE_CULL_CW);
		mesh->setTexture(_textures["particle_emitter"]);
		mesh->setBlend(BGFX_STATE_BLEND_NORMAL);

		_gizmo_emitters->addMesh(mesh);
	}

	void GIZMOS::removeEmitter(rawrbox::Emitter* l) {
		if (_gizmo_emitters == nullptr) return;

		auto& m = _gizmo_emitters->meshes();
		for (size_t i = 0; i < m.size(); i++) {
			if (m[i]->getName() != fmt::format("Emitter-{}", l->id())) continue;
			_gizmo_emitters->removeMesh(i);
			return;
		}
	}

#ifdef RAWRBOX_BASS
	void GIZMOS::addSound(rawrbox::SoundInstance* l) {
		if (!l->isValid() || !l->is3D()) return;

		std::shared_ptr<rawrbox::Mesh<typename MaterialBase::vertexBufferType>> mesh = _gizmo_sounds->generatePlane({}, {0.25F, 0.25F});
		mesh->setPos(l->getPosition());
		mesh->setName(fmt::format("Sound-{}", l->id()));
		mesh->setCulling(BGFX_STATE_CULL_CW);
		mesh->setTexture(_textures["sound_emitter"]);
		mesh->setBlend(BGFX_STATE_BLEND_NORMAL);

		_gizmo_sounds->addMesh(mesh);
	}

	void GIZMOS::removeSound(rawrbox::SoundInstance* l) {
		if (_gizmo_sounds == nullptr) return;

		auto& m = _gizmo_sounds->meshes();
		for (size_t i = 0; i < m.size(); i++) {
			if (m[i]->getName() != fmt::format("Sound-{}", l->id())) continue;
			_gizmo_sounds->removeMesh(i);
			return;
		}
	}
#endif
	// -----

	void GIZMOS::updateGizmo(const std::string& id, const rawrbox::Vector3f& pos) {
		for (auto& m : _gizmo_lights->meshes()) {
			if (m->getName() != id) continue;
			m->setPos(pos);
			return;
		}

		for (auto& m : _gizmo_sounds->meshes()) {
			if (m->getName() != id) continue;
			m->setPos(pos);
			return;
		}

		for (auto& m : _gizmo_emitters->meshes()) {
			if (m->getName() != id) continue;
			m->setPos(pos);
			return;
		}
	}

	void GIZMOS::draw() {
		if (!rawrbox::BGFX_INITIALIZED) return;

		if (_gizmo_lights != nullptr && _gizmo_lights->totalMeshes() > 0) _gizmo_lights->draw({});
		if (_gizmo_sounds != nullptr && _gizmo_sounds->totalMeshes() > 0) _gizmo_sounds->draw({});
		if (_gizmo_emitters != nullptr && _gizmo_emitters->totalMeshes() > 0) _gizmo_emitters->draw({});
	}
} // namespace rawrbox
