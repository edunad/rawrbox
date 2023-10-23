#pragma once
#include <rawrbox/math/vector4.hpp>
#include <rawrbox/render/post_process/base.hpp>
#include <rawrbox/render/texture/image.hpp>

#include <Common/interface/RefCntAutoPtr.hpp>

#include <unordered_map>

namespace rawrbox {
	enum class DITHER_MODE {
		SLOW_MODE = 0,
		FAST_MODE = 1
	};

	struct DitherSetings {
		std::array<float, 4> g_ScreenSize = {0, 0, 0, 0};

		float g_DitherMode = 0.0F;
		float g_DitherIntensity = 0.02F; // intensity * 0.01F
		float g_DitherDepth = 5.F;       // depth
		float g_DitherColorDepth = 32.F; // 2 ^ colorDepth
		float g_DitherThreshold = 1.F;   // threshold
	};

	class Dither : public rawrbox::PostProcessBase {
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _uniforms;

		rawrbox::DitherSetings _settings = {};
		rawrbox::DITHER_MODE _mode = rawrbox::DITHER_MODE::SLOW_MODE;

		void updateUniforms();

	public:
		explicit Dither(rawrbox::DITHER_MODE dither = rawrbox::DITHER_MODE::SLOW_MODE);
		Dither(Dither&&) = delete;
		Dither& operator=(Dither&&) = delete;
		Dither(const Dither&) = delete;
		Dither& operator=(const Dither&) = delete;
		~Dither() override;

		virtual void setIntensity(float in);
		virtual void setDepth(float dep);
		virtual void setColorDepth(float dep);
		virtual void setThreshold(float th);
		virtual void setMode(rawrbox::DITHER_MODE mode);

		void upload() override;
		void applyEffect(Diligent::ITextureView* texture) override;
	};
} // namespace rawrbox
