
#include <rawrbox/engine/static.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/barrier.hpp>
#include <rawrbox/webm/textures/webm.hpp>

namespace rawrbox {
	TextureWEBM::TextureWEBM(const std::filesystem::path& filePath, uint32_t flags, bool useFallback) : rawrbox::TextureAnimatedBase(filePath, useFallback), _flags(flags) { this->internalLoad({}, useFallback); }
	TextureWEBM::~TextureWEBM() {
		this->_webm.reset();
	}

	// PRIVATE ----
	void TextureWEBM::internalLoad(const std::vector<uint8_t>& /*buffer*/, bool useFallback) {
		this->_name = "RawrBox::Texture::WEBM";

		try {
			if (!std::filesystem::exists(this->_filePath)) RAWRBOX_CRITICAL("Video not found!");

			this->_webm = std::make_unique<rawrbox::WEBM>();
			this->_webm->load(this->_filePath, this->_flags);
			this->_webm->setLoop(this->_loop);
			this->_webm->setPaused(this->_pause);
			this->_webm->setSpeed(this->_speed);
			this->_webm->onEnd += [this]() { this->onEnd(); };

			const auto& image = this->_webm->getImage();

			this->_data.channels = 4;
			this->_data.size = image.valid() ? image.size : this->_webm->getSize();
			this->_data.clearFrames();

			if (image.valid()) {
				this->_data.createFrame(image.pixels);
			} else {
				this->_data.createFrame();
			}
		} catch (const std::exception& e) {
			if (useFallback) {
				this->_webm.reset();
				this->_logger->warn("Failed to load '{}' ──> \n\t{}\n\t\t  └── Loading fallback texture!", this->_filePath.generic_string(), e.what());
				this->loadFallback();
				return;
			}

			throw;
		}
	}

	void TextureWEBM::internalUpdate() {
		auto* context = rawrbox::RENDERER->context();

		Diligent::Box UpdateBox;
		UpdateBox.MinX = 0;
		UpdateBox.MinY = 0;
		UpdateBox.MaxX = this->_data.size.x;
		UpdateBox.MaxY = this->_data.size.y;

		Diligent::TextureSubResData SubresData;
		SubresData.Stride = this->_data.size.x * this->_data.channels;
		SubresData.pData = this->_data.pixels().data();

		rawrbox::BarrierUtils::barrier({{this->_tex, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		context->UpdateTexture(this->_tex, 0, 0, UpdateBox, SubresData, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
		rawrbox::BarrierUtils::barrier({{this->_tex, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
	}

	// ---------------

	// PUBLIC --------
	void TextureWEBM::update() {
		if (this->_failedToLoad || this->_handle == nullptr) return;
		if (this->_webm == nullptr) RAWRBOX_CRITICAL("WEBM loader not initialized!");

		if (!this->_started) {
			this->_started = true;
			return;
		}

		if (!this->_webm->update(rawrbox::DELTA_TIME)) return;

		const auto& image = this->_webm->getImage();
		if (!image.valid()) return;

		if (image.size != this->_data.size) this->resize(image.size);
		this->_data.pixels() = image.pixels;

		this->internalUpdate();
	}

	// UTILS ------
	void TextureWEBM::seek(uint64_t timeMS) {
		if (this->_webm == nullptr) return;
		this->_webm->seek(timeMS);
	}

	void TextureWEBM::reset() {
		if (this->_webm == nullptr) return;

		this->_pause = false;
		this->_webm->reset();
	}

	bool TextureWEBM::getLoop() const {
		if (this->_webm == nullptr) return this->_loop;
		return this->_webm->getLoop();
	}

	void TextureWEBM::setLoop(bool loop) {
		this->_loop = loop;
		if (this->_webm != nullptr) this->_webm->setLoop(loop);
	}

	bool TextureWEBM::getPaused() const {
		if (this->_webm == nullptr) return this->_pause;
		return this->_webm->getPaused();
	}

	void TextureWEBM::setPaused(bool paused) {
		this->_pause = paused;
		if (this->_webm != nullptr) this->_webm->setPaused(paused);
	}

	float TextureWEBM::getSpeed() const {
		if (this->_webm == nullptr) return this->_speed;
		return this->_webm->getSpeed();
	}

	void TextureWEBM::setSpeed(float speed) {
		this->_speed = speed;
		if (this->_webm != nullptr) this->_webm->setSpeed(speed);
	}

	uint32_t TextureWEBM::total() const {
		if (this->_webm == nullptr) return 0;
		return static_cast<uint32_t>(this->_webm->getInfo().frames);
	}

	const rawrbox::WEBMInfo& TextureWEBM::getInfo() const {
		if (this->_webm == nullptr) RAWRBOX_CRITICAL("WEBM loader not initialized!");
		return this->_webm->getInfo();
	}

	uint64_t TextureWEBM::getTime() const {
		if (this->_webm == nullptr) return 0;
		return this->_webm->getTime();
	}
	// ----

	// RENDER ------
	void TextureWEBM::upload(Diligent::TEXTURE_FORMAT /*format*/, bool /*dynamic*/) {
		if (this->_failedToLoad || this->_handle != nullptr) return;

		rawrbox::TextureBase::upload(this->_sRGB ? Diligent::TEX_FORMAT_BGRA8_UNORM_SRGB : Diligent::TEX_FORMAT_BGRA8_UNORM, true);
		this->_transparent = this->_webm != nullptr && this->_webm->getInfo().alpha;
	}
	// --------------------
	// ---------
} // namespace rawrbox
