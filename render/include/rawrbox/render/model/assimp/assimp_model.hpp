#pragma once

#include <rawrbox/render/model/assimp/assimp_importer.hpp>
#include <rawrbox/render/model/light/directional.hpp>
#include <rawrbox/render/model/light/point.hpp>
#include <rawrbox/render/model/light/spot.hpp>
#include <rawrbox/render/model/model.hpp>

namespace rawrbox {
	template <typename M = rawrbox::MaterialBase>
	class AssimpModel : public rawrbox::Model<M> {

		virtual void loadMeshes(std::shared_ptr<rawrbox::AssimpImporter> model) {
			for (auto assimpMesh : model->meshes) {
				auto mesh = std::make_shared<rawrbox::Mesh<typename M::vertexBufferType>>();

				mesh->name = assimpMesh.name;
				mesh->bbox = assimpMesh.bbox;
				mesh->offsetMatrix = assimpMesh.offsetMatrix;

				// Textures ---
				if (!assimpMesh.material.expired()) {
					auto mat = assimpMesh.material.lock();

					mesh->setTexture(rawrbox::WHITE_TEXTURE);               // Default
					mesh->setSpecularTexture(rawrbox::BLACK_TEXTURE, 25.F); // Default
					mesh->setEmissionTexture(rawrbox::BLACK_TEXTURE, 1.F);  // Default
					mesh->setOpacityTexture(rawrbox::WHITE_TEXTURE);        // Default

					mesh->setWireframe(mat->wireframe);
					mesh->setBlend(mat->blending);
					mesh->setCulling(mat->doubleSided ? 0 : BGFX_STATE_CULL_CCW);

					// DIFFUSE -----
					if (mat->diffuse != nullptr) {
						mesh->setTexture(mat->diffuse);
					}

					mesh->setColor(mat->diffuseColor);
					// --------

					if constexpr (supportsNormals<typename M::vertexBufferType>) {
						// SPECULAR -----
						if (mat->specular != nullptr) {
							mesh->setSpecularTexture(mat->specular, mat->shininess);
						}

						mesh->setSpecularColor(mat->specularColor);
						// --------

						// EMISSION -----
						if (mat->emissive != nullptr) {
							mesh->setEmissionTexture(mat->emissive, mat->intensity);
						}

						mesh->setEmissionColor(mat->emissionColor);
						// --------

						// OPACITY -----
						if (mat->opacity != nullptr) {
							mesh->setOpacityTexture(mat->opacity);
						}
						// --------
					}
					// --------
				}
				// ------------

				mesh->indices = assimpMesh.indices;
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

					mesh->vertices.push_back(m);
				}

				// Bones
				if constexpr (supportsBones<typename M::vertexBufferType>) {
					if (!assimpMesh.skeleton.expired()) {
						mesh->skeleton = assimpMesh.skeleton;
						mesh->setOptimizable(false);
					}
				}
				// -------------------

				mesh->baseVertex = 0;
				mesh->baseIndex = 0;
				mesh->totalVertex = static_cast<uint16_t>(mesh->vertices.size());
				mesh->totalIndex = static_cast<uint16_t>(mesh->indices.size());

				this->addMesh(mesh);
			}
		}

		virtual void loadAnimations(std::shared_ptr<rawrbox::AssimpImporter> model) {
			this->_animations = model->animations;
			this->_animatedMeshes.clear();

			// Mark animated meshes ---
			for (auto& anim : model->animatedMeshes) {
				auto fnd = std::find_if(this->_meshes.begin(), this->_meshes.end(), [anim](std::shared_ptr<rawrbox::Mesh<typename M::vertexBufferType>> msh) {
					return msh->getName() == anim.second->name;
				});
				if (fnd == this->_meshes.end()) continue;

				(*fnd)->setOptimizable(false);
				this->_animatedMeshes[anim.first] = *fnd;
			}
			// -----------------------
		}

		virtual void loadLights(std::shared_ptr<rawrbox::AssimpImporter> model) {
			for (auto& assimpLights : model->lights) {
				std::shared_ptr<rawrbox::LightBase> light = nullptr;

				switch (assimpLights.type) {
					case LightType::LIGHT_UNKNOWN: break;
					case LightType::LIGHT_POINT:
						light = std::make_shared<rawrbox::LightPoint>(assimpLights.pos, assimpLights.diffuse, assimpLights.specular, assimpLights.attenuationConstant, assimpLights.attenuationLinear, assimpLights.attenuationQuadratic);
						break;
					case LightType::LIGHT_SPOT:
						light = std::make_shared<rawrbox::LightSpot>(assimpLights.pos, assimpLights.direction, assimpLights.diffuse, assimpLights.specular, assimpLights.angleInnerCone, assimpLights.angleOuterCone, assimpLights.attenuationConstant, assimpLights.attenuationLinear, assimpLights.attenuationQuadratic);
						break;
					case LightType::LIGHT_DIR:
						light = std::make_shared<rawrbox::LightDirectional>(assimpLights.pos, assimpLights.direction, assimpLights.diffuse, assimpLights.specular);
						break;
				}

				if (light == nullptr) {
					fmt::print("[RawrBox-Assimp] Failed to create unknown light '{}'\n", assimpLights.name);
					continue;
				}

				this->addLight(light, assimpLights.parentID);
			}
		}

	public:
		using Model<M>::Model;
		~AssimpModel() {
			this->_animations.clear();
			this->_playingAnimations.clear();
			this->_animatedMeshes.clear();
		};

		virtual void load(std::shared_ptr<rawrbox::AssimpImporter> model) {
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
