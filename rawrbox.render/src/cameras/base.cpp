
#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/cameras/base.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/barrier.hpp>

namespace rawrbox {
	Diligent::RefCntAutoPtr<Diligent::IBuffer> CameraBase::staticUniforms;
	Diligent::RefCntAutoPtr<Diligent::IBuffer> CameraBase::uniforms;
	rawrbox::CameraStaticUniforms CameraBase::_uploadedStatic = {};

	CameraBase::CameraBase(const rawrbox::Vector2u& renderSize, bool depth) {
		this->_renderTarget = std::make_unique<rawrbox::TextureRender>(renderSize, depth);
	}

	CameraBase::~CameraBase() { this->_renderTarget.reset(); }

	void CameraBase::shutdown() {
		RAWRBOX_DESTROY(uniforms);
		RAWRBOX_DESTROY(staticUniforms);
	}

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

		// Shared across all the cameras
		if (staticUniforms != nullptr || uniforms != nullptr) RAWRBOX_CRITICAL("Camera buffers already initialized!");
		_uploadedStatic = this->getStaticData();

		// STATIC BUFFER ---
		Diligent::BufferDesc StaticDesc;
		StaticDesc.Name = "rawrbox::Camera::Static::Uniforms";
		StaticDesc.Usage = Diligent::USAGE_DEFAULT;
		StaticDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
		StaticDesc.Size = sizeof(rawrbox::CameraStaticUniforms);

		Diligent::BufferData sData;
		sData.DataSize = StaticDesc.Size;
		sData.pData = &_uploadedStatic;

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

