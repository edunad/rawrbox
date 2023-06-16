#pragma once

#include <rawrbox/render/model/assimp/assimp_importer.hpp>
#include <rawrbox/render/model/light/directional.hpp>
#include <rawrbox/render/model/light/point.hpp>
#include <rawrbox/render/model/light/spot.hpp>
#include <rawrbox/render/model/model.hpp>

namespace rawrbox {
	template <typename M = rawrbox::MaterialBase>
	class AssimpModel : public rawrbox::Model<M> {

		void loadMeshes(const rawrbox::AssimpImporter& model) {
			for (auto& assimpMesh : model.meshes) {
				rawrbox::Mesh<typename M::vertexBufferType> mesh;

				mesh.name = assimpMesh.name;
				mesh.bbox = assimpMesh.bbox;
				mesh.matrix = assimpMesh.matrix;

				// Textures ---
				if (assimpMesh.material != nullptr) {
					auto mat = assimpMesh.material;
					mesh.setWireframe(mat->wireframe);
					mesh.setBlend(mat->blending);
					mesh.setCulling(mat->doubleSided ? 0 : BGFX_STATE_CULL_CCW);

					// DIFFUSE -----
					if (mat->diffuse.has_value()) {
						mesh.setTexture(mat->diffuse.value() == nullptr ? rawrbox::MISSING_TEXTURE.get() : mat->diffuse.value().get());
					} else {
						mesh.setTexture(rawrbox::WHITE_TEXTURE.get()); // Default
					}

					mesh.setColor(mat->diffuseColor);
					// --------

					if constexpr (supportsNormals<typename M::vertexBufferType>) {
						// SPECULAR -----
						if (mat->specular != nullptr) {
							mesh.setSpecularTexture(mat->specular.get(), mat->shininess);
						}

						mesh.setSpecularColor(mat->specularColor);
						// --------

						// EMISSION -----
						if (mat->emissive != nullptr) {
							mesh.setEmissionTexture(mat->emissive.get(), mat->intensity);
						}

						mesh.setEmissionColor(mat->emissionColor);
						// --------

						// OPACITY -----
						if (mat->opacity != nullptr) {
							mesh.setOpacityTexture(mat->opacity.get());
						}
						// --------
					}
					// --------
				}
				// ------------

				mesh.indices = assimpMesh.indices;
				for (auto& vert : assimpMesh.vertices) {
					typename M::vertexBufferType m;
					m.position = vert.position;
					m.uv = vert.uv;
					m.abgr = vert.abgr;

					if constexpr (supportsNormals<typename M::vertexBufferType>) {
						m.normal = vert.normal;
					}

					if constexpr (supportsBones<typename M::vertexBufferType>) {
						m.index = vert.index;
						m.bone_indices = vert.bone_indices;
						m.bone_weights = vert.bone_weights;
					}

					mesh.vertices.push_back(m);
				}

				// Bones
				if constexpr (supportsBones<typename M::vertexBufferType>) {
					if (assimpMesh.skeleton != nullptr) {
						mesh.skeleton = assimpMesh.skeleton;
						mesh.setOptimizable(false);
					}
				}
				// -------------------

				mesh.baseVertex = 0;
				mesh.baseIndex = 0;
				mesh.totalVertex = static_cast<uint16_t>(mesh.vertices.size());
				mesh.totalIndex = static_cast<uint16_t>(mesh.indices.size());

				this->addMesh(mesh);
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

		void loadLights(const rawrbox::AssimpImporter& model) {
			for (auto& assimpLights : model.lights) {
				switch (assimpLights.type) {
					case LightType::LIGHT_POINT:
						this->template addLight<rawrbox::LightPoint>({assimpLights.pos, assimpLights.diffuse, assimpLights.specular, assimpLights.attenuationConstant, assimpLights.attenuationLinear, assimpLights.attenuationQuadratic}, assimpLights.parentID);
						break;
					case LightType::LIGHT_SPOT:
						this->template addLight<rawrbox::LightSpot>({assimpLights.pos, assimpLights.direction, assimpLights.diffuse, assimpLights.specular, assimpLights.angleInnerCone, assimpLights.angleOuterCone, assimpLights.attenuationConstant, assimpLights.attenuationLinear, assimpLights.attenuationQuadratic}, assimpLights.parentID);
						break;
					case LightType::LIGHT_DIR:
						this->template addLight<rawrbox::LightDirectional>({assimpLights.pos, assimpLights.direction, assimpLights.diffuse, assimpLights.specular}, assimpLights.parentID);
						break;

					default:
					case LightType::LIGHT_UNKNOWN:
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

			if constexpr (supportsBones<typename M::vertexBufferType>) {
				this->loadAnimations(model);
			}

			if constexpr (supportsNormals<typename M::vertexBufferType>) {
				this->loadLights(model);
			}

			this->upload();
		}
	};
} // namespace rawrbox
