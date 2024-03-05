#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/textures/base.hpp>
#include <rawrbox/render/utils/barrier.hpp>

namespace rawrbox {

	class TextureRender : public rawrbox::TextureBase {
	private:
		// Custom render target views ---
		std::vector<Diligent::RefCntAutoPtr<Diligent::ITexture>> _textures = {};
		std::vector<Diligent::ITextureView*> _views = {};
		std::vector<Diligent::ITextureView*> _viewsRT = {};
		// -------------------------------

		Diligent::ITextureView* _depthRTHandle = nullptr;
		Diligent::ITextureView* _depthHandle = nullptr;

		Diligent::RefCntAutoPtr<Diligent::ITexture> _depthTex;

		// BARRIERS ---
		std::vector<rawrbox::Barrier<Diligent::ITexture>> _barrierRead = {};
		std::vector<rawrbox::Barrier<Diligent::ITexture>> _barrierWrite = {};
		// -------------

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
		[[nodiscard]] virtual Diligent::ITextureView* getRTDepth() const;
		[[nodiscard]] virtual Diligent::ITextureView* getRT() const;

		[[nodiscard]] virtual Diligent::ITextureView* getView(size_t index) const;
		[[nodiscard]] virtual Diligent::ITextureView* getViewRT(size_t index) const;

		[[nodiscard]] virtual Diligent::ITexture* getDepthHandle() const;

		[[nodiscard]] Diligent::ITexture* getTexture(size_t index) const;

		[[nodiscard]] Diligent::ITexture* getTexture() const override;
		[[nodiscard]] Diligent::ITextureView* getHandle() const override;
		// ------------

		// ------RENDER
		virtual void startRecord(bool clear = true, size_t renderTargets = 0);
		virtual void stopRecord();

		virtual size_t addTexture(Diligent::TEXTURE_FORMAT format, Diligent::BIND_FLAGS flags, Diligent::USAGE usage = Diligent::USAGE_DEFAULT, Diligent::CPU_ACCESS_FLAGS cpu = Diligent::CPU_ACCESS_NONE);
		virtual void addView(size_t index, Diligent::TEXTURE_VIEW_TYPE format = Diligent::TEXTURE_VIEW_SHADER_RESOURCE);

		void upload(Diligent::TEXTURE_FORMAT format = Diligent::TEXTURE_FORMAT::TEX_FORMAT_UNKNOWN, bool dynamic = false) override;
		//  --------------------
	};
} // namespace rawrbox
