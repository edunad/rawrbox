#pragma once

#include <rawrbox/assimp/importer.hpp>
#include <rawrbox/assimp/utils/model.hpp>
#include <rawrbox/render/light/point.hpp>
#include <rawrbox/render/light/spot.hpp>
#include <rawrbox/render/models/model.hpp>

namespace rawrbox {

	template <typename M = MaterialUnlit>
	class AssimpModel : public rawrbox::Model<M> {
	protected:
		// INTERNAL -------
		void loadMeshes(const rawrbox::AssimpImporter& model) {
			for (size_t i = 0; i < model.meshes.size(); i++) {
				this->addMesh(rawrbox::AssimpUtils::extractMesh<M>(model, i));
			}
		}

		void loadAnimations(const rawrbox::AssimpImporter& model) {
			this->_animations = model.animations;
			this->_animatedMeshes.clear();

			// Mark animated meshes ---
			for (auto& anim : model.animatedMeshes) {
				auto fnd = std::find_if(this->_meshes.begin(), this->_meshes.end(), [anim](std::unique_ptr<rawrbox::Mesh<typename M::vertexBufferType>>& msh) {
					return msh->getName() == anim.second->name;
				});
				if (fnd == this->_meshes.end()) continue;

				(*fnd)->setOptimizable(false);
				this->_animatedMeshes[anim.first] = (*fnd).get();
			}
			// -----------------------
		}

		void loadBlendShapes(const rawrbox::AssimpImporter& model) {
			this->_blend_shapes.clear();

			for (auto& blend : model.blendShapes) {
				if (blend.second.mesh_index >= this->_meshes.size()) {
					fmt::print("[RawrBox-Assimp] Failed to find mesh {}", blend.second.mesh_index);
					continue;
				}

				auto s = std::make_unique<rawrbox::BlendShapes<M>>();
				s->normals = blend.second.norms;
				s->pos = blend.second.pos;
				s->weight = blend.second.weight;

				s->mesh = this->_meshes[blend.second.mesh_index].get();
				s->mesh->setOptimizable(false);

				this->_blend_shapes[blend.first] = std::move(s);
			}
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
		// -------------------

	public:
		AssimpModel() = default;
		AssimpModel(const AssimpModel&) = delete;
		AssimpModel(AssimpModel&&) = delete;
		AssimpModel& operator=(const AssimpModel&) = delete;
		AssimpModel& operator=(AssimpModel&&) = delete;
		~AssimpModel() override = default;

		void load(const rawrbox::AssimpImporter& model) {
			this->loadMeshes(model);
			this->loadBlendShapes(model);

			if constexpr (supportsBones<typename M::vertexBufferType>) {
				this->loadAnimations(model);
			}

			if constexpr (supportsNormals<typename M::vertexBufferType>) {
				this->loadLights(model);
			}
		}
	};
} // namespace rawrbox
