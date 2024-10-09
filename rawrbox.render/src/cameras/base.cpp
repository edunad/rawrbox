
#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/cameras/base.hpp>
#include <rawrbox/render/plugins/clustered.hpp>
#include <rawrbox/render/static.hpp>

namespace rawrbox {
	CameraBase::~CameraBase() {
		RAWRBOX_DESTROY(this->_uniforms);
		RAWRBOX_DESTROY(this->_staticUniforms);
	}

	void CameraBase::initialize() {
		if (this->_staticUniforms != nullptr) CRITICAL_RAWRBOX("Camera already initialized!");
		{
			auto staticData = this->getStaticData();

			Diligent::BufferDesc CBDesc;
			CBDesc.Name = "rawrbox::Camera::Static::Uniforms";
			CBDesc.Usage = Diligent::USAGE_IMMUTABLE;
			CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			CBDesc.Size = sizeof(rawrbox::CameraStaticUniforms);

			Diligent::BufferData bData;
			bData.DataSize = CBDesc.Size;
			bData.pData = &staticData;

			rawrbox::RENDERER->device()->CreateBuffer(CBDesc, &bData, &this->_staticUniforms);
		}

		{
			Diligent::BufferDesc CBDesc;
			CBDesc.Name = "rawrbox::Camera::Uniforms";
			CBDesc.Usage = Diligent::USAGE_DEFAULT;
			CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			CBDesc.Size = sizeof(rawrbox::CameraUniforms);

			rawrbox::RENDERER->device()->CreateBuffer(CBDesc, nullptr, &this->_uniforms);
		}

		// BARRIER -----
		rawrbox::BarrierUtils::barrier({{this->_staticUniforms, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_uniforms, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		// -----------

		this->_logger->info("Initializing camera");
	}

	void CameraBase::updateMtx() { CRITICAL_RAWRBOX("Not implemented"); };
	rawrbox::CameraStaticUniforms CameraBase::getStaticData() {
		auto screenSize = rawrbox::RENDERER->getSize().cast<float>();

		rawrbox::CameraStaticUniforms data = {};
		data.gProjection = rawrbox::Matrix4x4::mtxTranspose(this->getProjMtx());
		data.gProjectionInv = rawrbox::Matrix4x4::mtxInverse(data.gProjection);
		data.gViewport = {this->getZNear(), this->getZFar(), screenSize.x, screenSize.y}; // TODO: Support screen re-scaling, make this dynamic buffer then

		float nearZ = this->getZNear();
		float farZ = this->getZFar();
		auto gLightClustersNumZz = static_cast<float>(RB_RENDER_CLUSTERS_Z);

		data.gGridParams = {
		    gLightClustersNumZz / std::log(farZ / nearZ),
		    (gLightClustersNumZz * std::log(nearZ)) / std::log(farZ / nearZ)};
		return data;
	}

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
		return this->_projection * this->_view;
	}

	const rawrbox::Matrix4x4& CameraBase::getModelTransform() const {
		return this->_world;
	}

	void CameraBase::setModelTransform(const rawrbox::Matrix4x4& transform) {
		if (this->_world == transform) return;

		this->_world = transform;
		this->updateBuffer();
	}

	void CameraBase::updateBuffer() {
		if (this->_uniforms == nullptr) CRITICAL_RAWRBOX("Buffer not initialized! Did you call initialize?");

		auto view = rawrbox::Matrix4x4::mtxTranspose(this->getViewMtx());
		auto viewInv = rawrbox::Matrix4x4::mtxInverse(this->getViewMtx());
		auto projection = rawrbox::Matrix4x4::mtxTranspose(this->getProjMtx());
		auto world = rawrbox::Matrix4x4::mtxTranspose(this->_world);

		rawrbox::CameraUniforms data = {};
		data.gView = view;
		data.gViewInv = viewInv;
		data.gWorld = world;
		data.gWorldViewProj = data.gWorld * data.gView * projection;
		data.gPos = this->getPos();
		data.gDeltaTime = rawrbox::DELTA_TIME;

		rawrbox::BarrierUtils::barrier({{this->_uniforms, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		rawrbox::RENDERER->context()->UpdateBuffer(this->_uniforms, 0, sizeof(rawrbox::CameraUniforms), &data, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
		rawrbox::BarrierUtils::barrier({{this->_uniforms, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
	}

	void CameraBase::update() {}

	rawrbox::Vector3f CameraBase::worldToScreen(const rawrbox::Vector3f& /*pos*/) const {
		CRITICAL_RAWRBOX("Not implemented");
	}

	rawrbox::Vector3f CameraBase::screenToWorld(const rawrbox::Vector2f& /*screen_pos*/, const rawrbox::Vector3f& /*origin*/) const {
		CRITICAL_RAWRBOX("Not implemented");
	}

	Diligent::IBuffer* CameraBase::uniforms() const { return this->_uniforms; }
	Diligent::IBuffer* CameraBase::staticUniforms() const { return this->_staticUniforms; }
	// ----------------
} // namespace rawrbox
