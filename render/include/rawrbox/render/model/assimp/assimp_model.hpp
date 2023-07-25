#pragma once

#include <rawrbox/render/light/point.hpp>
#include <rawrbox/render/light/spot.hpp>
#include <rawrbox/render/model/assimp/assimp_importer.hpp>
#include <rawrbox/render/model/model.hpp>
#include <rawrbox/render/utils/assimp/model.hpp>

namespace rawrbox {
	template <typename M = rawrbox::MaterialBase>
	class AssimpModel : public rawrbox::Model<M> {
		void loadMeshes(const rawrbox::AssimpImporter& model) {
			for (size_t i = 0; i < model.meshes.size(); i++) {
				this->addMesh(rawrbox::AssimpUtils::extractMesh(model, i));
			}
		}

		void loadAnimations(const rawrbox::AssimpImporter& model) {
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

		void loadLights(const rawrbox::AssimpImporter& model) {
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

	public:
		AssimpModel() = default;
		AssimpModel(const AssimpModel&) = delete;
		AssimpModel(AssimpModel&&) = delete;
		AssimpModel& operator=(const AssimpModel&) = delete;
		AssimpModel& operator=(AssimpModel&&) = delete;
		~AssimpModel() override = default;

		void load(const rawrbox::AssimpImporter& model) {
			this->loadMeshes(model);

			if constexpr (supportsBones<M>) {
				this->loadAnimations(model);
			}

			if constexpr (supportsNormals<M>) {
				this->loadLights(model);
			}

			this->upload();
		}
	};
} // namespace rawrbox