	void CameraBase::updateStatic() {
		if (staticUniforms == nullptr) return;

		const auto data = this->getStaticData();
		if (data == _uploadedStatic) return; // Shared across all the cameras
		_uploadedStatic = data;

		rawrbox::BarrierUtils::barrier({{staticUniforms, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		rawrbox::RENDERER->context()->UpdateBuffer(staticUniforms, 0, sizeof(rawrbox::CameraStaticUniforms), &data, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
		rawrbox::BarrierUtils::barrier({{staticUniforms, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
	}

	rawrbox::CameraStaticUniforms CameraBase::getStaticData() {
		const auto targetSize = this->_renderTarget->getSize().cast<float>();

		rawrbox::CameraStaticUniforms data = {};
		data.gProjection = rawrbox::Matrix4x4::mtxTranspose(this->getProjMtx());
		data.gProjectionInv = rawrbox::Matrix4x4::mtxInverse(data.gProjection);
		data.gViewport = {this->getZNear(), this->getZFar(), targetSize.x, targetSize.y};

		return data;
	}

	// UTILS -----
	void CameraBase::setPos(const rawrbox::Vector3f& pos) {
		if (this->_pos == pos) return;
		if ((pos - this->_pos).length() > TELEPORT_DISTANCE) this->_teleportRevision = rawrbox::FRAME;

		this->_pos = pos;
		this->_moveRevision = rawrbox::FRAME;
		this->updateMtx();
	}

	const rawrbox::Vector3f& CameraBase::getPos() const {
		return this->_pos;
	}

	void CameraBase::setAngle(const rawrbox::Vector4f& angle) {
		if (this->_angle == angle) return;

		const auto forward = this->getForward();
		this->_angle = angle;

		if (this->getForward().dot(forward) < TELEPORT_ANGLE) this->_teleportRevision = rawrbox::FRAME;

		this->_moveRevision = rawrbox::FRAME;
		this->updateMtx();
	}

	const rawrbox::Vector4f& CameraBase::getAngle() const {
		return this->_angle;
	}

	uint64_t CameraBase::getMoveRevision() const { return this->_moveRevision; }
	uint64_t CameraBase::getTeleportRevision() const { return this->_teleportRevision; }

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
#ifdef _DEBUG
		if (this->_cullFrozen) return this->_cullMtx;
#endif
		return this->_projection * this->_view;
	}

#ifdef _DEBUG
	void CameraBase::setCullFrozen(bool frozen) {
		this->_cullFrozen = frozen;
		this->_cullMtx = this->_projection * this->_view;
	}

	bool CameraBase::isCullFrozen() const { return this->_cullFrozen; }
#endif

	rawrbox::Vector3f CameraBase::worldToScreen(const rawrbox::Vector3f& /*pos*/) const {
		RAWRBOX_CRITICAL("Not implemented");
	}

	rawrbox::Vector3f CameraBase::screenToWorld(const rawrbox::Vector2f& /*screen_pos*/, const rawrbox::Vector3f& /*origin*/) const {
		RAWRBOX_CRITICAL("Not implemented");
	}

	rawrbox::Vector3f CameraBase::screenRayDir(const rawrbox::Vector2f& /*screen_pos*/) const {
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

	void CameraBase::resize(const rawrbox::Vector2u& size) {
		if (this->_renderTarget == nullptr) RAWRBOX_CRITICAL("Render target not initialized!");
		if (size == this->_renderTarget->getSize()) return;

		this->_renderTarget->resize(size);
		if (rawrbox::RENDERER != nullptr) rawrbox::RENDERER->resizeCamera(*this);
	}

	Diligent::ITextureView* CameraBase::getDepth() const { return this->_renderTarget->getDepth(); }
	Diligent::ITextureView* CameraBase::getColor(bool rt) const { return rt ? this->_renderTarget->getRT() : this->_renderTarget->getHandle(); }

	rawrbox::TextureRender* CameraBase::getRenderTarget() const { return this->_renderTarget.get(); }
	// ----------------

	// PRESENT ----
	void CameraBase::setPresent(const rawrbox::Vector2i& offset, uint32_t scale) {
		this->_presentOffset = offset;
		this->_presentScale = scale;
	}

	rawrbox::AABBf CameraBase::getPresentRect() const {
		if (this->_presentScale == 0U) return {{}, rawrbox::RENDERER->getSize().cast<float>()};
		return {this->_presentOffset.cast<float>(), (this->_renderTarget->getSize() * this->_presentScale).cast<float>()};
	}

	rawrbox::Vector2f CameraBase::windowToTarget(const rawrbox::Vector2f& windowPos) const {
		const auto rect = this->getPresentRect();
		return (windowPos + rawrbox::Vector2f{0.5F, 0.5F} - rect.pos) * this->_renderTarget->getSize().cast<float>() / rect.size;
	}

	rawrbox::Vector2f CameraBase::targetToWindow(const rawrbox::Vector2f& targetPos) const {
		const auto rect = this->getPresentRect();
		return (targetPos * rect.size / this->_renderTarget->getSize().cast<float>()) + rect.pos - rawrbox::Vector2f{0.5F, 0.5F};
	}
	// ----------------

	void CameraBase::updateBuffer() {
		if (uniforms == nullptr) RAWRBOX_CRITICAL("Buffer not initialized! Did you call initialize?");

		auto view = rawrbox::Matrix4x4::mtxTranspose(this->getViewMtx());
		auto viewInv = rawrbox::Matrix4x4::mtxInverse(this->getViewMtx());
		auto projection = rawrbox::Matrix4x4::mtxTranspose(this->getProjMtx());

		rawrbox::CameraUniforms data = {};
		data.gView = view;
		data.gViewInv = viewInv;
		data.gViewProj = data.gView * projection;
		data.gPos = this->getPos();
		data.gDeltaTime = rawrbox::DELTA_TIME;

		rawrbox::BarrierUtils::barrier({{uniforms, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		rawrbox::RENDERER->context()->UpdateBuffer(uniforms, 0, sizeof(rawrbox::CameraUniforms), &data, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
		rawrbox::BarrierUtils::barrier({{uniforms, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
	}

	void CameraBase::update() {}
	// ----------------

} // namespace rawrbox