#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/texture/base.hpp>
#include <rawrbox/utils/event.hpp>

#include <filesystem>
#include <vector>

namespace rawrbox {
	struct Frame {
		float delay;
		std::vector<uint8_t> pixels;
	};

	class TextureAnimatedBase : public rawrbox::TextureBase {
	protected:
		std::vector<rawrbox::Frame> _frames;
		std::filesystem::path _filePath = "";

		int _currentFrame = 0;

		bool _loop = true;
		bool _pause = false;

		uint64_t _cooldown = 0;
		float _speed = 1.F;

		virtual void internalUpdate();
		virtual void internalLoad(const std::vector<uint8_t>& data, bool useFallback = true);

	public:
		rawrbox::Event<> onEnd;

		explicit TextureAnimatedBase(const std::filesystem::path& filePath, bool useFallback = true);
		explicit TextureAnimatedBase(const std::filesystem::path& filePath, const std::vector<uint8_t>& buffer, bool useFallback = true);

		void update() override;

		// ANIMATION
		virtual void reset();
		// --------------------

		// ------UTILS
		virtual bool getLoop();
		virtual void setLoop(bool loop);
		virtual bool getPaused();
		virtual void setPaused(bool paused);
		virtual bool getSpeed();
		virtual void setSpeed(float speed);
		// --------------------

		// ------RENDER
		void upload(Diligent::TEXTURE_FORMAT format = Diligent::TEXTURE_FORMAT::TEX_FORMAT_UNKNOWN, bool dynamic = false) override;
		// --------------------
	};
} // namespace rawrbox
