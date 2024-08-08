
#ifndef ENGINE_DLL
	#define ENGINE_DLL 1
#endif

#if RAWRBOX_SUPPORT_DX11
	#include <EngineFactoryD3D11.h>
#endif

#if RAWRBOX_SUPPORT_DX12
	#include <EngineFactoryD3D12.h>
#endif

#if RAWRBOX_SUPPORT_GL
	#include <EngineFactoryOpenGL.h>
#endif

#if RAWRBOX_SUPPORT_VULKAN
	#include <EngineFactoryVk.h>
#endif

#if RAWRBOX_SUPPORT_METAL
	#include <EngineFactoryMtl.h>
#endif

#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/renderer.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/text/engine.hpp>
#include <rawrbox/render/textures/utils/utils.hpp>
#include <rawrbox/render/textures/webp.hpp>
#include <rawrbox/render/utils/barrier.hpp>
#include <rawrbox/render/utils/render.hpp>
#include <rawrbox/utils/path.hpp>
#include <rawrbox/utils/threading.hpp>

#include <fmt/ranges.h>

#include <utility>

namespace rawrbox {
	RendererBase::RendererBase(Diligent::RENDER_DEVICE_TYPE type, Diligent::NativeWindow window, const rawrbox::Vector2u& size, const rawrbox::Vector2u& monitorSize, const rawrbox::Colorf& clearColor) : _clearColor(clearColor), _size(size), _monitorSize(monitorSize), _window(window), _type(type) {}
	RendererBase::~RendererBase() {
		this->_render.reset();
		this->_stencil.reset();
		this->_logger.reset();
		this->_GPUBlit.reset();

		RAWRBOX_DESTROY(this->_device);
		RAWRBOX_DESTROY(this->_context);
		RAWRBOX_DESTROY(this->_swapChain);
	}

