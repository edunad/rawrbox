
#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/cameras/base.hpp>
#include <rawrbox/render/plugins/clustered.hpp>
#include <rawrbox/render/static.hpp>

#include <stdexcept>

namespace rawrbox {
	CameraBase::~CameraBase() {
		RAWRBOX_DESTROY(this->_uniforms);
	}

	void CameraBase::initialize() {
		auto device = rawrbox::RENDERER->device();
		auto context = rawrbox::RENDERER->context();

		Diligent::BufferDesc CBDesc;
		CBDesc.Name = "rawrbox::Camera::Uniforms";
		CBDesc.Size = sizeof(rawrbox::CameraUniforms);
		CBDesc.Usage = Diligent::USAGE_DYNAMIC;
		CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
		CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

		rawrbox::RENDERER->device()->CreateBuffer(CBDesc, nullptr, &this->_uniforms);

		// Barrier ----
		rawrbox::BindlessManager::barrier(*this->_uniforms, rawrbox::BufferType::CONSTANT);
		// ------------
	}

	void CameraBase::updateMtx(){};

	// UTILS -----
	void CameraBase::setPos(const rawrbox::Vector3f& pos) {
		this->_pos = pos;
		this->updateMtx();
	}

	const rawrbox::Vector3f& CameraBase::getPos() const {
		return this->_pos;
	}

	void CameraBase::setAngle(const rawrbox::Vector4f& angle) {
		this->_angle = angle;
		this->updateMtx();
	}

	const rawrbox::Vector4f& CameraBase::getAngle() const {
		return this->_angle;
	}

	float CameraBase::getZFar() const { return this->_z_far; }
	float CameraBase::getZNear() const { return this->_z_near; }

	rawrbox::Vector3f CameraBase::getForward() const {
		return {
		    std::cos(this->_angle.y) * std::sin(this->_angle.x),
		    std::sin(this->_angle.y),
		    std::cos(this->_angle.y) * std::cos(this->_angle.x),
		};
	}

	rawrbox::Vector3f CameraBase::getRight() const {
		return {
		    std::sin(this->_angle.x - rawrbox::piHalf<float>),
		    0.0F,
		    std::cos(this->_angle.x - rawrbox::piHalf<float>),
		};
	}

	rawrbox::Vector3f CameraBase::getUp() const {
		auto right = this->getRight();
		auto forward = this->getForward();

		auto up = right.cross(forward);
		return {up.x, up.y, up.z};
	}

	const rawrbox::Matrix4x4& CameraBase::getViewMtx() const {
		return this->_view;
	}

	const rawrbox::Matrix4x4& CameraBase::getProjMtx() const {
		return this->_projection;
	}

	rawrbox::Matrix4x4 CameraBase::getViewProjMtx() const {
		return this->_view * this->_projection;
	}

	const rawrbox::Matrix4x4& CameraBase::getModelTransform() const {
		return this->_world;
	}

	void CameraBase::setModelTransform(const rawrbox::Matrix4x4& transform) {
		if (this->_world == transform) return;

		this->_world = transform;
		this->updateBuffer(); // TODO: Prob bad idea to keep updating buffer on every model
	}

	void CameraBase::updateBuffer() {
		auto renderer = rawrbox::RENDERER;
		auto context = renderer->context();

		auto& screenSize = renderer->getSize();
		auto screenSizeF = screenSize.cast<float>();

		// SETUP UNIFORMS ----------------------------
		Diligent::MapHelper<rawrbox::CameraUniforms> CBConstants(context, this->_uniforms, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);

		CBConstants->gView = rawrbox::Matrix4x4::mtxTranspose(this->getViewMtx());
		CBConstants->gViewInv = rawrbox::Matrix4x4::mtxInverse(CBConstants->gView);

		CBConstants->gProjection = rawrbox::Matrix4x4::mtxTranspose(this->getProjMtx());
		CBConstants->gProjectionInv = rawrbox::Matrix4x4::mtxInverse(CBConstants->gProjection);

		CBConstants->gViewProj = CBConstants->gView * CBConstants->gProjection;
		CBConstants->gViewProjInv = rawrbox::Matrix4x4::mtxInverse(CBConstants->gViewProj);

		CBConstants->gWorld = rawrbox::Matrix4x4::mtxTranspose(this->_world);
		CBConstants->gWorldViewProj = CBConstants->gWorld * CBConstants->gViewProj;

		CBConstants->gNearFar = {this->getZNear(), this->getZFar()};

		CBConstants->gViewport = {0, 0, screenSize.x, screenSize.y};
		CBConstants->gViewportInv = {0, 0, 1.F / screenSizeF.x, 1.F / screenSizeF.y};

		CBConstants->gPos = this->getPos();
		CBConstants->gAngle = this->getAngle();
		// ------------

		// Setup grid ----
		float nearZ = this->getZNear();
		float farZ = this->getZFar();
		auto gLightClustersNumZz = static_cast<float>(rawrbox::CLUSTERS_Z);

		CBConstants->gGridParams = {
		    gLightClustersNumZz / std::log(farZ / nearZ),
		    (gLightClustersNumZz * std::log(nearZ)) / std::log(farZ / nearZ)};
		// --------------
	}

	void CameraBase::update() {}

	const rawrbox::Vector3f CameraBase::worldToScreen(const rawrbox::Vector3f& /*pos*/) const {
		throw this->_logger->error("Not implemented");
	}

	const rawrbox::Vector3f CameraBase::screenToWorld(const rawrbox::Vector2f& /*screen_pos*/, const rawrbox::Vector3f& /*origin*/) const {
		throw this->_logger->error("Not implemented");
	}

	Diligent::IBuffer* CameraBase::uniforms() const { return this->_uniforms; }
	// ----------------
} // namespace rawrbox
