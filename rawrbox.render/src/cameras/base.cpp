
#include <rawrbox/render/cameras/base.hpp>
#include <rawrbox/render/static.hpp>

#include <stdexcept>

namespace rawrbox {
	CameraBase::~CameraBase() {
		RAWRBOX_DESTROY(this->_uniforms);
	}

	void CameraBase::initialize() {
		Diligent::BufferDesc CBDesc;
		CBDesc.Name = "rawrbox::Camera::Uniforms";
		CBDesc.Size = sizeof(rawrbox::CameraUniforms);
		CBDesc.Usage = Diligent::USAGE_DYNAMIC;
		CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
		CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

		rawrbox::RENDERER->device()->CreateBuffer(CBDesc, nullptr, &this->_uniforms);

		this->updateBuffer();
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
		return this->_model;
	}

	void CameraBase::setModelTransform(const rawrbox::Matrix4x4& transform) {
		this->_model = rawrbox::Matrix4x4::mtxTranspose(transform);
		this->updateBuffer();
	}

	void CameraBase::updateBuffer() {
		auto renderer = rawrbox::RENDERER;
		auto context = renderer->context();

		auto& screenSize = renderer->getSize();

		// SETUP UNIFORMS ----------------------------
		Diligent::MapHelper<rawrbox::CameraUniforms> CBConstants(context, this->_uniforms, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);

		CBConstants->gView = rawrbox::Matrix4x4::mtxTranspose(this->getViewMtx());
		CBConstants->gViewInv = rawrbox::Matrix4x4::mtxInverse(this->getViewMtx());

		CBConstants->gProjection = rawrbox::Matrix4x4::mtxTranspose(this->getProjMtx());
		CBConstants->gProjectionInv = rawrbox::Matrix4x4::mtxInverse(this->getProjMtx());

		CBConstants->gViewProj = CBConstants->gView * CBConstants->gProjection;
		CBConstants->gViewProjInv = rawrbox::Matrix4x4::mtxInverse(CBConstants->gView * CBConstants->gProjection);

		CBConstants->gModel = this->_model;
		CBConstants->gWorldViewProj = CBConstants->gModel * CBConstants->gViewProj;

		CBConstants->gNearFar = {this->getZNear(), this->getZFar()};
		CBConstants->gScreenSize = screenSize;

		CBConstants->gPos = this->getPos();
		CBConstants->gAngle = this->getAngle();
		// ------------
	}

	void CameraBase::update() { this->updateBuffer(); }

	const rawrbox::Vector3f CameraBase::worldToScreen(const rawrbox::Vector3f& /*pos*/) const {
		throw std::runtime_error("Not implemented");
	}

	const rawrbox::Vector3f CameraBase::screenToWorld(const rawrbox::Vector2f& /*screen_pos*/, const rawrbox::Vector3f& /*origin*/) const {
		throw std::runtime_error("Not implemented");
	}

	Diligent::IBuffer* CameraBase::uniforms() const { return this->_uniforms; }
	// ----------------
} // namespace rawrbox
