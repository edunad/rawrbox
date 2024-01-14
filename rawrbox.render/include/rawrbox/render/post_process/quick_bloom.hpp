/*#pragma once
#include <rawrbox/math/vector4.hpp>
#include <rawrbox/render/post_process/base.hpp>

#include <RefCntAutoPtr.hpp>

#include <Buffer.h>

namespace rawrbox {

	struct BloomSettings {
		rawrbox::Vector4f g_ScreenSize = {};
		float g_BloomIntensity = 0.0F;
	};

	class PostProcessQuickBloom : public rawrbox::PostProcessBase {
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _uniforms;
		rawrbox::BloomSettings _settings = {};

		void updateUniforms();

	public:
		explicit PostProcessQuickBloom(float intensity);
		PostProcessQuickBloom(PostProcessQuickBloom&&) = delete;
		PostProcessQuickBloom& operator=(PostProcessQuickBloom&&) = delete;
		PostProcessQuickBloom(const PostProcessQuickBloom&) = delete;
		PostProcessQuickBloom& operator=(const PostProcessQuickBloom&) = delete;
		~PostProcessQuickBloom() override;

		virtual void setIntensity(float in);

		void upload() override;
		void applyEffect(Diligent::ITextureView* texture) override;
	};
} // namespace rawrbox
*/
