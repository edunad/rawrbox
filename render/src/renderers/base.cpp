
#ifndef ENGINE_DLL
	#define ENGINE_DLL 1
#endif

#ifndef D3D11_SUPPORTED
	#define D3D11_SUPPORTED 0
#endif

#ifndef D3D12_SUPPORTED
	#define D3D12_SUPPORTED 0
#endif

#ifndef GL_SUPPORTED
	#define GL_SUPPORTED 0
#endif

#ifndef VULKAN_SUPPORTED
	#define VULKAN_SUPPORTED 0
#endif

#ifndef METAL_SUPPORTED
	#define METAL_SUPPORTED 0
#endif

#if D3D11_SUPPORTED
	#include <Graphics/GraphicsEngineD3D11/interface/EngineFactoryD3D11.h>
#endif

#if D3D12_SUPPORTED
	#include <Graphics/GraphicsEngineD3D12/interface/EngineFactoryD3D12.h>
#endif

#if GL_SUPPORTED
	#include <Graphics/GraphicsEngineOpenGL/interface/EngineFactoryOpenGL.h>
#endif

#if VULKAN_SUPPORTED
	#include <Graphics/GraphicsEngineVulkan/interface/EngineFactoryVk.h>
#endif

#if METAL_SUPPORTED
	#include <Graphics/GraphicsEngineMetal/interface/EngineFactoryMtl.h>
#endif

#include <rawrbox/render/materials/base.hpp>
#include <rawrbox/render/materials/instanced.hpp>
#include <rawrbox/render/materials/skinned.hpp>
#include <rawrbox/render/materials/text.hpp>
#include <rawrbox/render/renderers/base.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/texture/webp.hpp>
#include <rawrbox/render/utils/render.hpp>
#include <rawrbox/utils/path.hpp>
#include <rawrbox/utils/threading.hpp>
// #include <rawrbox/render_temp/decals/manager.hpp>

namespace rawrbox {
	RendererBase::RendererBase(Diligent::RENDER_DEVICE_TYPE type, Diligent::NativeWindow window, const rawrbox::Vector2i& size, const rawrbox::Colorf& clearColor) : _window(window), _type(type), _size(size), _clearColor(clearColor) {}
	RendererBase::~RendererBase() {
		this->_render.reset();
		this->_stencil.reset();
		this->_decals.reset();

		RAWRBOX_DESTROY(this->_device);
		RAWRBOX_DESTROY(this->_context);
		RAWRBOX_DESTROY(this->_swapChain);

		/*rawrbox::DECALS::shutdown();
		this->_decals.reset();
		bgfx::discard(BGFX_DISCARD_ALL);*/
	}

