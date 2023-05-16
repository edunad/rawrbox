#pragma once

#include <rawrbox/render/model/assimp/assimp_importer.hpp>
#include <rawrbox/render/model/model.hpp>

namespace rawrbox {
	template <typename M = rawrbox::MaterialBase>
	class AssimModel : public rawrbox::Model<M> {

		void loadMeshes(std::shared_ptr<rawrbox::AssimpImporter> model) {
			for (auto& assimpMesh : model->meshes) {
				auto mesh = std::make_shared<rawrbox::Mesh<typename M::vertexBufferType>>();

				mesh->setName(assimpMesh.name);
				mesh->setCulling(BGFX_STATE_CULL_CCW); // Default culling for assimp

				mesh->bbox = assimpMesh.bbox;
				mesh->offsetMatrix = assimpMesh.offsetMatrix; // Append matrix to our vertices, since pre-transform is disabled

				// Textures ---
				if (assimpMesh.material != nullptr) {
					mesh->setTexture(rawrbox::WHITE_TEXTURE);               // Default
					mesh->setSpecularTexture(rawrbox::WHITE_TEXTURE, 25.F); // Default
					mesh->setEmissionTexture(rawrbox::WHITE_TEXTURE, 1.F);  // Default

					mesh->setWireframe(assimpMesh.material->wireframe);
					mesh->setCulling(assimpMesh.material->matDisableCulling ? 0 : BGFX_STATE_CULL_CCW);
					mesh->setBlend(assimpMesh.material->blending);

					// DIFFUSE -----
					if (assimpMesh.material->diffuse != nullptr) {
						mesh->setTexture(assimpMesh.material->diffuse);
					}

					mesh->setColor(assimpMesh.material->diffuseColor);
					// --------

					// SPECULAR -----
					if constexpr (supportsNormals<typename M::vertexBufferType>) {
						if (assimpMesh.material->specular != nullptr) {
							mesh->setSpecularTexture(assimpMesh.material->specular, assimpMesh.material->shininess);
						}

						mesh->setSpecularColor(assimpMesh.material->specularColor);
						// --------

						// EMISSION -----
						if (assimpMesh.material->emissive != nullptr) {
							mesh->setEmissionTexture(assimpMesh.material->emissive, assimpMesh.material->intensity);
						}

						mesh->setEmissionColor(assimpMesh.material->emissionColor);
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
						m.bone_indices = vert.bone_indices;
						m.bone_weights = vert.bone_weights;
					}

					mesh->vertices.push_back(m);
				}

				// Bones
				if constexpr (supportsBones<typename M::vertexBufferType>) {
					if (assimpMesh.skeleton != nullptr) {
						mesh->skeleton = assimpMesh.skeleton;
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

		void loadLights(std::shared_ptr<rawrbox::AssimpImporter> model) {
			for (auto assimpLights : model->lights) {

				/*std::shared_ptr<rawrbox::Mesh<typename M::vertexBufferType>> parent = this->_meshes.front();
				if (!assimpLights.first.empty()) {
					auto fnd = std::find_if(this->_meshes.begin(), this->_meshes.end(), [assimpLights](std::shared_ptr<rawrbox::Mesh<typename M::vertexBufferType>> msh) {
						return msh->getName() == assimpLights.first;
					});

					if (fnd != this->_meshes.end()) parent = *fnd;
				}

				assimpLights.second->setParent(std::dynamic_pointer_cast<rawrbox::Mesh<>>(parent));
				rawrbox::LIGHTS::addLight(assimpLights.second);*/
				rawrbox::LIGHTS::addLight(assimpLights.second);
			}
		}

	public:
		AssimModel() = default;
		~AssimModel(){
		    // remove lights

		};

		virtual void setPos(const rawrbox::Vector3f& pos) override {
			rawrbox::Model<M>::setPos(pos);
		}

		virtual void load(std::shared_ptr<rawrbox::AssimpImporter> model) {
			this->loadMeshes(model);
			this->loadLights(model);

			this->upload();
		}
	};
} // namespace rawrbox
