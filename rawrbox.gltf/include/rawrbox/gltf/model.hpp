#pragma once

#include <rawrbox/gltf/importer.hpp>
#include <rawrbox/gltf/utils/model.hpp>
#include <rawrbox/render/lights/directional.hpp>
#include <rawrbox/render/lights/point.hpp>
#include <rawrbox/render/lights/spot.hpp>
#include <rawrbox/render/models/model.hpp>

namespace rawrbox {
	template <typename M = MaterialUnlit>
		requires(std::derived_from<M, rawrbox::MaterialBase>)
	class GLTFModel : public rawrbox::Model<M> {
	protected:
		// INTERNAL -------
		void loadMeshes(const rawrbox::GLTFImporter& model) {
			for (const auto& gltfMesh : model.meshes) {
				if (gltfMesh->primitives.empty()) continue; // Bone / empty
				for (const auto& gltfPrimitive : gltfMesh->primitives) {
					auto mesh = rawrbox::GLTFUtils::extractMesh<M>(*gltfMesh, gltfPrimitive);
					mesh.meshID = gltfMesh->index;

					this->addMesh(mesh);
				}
			}
		}

		rawrbox::Mesh<typename M::vertexBufferType>* getMeshByID(size_t index) {
			auto fnd = std::find_if(this->_meshes.begin(), this->_meshes.end(), [&index](auto& mesh) { return mesh->getID() == index; });
			if (fnd == this->_meshes.end()) return nullptr;

			return (*fnd).get();
		}

		void loadAnimations(const rawrbox::GLTFImporter& model) {
			// Get animations ---
			this->_animations.resize(model.animations.size());
			for (size_t i = 0; i < model.animations.size(); i++) {
				this->_animations[i] = model.animations[i].get();
			}
			// -------------------

			// Map vertex animations
			this->_vertexAnimations.clear();
			for (const auto& anim : model.vertexAnimation) {
				for (const auto& mesh : anim.second) {
					auto* rawrMesh = this->getMeshByID(mesh->index);
					if (rawrMesh == nullptr) continue;

					rawrMesh->setMergeable(false);
					rawrMesh->meshID = 0x00000000; // Reset id, we don't need it anymore

					this->_vertexAnimations[anim.first].push_back(rawrMesh); // Horrible, i know.
				}
			}
			// -----------------------
		}

		void loadBlendShapes(const rawrbox::GLTFImporter& model) {
			this->_blend_shapes.clear();

			for (size_t i = 0; i < model.meshes.size(); i++) {
				const auto& mesh = model.meshes[i];

				for (const auto& primitive : mesh->primitives) {
					for (const auto& blend : primitive.blendShapes) {
						auto s = std::make_unique<rawrbox::BlendShapes<M>>();
						s->normals = blend.norms;
						s->pos = blend.pos;
						s->weight = blend.weight;

						s->mesh = this->_meshes[i].get();
						s->mesh->setMergeable(false);

						this->_blend_shapes[blend.name] = std::move(s);
					}
				}
			}
		}

		void loadLights(const rawrbox::GLTFImporter& model) {
			for (const auto& gltfLight : model.lights) {
				rawrbox::LightBase* light = nullptr;

				switch (gltfLight->type) {
					case rawrbox::LightType::POINT:
						light = this->template addLight<rawrbox::PointLight>(gltfLight->parent, gltfLight->pos, gltfLight->color, gltfLight->radius);
						break;
					case rawrbox::LightType::SPOT:
						light = this->template addLight<rawrbox::SpotLight>(gltfLight->parent, gltfLight->pos, gltfLight->direction, gltfLight->color, gltfLight->angleInnerCone, gltfLight->angleOuterCone, gltfLight->radius);
						break;
					case rawrbox::LightType::DIRECTIONAL:
						light = this->template addLight<rawrbox::DirectionalLight>(gltfLight->parent, gltfLight->pos, gltfLight->direction, gltfLight->color);
						break;

					default:
					case rawrbox::LightType::UNKNOWN:
						this->_logger->warn("Failed to create unknown light '{}'", gltfLight->name);
						break;
				}

				if (light != nullptr) light->setIntensity(gltfLight->intensity);
			}
		}
		// -------------------

	public:
		GLTFModel() = default;
		GLTFModel(const GLTFModel&) = delete;
		GLTFModel(GLTFModel&&) = delete;
		GLTFModel& operator=(const GLTFModel&) = delete;
		GLTFModel& operator=(GLTFModel&&) = delete;
		~GLTFModel() override = default;

		void load(const rawrbox::GLTFImporter& model) {
			this->loadMeshes(model);
			this->loadBlendShapes(model);
			this->loadAnimations(model);

			if constexpr (supportsNormals<typename M::vertexBufferType>) {
				this->loadLights(model);
			}
		}
	};
} // namespace rawrbox