	void RendererBase::init() {
		if (!this->supported()) throw std::runtime_error(fmt::format("[RawrBox-Renderer] Renderer not supported by GPU!"));

		Diligent::SwapChainDesc SCDesc;
		switch (this->_type) {
#if D3D11_SUPPORTED
			case Diligent::RENDER_DEVICE_TYPE_D3D11:
				{
	#if ENGINE_DLL
					auto* GetEngineFactoryD3D11 = Diligent::LoadGraphicsEngineD3D11(); // Load the dll and import GetEngineFactoryD3D11() function
	#endif
					auto* pFactoryD3D11 = GetEngineFactoryD3D11();
					this->_engineFactory = pFactoryD3D11;

					Diligent::EngineD3D11CreateInfo EngineCI;
					pFactoryD3D11->CreateDeviceAndContextsD3D11(EngineCI, &this->_device, &this->_context);
					pFactoryD3D11->CreateSwapChainD3D11(this->_device, this->_context, SCDesc, Diligent::FullScreenModeDesc{}, this->_window, &this->_swapChain);
				}
				break;
#endif

#if D3D12_SUPPORTED
			case Diligent::RENDER_DEVICE_TYPE_D3D12:
				{
	#if ENGINE_DLL
					// Load the dll and import GetEngineFactoryD3D12() function
					auto* GetEngineFactoryD3D12 = Diligent::LoadGraphicsEngineD3D12();
	#endif
					auto* pFactoryD3D12 = GetEngineFactoryD3D12();
					this->_engineFactory = pFactoryD3D12;

					Diligent::EngineD3D12CreateInfo EngineCI;
					pFactoryD3D12->CreateDeviceAndContextsD3D12(EngineCI, &this->_device, &this->_context);
					pFactoryD3D12->CreateSwapChainD3D12(this->_device, this->_context, SCDesc, Diligent::FullScreenModeDesc{}, this->_window, &this->_swapChain);
				}
				break;
#endif // D3D12_SUPPORTED

#if GL_SUPPORTED
			case Diligent::RENDER_DEVICE_TYPE_GL:
				{
	#if EXPLICITLY_LOAD_ENGINE_GL_DLL
					// Load the dll and import GetEngineFactoryOpenGL() function
					auto GetEngineFactoryOpenGL = Diligent::LoadGraphicsEngineOpenGL();
					auto* pFactoryOpenGL = GetEngineFactoryOpenGL();
	#else
					auto* pFactoryOpenGL = Diligent::GetEngineFactoryOpenGL();
	#endif
					this->_engineFactory = pFactoryOpenGL;

					Diligent::EngineGLCreateInfo EngineCI;
					EngineCI.Window = this->_window;
					pFactoryOpenGL->CreateDeviceAndSwapChainGL(EngineCI, &this->_device, &this->_context, SCDesc, &this->_swapChain);
				}
				break;
#endif // GL_SUPPORTED

#if VULKAN_SUPPORTED
			case Diligent::RENDER_DEVICE_TYPE_VULKAN:
				{
	#if EXPLICITLY_LOAD_ENGINE_GL_DLL
					// Load the dll and import GetEngineFactoryVk() function
					auto* GetEngineFactoryVk = Diligent::LoadGraphicsEngineVk();
					auto* pFactoryVk = GetEngineFactoryVk();
	#else
					auto* pFactoryVk = Diligent::GetEngineFactoryVk();
	#endif
					this->_engineFactory = pFactoryVk;

					Diligent::EngineVkCreateInfo EngineCI;
					pFactoryVk->CreateDeviceAndContextsVk(EngineCI, &this->_device, &this->_context);
					pFactoryVk->CreateSwapChainVk(this->_device, this->_context, SCDesc, this->_window, &this->_swapChain);
				}
				break;
#endif // VULKAN_SUPPORTED
			default: throw std::runtime_error("[RawrBox-Window] Invalid diligent engine");
		}

		// Setup shader pipeline if not exists
		if (rawrbox::SHADER_FACTORY == nullptr) {
			auto dirs = rawrbox::PathUtils::glob("assets/shaders", true);
			auto paths = fmt::format("{}", fmt::join(dirs, ";"));

			this->_engineFactory->CreateDefaultShaderSourceStreamFactory(paths.c_str(), &rawrbox::SHADER_FACTORY);
		}
		// -----------

		if (this->_engineFactory == nullptr) throw std::runtime_error("[RawrBox-Renderer] Failed to initialize");

		// Init default textures ---
		if (rawrbox::MISSING_TEXTURE == nullptr) {
			rawrbox::MISSING_TEXTURE = std::make_shared<rawrbox::TextureMissing>();
			rawrbox::MISSING_TEXTURE->upload();
		}

		if (rawrbox::WHITE_TEXTURE == nullptr) {
			rawrbox::WHITE_TEXTURE = std::make_shared<rawrbox::TextureFlat>(rawrbox::Vector2i(2, 2), rawrbox::Colors::White());
			rawrbox::WHITE_TEXTURE->upload();
		}

		if (rawrbox::BLACK_TEXTURE == nullptr) {
			rawrbox::BLACK_TEXTURE = std::make_shared<rawrbox::TextureFlat>(rawrbox::Vector2i(2, 2), rawrbox::Colors::Black());
			rawrbox::BLACK_TEXTURE->upload();
		}

		if (rawrbox::NORMAL_TEXTURE == nullptr) {
			rawrbox::NORMAL_TEXTURE = std::make_shared<rawrbox::TextureFlat>(rawrbox::Vector2i(2, 2), rawrbox::Color::RGBHex(0xbcbcff));
			rawrbox::NORMAL_TEXTURE->upload();
		}

		if (!rawrbox::PipelineUtils::initialized) {
			rawrbox::PipelineUtils::init();
		}
		// -------------------------

		// Setup stencil ----
		this->_stencil = std::make_unique<rawrbox::Stencil>(this->_size);
		this->_stencil->upload();
		// ------------------

		// Setup renderer --
		this->_render = std::make_unique<rawrbox::TextureRender>(this->_size); // TODO: RESCALE
		this->_render->upload(Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB);
		// --------

		// Setup decals --
		this->_decals = std::make_unique<rawrbox::TextureRender>(this->_size); // TODO: RESCALE
		this->_decals->upload(Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB);
		//  rawrbox::DECALS::init();
		// --------

		this->playIntro();
		rawrbox::ENGINE_INITIALIZED = true;
	}

