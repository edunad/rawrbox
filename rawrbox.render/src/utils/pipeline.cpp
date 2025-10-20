#include <rawrbox/render/models/vertex.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

#include <magic_enum/magic_enum.hpp>

#include <ArchiverFactoryLoader.h>
#include <fmt/color.h>

namespace rawrbox {
	// STATICS ------
	// PRIVATE ----
	std::unordered_map<std::string, Diligent::RefCntAutoPtr<Diligent::IPipelineState>> PipelineUtils::_pipelines = {};
	std::unordered_map<std::string, Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding>> PipelineUtils::_binds = {};
	std::unordered_map<std::string, Diligent::RefCntAutoPtr<Diligent::IShader>> PipelineUtils::_shaders = {};
	std::unordered_map<uint32_t, Diligent::RefCntAutoPtr<Diligent::ISampler>> PipelineUtils::_samplers = {};
	std::unordered_map<Diligent::SHADER_TYPE, Diligent::ShaderMacroHelper> PipelineUtils::_globalMacros = {};

	Diligent::RefCntAutoPtr<Diligent::IRenderStateCache> PipelineUtils::_stateCache;
	std::filesystem::path PipelineUtils::_stateCachePath;

	std::unique_ptr<rawrbox::Logger> PipelineUtils::_logger = std::make_unique<rawrbox::Logger>("RawrBox-Pipeline");
	// -------------

	// PUBLIC ----
	Diligent::ISampler* PipelineUtils::defaultSampler = nullptr;
	bool PipelineUtils::initialized = false;
	// -----------------

	void PipelineUtils::init(Diligent::IRenderDevice& device) {
		if (initialized) return;
		_logger->info("Initializing pipeline utils");

		uint32_t id = Diligent::TEXTURE_ADDRESS_WRAP << 6 | Diligent::TEXTURE_ADDRESS_WRAP << 3 | Diligent::TEXTURE_ADDRESS_WRAP;
		Diligent::SamplerDesc desc{
		    Diligent::FILTER_TYPE_POINT, Diligent::FILTER_TYPE_POINT, Diligent::FILTER_TYPE_POINT,
		    Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP};

		defaultSampler = registerSampler(id, desc);

		// Initialize pipeline cache -----
		Diligent::RenderStateCacheCreateInfo CacheCI;
		CacheCI.pDevice = &device;
		CacheCI.pArchiverFactory = Diligent::LoadAndGetArchiverFactory();
		CacheCI.LogLevel = Diligent::RENDER_STATE_CACHE_LOG_LEVEL_DISABLED;

		Diligent::CreateRenderStateCache(CacheCI, &_stateCache);
		if (_stateCache == nullptr) RAWRBOX_CRITICAL("Failed to initialize PSO shader cache");
		// -------------------------

		// Load cache -----
		const std::filesystem::path cacheRoot = ".shader";
		if (!std::filesystem::exists(cacheRoot)) {
			std::filesystem::create_directory(cacheRoot);
		}

#ifdef _DEBUG
		_stateCachePath = cacheRoot / fmt::format("{}_d.bin", magic_enum::enum_name(rawrbox::RENDERER->getRenderType()));
#else
		_stateCachePath = cacheRoot / fmt::format("{}_r.bin", magic_enum::enum_name(rawrbox::RENDERER->getRenderType()));
#endif

		const auto pString = _stateCachePath.generic_string();
		if (std::filesystem::exists(_stateCachePath)) {
			Diligent::FileWrapper CacheDataFile(pString.c_str());
			auto pCacheData = Diligent::DataBlobImpl::Create();

			if (CacheDataFile->Read(pCacheData)) {
				_logger->debug("Loaded pipeline cache from '{}'", pString);
				_stateCache->Load(pCacheData);
			}
		}
		// ------------------

		initialized = true;
	}

