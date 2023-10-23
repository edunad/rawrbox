#pragma once
#include <rawrbox/math/vector4.hpp>
#include <rawrbox/render/post_process/base.hpp>

#include <Common/interface/RefCntAutoPtr.hpp>

#include <Graphics/GraphicsEngine/interface/Buffer.h>

namespace rawrbox {
	enum class DITHER_MODE {
		SLOW_MODE = 0,
		FAST_MODE = 1
	};

	struct DitherSettings {
		rawrbox::Vector4f g_ScreenSize = {};

		float g_DitherMode = 0.0F;
		float g_DitherIntensity = 0.02F; // intensity * 0.01F
		float g_DitherDepth = 5.F;       // depth
		float g_DitherColorDepth = 32.F; // 2 ^ colorDepth
		float g_DitherThreshold = 1.F;   // threshold
	};

	class PostProcessDither : public rawrbox::PostProcessBase {
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _uniforms;

		rawrbox::DitherSettings _settings = {};
		rawrbox::DITHER_MODE _mode = rawrbox::DITHER_MODE::SLOW_MODE;

		void updateUniforms();

	public:
		explicit PostProcessDither(rawrbox::DITHER_MODE dither = rawrbox::DITHER_MODE::SLOW_MODE);
		PostProcessDither(PostProcessDither&&) = delete;
		PostProcessDither& operator=(PostProcessDither&&) = delete;
		PostProcessDither(const PostProcessDither&) = delete;
		PostProcessDither& operator=(const PostProcessDither&) = delete;
		~PostProcessDither() override;

		virtual void setIntensity(float in);
		virtual void setDepth(float dep);
		virtual void setColorDepth(float dep);
		virtual void setThreshold(float th);
		virtual void setMode(rawrbox::DITHER_MODE mode);

		void upload() override;
		void applyEffect(Diligent::ITextureView* texture) override;
	};
} // namespace rawrbox
