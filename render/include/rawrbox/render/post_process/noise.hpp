#pragma once
#include <rawrbox/math/vector4.hpp>
#include <rawrbox/render/post_process/base.hpp>

#include <RefCntAutoPtr.hpp>

#include <Buffer.h>

namespace rawrbox {

	struct NoiseSettings {
		rawrbox::Vector4f g_ScreenSize = {};

		float g_NoiseTimer = 0.0F;
		float g_NoiseIntensity = 0.0F;
	};

	class PostProcessNoise : public rawrbox::PostProcessBase {
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _uniforms;
		rawrbox::NoiseSettings _settings = {};

		void updateUniforms();

	public:
		explicit PostProcessNoise(float intensity);
		PostProcessNoise(PostProcessNoise&&) = delete;
		PostProcessNoise& operator=(PostProcessNoise&&) = delete;
		PostProcessNoise(const PostProcessNoise&) = delete;
		PostProcessNoise& operator=(const PostProcessNoise&) = delete;
		~PostProcessNoise() override;

		virtual void setIntensity(float in);

		void upload() override;
		void applyEffect(Diligent::ITextureView* texture) override;
	};
} // namespace rawrbox
