#pragma once

#include <rawrbox/render/texture/base.hpp>
#include <rawrbox/render/webm/loader.hpp>

#include <filesystem>

namespace rawrbox {
	class TextureWEBM : public TextureBase {
	private:
		std::filesystem::path _filePath = "";
		std::unique_ptr<rawrbox::WEBM> _webm = nullptr;

		/*
		int _currentFrame = 0;
		int _maxFrames = 0;
		bool _loop = true;

		int64_t _cooldown = 0;
		float _speed = 1.F;*/

		uint32_t _trackId = 0;

		void update();
		void internalLoad();

	public:
		explicit TextureWEBM(const std::filesystem::path& filePath, uint32_t videoTrack = 0);

		TextureWEBM(const TextureWEBM&) = delete;
		TextureWEBM(TextureWEBM&&) = delete;
		TextureWEBM& operator=(const TextureWEBM&) = delete;
		TextureWEBM& operator=(TextureWEBM&&) = delete;
		~TextureWEBM() override;

		void step();

		// ------RENDER
		void upload(bgfx::TextureFormat::Enum format = bgfx::TextureFormat::RGBA8) override;
		// --------------------
	};
} // namespace rawrbox