	void RendererBase::resize(const rawrbox::Vector2i& size) {
		if (this->_swapChain == nullptr) return;

		this->_swapChain->Resize(size.x, size.y);
		if (this->_stencil != nullptr) this->_stencil->resize(size);

		//  this->_render->addTexture(bgfx::TextureFormat::R8);    // Decal stencil
		//  this->_render->addTexture(bgfx::TextureFormat::RGBA8); // GPU PICKING
		// this->_render->upload(Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB);

		/*
				this->_decals = std::make_unique<rawrbox::TextureRender>(size);
				this->_decals->upload();

				this->_GPUBlitTex = bgfx::createTexture2D(8, 8, false, 1, bgfx::TextureFormat::RGBA8, 0 | BGFX_TEXTURE_BLIT_DST | BGFX_TEXTURE_READ_BACK | BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP);
				bgfx::setName(this->_GPUBlitTex, "RAWRBOX-BLIT-GPU-PICK");

				auto w = static_cast<uint16_t>(size.x);
				auto h = static_cast<uint16_t>(size.y);

				// Setup view ---
				bgfx::setViewName(rawrbox::MAIN_WORLD_VIEW, "RAWRBOX-MAIN-WORLD");
				bgfx::setViewClear(rawrbox::MAIN_WORLD_VIEW, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 1.0F, 0, 0);
				bgfx::setViewRect(rawrbox::MAIN_WORLD_VIEW, 0, 0, w, h);

				bgfx::setViewName(rawrbox::MAIN_OVERLAY_VIEW, "RAWRBOX-MAIN-OVERLAY");
				bgfx::setViewClear(rawrbox::MAIN_OVERLAY_VIEW, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x00000000, 1.0F, 0);
				bgfx::setViewRect(rawrbox::MAIN_OVERLAY_VIEW, 0, 0, w, h);
				// -----*/

		this->_size = size;
	}

	void RendererBase::setWorldRender(std::function<void()> render) { this->worldRender = render; }
	void RendererBase::setOverlayRender(std::function<void()> render) { this->overlayRender = render; }
	void RendererBase::overridePostWorld(std::function<void()> post) { this->postRender = post; }

	void RendererBase::update() {
		if (this->_currentIntro != nullptr) {
			if (this->_introComplete) {
				this->_introList.erase(this->_introList.begin());

				// Done?
				if (this->_introList.empty()) {
					this->setOverlayRender(this->_tempOverlayRender);
					this->setWorldRender(this->_tempWorldRender);
					this->completeIntro();
				} else {
					this->_currentIntro = &this->_introList.begin()->second;
					this->_introComplete = false;
				}

				return;
			}

			this->_currentIntro->texture->update();
		} else {
			if (this->_camera != nullptr) {
				this->_camera->update();
			}
		}
	}

	void RendererBase::render() {
		if (this->_swapChain == nullptr || this->_context == nullptr || this->_device == nullptr) throw std::runtime_error("[Rawrbox-Renderer] Failed to bind swapChain/context/device! Did you call 'init' ?");

		if (this->worldRender == nullptr) throw std::runtime_error("[Rawrbox-Renderer] World render method not set! Did you call 'setWorldRender' ?");
		if (this->overlayRender == nullptr) throw std::runtime_error("[Rawrbox-Renderer] Overlay render method not set! Did you call 'setOverlayRender' ?");

		// Clear backbuffer ----
		this->clear();
		// ---------------------

		// No world / overlay only
		if (this->_camera == nullptr) {
			this->overlayRender();
			this->frame();
			return;
		}

		// Final Pass -------------
		this->finalRender();
		// ------------------------

		// Check GPU Picking -----
		// this->gpuCheck();
		// -------------------

		// Submit ---
		this->frame();
		// ----------
	}

