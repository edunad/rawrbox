#pragma once
#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/math/vector4.hpp>

#include <RefCntAutoPtr.hpp>

#include <Buffer.h>

namespace rawrbox {
	struct CameraUniforms {
		rawrbox::Matrix4x4 gView = {};
		rawrbox::Matrix4x4 gViewInv = {};

		rawrbox::Matrix4x4 gProjection = {};
		rawrbox::Matrix4x4 gProjectionInv = {};

		rawrbox::Matrix4x4 gViewProj = {};
		rawrbox::Matrix4x4 gViewProjInv = {};

		rawrbox::Matrix4x4 gWorld = {};
		rawrbox::Matrix4x4 gWorldViewProj = {};

		rawrbox::Vector4f gNearFar = {};

		rawrbox::Vector4i gViewport = {};
		rawrbox::Vector4f gViewportInv = {};

		rawrbox::Vector4f gPos = {};
		rawrbox::Vector4f gAngle = {};

		rawrbox::Vector4f gGridParams = {};
	};

	class CameraBase {
	protected:
		rawrbox::Vector3f _pos = {};
		rawrbox::Vector4f _angle = {};

		rawrbox::Matrix4x4 _view = {};
		rawrbox::Matrix4x4 _projection = {};

		rawrbox::Matrix4x4 _world = {};

		float _z_near = 0.01F;
		float _z_far = 100.F;

		Diligent::RefCntAutoPtr<Diligent::IBuffer> _uniforms;

		virtual void updateMtx();

	public:
		virtual ~CameraBase();

		CameraBase() = default;
		CameraBase(CameraBase&&) = default;
		CameraBase& operator=(CameraBase&&) = default;
		CameraBase(const CameraBase&) = default;
		CameraBase& operator=(const CameraBase&) = default;

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

		[[nodiscard]] virtual const rawrbox::Vector3f worldToScreen(const rawrbox::Vector3f& pos) const;
		[[nodiscard]] virtual const rawrbox::Vector3f screenToWorld(const rawrbox::Vector2f& screen_pos, const rawrbox::Vector3f& origin = {0, 0, 0}) const;
		// ----------------

		[[nodiscard]] virtual Diligent::IBuffer* uniforms() const;

		virtual void initialize();
		virtual void update();
		virtual void updateBuffer();
	};
} // namespace rawrbox
