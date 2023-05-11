
#include <rawrbox/debug/gizmos.hpp>
#include <rawrbox/render/model/light/base.hpp>
#include <rawrbox/render/particles/emitter.hpp>

#ifdef RAWRBOX_BASS
	#include <rawrbox/bass/sound/instance.hpp>
#endif

#include <memory>
#include <stdexcept>

namespace rawrbox {

	void GIZMOS::shutdown() {
		this->_gizmo_lights = nullptr;
		this->_gizmo_sounds = nullptr;
		this->_gizmo_emitters = nullptr;

		this->_textures.clear();
	}

	void GIZMOS::upload() {
		if (!rawrbox::BGFX_INITIALIZED) return;
		if (!this->_textures.empty()) throw std::runtime_error(fmt::format("[RawrBox-Debug] GIZMOS already initialized!"));

		// Lights
		this->_textures["light_point"] = std::make_shared<rawrbox::TextureImage>("./content/textures/debug/gizmo_lights/point.png");
		this->_textures["light_dir"] = std::make_shared<rawrbox::TextureImage>("./content/textures/debug/gizmo_lights/dir.png");
		this->_textures["light_spot"] = std::make_shared<rawrbox::TextureImage>("./content/textures/debug/gizmo_lights/spot.png");

		// Sound
		this->_textures["sound_emitter"] = std::make_shared<rawrbox::TextureImage>("./content/textures/debug/gizmo_sounds/emitter.png");

		// Particle
		this->_textures["particle_emitter"] = std::make_shared<rawrbox::TextureImage>("./content/textures/debug/gizmo_emitter/emitter.png");

		for (auto& t : this->_textures) {
			t.second->upload();
		}

		this->_gizmo_lights->upload(true);   // Dynamic
		this->_gizmo_sounds->upload(true);   // Dynamic
		this->_gizmo_emitters->upload(true); // Dynamic
	}

	// UTILS ----
	void GIZMOS::addLight(rawrbox::LightBase* l) {
		auto pos = l->getPosMatrix();

		std::shared_ptr<rawrbox::Mesh<typename MaterialBase::vertexBufferType>> mesh = this->_gizmo_lights->generatePlane({pos[0], pos[1], pos[2]}, {0.25F, 0.25F});
		mesh->setName(fmt::format("Light-{}", l->id()));
		mesh->setCulling(BGFX_STATE_CULL_CW);

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

		this->_gizmo_lights->addMesh(mesh);
	}

	void GIZMOS::removeLight(rawrbox::LightBase* l) {
		if (this->_gizmo_lights == nullptr) return;

		auto& m = this->_gizmo_lights->meshes();
		for (size_t i = 0; i < m.size(); i++) {
			if (m[i]->getName() != fmt::format("Light-{}", l->id())) continue;
			this->_gizmo_lights->removeMesh(i);
			return;
		}
	}

	void GIZMOS::addEmitter(rawrbox::Emitter* l) {
		std::shared_ptr<rawrbox::Mesh<typename MaterialBase::vertexBufferType>> mesh = this->_gizmo_emitters->generatePlane(l->getPos(), {0.25F, 0.25F});
		mesh->setName(fmt::format("Emitter-{}", l->id()));
		mesh->setCulling(BGFX_STATE_CULL_CW);
		mesh->setTexture(_textures["particle_emitter"]);

		this->_gizmo_emitters->addMesh(mesh);
	}

	void GIZMOS::removeEmitter(rawrbox::Emitter* l) {
		if (this->_gizmo_emitters == nullptr) return;

		auto& m = this->_gizmo_emitters->meshes();
		for (size_t i = 0; i < m.size(); i++) {
			if (m[i]->getName() != fmt::format("Emitter-{}", l->id())) continue;
			this->_gizmo_emitters->removeMesh(i);
			return;
		}
	}

#ifdef RAWRBOX_BASS
	void GIZMOS::addSound(rawrbox::SoundInstance* l) {
		if (!l->isValid() || !l->is3D()) return;

		std::shared_ptr<rawrbox::Mesh<typename MaterialBase::vertexBufferType>> mesh = this->_gizmo_sounds->generatePlane(l->getPosition(), {0.25F, 0.25F});
		mesh->setName(fmt::format("Sound-{}", l->id()));
		mesh->setCulling(BGFX_STATE_CULL_CW);
		mesh->setTexture(_textures["sound_emitter"]);

		this->_gizmo_sounds->addMesh(mesh);
	}

	void GIZMOS::removeSound(rawrbox::SoundInstance* l) {
		if (this->_gizmo_sounds == nullptr) return;

		auto& m = this->_gizmo_sounds->meshes();
		for (size_t i = 0; i < m.size(); i++) {
			if (m[i]->getName() != fmt::format("Sound-{}", l->id())) continue;
			this->_gizmo_sounds->removeMesh(i);
			return;
		}
	}
#endif
	// -----

	void GIZMOS::updateGizmo(const std::string& id, const rawrbox::Vector3f& pos) {
		for (auto& m : this->_gizmo_lights->meshes()) {
			if (m->getName() != id) continue;
			m->setPos(pos);
			return;
		}

		for (auto& m : this->_gizmo_sounds->meshes()) {
			if (m->getName() != id) continue;
			m->setPos(pos);
			return;
		}

		for (auto& m : this->_gizmo_emitters->meshes()) {
			if (m->getName() != id) continue;
			m->setPos(pos);
			return;
		}
	}

	void GIZMOS::draw() {
		if (!rawrbox::BGFX_INITIALIZED) return;

		if (this->_gizmo_lights != nullptr && this->_gizmo_lights->totalMeshes() > 0) this->_gizmo_lights->draw({});
		if (this->_gizmo_sounds != nullptr && this->_gizmo_sounds->totalMeshes() > 0) this->_gizmo_sounds->draw({});
		if (this->_gizmo_emitters != nullptr && this->_gizmo_emitters->totalMeshes() > 0) this->_gizmo_emitters->draw({});
	}
} // namespace rawrbox
