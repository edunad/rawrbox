#pragma once

#include <rawrbox/engine/static.hpp>
#include <rawrbox/render/light/manager.hpp>
#include <rawrbox/render/models/animation.hpp>
#include <rawrbox/render/models/base.hpp>
#include <rawrbox/render/models/skeleton.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/anim.hpp>

#ifdef RAWRBOX_SCRIPTING
	#include <rawrbox/render/scripting/wrappers/model/model_wrapper.hpp>
	#include <sol/sol.hpp>
#endif

namespace rawrbox {

	class Model : public rawrbox::ModelBase {

	protected:
		std::unordered_map<std::string, rawrbox::Animation> _animations = {};

		std::vector<rawrbox::PlayingAnimationData> _playingAnimations = {};
		std::vector<rawrbox::LightBase> _lights = {};

		std::vector<std::unique_ptr<rawrbox::Mesh>> _meshes = {};
		rawrbox::BBOX _bbox = {};

		// SKINNING ----
		std::unordered_map<std::string, rawrbox::Mesh*> _animatedMeshes = {}; // Map for quick lookup
		// --------

		bool _canOptimize = true;

		// ANIMATIONS ----
		void animate(const rawrbox::Mesh& mesh) const;
		void animateBones(std::unordered_map<uint8_t, rawrbox::Matrix4x4>& calcs, const rawrbox::Skeleton& skeleton, const rawrbox::Bone& parentBone, const rawrbox::Matrix4x4& parentTransform) const;
		void readAnims(rawrbox::Matrix4x4& nodeTransform, const std::string& nodeName) const;

		void preDraw();
		void postDraw();
		// --------------

		void updateLights();

		// BLEND SHAPES ---
		void applyBlendShapes() override;
		// --------------

#ifdef RAWRBOX_SCRIPTING
		void initializeLua() override;
#endif

	public:
		Model() = default;
		Model(const Model&) = delete;
		Model(Model&&) = delete;
		Model& operator=(const Model&) = delete;
		Model& operator=(Model&&) = delete;
		~Model() override;

		virtual void flattenMeshes(bool optimize = true, bool sort = true);
		virtual void setOptimizable(bool status);
		virtual void optimize();

		void updateBuffers() override;

		// ANIMATIONS ----
		virtual bool blendAnimation(const std::string& /*otherAnim*/, float /*blend*/);
		virtual bool playAnimation(const std::string& name, bool loop = true, float speed = 1.F);
		virtual bool stopAnimation(const std::string& name);
		// --------------

		// BLEND SHAPES ---	template <typename T = rawrbox::BlendShapes>
		void createBlendShape(size_t mesh, const std::string& id, const std::vector<rawrbox::Vector3f>& newVertexPos, const std::vector<rawrbox::Vector3f>& newNormPos, float weight = 0.F) {
			if (mesh >= this->_meshes.size()) throw std::runtime_error(fmt::format("[RawrBox-ModelBase] Mesh '{}' not found!", mesh));

			auto blend = std::make_unique<rawrbox::BlendShapes>();
			blend->pos = newVertexPos;
			blend->normals = newNormPos;
			blend->weight = weight;
			blend->mesh = this->_meshes[mesh].get();

			this->_blend_shapes[id] = std::move(blend);
		}
		// --------------

		// LIGHTS ------
		template <typename T = rawrbox::LightBase, typename... CallbackArgs>
		void addLight(const std::string& parentMesh = "", CallbackArgs&&... args) {
			if ((this->_material->supports() & rawrbox::MaterialFlags::NORMALS) != 0) {
				auto parent = this->_meshes.back().get();
				if (!parentMesh.empty()) {
					auto fnd = std::find_if(this->_meshes.begin(), this->_meshes.end(), [parentMesh](auto& msh) {
						return msh->getName() == parentMesh;
					});

					if (fnd != this->_meshes.end()) parent = fnd->get();
				}

				auto light = rawrbox::LIGHTS::addLight<T>(std::forward<CallbackArgs>(args)...);
				light->setOffsetPos(parent->getPos() + this->getPos());
				parent->lights.push_back(light);
			}
		}
		// -----

		void setPos(const rawrbox::Vector3f& pos) override;
		void setAngle(const rawrbox::Vector4f& angle) override;
		void setEulerAngle(const rawrbox::Vector3f& angle) override;
		void setScale(const rawrbox::Vector3f& size) override;

		[[nodiscard]] virtual const rawrbox::BBOX& getBBOX() const;
		[[nodiscard]] virtual size_t totalMeshes() const;
		[[nodiscard]] virtual bool empty() const;

		virtual void removeMeshByName(const std::string& id);
		virtual void removeMesh(size_t index);

		virtual rawrbox::Mesh* addMesh(rawrbox::Mesh mesh);

		virtual rawrbox::Mesh* getMeshByName(const std::string& id);

		virtual rawrbox::Mesh* getMesh(size_t id = 0);

		virtual bool hasMesh(size_t id);

		virtual void setCulling(Diligent::CULL_MODE cull, int id = -1);
		virtual void setWireframe(bool wireframe, int id = -1);
		virtual void setBlend(uint64_t blend, int id = -1);
		virtual void setDepthTest(uint64_t depth, int id = -1);
		virtual void setColor(const rawrbox::Color& color, int id = -1);
		virtual void setRecieveDecals(bool recieve, int id = -1);
		virtual void setTexture(rawrbox::TextureBase* tex, int id = -1);

		virtual std::vector<std::unique_ptr<rawrbox::Mesh>>& meshes();

		void upload(bool dynamic = false) override;
		void draw() override;
	};
} // namespace rawrbox
