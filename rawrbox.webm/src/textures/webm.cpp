

#include <rawrbox/render/static.hpp>
#include <rawrbox/utils/path.hpp>
#include <rawrbox/utils/time.hpp>
#include <rawrbox/webm/textures/webm.hpp>

#include <fmt/format.h>

namespace rawrbox {
	TextureWEBM::TextureWEBM(const std::filesystem::path& filePath, uint32_t flags, bool useFallback) : rawrbox::TextureAnimatedBase(filePath, useFallback), _flags(flags) { this->internalLoad({}, useFallback); }
	TextureWEBM::~TextureWEBM() {
		this->_webm.reset();
	}

	void TextureWEBM::internalLoad(const std::vector<uint8_t>& /*buffer*/, bool useFallback) { // buffer not supported on webm :(
		this->_name = "RawrBox::Texture::WEBM";

		try {
			if (!std::filesystem::exists(this->_filePath)) throw std::runtime_error("Video not found!");

			this->_webm = std::make_unique<rawrbox::WEBM>();
			this->_webm->load(this->_filePath, this->_flags);
			this->_webm->setLoop(this->_loop);
			this->_webm->setPaused(this->_pause);
			this->_webm->onEnd += [this]() { this->onEnd(); };

			this->_channels = 4; // Force 4 channels
			this->_size = this->_webm->getSize();
		} catch (std::runtime_error err) {
			if (useFallback) {
				fmt::print("[RawrBox-TextureWEBM] Failed to load '{}' ──> {}\n  └── Loading fallback texture!\n", this->_filePath.generic_string(), err.what());
				this->loadFallback();
				return;
			}

			throw err;
		}
	}

	void TextureWEBM::update() {
		if (this->_failedToLoad || this->_handle == nullptr) return; // Not bound
		if (this->_pause || this->_cooldown >= rawrbox::TimeUtils::curtime()) return;

		if (this->_webm == nullptr) throw std::runtime_error("[RawrBox-TextureWEBM] WEBM loader not initialized!");

		rawrbox::WEBMImage img;
		if (!this->_webm->getNextFrame(img)) return; // Reached end

		this->_pixels = img.pixels;
		this->_size = img.size;
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
		throw std::runtime_error("[RawrBox-TextureWEBM] Not supported");
	}

	void TextureWEBM::setSpeed(float /*speed*/) {
		throw std::runtime_error("[RawrBox-TextureWEBM] Not supported");
	}
	// ----

	// RENDER ------
	void TextureWEBM::upload(Diligent::TEXTURE_FORMAT /*format*/, bool /*dynamic*/) {
		if (this->_failedToLoad || this->_handle != nullptr) return; // Failed texture is already bound, so skip it
		rawrbox::TextureBase::upload(this->_sRGB ? Diligent::TEX_FORMAT_BGRA8_UNORM_SRGB : Diligent::TEX_FORMAT_BGRA8_UNORM, true);
	}
	// --------------------
} // namespace rawrbox
