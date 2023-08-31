
#include <rawrbox/utils/time.hpp>
#include <rawrbox/webm/texture/webm.hpp>

#include <fmt/format.h>

namespace rawrbox {
	// NOLINTBEGIN(modernize-pass-by-value)
	TextureWEBM::TextureWEBM(const std::filesystem::path& filePath, uint32_t videoTrack) : _filePath(filePath), _trackId(videoTrack) {
		this->internalLoad();
	}
	// NOLINTEND(modernize-pass-by-value)

	void TextureWEBM::internalLoad() {
		this->_webm = std::make_unique<rawrbox::WEBM>();
		this->_webm->load(this->_filePath);
		this->_webm->setLoop(true);
		this->_webm->onEnd += [this]() { this->onEnd(); };

		this->_channels = 4;
		this->_size = this->_webm->getSize();
	}

	TextureWEBM::~TextureWEBM() {
		this->_webm.reset();
	}

	void TextureWEBM::internalUpdate() {
		if (this->_webm == nullptr) throw std::runtime_error("[RawrBox-TextureWEBM] WEBM loader not initialized!");

		bool success = this->_webm->advance();
		if (!success) return;

		auto frame = this->_webm->getFrame();
		if (!frame.valid()) throw std::runtime_error("[RawrBox-TextureWEBM] Failed to find frame");

		if (!rawrbox::WEBMDecoder::decode(frame)) throw std::runtime_error("[RawrBox-TextureWEBM] Failed to decode frame");

		rawrbox::WEBMImage img = rawrbox::WEBMDecoder::getImageFrame();
		if (!img.valid()) throw std::runtime_error("[RawrBox-TextureWEBM] Failed to decode frame");

		bgfx::updateTexture2D(this->_handle, 0, 0, 0, 0, static_cast<uint16_t>(this->_size.x), static_cast<uint16_t>(this->_size.y), bgfx::copy(img.pixels.data(), static_cast<uint32_t>(img.pixels.size())));
	}

	void TextureWEBM::update() {
		if (!bgfx::isValid(this->_handle) || this->_cooldown >= rawrbox::TimeUtils::curtime()) return; // Not bound

		this->_cooldown = rawrbox::TimeUtils::curtime() + 20; // TODO: FIX TIME SCALE
		this->internalUpdate();
	}

	// UTILS ----
	bool TextureWEBM::getLoop() {
		if (this->_webm == nullptr) return false;
		return this->_webm->getLoop();
	}

	void TextureWEBM::setLoop(bool loop) {
		if (this->_webm == nullptr) return;
		this->_webm->setLoop(loop);
	}

	bool TextureWEBM::getPaused() {
		if (this->_webm == nullptr) return false;
		return this->_webm->getPaused();
	}

	void TextureWEBM::setPaused(bool paused) {
		if (this->_webm == nullptr) return;
		this->_webm->setPaused(paused);
	}

	void TextureWEBM::seek(uint64_t timeMS) {
		if (this->_webm == nullptr) return;
		this->_webm->seek(timeMS);
	}

	void TextureWEBM::reset() {
		if (this->_webm == nullptr) return;
		this->_webm->reset();
	}
	// ------

	// ------RENDER
	void TextureWEBM::upload(bgfx::TextureFormat::Enum /*format*/) {
		if (bgfx::isValid(this->_handle)) return; // Failed texture is already bound, so skip it
		this->_handle = bgfx::createTexture2D(static_cast<uint16_t>(this->_size.x), static_cast<uint16_t>(this->_size.y), false, 0, bgfx::TextureFormat::BGRA8, 0 | this->_flags);

		if (!bgfx::isValid(this->_handle)) throw std::runtime_error("[TextureWEBM] Failed to bind texture");
		bgfx::setName(this->_handle, fmt::format("RAWR-WEBM-TEXTURE-{}", this->_handle.idx).c_str());
	}
	// --------------------
} // namespace rawrbox
