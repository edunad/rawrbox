
#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/textures/render.hpp>

#include <fmt/format.h>

namespace rawrbox {
	TextureRender::TextureRender(const rawrbox::Vector2i& size, bool depth) : _size(size), _depth(depth) { this->_name = "RawrBox::RenderTarget"; }
	TextureRender::~TextureRender() {
		RAWRBOX_DESTROY(this->_depthHandle);
		RAWRBOX_DESTROY(this->_depthTex);
	}

	// ------ UTILS
	Diligent::ITextureView* TextureRender::getDepth() const { return this->_depthHandle; }
	Diligent::ITextureView* TextureRender::getRTDepth() const { return this->_depthRTHandle; }
	Diligent::ITextureView* TextureRender::getRT() const { return this->_rtHandle; }

	Diligent::ITexture* TextureRender::getDepthHandle() const { return this->_depthTex; }
	// ------------

	// ------ RENDER
	void TextureRender::startRecord(bool clear) {
		if (this->_recording) throw this->_logger->error("Already recording");

		// BARRIER ----
		if (this->_depth) {
			rawrbox::BindlessManager::barrier<Diligent::ITexture>({this->_tex, this->_depthTex}, {Diligent::RESOURCE_STATE_RENDER_TARGET, Diligent::RESOURCE_STATE_DEPTH_WRITE});
		} else {
			rawrbox::BindlessManager::barrier<Diligent::ITexture>({this->_tex}, {Diligent::RESOURCE_STATE_RENDER_TARGET});
		}
		//   --------

		auto* context = rawrbox::RENDERER->context();
		context->SetRenderTargets(1, &this->_rtHandle, this->_depthRTHandle, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);

		if (clear) {
			const std::array<float, 4> ClearColor = {0.0F, 0.0F, 0.0F, 0.0F};
			context->ClearRenderTarget(this->_rtHandle, ClearColor.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);

			if (this->_depthRTHandle != nullptr) {
				context->ClearDepthStencil(this->_depthRTHandle, Diligent::CLEAR_DEPTH_FLAG, 1.0F, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
			}
		}

		this->_recording = true;
	}

	void TextureRender::stopRecord() {
		if (!this->_recording) throw this->_logger->error("Not recording");
		auto* pRTV = rawrbox::RENDERER->swapChain()->GetCurrentBackBufferRTV();
		auto* depth = rawrbox::RENDERER->swapChain()->GetDepthBufferDSV();

		rawrbox::RENDERER->context()->SetRenderTargets(1, &pRTV, depth, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);

		// BARRIER ----
		if (this->_depth) {
			rawrbox::BindlessManager::barrier<Diligent::ITexture>({this->_tex, this->_depthTex}, {Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::RESOURCE_STATE_DEPTH_READ});
		} else {
			rawrbox::BindlessManager::barrier<Diligent::ITexture>({this->_tex}, {Diligent::RESOURCE_STATE_SHADER_RESOURCE});
		}
		// --------

		this->_recording = false;
	}

	void TextureRender::upload(Diligent::TEXTURE_FORMAT format, bool /*dynamic*/) {
		if (this->_rtHandle != nullptr || this->_depthHandle != nullptr) return; // Failed texture is already bound, so skip it
		if (format == Diligent::TEXTURE_FORMAT::TEX_FORMAT_UNKNOWN) throw this->_logger->error("Invalid format");

		// Render target -----
		Diligent::TextureDesc desc;
		desc.Type = Diligent::RESOURCE_DIM_TEX_2D_ARRAY;
		desc.BindFlags = Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_RENDER_TARGET;
		desc.Width = this->_size.x;
		desc.Height = this->_size.y;
		desc.MipLevels = 1;
		desc.Format = format;
		desc.Name = this->_name.c_str();
		desc.ClearValue.Format = desc.Format;
		desc.ClearValue.Color[0] = 0.F;
		desc.ClearValue.Color[1] = 0.F;
		desc.ClearValue.Color[2] = 0.F;
		desc.ClearValue.Color[3] = 0.F;

		rawrbox::RENDERER->device()->CreateTexture(desc, nullptr, &this->_tex);
		// --------------

		// Depth ----
		if (_depth) {
			const auto& swapDesc = rawrbox::RENDERER->swapChain()->GetDesc();

			std::string depthName = fmt::format("{}::DEPTH", this->_name);
			desc.Name = depthName.c_str();
			desc.Format = swapDesc.DepthBufferFormat;
			desc.BindFlags = Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_DEPTH_STENCIL;
			desc.ClearValue.Format = desc.Format;
			desc.ClearValue.DepthStencil.Depth = 1;
			desc.ClearValue.DepthStencil.Stencil = 0;

			rawrbox::RENDERER->device()->CreateTexture(desc, nullptr, &this->_depthTex);
			rawrbox::BindlessManager::barrier<Diligent::ITexture>({this->_tex, this->_depthTex}, {Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::RESOURCE_STATE_DEPTH_READ});

			this->_depthHandle = this->_depthTex->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
			this->_depthRTHandle = this->_depthTex->GetDefaultView(Diligent::TEXTURE_VIEW_DEPTH_STENCIL);
		} else {
			rawrbox::BindlessManager::barrier<Diligent::ITexture>({this->_tex}, {Diligent::RESOURCE_STATE_SHADER_RESOURCE});
		}
		// ----------

		// Get handles --
		this->_rtHandle = this->_tex->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET);
		this->_handle = this->_tex->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
		// --------------

		rawrbox::BindlessManager::registerTexture(*this);
	}

} // namespace rawrbox