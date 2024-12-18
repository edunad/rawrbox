#pragma once
#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/math/vector4.hpp>
#include <rawrbox/render/render_config.hpp>
#include <rawrbox/render/textures/render.hpp>
#include <rawrbox/utils/logger.hpp>

#include <RefCntAutoPtr.hpp>

#include <Buffer.h>

#include <memory>

#define CREATE_FLAGS(name, value) \
	const uint32_t name = 1 << value;

namespace rawrbox {
	struct CameraStaticUniforms { // Uniforms that won't be updated that frequently
		rawrbox::Matrix4x4 gProjection = {};
		rawrbox::Matrix4x4 gProjectionInv = {};

		rawrbox::Vector4f gViewport = {};
		rawrbox::Vector2f gGridParams = {};

		bool operator==(const CameraStaticUniforms& other) const {
			return this->gProjection == other.gProjection && this->gProjectionInv == other.gProjectionInv && this->gViewport == other.gViewport && this->gGridParams == other.gGridParams;
		}

		bool operator!=(const CameraStaticUniforms& other) const { return !operator==(other); }
	};

	namespace CameraLayers {
		// USER DEFINED
		RB_RENDER_CAMERA_LAYERS(CREATE_FLAGS)
		// ------------
	}; // namespace CameraLayers

	struct CameraUniforms {
		rawrbox::Matrix4x4 gView = {};
		rawrbox::Matrix4x4 gViewInv = {};

		rawrbox::Matrix4x4 gWorld = {};
		rawrbox::Matrix4x4 gWorldViewProj = {};

		// ----------
		rawrbox::Vector3f gPos = {};
		float gDeltaTime = 0;
		// ----------------

		bool operator==(const CameraUniforms& other) const {
			return this->gView == other.gView && this->gWorld == other.gWorld && this->gWorldViewProj == other.gWorldViewProj && this->gPos == other.gPos;
		}

		bool operator!=(const CameraUniforms& other) const { return !operator==(other); }
	};

	class CameraBase {
	protected:
		bool _enabled = true;

		rawrbox::Vector3f _pos = {};
		rawrbox::Vector4f _angle = {};

		rawrbox::Matrix4x4 _view = {};
		rawrbox::Matrix4x4 _projection = {};

		rawrbox::Matrix4x4 _world = {};

		uint32_t _layers = 0; // None by default, it's up to the user to use this or not

		// Renderer ----
		std::unique_ptr<rawrbox::TextureRender> _renderTarget = nullptr;
		// --------------------------

		float _z_near = 0.01F;
		float _z_far = 100.F;

		// LOGGER ------
		std::unique_ptr<rawrbox::Logger> _logger = std::make_unique<rawrbox::Logger>("RawrBox-Camera");
		// -------------

		virtual void updateMtx();
		virtual rawrbox::CameraStaticUniforms getStaticData();

	public:
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> staticUniforms;
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> uniforms;

		CameraBase(const rawrbox::Vector2u& renderSize, bool depth = true);
		CameraBase(CameraBase&&) = default;
		CameraBase& operator=(CameraBase&&) = default;
		CameraBase(const CameraBase&) = delete;
		CameraBase& operator=(const CameraBase&) = delete;
		virtual ~CameraBase();

		// UTILS -----
		virtual void setPos(const rawrbox::Vector3f& pos);
		[[nodiscard]] virtual const rawrbox::Vector3f& getPos() const;

		virtual void setAngle(const rawrbox::Vector4f& ang);
		[[nodiscard]] virtual const rawrbox::Vector4f& getAngle() const;

		[[nodiscard]] virtual rawrbox::Vector3f getForward() const;
		[[nodiscard]] virtual rawrbox::Vector3f getRight() const;
		[[nodiscard]] virtual rawrbox::Vector3f getUp() const;

		[[nodiscard]] virtual float getZFar() const;
		[[nodiscard]] virtual float getZNear() const;

		[[nodiscard]] virtual const rawrbox::Matrix4x4& getViewMtx() const;
		[[nodiscard]] virtual const rawrbox::Matrix4x4& getProjMtx() const;
		[[nodiscard]] virtual rawrbox::Matrix4x4 getViewProjMtx() const;

		[[nodiscard]] virtual const rawrbox::Matrix4x4& getModelTransform() const;
		virtual void setModelTransform(const rawrbox::Matrix4x4& transform);

		[[nodiscard]] virtual rawrbox::Vector3f worldToScreen(const rawrbox::Vector3f& pos) const;
		[[nodiscard]] virtual rawrbox::Vector3f screenToWorld(const rawrbox::Vector2f& screen_pos, const rawrbox::Vector3f& origin = {0, 0, 0}) const;

		virtual bool isEnabled() const;
		virtual void setEnabled(bool enabled);

		virtual uint32_t getLayers() const;
		virtual void setLayers(uint32_t layers);
		virtual bool shouldRenderLayer(uint32_t layer) const;
		// ----------------

		// RENDER TARGET ----
		virtual void begin();
		virtual void end();

		[[nodiscard]] virtual Diligent::ITextureView* getDepth() const;
		[[nodiscard]] virtual Diligent::ITextureView* getColor(bool rt = false) const;
		[[nodiscard]] virtual rawrbox::TextureRender* getRenderTarget() const;
		// ------------

		virtual void initialize();
		virtual void upload();

		virtual void updateBuffer();
		virtual void update();
	};
} // namespace rawrbox