	void PipelineUtils::shutdown() {
		// Save cache -----
		Diligent::RefCntAutoPtr<Diligent::IDataBlob> pCacheData;

		if (_stateCache->WriteToBlob(0, &pCacheData)) {
			if (pCacheData != nullptr) {
				const auto pString = _stateCachePath.generic_string();
				Diligent::FileWrapper CacheDataFile{pString.c_str(), Diligent::EFileAccessMode::Overwrite};
				if (CacheDataFile->Write(pCacheData->GetConstDataPtr(), pCacheData->GetSize())) {
					_logger->debug("Saved pipeline cache to '{}'", pString);
				} else {
					_logger->error("Failed to save pipeline cache to '{}'", pString);
				}
			}
		}
		// ------

		_pipelines.clear();
		_binds.clear();
		_shaders.clear();
		_samplers.clear();
		_globalMacros.clear();

		_logger.reset();

		defaultSampler = nullptr;
		initialized = false;
	}

	Diligent::ISampler* PipelineUtils::registerSampler(uint32_t id, Diligent::SamplerDesc desc) {
		auto fnd = _samplers.find(id);
		if (fnd != _samplers.end()) {
			return fnd->second;
		}

		Diligent::RefCntAutoPtr<Diligent::ISampler> sampler;
		rawrbox::RENDERER->device()->CreateSampler(desc, &sampler);

		_samplers[id] = std::move(sampler);
		return _samplers[id];
	}

	Diligent::IShader* PipelineUtils::compileShader(const std::string& name, Diligent::SHADER_TYPE type, const Diligent::ShaderMacroHelper& macros) {
		if (name.empty()) return nullptr;

		Diligent::ShaderMacroHelper helper = _globalMacros[type] + macros;
		Diligent::ShaderMacroArray a = helper; // TODO: FIX ME
		std::string id = fmt::format("{}-{}", name, a.Count);
		auto fnd = _shaders.find(id);
		if (fnd != _shaders.end()) {
			return fnd->second;
		}

		Diligent::ShaderCreateInfo ShaderCI;
		ShaderCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
		ShaderCI.pShaderSourceStreamFactory = rawrbox::SHADER_FACTORY;
		ShaderCI.CompileFlags = Diligent::SHADER_COMPILE_FLAG_ENABLE_UNBOUNDED_ARRAYS;
		ShaderCI.GLSLExtensions = "#extension GL_EXT_nonuniform_qualifier : require\n";
		if (rawrbox::RENDERER->getRenderType() != Diligent::RENDER_DEVICE_TYPE_D3D12) { // There is a weird bug where DXC runs out of memory when compiling DX12
			ShaderCI.ShaderCompiler = Diligent::SHADER_COMPILER_DXC;
		}

		std::string shaderName = fmt::format("RawrBox::SHADER::{}", name);
		ShaderCI.Desc.ShaderType = type;
		ShaderCI.Desc.UseCombinedTextureSamplers = false;
		ShaderCI.EntryPoint = "main";
		ShaderCI.FilePath = name.c_str();
		ShaderCI.Desc.Name = shaderName.c_str();
		ShaderCI.Macros = helper;

		Diligent::RefCntAutoPtr<Diligent::IShader> shader;
		_stateCache->CreateShader(ShaderCI, &shader);

		if (shader != nullptr) {
			_logger->info("Shader '{}' {}", fmt::styled(name, fmt::fg(fmt::color::coral)), fmt::styled(" [✓ OK]", fmt::fg(fmt::color::green_yellow)));
		} else {
			RAWRBOX_CRITICAL("Shader '{}' {}", fmt::styled(name, fmt::fg(fmt::color::coral)), fmt::styled(" [✖ FAILED]", fmt::fg(fmt::color::red)));
		}

		if (shader == nullptr) RAWRBOX_CRITICAL("Failed to compile shader '{}'", name);
		_shaders[id] = std::move(shader);
		return _shaders[id];
	}

	std::vector<Diligent::ImmutableSamplerDesc> PipelineUtils::compileSamplers(const std::vector<rawrbox::PipeSampler>& samplers) {
		std::vector<Diligent::ImmutableSamplerDesc> ret = {};
		if (samplers.empty()) return ret;

		Diligent::SamplerDesc SamLinearClampDesc{
		    Diligent::FILTER_TYPE_POINT, Diligent::FILTER_TYPE_POINT, Diligent::FILTER_TYPE_POINT,
		    Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP};

		for (const auto& sampler : samplers) {
			ret.emplace_back(sampler.type, sampler.name.c_str(), SamLinearClampDesc);
		}

		return ret;
	}

