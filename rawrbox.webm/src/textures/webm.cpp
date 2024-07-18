

#include <rawrbox/render/static.hpp>
#include <rawrbox/utils/path.hpp>
#include <rawrbox/utils/time.hpp>
#include <rawrbox/webm/textures/webm.hpp>

namespace rawrbox {
	TextureWEBM::TextureWEBM(const std::filesystem::path& filePath, uint32_t flags, bool useFallback) : rawrbox::TextureAnimatedBase(filePath, useFallback), _flags(flags) { this->internalLoad({}, useFallback); }
	TextureWEBM::~TextureWEBM() {
		this->_webm.reset();
	}

	// PRIVATE ----
	void TextureWEBM::internalLoad(const std::vector<uint8_t>& /*buffer*/, bool useFallback) { // buffer not supported on webm :(
		this->_name = "RawrBox::Texture::WEBM";

		try {
			if (!std::filesystem::exists(this->_filePath)) throw this->_logger->error("Video not found!");

			this->_webm = std::make_unique<rawrbox::WEBM>();
			this->_webm->load(this->_filePath, this->_flags);
			this->_webm->setLoop(this->_loop);
			this->_webm->setPaused(this->_pause);
			this->_webm->onEnd += [this]() { this->onEnd(); };

			this->_data.channels = 4; // Force 4 channels
			this->_data.size = this->_webm->getSize();
			this->_data.createFrame();

		} catch (const std::runtime_error& err) {
			if (useFallback) {
				_logger->warn("Failed to load '{}' ──> {}\n  └── Loading fallback texture!", this->_filePath.generic_string(), err.what());
				this->loadFallback();
				return;
			}

			throw err;
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
		if (this->_failedToLoad || this->_handle == nullptr) return; // Not bound
		if (this->_pause || this->_cooldown >= rawrbox::TimeUtils::curtime()) return;
		if (this->_webm == nullptr) throw this->_logger->error("WEBM loader not initialized!");

		rawrbox::WEBMImage img;
		if (!this->_webm->getNextFrame(img)) return; // Reached end

		std::memcpy(this->_data.pixels().data(), img.pixels.data(), img.pixels.size());
		this->_cooldown = rawrbox::TimeUtils::curtime() + 20; // TODO: FIX TIME SCALE

		this->internalUpdate();
	}

	// UTILS ------
	void TextureWEBM::seek(uint64_t timeMS) {
		if (this->_webm == nullptr) return;
		this->_webm->seek(timeMS);
	}

	void TextureWEBM::reset() {
		if (this->_webm == nullptr) return;

		this->_webm->reset();
		this->_cooldown = rawrbox::TimeUtils::curtime() + 20;
	}

	bool TextureWEBM::getLoop() const {
		if (this->_webm == nullptr) return false;
		return this->_webm->getLoop();
	}

	void TextureWEBM::setLoop(bool loop) {
		if (this->_webm == nullptr) return;
		this->_webm->setLoop(loop);
	}

	bool TextureWEBM::getPaused() const {
		if (this->_webm == nullptr) return false;
		return this->_webm->getPaused();
	}

	void TextureWEBM::setPaused(bool paused) {
		if (this->_webm == nullptr) return;
		this->_webm->setPaused(paused);
	}

	float TextureWEBM::getSpeed() const {
		throw this->_logger->error("Not supported");
	}

	void TextureWEBM::setSpeed(float /*speed*/) {
		throw this->_logger->error("Not supported");
	}
	// ----

	// RENDER ------
	void TextureWEBM::upload(Diligent::TEXTURE_FORMAT /*format*/, bool /*dynamic*/) {
		if (this->_failedToLoad || this->_handle != nullptr) return; // Failed texture is already bound, so skip it
		rawrbox::TextureBase::upload(this->_sRGB ? Diligent::TEX_FORMAT_BGRA8_UNORM_SRGB : Diligent::TEX_FORMAT_BGRA8_UNORM, true);
	}
	// --------------------
	// ---------
} // namespace rawrbox
