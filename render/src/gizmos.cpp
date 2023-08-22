
#include <rawrbox/render/gizmos.hpp>
#include <rawrbox/render/model/utils/mesh.hpp>
#include <rawrbox/utils/string.hpp>

namespace rawrbox {

	std::unique_ptr<rawrbox::Sprite> GIZMOS::_gizmos = std::make_unique<rawrbox::Sprite>();
	std::unordered_map<std::string, std::unique_ptr<rawrbox::TextureImage>> GIZMOS::_textures = {};
	uint32_t GIZMOS::_ID = 0;

	// UTILS ----
	uint32_t GIZMOS::addGizmo(const std::string& type, const rawrbox::Vector3f& pos) {
		auto id = ++_ID;

		auto mesh = rawrbox::MeshUtils::generatePlane({}, {0.25F, 0.25F});
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

	void GIZMOS::updateGizmo(uint32_t id, const rawrbox::Vector3f& /*pos*/) {
		_gizmos->getMeshByName(fmt::format("GIZMO-{}", id));
	}

	void GIZMOS::shutdown() {
		_gizmos.reset();
		_textures.clear();
	}

	void GIZMOS::draw() {
		if (!rawrbox::BGFX_INITIALIZED) return;
		if (_gizmos != nullptr && _gizmos->totalMeshes() > 0) _gizmos->draw();
	}
	// ------
} // namespace rawrbox