	void RendererBase::init(Diligent::DeviceFeatures features) {
		Diligent::SwapChainDesc SCDesc;
		SCDesc.ColorBufferFormat = Diligent::TEX_FORMAT_RGBA8_UNORM; // Don't use SRGB

		// Enable required features --------------------------
		features.WireframeFill = Diligent::DEVICE_FEATURE_STATE_ENABLED;
		features.SparseResources = Diligent::DEVICE_FEATURE_STATE_ENABLED;

		features.GeometryShaders = Diligent::DEVICE_FEATURE_STATE_ENABLED;
		features.ComputeShaders = Diligent::DEVICE_FEATURE_STATE_ENABLED;

		features.BindlessResources = Diligent::DEVICE_FEATURE_STATE_ENABLED;
		features.ShaderResourceRuntimeArrays = Diligent::DEVICE_FEATURE_STATE_ENABLED;
		features.VertexPipelineUAVWritesAndAtomics = Diligent::DEVICE_FEATURE_STATE_ENABLED;
		// features.DepthClamp = Diligent::DEVICE_FEATURE_STATE_ENABLED;
		// features.DepthBiasClamp = Diligent::DEVICE_FEATURE_STATE_ENABLED;
		// ---------------------------------------------------

#ifdef _DEBUG
		features.PipelineStatisticsQueries = Diligent::DEVICE_FEATURE_STATE_OPTIONAL;
		features.DurationQueries = Diligent::DEVICE_FEATURE_STATE_OPTIONAL;
#endif
		// --------------

		// PLUGIN REQUIREMENTS ---
		for (auto& plugin : this->_renderPlugins) {
			if (plugin.second == nullptr) continue;
			plugin.second->requirements(features);
		}
		// -----------------------

		// Initialize engine -----
		switch (this->_type) {
#if RAWRBOX_SUPPORT_DX12
			case Diligent::RENDER_DEVICE_TYPE_D3D12:
				{
	#if ENGINE_DLL
					// Load the dll and import GetEngineFactoryD3D12() function
					auto* GetEngineFactoryD3D12 = Diligent::LoadGraphicsEngineD3D12();
	#endif
					auto* pFactoryD3D12 = GetEngineFactoryD3D12();
					this->_engineFactory = pFactoryD3D12;

					Diligent::EngineD3D12CreateInfo EngineCI;
					EngineCI.Features = features;
					// EngineCI.D3D12ValidationFlags = Diligent::D3D12_VALIDATION_FLAG_NONE;
					// EngineCI.EnableValidation = false;

					if (this->overrideHEAP != nullptr) {
						auto heap = this->overrideHEAP();

						EngineCI.GPUDescriptorHeapDynamicSize[0] = heap.first;
						EngineCI.GPUDescriptorHeapSize[0] = heap.second;
					} else {
						EngineCI.GPUDescriptorHeapDynamicSize[0] = 32768;
						EngineCI.GPUDescriptorHeapSize[1] = 128;
						EngineCI.GPUDescriptorHeapDynamicSize[1] = 2048 - 128;
						EngineCI.DynamicDescriptorAllocationChunkSize[0] = 32;
						EngineCI.DynamicDescriptorAllocationChunkSize[1] = 8;
					}

					pFactoryD3D12->CreateDeviceAndContextsD3D12(EngineCI, &this->_device, &this->_context);
					pFactoryD3D12->CreateSwapChainD3D12(this->_device, this->_context, SCDesc, Diligent::FullScreenModeDesc(false), this->_window, &this->_swapChain);
				}
				break;
#endif // D3D12_SUPPORTED

#if RAWRBOX_SUPPORT_VULKAN
			case Diligent::RENDER_DEVICE_TYPE_VULKAN:
				{
	#if EXPLICITLY_LOAD_ENGINE_VK_DLL
					// Load the dll and import GetEngineFactoryVk() function
					auto* GetEngineFactoryVk = Diligent::LoadGraphicsEngineVk();
					auto* pFactoryVk = GetEngineFactoryVk();
	#else
					auto* pFactoryVk = Diligent::GetEngineFactoryVk();
	#endif
					this->_engineFactory = pFactoryVk;

					Diligent::EngineVkCreateInfo EngineCI;
					EngineCI.Features = features;

	#ifndef _WIN32
					EngineCI.pDxCompilerPath = "dxcompiler";
	#endif

					if (this->overrideHEAP != nullptr) {
						auto heap = this->overrideHEAP();

						EngineCI.DynamicHeapSize = heap.first;
						EngineCI.DynamicHeapPageSize = heap.second;
					} else {
						EngineCI.DynamicHeapSize = 128 << 20;
						EngineCI.DynamicHeapPageSize = 2 << 20;
					}

					pFactoryVk->CreateDeviceAndContextsVk(EngineCI, &this->_device, &this->_context);
					pFactoryVk->CreateSwapChainVk(this->_device, this->_context, SCDesc, this->_window, &this->_swapChain);
				}
				break;
#endif // VULKAN_SUPPORTED

#if RAWRBOX_SUPPORT_DX11
			case Diligent::RENDER_DEVICE_TYPE_D3D11:
				{
	#if ENGINE_DLL
					auto* GetEngineFactoryD3D11 = Diligent::LoadGraphicsEngineD3D11(); // Load the dll and import GetEngineFactoryD3D11() function
	#endif
					auto* pFactoryD3D11 = GetEngineFactoryD3D11();
					this->_engineFactory = pFactoryD3D11;

					Diligent::EngineD3D11CreateInfo EngineCI;
					EngineCI.Features = features;

					pFactoryD3D11->CreateDeviceAndContextsD3D11(EngineCI, &this->_device, &this->_context);
					pFactoryD3D11->CreateSwapChainD3D11(this->_device, this->_context, SCDesc, Diligent::FullScreenModeDesc{}, this->_window, &this->_swapChain);
				}
				break;
#endif

#if RAWRBOX_SUPPORT_GL
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
					EngineCI.Features = features;
					EngineCI.Window = this->_window;
					pFactoryOpenGL->CreateDeviceAndSwapChainGL(EngineCI, &this->_device, &this->_context, SCDesc, &this->_swapChain);
				}
				break;
#endif // GL_SUPPORTED

			default: throw this->_logger->error("Invalid diligent api");
		}

