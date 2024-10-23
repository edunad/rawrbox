
#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/textures/render.hpp>

#include <fmt/format.h>

namespace rawrbox {
	TextureRender::TextureRender(const rawrbox::Vector2u& size, bool depth) : _depth(depth) {
		this->_data.size = size;
		this->_data.channels = 4;

		this->_name = "RawrBox::RenderTarget";
	}

	TextureRender::~TextureRender() {
		this->_views.clear();
		this->_viewsRT.clear();
		this->_textures.clear();

		this->_barrierWrite.clear();
		this->_barrierRead.clear();

		RAWRBOX_DESTROY(this->_depthTex);
	}

	// ------ UTILS
	Diligent::ITextureView* TextureRender::getDepth() const { return this->_depthHandle; }
	Diligent::ITextureView* TextureRender::getRTDepth() const { return this->_depthRTHandle; }
	Diligent::ITextureView* TextureRender::getRT() const { return this->getViewRT(0); }

	Diligent::ITextureView* TextureRender::getView(size_t index) const {
		if (index > this->_views.size()) return nullptr;
		return this->_views[index];
	}

	Diligent::ITextureView* TextureRender::getViewRT(size_t index) const {
		if (index > this->_viewsRT.size()) return nullptr;
		return this->_viewsRT[index];
	}

	Diligent::ITexture* TextureRender::getDepthHandle() const { return this->_depthTex; }
	Diligent::ITexture* TextureRender::getTexture() const { return this->getTexture(0); }

	Diligent::ITextureView* TextureRender::getHandle() const { return this->getView(0); }
	Diligent::ITexture* TextureRender::getTexture(size_t index) const {
		if (index > this->_views.size()) return nullptr;
		return this->_textures[index];
	}
	// ------------

	// ------ RENDER
	void TextureRender::startRecord(bool clear, size_t renderTargets) {
		if (this->_recording) RAWRBOX_CRITICAL("Already recording");
		auto* context = rawrbox::RENDERER->context();

		// BARRIER ----
		rawrbox::BarrierUtils::barrier(this->_barrierWrite);
		//  --------

		size_t totalTargets = renderTargets == 0 ? this->_viewsRT.size() : renderTargets;
		context->SetRenderTargets(static_cast<uint32_t>(totalTargets), this->_viewsRT.data(), this->getRTDepth(), Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);

		if (clear) {
			const std::array<float, 4> ClearColor = {0.0F, 0.0F, 0.0F, 0.0F};
			for (size_t i = 0; i < totalTargets; i++) {
				context->ClearRenderTarget(this->_viewsRT[i], ClearColor.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
			}

			if (this->_depthRTHandle != nullptr) {
				context->ClearDepthStencil(this->getRTDepth(), Diligent::CLEAR_DEPTH_FLAG, 1.0F, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
			}
		}

		this->_recording = true;
	}

	void TextureRender::stopRecord() {
		if (!this->_recording) RAWRBOX_CRITICAL("Not recording");
		auto* pRTV = rawrbox::RENDERER->swapChain()->GetCurrentBackBufferRTV();
		auto* depth = rawrbox::RENDERER->swapChain()->GetDepthBufferDSV();

		rawrbox::RENDERER->context()->SetRenderTargets(1, &pRTV, depth, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY); // Restore

		// BARRIER ----
		rawrbox::BarrierUtils::barrier(this->_barrierRead);
		//  --------

		this->_recording = false;
	}

	size_t TextureRender::addTexture(Diligent::TEXTURE_FORMAT format, Diligent::BIND_FLAGS flags) {
		bool isDepth = (flags & Diligent::BIND_DEPTH_STENCIL) != 0;
		if (isDepth && this->_depthHandle != nullptr) RAWRBOX_CRITICAL("Only one depth texture is allowed");

		std::string name = isDepth ? fmt::format("{}::DEPTH", this->_name) : this->_name;

		Diligent::TextureDesc desc;
		desc.Type = Diligent::RESOURCE_DIM_TEX_2D_ARRAY;
		desc.BindFlags = flags;
		desc.Width = this->_data.size.x;
		desc.Height = this->_data.size.y;
		desc.MipLevels = 1;
		desc.SampleCount = 1;
		desc.Format = format;
		desc.ClearValue.Format = desc.Format;
		desc.Usage = Diligent::USAGE_DEFAULT;
		desc.CPUAccessFlags = Diligent::CPU_ACCESS_NONE;

		if (isDepth) {
			desc.ClearValue.Format = desc.Format;
			desc.ClearValue.DepthStencil.Depth = 1;
			desc.ClearValue.DepthStencil.Stencil = 0;
			desc.Name = name.c_str();
		} else {
			desc.ClearValue.Color[0] = 0.F;
			desc.ClearValue.Color[1] = 0.F;
			desc.ClearValue.Color[2] = 0.F;
			desc.ClearValue.Color[3] = 0.F;
			desc.Name = name.c_str();
		}

		if (isDepth) {
			rawrbox::RENDERER->device()->CreateTexture(desc, nullptr, &this->_depthTex);
			rawrbox::BarrierUtils::barrier({{this->_depthTex, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_DEPTH_READ, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});

			this->_barrierRead.emplace_back(this->_depthTex, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_DEPTH_READ, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);
			this->_barrierWrite.emplace_back(this->_depthTex, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_DEPTH_WRITE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);
		} else {
			Diligent::RefCntAutoPtr<Diligent::ITexture> texture;

			rawrbox::RENDERER->device()->CreateTexture(desc, nullptr, &texture);
			rawrbox::BarrierUtils::barrier({{texture, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});

			this->_barrierRead.emplace_back(texture, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);
			this->_barrierWrite.emplace_back(texture, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_RENDER_TARGET, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);

			this->_textures.push_back(std::move(texture));
		}

		return this->_textures.size() - 1;
	}

	void TextureRender::addView(size_t index, Diligent::TEXTURE_VIEW_TYPE format) {
		auto* tex = this->getTexture(index);
		if (tex == nullptr) RAWRBOX_CRITICAL("Invalid texture index '{}'! Did you call 'addTexture`?", index);

		if (format == Diligent::TEXTURE_VIEW_RENDER_TARGET) {
			this->_viewsRT.push_back(tex->GetDefaultView(format));
		} else {
			this->_views.push_back(tex->GetDefaultView(format));
		}
	}

	void TextureRender::upload(Diligent::TEXTURE_FORMAT format, bool /*dynamic*/) {
		if (format == Diligent::TEXTURE_FORMAT::TEX_FORMAT_UNKNOWN) RAWRBOX_CRITICAL("Invalid format");

		auto view = this->addTexture(format);
		this->addView(view, Diligent::TEXTURE_VIEW_RENDER_TARGET);
		this->addView(view, Diligent::TEXTURE_VIEW_SHADER_RESOURCE);

		// Depth ----
		if (_depth) {
			const auto& swapDesc = rawrbox::RENDERER->swapChain()->GetDesc();

			this->addTexture(swapDesc.DepthBufferFormat, Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_DEPTH_STENCIL);
			this->_depthHandle = this->_depthTex->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
			this->_depthRTHandle = this->_depthTex->GetDefaultView(Diligent::TEXTURE_VIEW_DEPTH_STENCIL);
		}
		// ----------

		rawrbox::BindlessManager::registerTexture(*this);
	}

} // namespace rawrbox
