
#include <rawrbox/utils/time.hpp>
#include <rawrbox/webm/textures/webm.hpp>

#include <fmt/format.h>

#include <memory>
#include <thread>

namespace rawrbox {
	// NOLINTBEGIN(modernize-pass-by-value)
	TextureWEBM::TextureWEBM(const std::filesystem::path& filePath, uint32_t flags) : _filePath(filePath), _flags(flags) {
		this->internalLoad();
	}
	// NOLINTEND(modernize-pass-by-value)

	void TextureWEBM::internalLoad() {
		this->_webm = std::make_unique<rawrbox::WEBM>();
		this->_webm->load(this->_filePath, this->_flags);
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

		rawrbox::WEBMImage img;
		bool success = this->_webm->getNextFrame(img);
		if (!success) return;

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
		this->_cooldown = rawrbox::TimeUtils::curtime() + 20;
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