		// Setup shader pipeline if not exists
		if (rawrbox::SHADER_FACTORY == nullptr) {
			auto rootDir = this->getShadersDirectory();
			if (!std::filesystem::exists(rootDir)) throw this->_logger->error("Shaders directory '{}' not found!", rootDir.generic_string());

			auto dirs = rawrbox::PathUtils::glob(rootDir, true);
			auto paths = fmt::format("{}", fmt::join(dirs, ";"));

			this->_logger->info("Initializing shader factory (using {}):", fmt::styled(rootDir.generic_string(), fmt::fg(fmt::color::coral)));
			for (const auto& dir : dirs) {
				this->_logger->info("\t{}", dir);
			}

			this->_engineFactory->CreateDefaultShaderSourceStreamFactory(paths.c_str(), &rawrbox::SHADER_FACTORY);
		}
		// -----------

		if (this->_engineFactory == nullptr) throw this->_logger->error("Failed to initialize");

		// Single draw call to setup window background
		this->clear();
		this->frame();
		// --------------

		// Init pipelines ---
		rawrbox::PipelineUtils::init(*this->device());
		rawrbox::PipelineUtils::registerGlobalMacro(Diligent::SHADER_TYPE_VERTEX, "MAX_BONES_PER_MODEL", rawrbox::MAX_BONES_PER_MODEL);
		rawrbox::PipelineUtils::registerGlobalMacro(Diligent::SHADER_TYPE_VERTEX, "MAX_BONES_PER_VERTEX", rawrbox::MAX_BONES_PER_VERTEX);
		rawrbox::PipelineUtils::registerGlobalMacro(Diligent::SHADER_TYPE_PIXEL, "MAX_POST_DATA", rawrbox::MAX_POST_DATA);
		// ----------------------

		// Setup camera -----
		if (this->_camera == nullptr) throw this->_logger->error("No camera found!");
		this->_camera->initialize();
		// ------------------

		// Init plugins ---
		for (auto& plugin : this->_renderPlugins) {
			if (plugin.second == nullptr) continue;
			this->_logger->info("Initializing '{}' renderer plugin", fmt::styled(plugin.first, fmt::fg(fmt::color::coral)));
			plugin.second->initialize(this->getSize());
		}
		// -----------------------

		// Init bindless ---
		rawrbox::BindlessManager::init();
		// -----------------

		// Init default textures ---
		this->_logger->info("Initializing default textures");

		if (rawrbox::MISSING_TEXTURE == nullptr) {
			rawrbox::MISSING_TEXTURE = std::make_shared<rawrbox::TextureMissing>();
			rawrbox::MISSING_TEXTURE->upload();
		}

		if (rawrbox::MISSING_VERTEX_TEXTURE == nullptr) {
			rawrbox::MISSING_VERTEX_TEXTURE = std::make_shared<rawrbox::TextureMissing>();
			rawrbox::MISSING_VERTEX_TEXTURE->setType(rawrbox::TEXTURE_TYPE::VERTEX);
			rawrbox::MISSING_VERTEX_TEXTURE->upload();
		}

		if (rawrbox::WHITE_TEXTURE == nullptr) {
			rawrbox::WHITE_TEXTURE = std::make_shared<rawrbox::TextureFlat>(rawrbox::Vector2u(2U, 2U), rawrbox::Colors::White());
			rawrbox::WHITE_TEXTURE->upload();
		}

		if (rawrbox::BLACK_TEXTURE == nullptr) {
			rawrbox::BLACK_TEXTURE = std::make_shared<rawrbox::TextureFlat>(rawrbox::Vector2u(2U, 2U), rawrbox::Colors::Black());
			rawrbox::BLACK_TEXTURE->upload();
		}

		if (rawrbox::NORMAL_TEXTURE == nullptr) {
			rawrbox::NORMAL_TEXTURE = std::make_shared<rawrbox::TextureFlat>(rawrbox::Vector2u(2U, 2U), rawrbox::Color::RGBHex(0x7f7fff));
			rawrbox::NORMAL_TEXTURE->upload();
		}

		if (rawrbox::CHECKER_TEXTURE == nullptr) {
			auto checker = rawrbox::TextureUtils::generateCheckboard({256U, 256U}, rawrbox::Color::RGBHex(0x161618), rawrbox::Color::RGBHex(0x2a2a2d), 8U);

			rawrbox::CHECKER_TEXTURE = std::make_shared<rawrbox::TextureImage>(rawrbox::Vector2u(256U, 256U), checker, uint8_t(4));
			rawrbox::CHECKER_TEXTURE->setName("CHECKER_TEXTURE");
			rawrbox::CHECKER_TEXTURE->upload();
		}

