#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/texture/base.hpp>

#include <bgfx/bgfx.h>

#include <filesystem>
#include <string>
#include <vector>

namespace rawrbox {
	struct GIFFrame {
		int delay;
		std::vector<uint8_t> pixels;
	};

	class TextureGIF : public TextureBase {
	private:
		std::vector<GIFFrame> _frames;
		std::filesystem::path _filePath = "";

		int _currentFrame = 0;
		bool _loop = true;
		bool _failedToLoad = false;
		bool _transparent = false;

		int64_t _cooldown = 0;
		float _speed = 1.F;

		void internalUpdate();
		void internalLoad(const std::vector<uint8_t>& data, bool useFallback = true);

	public:
		explicit TextureGIF(const std::filesystem::path& filePath, bool useFallback = true);
		explicit TextureGIF(const std::filesystem::path& filePath, const std::vector<uint8_t>& buffer, bool useFallback = true);

		void update() override;

		// ANIMATION
		virtual void reset();
		// --------------------

		// ------UTILS
		virtual void setLoop(bool loop);
		virtual void setSpeed(float speed);
		[[nodiscard]] bool hasTransparency() const override;
		// --------------------

		// ------RENDER
		void upload(bgfx::TextureFormat::Enum format = bgfx::TextureFormat::RGBA8) override;
		// --------------------
	};
} // namespace rawrbox
