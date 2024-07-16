#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/textures/base.hpp>
#include <rawrbox/utils/event.hpp>

#include <filesystem>
#include <vector>

namespace rawrbox {

	class TextureAnimatedBase : public rawrbox::TextureBase {
	protected:
		std::filesystem::path _filePath = "";

		bool _loop = true;
		bool _pause = false;

		uint64_t _cooldown = 0;
		float _speed = 1.F;

		virtual void internalLoad(const std::vector<uint8_t>& data, bool useFallback = true);

	public:
		rawrbox::Event<> onEnd;

		explicit TextureAnimatedBase(const std::filesystem::path& filePath, bool useFallback = true);
		explicit TextureAnimatedBase(const std::filesystem::path& filePath, const std::vector<uint8_t>& buffer, bool useFallback = true);

		void update() override;

		// ANIMATION
		virtual void reset();
		// --------------------

		// UTILS -----------
		[[nodiscard]] virtual bool getLoop() const;
		virtual void setLoop(bool loop);
		[[nodiscard]] virtual bool getPaused() const;
		virtual void setPaused(bool paused);
		[[nodiscard]] virtual float getSpeed() const;
		virtual void setSpeed(float speed);

		[[nodiscard]] virtual uint32_t total() const;
		// --------------------

		[[nodiscard]] bool requiresUpdate() const override;
	};
} // namespace rawrbox