		// -------------------------

		// Init default fonts ------
		this->_logger->info("Loading default fonts");

		if (rawrbox::DEBUG_FONT_REGULAR == nullptr) {
			rawrbox::DEBUG_FONT_REGULAR = rawrbox::TextEngine::load("./assets/fonts/SourceCodePro-Regular.ttf", 11U);
		}

		if (rawrbox::DEBUG_FONT_BOLD == nullptr) {
			rawrbox::DEBUG_FONT_BOLD = rawrbox::TextEngine::load("./assets/fonts/SourceCodePro-Bold.ttf", 11U);
		}

		if (rawrbox::DEBUG_FONT_ITALIC == nullptr) {
			rawrbox::DEBUG_FONT_ITALIC = rawrbox::TextEngine::load("./assets/fonts/SourceCodePro-Italic.ttf", 11U);
		}
		// ------

		// Init render utils ---
		RenderUtils::init();
		// -------------------------

		// Upload plugins ---
		for (auto& plugin : this->_renderPlugins) {
			if (plugin.second == nullptr) continue;
			this->_logger->info("Uploading '{}' renderer plugin", fmt::styled(plugin.first, fmt::fg(fmt::color::coral)));
			plugin.second->upload();
		}
		// -----------------------

		// Setup blit ----
		this->_GPUBlit = std::make_unique<rawrbox::TextureBLIT>(rawrbox::Vector2u{GPU_PICK_SAMPLE_SIZE, GPU_PICK_SAMPLE_SIZE});
		this->_GPUBlit->upload();
		// ---------------

		// Setup stencil ----
		this->_stencil = std::make_unique<rawrbox::Stencil>(this->_size);
		this->_stencil->upload();
		// ------------------

		// Setup renderer --
		this->_render = std::make_unique<rawrbox::TextureRender>(this->_size); // TODO: RESCALE
		this->_render->upload(Diligent::TEX_FORMAT_RGBA8_UNORM);

		// GPU PICKING TEXTURE
		auto idIndex = this->_render->addTexture(Diligent::TEX_FORMAT_RGBA8_UNORM);
		this->_render->addView(idIndex, Diligent::TEXTURE_VIEW_RENDER_TARGET);
		// --------

