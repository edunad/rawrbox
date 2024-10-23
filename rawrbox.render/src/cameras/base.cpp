
#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/cameras/base.hpp>
#include <rawrbox/render/plugins/clustered.hpp>
#include <rawrbox/render/static.hpp>

namespace rawrbox {
	Diligent::RefCntAutoPtr<Diligent::IBuffer> CameraBase::staticUniforms;
	Diligent::RefCntAutoPtr<Diligent::IBuffer> CameraBase::uniforms;

	CameraBase::CameraBase(const rawrbox::Vector2u& renderSize, bool depth) {
		this->_renderTarget = std::make_unique<rawrbox::TextureRender>(renderSize, depth);
	}

	CameraBase::~CameraBase() { this->_renderTarget.reset(); }

	void CameraBase::initialize() {
		if (this->_renderTarget == nullptr) RAWRBOX_CRITICAL("Render target not initialized!");
		this->_renderTarget->upload(Diligent::TEX_FORMAT_RGBA8_UNORM);

		// ADD GPU PICKING TEXTURE
		auto idIndex = this->_renderTarget->addTexture(Diligent::TEX_FORMAT_RGBA8_UNORM);
		this->_renderTarget->addView(idIndex, Diligent::TEXTURE_VIEW_RENDER_TARGET);
		// --------
	}

	void CameraBase::upload() {
		if (rawrbox::RENDERER == nullptr) RAWRBOX_CRITICAL("Renderer not initialized!");

		auto* device = rawrbox::RENDERER->device();
		if (device == nullptr) RAWRBOX_CRITICAL("Device not initialized!");

		// These are shared across all the cameras. For performance reasons, we won't support different cameras (for now)
		if (staticUniforms != nullptr || uniforms != nullptr) RAWRBOX_CRITICAL("Camera buffers already initialized!");
		auto staticData = this->getStaticData();

		// STATIC BUFFER ---
		Diligent::BufferDesc StaticDesc;
		StaticDesc.Name = "rawrbox::Camera::Static::Uniforms";
		StaticDesc.Usage = Diligent::USAGE_IMMUTABLE;
		StaticDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
		StaticDesc.Size = sizeof(rawrbox::CameraStaticUniforms);

		Diligent::BufferData sData;
		sData.DataSize = StaticDesc.Size;
		sData.pData = &staticData;

		device->CreateBuffer(StaticDesc, &sData, &staticUniforms);
		// ------------

		// DYNAMIC BUFFER ---
		Diligent::BufferDesc CBDesc;
		CBDesc.Name = "rawrbox::Camera::Uniforms";
		CBDesc.Usage = Diligent::USAGE_DEFAULT;
		CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
		CBDesc.Size = sizeof(rawrbox::CameraUniforms);

		device->CreateBuffer(CBDesc, nullptr, &uniforms);
		// ------------

		// BARRIER -----
		rawrbox::BarrierUtils::barrier({{staticUniforms, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {uniforms, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		// -----------

		this->_logger->info("Initializing camera buffers");
	}

	void CameraBase::updateMtx() { RAWRBOX_CRITICAL("Not implemented"); };
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

	rawrbox::Vector3f CameraBase::worldToScreen(const rawrbox::Vector3f& /*pos*/) const {
		RAWRBOX_CRITICAL("Not implemented");
	}

	rawrbox::Vector3f CameraBase::screenToWorld(const rawrbox::Vector2f& /*screen_pos*/, const rawrbox::Vector3f& /*origin*/) const {
		RAWRBOX_CRITICAL("Not implemented");
	}

	bool CameraBase::isEnabled() const { return this->_enabled; }
	void CameraBase::setEnabled(bool enabled) { this->_enabled = enabled; }

	uint32_t CameraBase::getLayers() const { return this->_layers; }
	void CameraBase::setLayers(uint32_t layers) { this->_layers = layers; }
	bool CameraBase::shouldRenderLayer(uint32_t layer) const { return (this->_layers & layer) > 1; }
	// -----------

	// RENDER TARGET ----
	void CameraBase::begin() {
		if (this->_renderTarget == nullptr) RAWRBOX_CRITICAL("Render target not initialized!");
		this->_renderTarget->startRecord();
	}

	void CameraBase::end() {
		if (this->_renderTarget == nullptr) RAWRBOX_CRITICAL("Render target not initialized!");
		this->_renderTarget->stopRecord();
	}

	Diligent::ITextureView* CameraBase::getDepth() const { return this->_renderTarget->getDepth(); }
	Diligent::ITextureView* CameraBase::getColor(bool rt) const { return rt ? this->_renderTarget->getRT() : this->_renderTarget->getHandle(); }

	rawrbox::TextureRender* CameraBase::getRenderTarget() const { return this->_renderTarget.get(); }
	// ----------------

	void CameraBase::updateBuffer() {
		if (uniforms == nullptr) RAWRBOX_CRITICAL("Buffer not initialized! Did you call initialize?");

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

		rawrbox::BarrierUtils::barrier({{uniforms, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		rawrbox::RENDERER->context()->UpdateBuffer(uniforms, 0, sizeof(rawrbox::CameraUniforms), &data, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
		rawrbox::BarrierUtils::barrier({{uniforms, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
	}

	void CameraBase::update() {}
	// ----------------

} // namespace rawrbox
