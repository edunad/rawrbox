#pragma once

#include <rawrbox/render/materials/base.hpp>

#ifdef RAWRBOX_SCRIPTING
	#include <sol/sol.hpp>
#endif

namespace rawrbox {

	struct BlendShapes {
	public:
		float weight = 0.F;

		rawrbox::Mesh* mesh = nullptr; // For quick access

		std::vector<rawrbox::Vector3f> pos = {};
		std::vector<rawrbox::Vector3f> normals = {};

		bool isActive() { return weight > 0.F; }
		BlendShapes() = default;
	};

#ifdef RAWRBOX_SCRIPTING
	class ModelBase : public std::enable_shared_from_this<rawrbox::ModelBase> {
#else
	class ModelBase {
#endif

	protected:
		bgfx::DynamicVertexBufferHandle _vbdh = BGFX_INVALID_HANDLE; // Vertices - Dynamic
		bgfx::VertexBufferHandle _vbh = BGFX_INVALID_HANDLE;         // Vertices - Static

		bgfx::DynamicIndexBufferHandle _ibdh = BGFX_INVALID_HANDLE; // Indices - Dynamic
		bgfx::IndexBufferHandle _ibh = BGFX_INVALID_HANDLE;         // Indices - Static

		std::unique_ptr<rawrbox::Mesh> _mesh = std::make_unique<rawrbox::Mesh>();
		std::unique_ptr<rawrbox::MaterialBase> _material = std::make_unique<rawrbox::MaterialBase>();

		std::unordered_map<std::string, std::unique_ptr<rawrbox::BlendShapes>> _blend_shapes = {};

		// BGFX DYNAMIC SUPPORT ---
		bool _isDynamic = false;
		// ----

#ifdef RAWRBOX_SCRIPTING
		sol::object _luaWrapper;
		virtual void initializeLua();
#endif

		// BLEND SHAPES ---
		virtual void applyBlendShapes();
		// --------------

	public:
		ModelBase() = default;
		ModelBase(ModelBase&&) = delete;
		ModelBase& operator=(ModelBase&&) = delete;
		ModelBase(const ModelBase&) = delete;
		ModelBase& operator=(const ModelBase&) = delete;
		virtual ~ModelBase();

		template <typename M = rawrbox::MaterialBase>
		void setMaterial() {
			this->_material = std::make_unique<M>();
		}

		// BLEND SHAPES ---
		template <typename T = rawrbox::BlendShapes>
		bool createBlendShape(const std::string& id, const std::vector<rawrbox::Vector3f>& newVertexPos, const std::vector<rawrbox::Vector3f>& newNormPos, float weight = 0.F) {
			if (this->_mesh == nullptr) throw std::runtime_error("[RawrBox-ModelBase] Mesh not initialized!");

			auto blend = std::make_unique<rawrbox::BlendShapes>();
			blend->pos = newVertexPos;
			blend->normals = newNormPos;
			blend->weight = weight;
			blend->mesh = this->_mesh.get();

			this->_blend_shapes[id] = std::move(blend);
		}

		virtual bool removeBlendShape(const std::string& id);
		virtual bool setBlendShape(const std::string& id, float weight);
		virtual bool setBlendShapeByKey(const std::string& id, float weight);
		// --------------

		// UTIL ---
		virtual void updateBuffers();

		[[nodiscard]] virtual const rawrbox::Vector3f& getPos() const;
		virtual void setPos(const rawrbox::Vector3f& pos);

		[[nodiscard]] virtual const rawrbox::Vector3f& getScale() const;
		virtual void setScale(const rawrbox::Vector3f& scale);

		[[nodiscard]] virtual const rawrbox::Vector4f& getAngle() const;
		virtual void setAngle(const rawrbox::Vector4f& ang);
		virtual void setEulerAngle(const rawrbox::Vector3f& ang);

		// virtual void setBlendShape(const std::string& key, float value);

		[[nodiscard]] virtual const rawrbox::Matrix4x4& getMatrix() const;

		[[nodiscard]] virtual bool isDynamic() const;
		[[nodiscard]] virtual bool isUploaded() const;

		virtual rawrbox::Mesh* mesh();

		// ----
		virtual void upload(bool dynamic = false);
		virtual void draw();

#ifdef RAWRBOX_SCRIPTING
		virtual sol::object& getScriptingWrapper();
#endif
	};
} // namespace rawrbox