	Diligent::IPipelineState* PipelineUtils::createComputePipeline(const std::string& name, rawrbox::PipeComputeSettings settings) {
		if (settings.pCS.empty()) RAWRBOX_CRITICAL("Failed to create shader {}, pCS shader cannot be empty!", name);

		auto fnd = _pipelines.find(name);
		if (fnd != _pipelines.end()) return fnd->second;

		Diligent::RefCntAutoPtr<Diligent::IPipelineState> pipe;

		Diligent::ComputePipelineStateCreateInfo PSOCreateInfo;
		std::string pipeName = fmt::format("RawrBox::COMPUTE::{}", name);

		Diligent::PipelineStateDesc& PSODesc = PSOCreateInfo.PSODesc;
		PSODesc.Name = pipeName.c_str();
		PSODesc.PipelineType = Diligent::PIPELINE_TYPE_COMPUTE;
		PSODesc.ResourceLayout.DefaultVariableType = settings.resourceType;

		if (!settings.signatures.empty()) {
			PSOCreateInfo.ppResourceSignatures = settings.signatures.data();
			PSOCreateInfo.ResourceSignaturesCount = static_cast<uint32_t>(settings.signatures.size());
		}

		if (!settings.resources.empty()) {
			PSODesc.ResourceLayout.Variables = settings.resources.data();
			PSODesc.ResourceLayout.NumVariables = static_cast<uint32_t>(settings.resources.size());
		}

		PSOCreateInfo.pCS = rawrbox::PipelineUtils::compileShader(settings.pCS, Diligent::SHADER_TYPE_COMPUTE, settings.macros);

		_stateCache->CreateComputePipelineState(PSOCreateInfo, &pipe);
		if (pipe == nullptr) RAWRBOX_CRITICAL("Failed to create pipeline '{}'", name);

		if (settings.signatures.empty()) {
			for (auto& uni : settings.uniforms) {
				if (uni.uniform == nullptr) continue;
				auto* var = pipe->GetStaticVariableByName(uni.type, uni.name.c_str());

				if (var == nullptr) RAWRBOX_CRITICAL("Failed to create pipeline '{}', could not find variable '{}' on '{}'", name, uni.name, magic_enum::enum_name(uni.type));
				var->Set(uni.uniform);
			}
		}

		// Bind ----
		if (!settings.bind.empty() && settings.signatures.empty()) {
			pipe->CreateShaderResourceBinding(&_binds[settings.bind], true);
		}
		//-----

		_pipelines[name] = std::move(pipe);
		return _pipelines[name];
	}

