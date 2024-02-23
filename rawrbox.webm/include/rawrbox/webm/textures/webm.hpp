#pragma once

#include <rawrbox/render/textures/animated.hpp>
#include <rawrbox/utils/event.hpp>
#include <rawrbox/webm/loader.hpp>

#include <filesystem>

namespace rawrbox {
	class TextureWEBM : public rawrbox::TextureAnimatedBase {
	private:
		uint32_t _flags = 0;
		std::unique_ptr<rawrbox::WEBM> _webm = nullptr;

		void internalLoad(const std::vector<uint8_t>& data, bool useFallback = true) override;

	public:
		rawrbox::Event<> onEnd;

		explicit TextureWEBM(const std::filesystem::path& filePath, uint32_t flags = 0, bool useFallback = true);
		TextureWEBM(const TextureWEBM&) = delete;
		TextureWEBM(TextureWEBM&&) = delete;
		TextureWEBM& operator=(const TextureWEBM&) = delete;
		TextureWEBM& operator=(TextureWEBM&&) = delete;
		~TextureWEBM() override;

		void update() override;

		// UTILS ------
		void seek(uint64_t timeMS);
		void reset() override;

		bool getLoop() override;
		void setLoop(bool loop) override;

		bool getPaused() override;
		void setPaused(bool paused) override;

		float getSpeed() override;
		void setSpeed(float speed) override;
		// ----

		// ------RENDER
		void upload(Diligent::TEXTURE_FORMAT format = Diligent::TEXTURE_FORMAT::TEX_FORMAT_UNKNOWN, bool dynamic = false) override;
		// --------------------
	};
} // namespace rawrbox
