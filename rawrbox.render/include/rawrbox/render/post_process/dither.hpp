#pragma once
#include <rawrbox/math/vector4.hpp>
#include <rawrbox/render/post_process/base.hpp>

#include <RefCntAutoPtr.hpp>

#include <Buffer.h>

namespace rawrbox {
	enum class DITHER_MODE {
		SLOW_MODE = 0,
		FAST_MODE = 1
	};

	class PostProcessDither : public rawrbox::PostProcessBase {
		rawrbox::DITHER_MODE _mode = rawrbox::DITHER_MODE::SLOW_MODE;

	public:
		explicit PostProcessDither(rawrbox::DITHER_MODE dither = rawrbox::DITHER_MODE::SLOW_MODE);
		PostProcessDither(PostProcessDither&&) = delete;
		PostProcessDither& operator=(PostProcessDither&&) = delete;
		PostProcessDither(const PostProcessDither&) = delete;
		PostProcessDither& operator=(const PostProcessDither&) = delete;
		~PostProcessDither() override = default;

		virtual void setIntensity(float in);
		virtual void setColorDepth(float dep);
		virtual void setThreshold(float th);
		virtual void setMode(rawrbox::DITHER_MODE mode);

		void init() override;
	};
} // namespace rawrbox