	/*void RendererBase::gpuCheck() {
		if (this->_gpuReadFrame == rawrbox::BGFX_FRAME) {
			std::unordered_map<uint32_t, uint32_t> ids = {};
			const bgfx::Caps* caps = bgfx::getCaps();

			uint32_t max = 0;
			uint32_t id = 0;

			// ----------
			for (uint8_t* x = this->_gpuPixelData.data(); x < this->_gpuPixelData.data() + rawrbox::GPU_PICK_SAMPLE_SIZE;) {
				uint8_t rr = *x++;
				uint8_t gg = *x++;
				uint8_t bb = *x++;
				uint8_t aa = *x++;

				if (aa == 0) continue;
				if (caps->rendererType == bgfx::RendererType::Direct3D9) {
					std::swap(rr, bb);
				}

				uint32_t hashKey = rawrbox::PackUtils::toABGR(rr / 255.F, gg / 255.F, bb / 255.F, 1.F);
				auto& v = ids[hashKey];
				v++;

				if (v > max) {
					max = v;
					id = hashKey;
				}
			}

			// --------------
			for (auto& p : this->_gpuPickCallbacks) {
				p(id);
			}

			this->_gpuPickCallbacks.clear();
		}
	}*/

	void RendererBase::finalRender() {
		// Record world ---
		this->_render->startRecord();
		this->worldRender();
		this->_render->stopRecord();
		//  ----------------

		// Render world ---
		if (this->postRender == nullptr) {
			rawrbox::RenderUtils::renderQUAD(this->_render->getHandle());
		} else {
			this->postRender();
		}
		// rawrbox::RenderUtils::drawQUAD(this->_decals->getHandle(), this->_size, true, BGFX_STATE_BLEND_ALPHA);
		//  ----------------

		this->overlayRender();

		/*// Record world ---
		this->_render->startRecord();
		this->worldRender();
		bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, rawrbox::MAIN_CAMERA->getViewMtx().data(), rawrbox::MAIN_CAMERA->getProjMtx().data());
		this->_render->stopRecord();
		// ----------------

		// Record decals ---
		this->_decals->startRecord();
		rawrbox::DECALS::draw();
		bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, rawrbox::MAIN_CAMERA->getViewMtx().data(), rawrbox::MAIN_CAMERA->getProjMtx().data());
		this->_decals->stopRecord();
		// -------------------

		// Render world ---
		auto prevId = rawrbox::CURRENT_VIEW_ID;
		rawrbox::CURRENT_VIEW_ID = rawrbox::MAIN_WORLD_VIEW;
		// ---

		if (this->postRender == nullptr) {
			rawrbox::RenderUtils::drawQUAD(this->_render->getHandle(), this->_size);
			rawrbox::RenderUtils::drawQUAD(this->_decals->getHandle(), this->_size, true, BGFX_STATE_BLEND_ALPHA);
		} else {
			this->postRender();
		}

		// -----------------

		// Restore id -----
		rawrbox::CURRENT_VIEW_ID = prevId;
		bgfx::discard(BGFX_DISCARD_ALL);
		// ------------------------

		// Render overlay ---
		prevId = rawrbox::CURRENT_VIEW_ID;
		rawrbox::CURRENT_VIEW_ID = rawrbox::MAIN_OVERLAY_VIEW;
		// ---
		bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, nullptr, nullptr);
		this->overlayRender();
		// ----------------

		// Restore id -----
		rawrbox::CURRENT_VIEW_ID = prevId;
		bgfx::discard(BGFX_DISCARD_ALL);
		// ------------------------*/
	}

