#pragma once

#include <rawrbox/render/materials/base.hpp>
#ifdef RAWRBOX_SCRIPTING
	#include <sol/sol.hpp>
#endif

namespace rawrbox {

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

		// BGFX DYNAMIC SUPPORT ---
		bool _isDynamic = false;
		// ----

#ifdef RAWRBOX_SCRIPTING
		sol::object _luaWrapper;
		virtual void initializeLua();
#endif

		virtual void updateBuffers();

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

		// UTIL ---
		[[nodiscard]] virtual const rawrbox::Vector3f& getPos() const;
		virtual void setPos(const rawrbox::Vector3f& pos);

		[[nodiscard]] virtual const rawrbox::Vector3f& getScale() const;
		virtual void setScale(const rawrbox::Vector3f& scale);

		[[nodiscard]] virtual const rawrbox::Vector4f& getAngle() const;
		virtual void setAngle(const rawrbox::Vector4f& ang);
		virtual void setEulerAngle(const rawrbox::Vector3f& ang);

		[[nodiscard]] virtual const rawrbox::Matrix4x4& getMatrix() const;

		[[nodiscard]] virtual bool isDynamic() const;
		[[nodiscard]] virtual bool isUploaded() const;

		// ----
		virtual void upload(bool dynamic = false);
		virtual void draw();

#ifdef RAWRBOX_SCRIPTING
		virtual sol::object& getScriptingWrapper();
#endif
	};
} // namespace rawrbox
