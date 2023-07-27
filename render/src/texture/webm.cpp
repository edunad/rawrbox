
#include <rawrbox/render/texture/webm.hpp>

#include <fmt/format.h>

namespace rawrbox {
	// NOLINTBEGIN(modernize-pass-by-value)
	TextureWEBM::TextureWEBM(const std::filesystem::path& filePath, uint32_t videoTrack) : _filePath(filePath), _trackId(videoTrack) {
		this->internalLoad();
	}
	// NOLINTEND(modernize-pass-by-value)

	// https://github.com/zaps166/libsimplewebm/blob/master/WebMDemuxer.cpp
	void TextureWEBM::internalLoad() {
		this->_channels = 4; // Force 4 channels on videos

		this->_webm = std::make_unique<rawrbox::WEBM>();
		this->_webm->load(this->_filePath);

		this->_webm->getVideoFrame(0);
	}

	TextureWEBM::~TextureWEBM() {
		this->_webm.reset();
	}

	void TextureWEBM::update() {
	}

	void TextureWEBM::step() {
		if (!bgfx::isValid(this->_handle)) return; // Not bound

		/*if (!this->_loop && static_cast<size_t>(this->_currentFrame) >= this->_maxFrames) return;
		if (this->_cooldown >= rawrbox::TimeUtils::curtime()) return;

		this->_cooldown = static_cast<int64_t>(this->_frames[this->_currentFrame].delay * this->_speed) + rawrbox::TimeUtils::curtime(); // TODO: FIX SPEED
		this->_currentFrame = MathUtils::repeat<int>(this->_currentFrame + 1, 0, static_cast<int>(this->_frames.size()) - 1);

		this->update();*/
	}

	// ------RENDER
	void TextureWEBM::upload(bgfx::TextureFormat::Enum format) {
	}
	// --------------------
} // namespace rawrbox
