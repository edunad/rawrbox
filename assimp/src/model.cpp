#include <rawrbox/assimp/model.hpp>
#include <rawrbox/assimp/utils/model.hpp>
#include <rawrbox/render/light/point.hpp>
#include <rawrbox/render/light/spot.hpp>

namespace rawrbox {

	// INTERNAL -----
	void AssimpModel::loadMeshes(const rawrbox::AssimpImporter& model) {
		for (size_t i = 0; i < model.meshes.size(); i++) {
			this->addMesh(rawrbox::AssimpUtils::extractMesh(model, i));
		}
	}

	void AssimpModel::loadAnimations(const rawrbox::AssimpImporter& model) {
		this->_animations = model.animations;
		this->_animatedMeshes.clear();

		// Mark animated meshes ---
		for (auto& anim : model.animatedMeshes) {
			auto fnd = std::find_if(this->_meshes.begin(), this->_meshes.end(), [anim](std::unique_ptr<rawrbox::Mesh>& msh) {
				return msh->getName() == anim.second->name;
			});
			if (fnd == this->_meshes.end()) continue;

			(*fnd)->setOptimizable(false);
			this->_animatedMeshes[anim.first] = (*fnd).get();
		}
		// -----------------------
	}

	void AssimpModel::loadBlendShapes(const rawrbox::AssimpImporter& model) {
		this->_blend_shapes.clear();

		for (auto& blend : model.blendShapes) {
			if (blend.second.mesh_index >= this->_meshes.size()) {
				fmt::print("[RawrBox-Assimp] Failed to find mesh {}", blend.second.mesh_index);
				continue;
			}

			auto s = std::make_unique<rawrbox::BlendShapes>();
			s->normals = blend.second.norms;
			s->pos = blend.second.pos;
			s->weight = blend.second.weight;

			s->mesh = this->_meshes[blend.second.mesh_index].get();
			s->mesh->setOptimizable(false);

			_blend_shapes[blend.first] = std::move(s);
		}
	}

	void AssimpModel::loadLights(const rawrbox::AssimpImporter& model) {
		for (auto& assimpLights : model.lights) {

			switch (assimpLights.type) {
				case rawrbox::LightType::POINT:
					this->template addLight<rawrbox::PointLight>(assimpLights.parentID, assimpLights.pos, assimpLights.diffuse, assimpLights.angleInnerCone);
					break;
				case rawrbox::LightType::SPOT:
					this->template addLight<rawrbox::SpotLight>(assimpLights.parentID, assimpLights.pos, assimpLights.direction, assimpLights.diffuse, assimpLights.angleInnerCone, assimpLights.angleOuterCone, 100.F);
					break;
				case rawrbox::LightType::DIR:
					rawrbox::LIGHTS::setSun(assimpLights.direction, assimpLights.diffuse);
					break;

				default:
				case rawrbox::LightType::UNKNOWN:
					fmt::print("[RawrBox-Assimp] Failed to create unknown light '{}'\n", assimpLights.name);
					break;
			}
		}
	}
	// ----------------

	void AssimpModel::load(const rawrbox::AssimpImporter& model) {
		this->loadMeshes(model);

		if ((this->_material->supports() & rawrbox::MaterialFlags::BONES) != 0) {
			this->loadAnimations(model);
			this->loadBlendShapes(model);
		}

		if ((this->_material->supports() & rawrbox::MaterialFlags::NORMALS) != 0) {
			this->loadLights(model);
		}
	}

} // namespace rawrbox
