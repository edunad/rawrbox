#pragma once
#include <rawrbox/math/aabb.hpp>
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

		bool operator==(const CameraStaticUniforms& other) const {
			return this->gProjection == other.gProjection && this->gProjectionInv == other.gProjectionInv && this->gViewport == other.gViewport;
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
		rawrbox::Matrix4x4 gViewProj = {};

		// ----------
		rawrbox::Vector3f gPos = {};
		float gDeltaTime = 0;
		// ----------------

		bool operator==(const CameraUniforms& other) const {
			return this->gView == other.gView && this->gViewProj == other.gViewProj && this->gPos == other.gPos;
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

		// MOTION ----
		static constexpr float TELEPORT_DISTANCE = 0.5F;
		static constexpr float TELEPORT_ANGLE = 0.966F;

		uint64_t _moveRevision = 0;
		uint64_t _teleportRevision = 0; // Far positions
		// -----------

		uint32_t _layers = 0; // None by default, it's up to the user to use this or not

		// Renderer ----
		std::unique_ptr<rawrbox::TextureRender> _renderTarget = nullptr;
		// --------------------------

		// PRESENT ----
		rawrbox::Vector2i _presentOffset = {};
		uint32_t _presentScale = 0; // 0 = stretched over the window
		// -----------

		float _z_near = 0.01F;
		float _z_far = 100.F;

		// CULLING ----
#ifdef _DEBUG
		bool _cullFrozen = false;
		rawrbox::Matrix4x4 _cullMtx = {};
#endif
		// -----------

		// LOGGER ------
		std::unique_ptr<rawrbox::Logger> _logger = std::make_unique<rawrbox::Logger>("RawrBox-Camera");
		// -------------

		virtual void updateMtx();
		virtual rawrbox::CameraStaticUniforms getStaticData();

		static rawrbox::CameraStaticUniforms _uploadedStatic;

	public:
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> staticUniforms;
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> uniforms;

		static void shutdown();

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

		[[nodiscard]] virtual uint64_t getMoveRevision() const;
		[[nodiscard]] virtual uint64_t getTeleportRevision() const;

		[[nodiscard]] virtual float getZFar() const;
		[[nodiscard]] virtual float getZNear() const;

		[[nodiscard]] virtual const rawrbox::Matrix4x4& getViewMtx() const;
		[[nodiscard]] virtual const rawrbox::Matrix4x4& getProjMtx() const;
		[[nodiscard]] virtual rawrbox::Matrix4x4 getViewProjMtx() const;
#ifdef _DEBUG
		virtual void setCullFrozen(bool frozen);
		[[nodiscard]] virtual bool isCullFrozen() const;
#endif

		[[nodiscard]] virtual rawrbox::Vector3f worldToScreen(const rawrbox::Vector3f& pos) const;
		[[nodiscard]] virtual rawrbox::Vector3f screenToWorld(const rawrbox::Vector2f& screen_pos, const rawrbox::Vector3f& origin = {0, 0, 0}) const;
		[[nodiscard]] virtual rawrbox::Vector3f screenRayDir(const rawrbox::Vector2f& screen_pos) const;

		virtual bool isEnabled() const;
		virtual void setEnabled(bool enabled);

		virtual uint32_t getLayers() const;
		virtual void setLayers(uint32_t layers);
		virtual bool shouldRenderLayer(uint32_t layer) const;
		// ----------------

		// RENDER TARGET ----
		virtual void begin();
		virtual void end();

		virtual void resize(const rawrbox::Vector2u& size);

		[[nodiscard]] virtual Diligent::ITextureView* getDepth() const;
		[[nodiscard]] virtual Diligent::ITextureView* getColor(bool rt = false) const;
		[[nodiscard]] virtual rawrbox::TextureRender* getRenderTarget() const;
		// ------------

		// PRESENT ----
		virtual void setPresent(const rawrbox::Vector2i& offset, uint32_t scale);
		[[nodiscard]] virtual rawrbox::AABBf getPresentRect() const;

		[[nodiscard]] virtual rawrbox::Vector2f windowToTarget(const rawrbox::Vector2f& windowPos) const;
		[[nodiscard]] virtual rawrbox::Vector2f targetToWindow(const rawrbox::Vector2f& targetPos) const;
		// ------------

		virtual void initialize();
		virtual void upload();

		virtual void updateStatic();
		virtual void updateBuffer();
		virtual void update();
	};
} // namespace rawrbox