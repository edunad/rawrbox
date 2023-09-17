#pragma once

#include <rawrbox/render/texture/base.hpp>
#include <rawrbox/utils/event.hpp>
#include <rawrbox/webm/loader.hpp>

#include <filesystem>
#include <memory>
#include <thread>
#include <vector>

namespace rawrbox {
	class TextureWEBM : public rawrbox::TextureBase {
	private:
		std::filesystem::path _filePath = "";
		uint32_t _flags = 0;

		std::unique_ptr<rawrbox::WEBM> _webm = nullptr;
		uint64_t _cooldown = 0;

		void internalUpdate();
		void internalLoad();

	public:
		rawrbox::Event<> onEnd;

		explicit TextureWEBM(const std::filesystem::path& filePath, uint32_t flags = 0);
		TextureWEBM(const TextureWEBM&) = delete;
		TextureWEBM(TextureWEBM&&) = delete;
		TextureWEBM& operator=(const TextureWEBM&) = delete;
		TextureWEBM& operator=(TextureWEBM&&) = delete;
		~TextureWEBM() override;

		void update() override;

		// UTILS ------
		bool getLoop();
		void setLoop(bool loop);

		bool getPaused();
		void setPaused(bool paused);

		void seek(uint64_t timeMS);
		void reset();
		// ----

		// ------RENDER
		void upload(bgfx::TextureFormat::Enum format = bgfx::TextureFormat::BGRA8) override;
		// --------------------
	};
} // namespace rawrbox
