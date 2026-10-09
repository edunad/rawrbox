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
					mesh.meshID = static_cast<uint32_t>(gltfMesh->index);

					this->addMesh(mesh);
				}
			}
		}

		std::vector<rawrbox::Mesh<typename M::vertexBufferType>*> getMeshesByID(uint32_t id) {
			std::vector<rawrbox::Mesh<typename M::vertexBufferType>*> meshes = {};

			for (auto& mesh : this->_meshes) {
				if (mesh == nullptr || mesh->getID() != id) continue;
				meshes.push_back(mesh.get());
			}

			return meshes;
		}

		void loadAnimations(const rawrbox::GLTFImporter& model) {
			// Get animations ---
			this->_animations.resize(model.animations.size());
			for (size_t i = 0; i < model.animations.size(); i++) {
				this->_animations[i] = model.animations[i].get();
			}
			// -------------------

			// Map vertex animations
			this->_animatedMeshes.clear();

			for (const auto& anim : model.animations) {
				for (const auto& part : anim->getParts()) {
					if (part.type != rawrbox::AnimationType::VERTEX) continue;

					for (const auto& id : part.meshes) {
						if (this->_animatedMeshes.contains(id)) continue;

						auto meshes = this->getMeshesByID(id);
						if (meshes.empty()) {
							this->_logger->warn("Missing animation '{}' target mesh id {}", anim->name, id);
							continue;
						}

						for (auto* mesh : meshes) {
							mesh->setMergeable(false);
						}

						this->_animatedMeshes[id] = std::move(meshes);
					}
				}
			}

			// Reset lookup ids (GPU does not need them)
			for (auto& animated : this->_animatedMeshes) {
				for (auto* mesh : animated.second) {
					mesh->meshID = 0x00000000;
				}
			}
			// ---------------------------------
		}

		void loadBlendShapes(const rawrbox::GLTFImporter& model) {
			this->_blend_shapes.clear();

			for (const auto& gltfMesh : model.meshes) {
				auto meshes = this->getMeshesByID(static_cast<uint32_t>(gltfMesh->index));

				for (size_t p = 0; p < gltfMesh->primitives.size(); p++) {
					const auto& primitive = gltfMesh->primitives[p];
					if (primitive.blendShapes.empty()) continue;

					if (p >= meshes.size()) {
						this->_logger->warn("Mesh '{}' -> '{}' has blend shapes but is missing meshes!", gltfMesh->name, p);
						break;
					}

					for (const auto& blend : primitive.blendShapes) {
						auto s = std::make_unique<rawrbox::BlendShapes<M>>();
						s->normals = blend.norms;
						s->pos = blend.pos;
						s->weight = blend.weight;

						s->mesh = meshes[p];
						s->mesh->setMergeable(false);

						const std::string name = gltfMesh->primitives.size() > 1 ? fmt::format("{}-{}", blend.name, p) : blend.name;
						if (this->_blend_shapes.contains(name)) this->_logger->warn("Duplicate blend shape '{}'", name);

						this->_blend_shapes[name] = std::move(s);
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