	void RendererBase::clear() {
		if (this->_swapChain == nullptr || this->_context == nullptr) return;

		auto* pRTV = this->_swapChain->GetCurrentBackBufferRTV();
		auto* pDSV = this->_swapChain->GetDepthBufferDSV();

		// Reset render target
		this->_context->SetRenderTargets(1, &pRTV, pDSV, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

		// Clear the back buffer
		this->_context->ClearRenderTarget(pRTV, this->_clearColor.data().data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		this->_context->ClearDepthStencil(pDSV, Diligent::CLEAR_DEPTH_FLAG, 1.F, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	}

	void RendererBase::frame() {
		this->_swapChain->Present(this->_vsync ? 1 : 0); // Submit
		rawrbox::FRAME++;
	}

	// INTRO ------
	void RendererBase::playIntro() {
		if (this->_skipIntros) {
			this->completeIntro();
			return;
		}

		// Temp store renders for overriding ---
		this->_tempOverlayRender = this->overlayRender;
		this->_tempWorldRender = this->worldRender;
		// -------------

		this->worldRender = []() {};
		this->overlayRender = [this]() {
			this->_stencil->drawBox({}, this->_size.cast<float>(), rawrbox::Colors::Black());

			if (this->_currentIntro != nullptr) {
				auto screenSize = this->_size.cast<float>();

				if (this->_currentIntro->cover) {
					this->_stencil->drawTexture({0, 0}, {screenSize.x, screenSize.y}, *this->_currentIntro->texture);
				} else {
					auto size = this->_currentIntro->texture->getSize().cast<float>();
					this->_stencil->drawTexture({screenSize.x / 2.F - size.x / 2.F, screenSize.y / 2.F - size.y / 2.F}, {size.x, size.y}, *this->_currentIntro->texture);
				}
			}

			this->_stencil->render();
		};

		// Load webp intros -----------------------
		rawrbox::ASYNC::run([this]() {
			// Load ----
			for (auto& intro : this->_introList) {
				intro.second.texture = std::make_shared<rawrbox::TextureWEBP>(intro.first);
				intro.second.texture->setLoop(false);
				intro.second.texture->setSpeed(intro.second.speed);
				intro.second.texture->onEnd += [this]() {
					this->_introComplete = true;
				};

				intro.second.texture->upload();
			}

			// First intro on the list
			this->_currentIntro = &this->_introList.begin()->second;
		});
		// -------------------------
	}

	void RendererBase::completeIntro() {
		// Init & load materials ---
		rawrbox::MaterialBase::init();
		rawrbox::MaterialText3D::init();
		rawrbox::MaterialInstanced::init();
		rawrbox::MaterialSkinned::init();
		// -----

		this->_introList.clear();
		this->_currentIntro = nullptr;
		this->_introComplete = true;

		this->onIntroCompleted();
	}

	void RendererBase::skipIntros(bool skip) {
		if (skip) fmt::print("[RawrBox] Skipping intros :(\n");
		this->_skipIntros = skip;
	}

	void RendererBase::addIntro(const std::filesystem::path& webpPath, float speed, bool cover) {
		if (webpPath.extension() != ".webp") throw std::runtime_error(fmt::format("[RawrBox-RenderBase] Invalid intro '{}', format needs to be .webp!", webpPath.generic_string()));

		rawrbox::RawrboxIntro intro;
		intro.cover = cover;
		intro.speed = speed;
		intro.texture = nullptr;

		this->_introList[webpPath.generic_string()] = intro;
	}
	//-------------------------

	// Utils ----
	rawrbox::CameraBase* RendererBase::camera() const { return this->_camera.get(); }
	rawrbox::Stencil* RendererBase::stencil() const { return this->_stencil.get(); }

	Diligent::IDeviceContext* RendererBase::context() const { return this->_context; }
	Diligent::ISwapChain* RendererBase::swapChain() const { return this->_swapChain; }
	Diligent::IRenderDevice* RendererBase::device() const { return this->_device; }

	Diligent::ITextureView* RendererBase::getDepth() const {
		return this->_render->getDepth();
	}

	Diligent::ITextureView* RendererBase::getColor(bool rt) const {
		return rt ? this->_render->getRT() : this->_render->getHandle();
	}

	/*const bgfx::TextureHandle RendererBase::getMask() const {
		if (this->_render == nullptr) return BGFX_INVALID_HANDLE;
		return this->_render->getTexture(1);
	}

	const bgfx::TextureHandle RendererBase::getGPUPick() const {
		if (this->_render == nullptr) return BGFX_INVALID_HANDLE;
		return this->_render->getTexture(2);
	}*/

	const rawrbox::Vector2i RendererBase::getSize() const { return this->_size; }

	bool RendererBase::getVSync() const { return this->_vsync; }
	void RendererBase::setVSync(bool vsync) { this->_vsync = vsync; }

	/*void RendererBase::gpuPick(const rawrbox::Vector2i& pos, std::function<void(uint32_t)> callback) {
		if (this->_render == nullptr || pos.x < 0 || pos.y < 0 || pos.x >= this->_size.x || pos.y >= this->_size.y) return;

		auto tex = this->getGPUPick();
		if (!bgfx::isValid(tex)) return;

		bgfx::blit(rawrbox::BLIT_VIEW, this->_GPUBlitTex, 0, 0, tex, static_cast<uint16_t>(pos.x), static_cast<uint16_t>(pos.y));
		this->_gpuReadFrame = bgfx::readTexture(this->_GPUBlitTex, this->_gpuPixelData.data());
		this->_gpuPickCallbacks.emplace_back(callback);
	}*/
	// ------

	// Is it supported by the GPU?
	bool RendererBase::supported() const {
		// const auto& features = this->_device->GetDeviceInfo().Features;
		return true;
	}
} // namespace rawrbox