	Diligent::IPipelineState* PipelineUtils::createPipeline(const std::string& name, rawrbox::PipeSettings settings) {
		auto fnd = _pipelines.find(name);
		if (fnd != _pipelines.end()) return fnd->second;

		const auto& desc = rawrbox::RENDERER->swapChain()->GetDesc();

		Diligent::RefCntAutoPtr<Diligent::IPipelineState> pipe;
		std::string pipeName = fmt::format("RawrBox::{}", name);

		// Create pipe info ----
		Diligent::GraphicsPipelineStateCreateInfo info;
		info.PSODesc.Name = pipeName.c_str();
		info.PSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;
		info.PSODesc.ResourceLayout.DefaultVariableType = settings.resourceType;
		info.GraphicsPipeline.NumRenderTargets = settings.renderTargets;

		info.GraphicsPipeline.PrimitiveTopology = settings.topology;
		info.GraphicsPipeline.RasterizerDesc.CullMode = settings.cull;
		info.GraphicsPipeline.DepthStencilDesc.DepthEnable = settings.depth != Diligent::COMPARISON_FUNC_UNKNOWN;
		info.GraphicsPipeline.DepthStencilDesc.DepthFunc = settings.depth;
		info.GraphicsPipeline.DepthStencilDesc.DepthWriteEnable = settings.depthWrite;
		info.GraphicsPipeline.RasterizerDesc.ScissorEnable = settings.scissors;
		info.GraphicsPipeline.RasterizerDesc.FillMode = settings.fill;

		if (!settings.signatures.empty()) {
			info.ppResourceSignatures = settings.signatures.data();
			info.ResourceSignaturesCount = static_cast<uint32_t>(settings.signatures.size());
		}

		if (settings.depthFormat) {
			info.GraphicsPipeline.DSVFormat = desc.DepthBufferFormat;
		}

		// When using render pass, renderTargets must be 0 and Depth unknown
		if (settings.renderPass.pass != nullptr) {
			info.GraphicsPipeline.pRenderPass = settings.renderPass.pass;
			info.GraphicsPipeline.SubpassIndex = settings.renderPass.index;
			info.GraphicsPipeline.NumRenderTargets = 0;
		} else {
			for (size_t i = 0; i < settings.renderTargets; i++) {
				info.GraphicsPipeline.RTVFormats[i] = desc.ColorBufferFormat;
			}
		}

		if (settings.blending.src != Diligent::BLEND_FACTOR_UNDEFINED && settings.blending.dest != Diligent::BLEND_FACTOR_UNDEFINED) {
			Diligent::BlendStateDesc BlendState;
			BlendState.RenderTargets[0].BlendEnable = true;
			BlendState.RenderTargets[0].SrcBlend = settings.blending.src;
			BlendState.RenderTargets[0].DestBlend = settings.blending.dest;

			info.GraphicsPipeline.BlendDesc = BlendState;
		}
		// -----

		// SHADERS ----
		info.pVS = rawrbox::PipelineUtils::compileShader(settings.pVS, Diligent::SHADER_TYPE_VERTEX, settings.macros);
		info.pPS = rawrbox::PipelineUtils::compileShader(settings.pPS, Diligent::SHADER_TYPE_PIXEL, settings.macros);
		info.pGS = rawrbox::PipelineUtils::compileShader(settings.pGS, Diligent::SHADER_TYPE_GEOMETRY, settings.macros);
		// -------------

		// Layout
		if (!settings.layout.empty()) {
			info.GraphicsPipeline.InputLayout.LayoutElements = settings.layout.data();
			info.GraphicsPipeline.InputLayout.NumElements = static_cast<uint32_t>(settings.layout.size());
		}
		// ----

		// Resources ----
		auto samplers = compileSamplers(settings.immutableSamplers);
		if (!settings.resources.empty()) {
			info.PSODesc.ResourceLayout.Variables = settings.resources.data();
			info.PSODesc.ResourceLayout.NumVariables = static_cast<uint32_t>(settings.resources.size());

			if (!samplers.empty()) {
				info.PSODesc.ResourceLayout.ImmutableSamplers = samplers.data();
				info.PSODesc.ResourceLayout.NumImmutableSamplers = static_cast<uint32_t>(samplers.size());
			}
		}

		// ---------------------
		_stateCache->CreateGraphicsPipelineState(info, &pipe);
		if (pipe == nullptr) RAWRBOX_CRITICAL("Failed to create pipeline '{}'", name);

		if (settings.signatures.empty()) {
			for (auto& uni : settings.uniforms) {
				if (uni.uniform == nullptr) continue;

				auto* var = pipe->GetStaticVariableByName(uni.type, uni.name.c_str());
				if (var == nullptr) RAWRBOX_CRITICAL("Failed to create pipeline '{}', could not find variable '{}' on '{}'", name, uni.name, magic_enum::enum_name(uni.type));

				var->Set(uni.uniform);
			}
		}

		// Bind ----
		if (!settings.bind.empty() && settings.signatures.empty()) {
			pipe->CreateShaderResourceBinding(&_binds[settings.bind], true);
		}
		//-----

		_pipelines[name] = std::move(pipe);
		return _pipelines[name];
	}

	Diligent::IShaderResourceBinding* PipelineUtils::getBind(const std::string& bindName) {
		auto fnd = _binds.find(bindName);
		if (fnd == _binds.end()) return nullptr;
		return fnd->second;
	}

	Diligent::ISampler* PipelineUtils::getSampler(uint32_t id) {
		auto fnd = _samplers.find(id);
		if (fnd == _samplers.end()) return nullptr;
		return fnd->second;
	}

	Diligent::IPipelineState* PipelineUtils::getPipeline(const std::string& pipe) {
		auto fnd = _pipelines.find(pipe);
		if (fnd == _pipelines.end()) return nullptr;
		return fnd->second;
	}

} // namespace rawrbox