		this->playIntro();
		this->_initialized = true;
	}

	void RendererBase::resize(const rawrbox::Vector2u& size, const rawrbox::Vector2u& monitorSize) {
		if (this->_swapChain == nullptr) return;
		this->_swapChain->Resize(size.x, size.y);

		// Resize plugins --
		for (auto& plugin : this->_renderPlugins) {
			if (plugin.second == nullptr) continue;
			plugin.second->resize(size);
		}
		// --------------------

		if (this->_stencil != nullptr) this->_stencil->resize(size);

		this->_size = size;
		this->_monitorSize = monitorSize;
	}

	// PLUGINS ---------------------------
	bool RendererBase::hasPlugin(const std::string& plugin) const { return this->_renderPlugins.find(plugin) != this->_renderPlugins.end(); }
	const std::map<std::string, std::unique_ptr<rawrbox::RenderPlugin>>& RendererBase::getPlugins() const { return this->_renderPlugins; }
	// -----------------------------------

	void RendererBase::setDrawCall(std::function<void(const rawrbox::DrawPass& pass)> call) { this->_drawCall = std::move(call); }

	void RendererBase::update() {
		if (this->_currentIntro != nullptr) {
			if (this->_introComplete) {
				this->_introList.erase(this->_introList.begin());

				// Done?
				if (this->_introList.empty()) {
					this->setDrawCall(this->_tempRender);
					this->introComplete();
				} else {
					this->_currentIntro = &this->_introList.begin()->second;
					this->_introComplete = false;
				}

				return;
			}
		} else {
			if (this->_camera != nullptr) this->_camera->update();

			// Update plugins --
			for (auto& plugin : this->_renderPlugins) {
				if (plugin.second == nullptr || !plugin.second->isEnabled()) continue;
				plugin.second->update();
			}
			// --------------------
		}
	}

	void RendererBase::render() {
		if (this->_swapChain == nullptr || this->_context == nullptr || this->_device == nullptr) throw this->_logger->error("Failed to bind swapChain / context / device! Did you call 'init' ?");
		if (this->_drawCall == nullptr) throw this->_logger->error("Missing draw call! Did you call 'setDrawCall' ?");

		// Clear backbuffer ----
		this->clear();
		// ---------------------

		// Update textures ---
		rawrbox::BindlessManager::update();
		// --------------------

		// Update camera buffer --
		this->_camera->updateBuffer();
		// -----------

		// Perform pre-render --
		for (auto& plugin : this->_renderPlugins) {
			if (plugin.second == nullptr || !plugin.second->isEnabled()) continue;
#ifdef _DEBUG
				// this->_context->BeginDebugGroup(plugin.first.c_str());
#endif
			plugin.second->preRender();
#ifdef _DEBUG
			// this->_context->EndDebugGroup();
#endif
		}
		// -----------------------

		// Commit graphics signature --
		this->_context->CommitShaderResources(rawrbox::BindlessManager::signatureBind, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
		// -----------------------

		// Perform world --
#ifdef _DEBUG
		// this->_context->BeginDebugGroup("OPAQUE");
		// this->beginQuery("OPAQUE");
#endif
		this->_render->startRecord();
		this->_drawCall(rawrbox::DrawPass::PASS_WORLD);
		this->_render->stopRecord();
#ifdef _DEBUG
		// this->endQuery("OPAQUE");
		// this->_context->EndDebugGroup();
#endif
		//  -----------------

		// Perform post-render --
		for (auto& plugin : this->_renderPlugins) {
			if (plugin.second == nullptr || !plugin.second->isEnabled()) continue;
#ifdef _DEBUG
				// this->_context->BeginDebugGroup(plugin.first.c_str());
#endif
			plugin.second->postRender(*this->_render);
#ifdef _DEBUG
			// this->_context->EndDebugGroup();
#endif
		}
		// -----------------------

		// Render world ----
		rawrbox::RenderUtils::renderQUAD(*this->_render);
		// ------------------

		// Perform overlay --
#ifdef _DEBUG
		// this->_context->BeginDebugGroup("OVERLAY");
		// this->beginQuery("OVERLAY");
#endif
		this->_drawCall(rawrbox::DrawPass::PASS_OVERLAY);
		if (this->_stencil != nullptr) this->_stencil->render();
#ifdef _DEBUG
			// this->endQuery("OVERLAY");
			// this->_context->EndDebugGroup();
#endif
		//  ------------------

		// Submit ---
		this->frame();
		// ---------------------
	}

	void RendererBase::clear() {
		if (this->_swapChain == nullptr || this->_context == nullptr) return;

		auto* pRTV = this->_swapChain->GetCurrentBackBufferRTV();
		auto* pDSV = this->_swapChain->GetDepthBufferDSV();

		// Reset render target
		this->_context->SetRenderTargets(1, &pRTV, pDSV, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

		// Clear the back buffer
		if (!this->_introComplete) {
			std::array<float, 4> clear = {0, 0, 0, 1};
			this->_context->ClearRenderTarget(pRTV, clear.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		} else {
			this->_context->ClearRenderTarget(pRTV, this->_clearColor.array().data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		}

		this->_context->ClearDepthStencil(pDSV, Diligent::CLEAR_DEPTH_FLAG, 1.F, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

		// Clear barrier cache ---
		rawrbox::BarrierUtils::clearBarrierCache();
		// -----------
	}

	void RendererBase::frame() {
		this->_swapChain->Present(this->_vsync ? 1 : 0); // Submit
		rawrbox::FRAME = this->_context->GetFrameNumber();
	}

	// INTRO ------
	void RendererBase::playIntro() {
		if (this->_skipIntros) {
			this->introComplete();
			return;
		}

		// Temp store renders for overriding ---
		this->_tempRender = this->_drawCall;
		// -------------

		this->_drawCall = [this](const rawrbox::DrawPass& pass) {
			if (pass != rawrbox::DrawPass::PASS_OVERLAY) return;
			auto screenSize = this->_size.cast<float>();

			if (this->_currentIntro != nullptr && this->_currentIntro->isValid() && this->_currentIntro->texture->isValid()) {
				this->_stencil->drawBox({}, screenSize, this->_currentIntro->background); // Background

				if (this->_currentIntro->cover) {
					this->_stencil->drawTexture({0, 0}, {screenSize.x, screenSize.y}, *this->_currentIntro->texture);
				} else {
					auto size = this->_currentIntro->texture->getSize().cast<float>();
					this->_stencil->drawTexture({screenSize.x / 2.F - size.x / 2.F, screenSize.y / 2.F - size.y / 2.F}, {size.x, size.y}, *this->_currentIntro->texture);
				}

			} else {
				this->_stencil->drawBox({}, screenSize, rawrbox::Colors::Black()); // Background
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

	void RendererBase::introComplete() {
		this->_introList.clear();
		this->_currentIntro = nullptr;
		this->_introComplete = true;

		this->onIntroCompleted();
	}

	void RendererBase::skipIntros(bool skip) {
		if (skip) this->_logger->info("Skipping intros :(");
		this->_skipIntros = skip;
	}

	void RendererBase::addIntro(const std::filesystem::path& webpPath, float speed, bool cover, const rawrbox::Colorf& color) {
		if (webpPath.extension() != ".webp") throw this->_logger->error("Invalid intro '{}', only '.webp' format is supported!", webpPath.generic_string());
		this->_introList[webpPath.generic_string()] = {speed, cover, color};
	}
	//-------------------------

	// QUERIES ------
#ifdef _DEBUG
	void RendererBase::beginQuery(const std::string& query) {
		const auto& supportedFeatures = this->device()->GetDeviceInfo().Features;
		if ((supportedFeatures.PipelineStatisticsQueries == 0U) || (supportedFeatures.DurationQueries == 0U)) return;

		auto pipeName = fmt::format("{}::PIPELINE", query);
		auto durationName = fmt::format("{}::DURATION", query);

		Diligent::ScopedQueryHelper* pipelineHelper = nullptr;
		Diligent::ScopedQueryHelper* durationHelper = nullptr;

		// PIPELINE QUERY -----
		auto fndPipeline = this->_query.find(pipeName);
		if (fndPipeline == this->_query.end()) {
			auto name = fmt::format("RawrBox::STATS::PIPELINE::{}", query);

			Diligent::QueryDesc queryDesc;
			queryDesc.Name = name.c_str();
			queryDesc.Type = Diligent::QUERY_TYPE_PIPELINE_STATISTICS;

			auto helper = std::make_unique<Diligent::ScopedQueryHelper>(this->device(), queryDesc, 1);
			pipelineHelper = helper.get();
			this->_query[pipeName] = std::move(helper);

			this->_logger->info("Created query '{}' -> QUERY_TYPE_PIPELINE_STATISTICS", fmt::styled(query, fmt::fg(fmt::color::blue_violet)));
		} else {
			pipelineHelper = fndPipeline->second.get();
		}
		// --------------

		// DURATION QUERY -----
		auto fndDuration = this->_query.find(durationName);
		if (fndDuration == this->_query.end()) {
			auto name = fmt::format("RawrBox::STATS::DURATION::{}", query);

			Diligent::QueryDesc queryDesc;
			queryDesc.Name = name.c_str();
			queryDesc.Type = Diligent::QUERY_TYPE_DURATION;

			auto helper = std::make_unique<Diligent::ScopedQueryHelper>(this->device(), queryDesc, 1);
			durationHelper = helper.get();
			this->_query[durationName] = std::move(helper);

			this->_logger->info("Created query '{}' -> QUERY_TYPE_DURATION", fmt::styled(query, fmt::fg(fmt::color::blue_violet)));
		} else {
			durationHelper = fndDuration->second.get();
		}
		// --------------

		if (pipelineHelper == nullptr || durationHelper == nullptr) {
			throw this->_logger->error("Failed to create & begin query '{}'", query);
		}

		// Start queries ---
		pipelineHelper->Begin(this->context());
		durationHelper->Begin(this->context());
		// -----------------
	}

	void RendererBase::endQuery(const std::string& query) {
		const auto& supportedFeatures = this->device()->GetDeviceInfo().Features;
		if ((supportedFeatures.PipelineStatisticsQueries == 0U) || (supportedFeatures.DurationQueries == 0U)) return;

		auto pipeName = fmt::format("{}::PIPELINE", query);
		auto durationName = fmt::format("{}::DURATION", query);

		auto fndPipeline = this->_query.find(pipeName);
		auto fndDuration = this->_query.find(durationName);

		if (fndPipeline->second == nullptr || fndDuration->second == nullptr) {
			throw this->_logger->error("Failed to end query '{}', not found!", query);
		}

		// End queries ---
		auto* pipeData = &this->_pipelineData[pipeName];
		auto* durationData = &this->_durationData[durationName];

		fndPipeline->second->End(this->context(), pipeData, sizeof(Diligent::QueryDataPipelineStatistics));
		fndDuration->second->End(this->context(), durationData, sizeof(Diligent::QueryDataDuration));
		// -----------------
	}
#endif
	// ----------------

	// Utils ----
	void RendererBase::setMainCamera(rawrbox::CameraBase* camera) const { rawrbox::MAIN_CAMERA = camera; }
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

	std::filesystem::path RendererBase::getShadersDirectory() const {
		return "./assets/shaders";
	}

	const Diligent::RENDER_DEVICE_TYPE& RendererBase::getRenderType() const {
		return this->_type;
	}

#ifdef _DEBUG
	// Only available on DEBUG, since it might hit performance :S
	const Diligent::QueryDataPipelineStatistics& RendererBase::getPipelineStats(const std::string& query) {
		auto pipeName = fmt::format("{}::PIPELINE", query);
		return this->_pipelineData[pipeName];
	}
	// Only available on DEBUG, since it might hit performance :S
	const Diligent::QueryDataDuration& RendererBase::getDurationStats(const std::string& query) {
		auto durationName = fmt::format("{}::DURATION", query);
		return this->_durationData[durationName];
	}
#endif

	const rawrbox::Vector2u& RendererBase::getSize() const { return this->_size; }

	bool RendererBase::getVSync() const { return this->_vsync; }
	void RendererBase::setVSync(bool vsync) { this->_vsync = vsync; }

	void RendererBase::gpuPick(const rawrbox::Vector2i& pos, const std::function<void(uint32_t)>& callback) {
		if (this->_render == nullptr) throw _logger->error("Render target texture not initialized");
		if (callback == nullptr) throw _logger->error("Render target texture not initialized");

		auto size = this->_size.cast<int>();
		if (pos.x < 0 || pos.y < 0 || pos.x >= size.x || pos.y >= size.y) throw _logger->error("Outside of window range");

		Diligent::Box MapRegion;
		MapRegion.MinX = pos.x;
		MapRegion.MinY = pos.y;
		MapRegion.MaxX = MapRegion.MinX + GPU_PICK_SAMPLE_SIZE;
		MapRegion.MaxY = MapRegion.MinY + GPU_PICK_SAMPLE_SIZE;

		auto* tex = this->_render->getTexture(1); // GPU pick texture
		this->_GPUBlit->copy(tex, &MapRegion, [this, callback]() {
			this->_GPUBlit->blit(nullptr, [callback](const uint8_t* pixels, const uint64_t stride) {
				uint32_t max = 0;
				uint32_t id = 0;
				std::unordered_map<uint32_t, uint32_t> ids = {};

				for (size_t y = 0; y < GPU_PICK_SAMPLE_SIZE; y++) {
					for (size_t x = 0; x < GPU_PICK_SAMPLE_SIZE; x++) {
						size_t pixelIndex = x * 4 + y * stride;

						uint8_t r = pixels[pixelIndex];
						uint8_t g = pixels[pixelIndex + 1];
						uint8_t b = pixels[pixelIndex + 2];

						if ((r | g | b) == 0) continue;

						uint32_t hashKey = rawrbox::PackUtils::toRGBA(r, g, b, 255);
						auto& v = ids[hashKey];
						v++;

						// Get the average pixel
						if (v > max) {
							max = v;
							id = hashKey;
						}
					}
				}

				callback(id);
			});
		});
	}
	// ------
} // namespace rawrbox
