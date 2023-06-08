
#include <rawrbox/render/gizmos.hpp>
#include <rawrbox/utils/string.hpp>

namespace rawrbox {

	std::unique_ptr<rawrbox::Sprite<>> GIZMOS::_gizmos = std::make_unique<rawrbox::Sprite<>>();
	std::unordered_map<std::string, std::unique_ptr<rawrbox::TextureImage>> GIZMOS::_textures = {};
	uint32_t GIZMOS::_ID = 0;

	// UTILS ----
	uint32_t GIZMOS::addGizmo(const std::string& type, const rawrbox::Vector3f& pos) {
		auto id = ++_ID;

		auto mesh = _gizmos->generatePlane({}, {0.25F, 0.25F});
		mesh.setPos(pos);
		mesh.setName(fmt::format("GIZMO-{}", ++_ID));

		auto upperType = rawrbox::StrUtils::toUpper(type);
		auto tex = _textures.find(upperType);
		if (tex == _textures.end()) {
			_textures[upperType] = std::make_unique<rawrbox::TextureImage>(fmt::format("./content/textures/gizmos/{}.png", upperType));
			_textures[upperType]->upload();
			mesh.setTexture(_textures[upperType].get());
		} else {
			mesh.setTexture((*tex).second.get());
		}

		_gizmos->addMesh(mesh);
		if (!_gizmos->isUploaded()) _gizmos->upload(true); // Dynamic

		return id;
	}

	void GIZMOS::removeGizmo(uint32_t id) {
		_gizmos->removeMeshByName(fmt::format("GIZMO-{}", id));
	}

	void GIZMOS::updateGizmo(uint32_t id, const rawrbox::Vector3f& pos) {
		_gizmos->getMeshByName(fmt::format("GIZMO-{}", id));
	}

	void GIZMOS::shutdown() {
		_gizmos.reset();
		_textures.clear();
	}

	void GIZMOS::draw() {
		if (!rawrbox::BGFX_INITIALIZED) return;
		if (_gizmos != nullptr && _gizmos->totalMeshes() > 0) _gizmos->draw({});
	}
	// ------

	/*std::unique_ptr<rawrbox::Sprite<>> GIZMOS::_gizmo_lights = std::make_unique<rawrbox::Sprite<>>();
	std::unique_ptr<rawrbox::Sprite<>> GIZMOS::_gizmo_emitters = std::make_unique<rawrbox::Sprite<>>();

	std::unordered_map<std::string, std::unique_ptr<rawrbox::TextureImage>> GIZMOS::_textures = {};

	void GIZMOS::shutdown() {
		_gizmo_lights.reset();
		_gizmo_emitters.reset();

		_textures.clear();
	}

	void GIZMOS::upload() {
		if (!rawrbox::BGFX_INITIALIZED) return;
		if (!_textures.empty()) throw std::runtime_error(fmt::format("[RawrBox-Debug] GIZMOS already initialized!"));

		// Lights
		_textures["light_point"] = std::make_unique<rawrbox::TextureImage>("./content/textures/gizmos/light_point.png");
		_textures["light_dir"] = std::make_unique<rawrbox::TextureImage>("./content/textures/debug/gizmo_lights/dir.png");
		_textures["light_spot"] = std::make_unique<rawrbox::TextureImage>("./content/textures/debug/gizmo_lights/spot.png");

		// Particle
		_textures["particle_emitter"] = std::make_unique<rawrbox::TextureImage>("./content/textures/debug/gizmo_emitter/emitter.png");

		for (auto& t : _textures) {
			t.second->upload();
		}

		_gizmo_lights->upload(true);   // Dynamic
		_gizmo_emitters->upload(true); // Dynamic
	}

	// UTILS ----
	void GIZMOS::addLight(rawrbox::LightBase* l) {
		if (l == nullptr || _gizmo_lights == nullptr) return;

		auto mesh = _gizmo_lights->generatePlane({}, {0.25F, 0.25F});
		auto p = l->getPosMatrix();
		mesh.setPos({p[0], p[1], p[2]});
		mesh.setName(fmt::format("Light-{}", l->id()));

		switch (l->getType()) {
			case LightType::LIGHT_SPOT:
				mesh.setTexture(_textures["light_spot"].get());
				break;
			case LightType::LIGHT_DIR:
				mesh.setTexture(_textures["light_dir"].get());
				break;
			default:
			case LightType::LIGHT_POINT:
				mesh.setTexture(_textures["light_point"].get());
				break;
		}

		_gizmo_lights->addMesh(mesh);
	}

	void GIZMOS::removeLight(rawrbox::LightBase* l) {
		if (l == nullptr || _gizmo_lights == nullptr) return;

		auto& m = _gizmo_lights->meshes();
		for (size_t i = 0; i < m.size(); i++) {
			if (m[i]->getName() != fmt::format("Light-{}", l->id())) continue;
			_gizmo_lights->removeMesh(i);
			return;
		}
	}

	void GIZMOS::addEmitter(rawrbox::Emitter* l) {
		if (l == nullptr || _gizmo_emitters == nullptr) return;

		auto mesh = _gizmo_emitters->generatePlane({}, {0.25F, 0.25F});
		mesh.setPos(l->getPos());
		mesh.setName(fmt::format("Emitter-{}", l->id()));
		mesh.setTexture(_textures["particle_emitter"].get());

		_gizmo_emitters->addMesh(mesh);
	}

	void GIZMOS::removeEmitter(rawrbox::Emitter* l) {
		if (l == nullptr || _gizmo_emitters == nullptr) return;

		auto& m = _gizmo_emitters->meshes();
		for (size_t i = 0; i < m.size(); i++) {
			if (m[i]->getName() != fmt::format("Emitter-{}", l->id())) continue;
			_gizmo_emitters->removeMesh(i);
			return;
		}
	}

#if RAWRBOX_BASS
	void GIZMOS::addSound(rawrbox::SoundInstance* l) {
		if (l == nullptr || _gizmo_sounds == nullptr || !l->isValid() || !l->is3D()) return;

		auto mesh = _gizmo_sounds->generatePlane({}, {0.25F, 0.25F});
		mesh.setPos(l->getPosition());
		mesh.setName(fmt::format("Sound-{}", l->id()));
		mesh.setTexture(_textures["sound_emitter"].get());

		_gizmo_sounds->addMesh(mesh);
	}

	void GIZMOS::removeSound(rawrbox::SoundInstance* l) {
		if (l == nullptr || _gizmo_sounds == nullptr) return;

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
	}*/
} // namespace rawrbox
