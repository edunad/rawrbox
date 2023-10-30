#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/texture/base.hpp>

namespace rawrbox {
	class TextureRender : public rawrbox::TextureBase {
	private:
		Diligent::RefCntAutoPtr<Diligent::ITextureView> _rtHandle;
		Diligent::RefCntAutoPtr<Diligent::ITextureView> _depthHandle;
		Diligent::RefCntAutoPtr<Diligent::ITexture> _depthTex;

		rawrbox::Vector2i _size = {};
		bool _depth = true;
		bool _recording = false;

	public:
		explicit TextureRender(const rawrbox::Vector2i& size, bool depth = true);

		TextureRender(TextureRender&&) = delete;
		TextureRender& operator=(TextureRender&&) = delete;
		TextureRender(const TextureRender&) = delete;
		TextureRender& operator=(const TextureRender&) = delete;

		~TextureRender() override;

		// ------UTILS
		[[nodiscard]] virtual Diligent::ITextureView* getDepth() const;
		[[nodiscard]] virtual Diligent::ITextureView* getRT() const;
		// ------------

		// ------RENDER
		virtual void startRecord(bool clear = true);
		virtual void stopRecord();

		void upload(Diligent::TEXTURE_FORMAT format = Diligent::TEXTURE_FORMAT::TEX_FORMAT_UNKNOWN, bool dynamic = false) override;
		//  --------------------
	};
} // namespace rawrbox
