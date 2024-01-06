#include <rawrbox/render/static.hpp>
#include <rawrbox/render/textures/manager.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

#include <magic_enum.hpp>

#include <fmt/format.h>

namespace rawrbox {
	// STATICS ------
	// PRIVATE ----
	std::unordered_map<std::string, Diligent::RefCntAutoPtr<Diligent::IPipelineState>> PipelineUtils::_pipelines = {};
	std::unordered_map<std::string, Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding>> PipelineUtils::_binds = {};
	std::unordered_map<std::string, Diligent::RefCntAutoPtr<Diligent::IShader>> PipelineUtils::_shaders = {};
	std::unordered_map<uint32_t, Diligent::RefCntAutoPtr<Diligent::ISampler>> PipelineUtils::_samplers = {};

	Diligent::RefCntAutoPtr<Diligent::IRenderStateCache> PipelineUtils::_stateCache;
	// -------------

	// PUBLIC ----
	Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature> PipelineUtils::signature;
	Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> PipelineUtils::signatureBind;

	Diligent::ISampler* PipelineUtils::defaultSampler = nullptr;
	bool PipelineUtils::initialized = false;
	// -----------------

	void PipelineUtils::init(Diligent::IRenderDevice& device) {
		if (initialized) return;

		uint32_t id = Diligent::TEXTURE_ADDRESS_WRAP << 6 | Diligent::TEXTURE_ADDRESS_WRAP << 3 | Diligent::TEXTURE_ADDRESS_WRAP;
		Diligent::SamplerDesc desc{
		    Diligent::FILTER_TYPE_POINT, Diligent::FILTER_TYPE_POINT, Diligent::FILTER_TYPE_POINT,
		    Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP};

		defaultSampler = registerSampler(id, desc);

		// Initialize pipeline cache -----
		Diligent::RenderStateCacheCreateInfo CacheCI;
		CacheCI.pDevice = &device;
#ifdef _DEBUG
		CacheCI.LogLevel = Diligent::RENDER_STATE_CACHE_LOG_LEVEL_VERBOSE;
		CacheCI.EnableHotReload = true;
#endif
		Diligent::CreateRenderStateCache(CacheCI, &_stateCache);
		// -------------------------

		initialized = true;
	}

	void PipelineUtils::createSignature() {
		if (signature != nullptr) throw std::runtime_error("[RawrBox-Pipeline] Signature already bound!");

		Diligent::PipelineResourceSignatureDesc PRSDesc;
		PRSDesc.Name = "RawrBox::SIGNATURE::BINDLESS";
		PRSDesc.BindingIndex = 0;

		// RESOURCES -----
		std::vector<Diligent::PipelineResourceDesc> resources = {
		    {Diligent::SHADER_TYPE_VERTEX, "Camera", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
		    {Diligent::SHADER_TYPE_PIXEL, "Camera", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},

		    {Diligent::SHADER_TYPE_VERTEX, "Constants", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
		    {Diligent::SHADER_TYPE_PIXEL, "Constants", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},

		    {Diligent::SHADER_TYPE_PIXEL, "g_Textures", rawrbox::RENDERER->MAX_TEXTURES, Diligent::SHADER_RESOURCE_TYPE_TEXTURE_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE, Diligent::PIPELINE_RESOURCE_FLAG_RUNTIME_ARRAY},
		    {Diligent::SHADER_TYPE_PIXEL, "g_Textures_sampler", 1, Diligent::SHADER_RESOURCE_TYPE_SAMPLER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
		};

		PRSDesc.Resources = resources.data();
		PRSDesc.NumResources = static_cast<uint8_t>(resources.size());
		PRSDesc.UseCombinedTextureSamplers = true;
		// --------------

		// SAMPLERS -----
		Diligent::SamplerDesc SamLinearClampDesc{
		    Diligent::FILTER_TYPE_POINT, Diligent::FILTER_TYPE_POINT, Diligent::FILTER_TYPE_POINT,
		    Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP};

		std::vector<Diligent::ImmutableSamplerDesc> samplers = {
		    {Diligent::SHADER_TYPE_PIXEL, "g_Textures", SamLinearClampDesc},
		};

		PRSDesc.ImmutableSamplers = samplers.data();
		PRSDesc.NumImmutableSamplers = static_cast<uint32_t>(samplers.size());
		// --------------

		rawrbox::RENDERER->device()->CreatePipelineResourceSignature(PRSDesc, &signature);

		// Setup binds ---
		signature->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "Camera")->Set(rawrbox::RENDERER->camera()->uniforms());
		signature->GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "Camera")->Set(rawrbox::RENDERER->camera()->uniforms());
		// ----------------

		signature->CreateShaderResourceBinding(&signatureBind, true);

		// Setup textures ---
		signatureBind->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Textures")->SetArray(rawrbox::TextureManager::getHandles().data(), 0, static_cast<uint32_t>(rawrbox::TextureManager::total()), Diligent::SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
		rawrbox::TextureManager::onUpdate += []() {
			signatureBind->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Textures")->SetArray(rawrbox::TextureManager::getHandles().data(), 0, static_cast<uint32_t>(rawrbox::TextureManager::total()), Diligent::SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
		};
		// ------------------
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

	Diligent::IShader* PipelineUtils::compileShader(const std::string& name, Diligent::SHADER_TYPE type, Diligent::ShaderMacroArray macros) {
		if (name.empty()) return nullptr;
		std::string id = fmt::format("{}-{}", name, macros.Count);

		auto fnd = _shaders.find(id);
		if (fnd != _shaders.end()) {
			return fnd->second;
		}

		Diligent::ShaderCreateInfo ShaderCI;
		ShaderCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
		ShaderCI.Desc.UseCombinedTextureSamplers = true; // (g_Texture + g_Texture_sampler combination)
		ShaderCI.pShaderSourceStreamFactory = rawrbox::SHADER_FACTORY;
		ShaderCI.CompileFlags = Diligent::SHADER_COMPILE_FLAG_ENABLE_UNBOUNDED_ARRAYS;

		std::string shaderName = fmt::format("RawrBox::SHADER::{}", name);

		ShaderCI.Desc.ShaderType = type;
		ShaderCI.EntryPoint = "main";
		ShaderCI.Desc.Name = shaderName.c_str();
		ShaderCI.FilePath = name.c_str();
		ShaderCI.Macros = macros;

		Diligent::RefCntAutoPtr<Diligent::IShader> shader;
		Diligent::RefCntAutoPtr<Diligent::IDataBlob> output;
		rawrbox::RENDERER->device()->CreateShader(ShaderCI, &shader, &output);
		if (shader == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Pipeline] Failed to compile shader '{}'", name));

		std::string_view compilerOutput = output != nullptr ? std::bit_cast<const char*>(output->GetConstDataPtr()) : "";
		fmt::print("[RawrBox-Shader] Compiled shader '{}'\n", name);
		if (!compilerOutput.empty()) fmt::print("  └── {}\n", compilerOutput);

		_shaders[id] = std::move(shader);
		return _shaders[id];
	}

	std::vector<Diligent::ImmutableSamplerDesc> PipelineUtils::compileSamplers(const std::vector<rawrbox::PipeSampler>& samplers) {
		std::vector<Diligent::ImmutableSamplerDesc> ret = {};
		if (samplers.empty()) return ret;

		Diligent::SamplerDesc SamLinearClampDesc{
		    Diligent::FILTER_TYPE_POINT, Diligent::FILTER_TYPE_POINT, Diligent::FILTER_TYPE_POINT,
		    Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP};

		for (auto& sampler : samplers) {
			ret.emplace_back(sampler.type, sampler.name.c_str(), SamLinearClampDesc);
		}

		return ret;
	}

	Diligent::IPipelineState* PipelineUtils::createComputePipeline(const std::string& name, rawrbox::PipeComputeSettings settings) {
		if (settings.pCS.empty()) throw std::runtime_error(fmt::format("[RawrBox-Pipeline] Failed to create shader {}, pCS shader cannot be empty!", name));

		auto fnd = _pipelines.find(name);
		if (fnd != _pipelines.end()) return fnd->second;

		Diligent::RefCntAutoPtr<Diligent::IPipelineState> pipe;

		Diligent::ComputePipelineStateCreateInfo PSOCreateInfo;
		std::string pipeName = fmt::format("RawrBox::COMPUTE::{}", name);

		Diligent::PipelineStateDesc& PSODesc = PSOCreateInfo.PSODesc;
		PSODesc.Name = pipeName.c_str();
		PSODesc.PipelineType = Diligent::PIPELINE_TYPE_COMPUTE;
		PSODesc.ResourceLayout.DefaultVariableType = settings.resourceType;

		std::array<Diligent::IPipelineResourceSignature*, 1> signatures = {settings.signature};
		if (settings.signature != nullptr) {
			PSOCreateInfo.ppResourceSignatures = signatures.data();
			PSOCreateInfo.ResourceSignaturesCount = 1U;
		}

		if (!settings.resources.empty()) {
			PSODesc.ResourceLayout.Variables = settings.resources.data();
			PSODesc.ResourceLayout.NumVariables = static_cast<uint32_t>(settings.resources.size());
		}

		PSOCreateInfo.pCS = rawrbox::PipelineUtils::compileShader(settings.pCS, Diligent::SHADER_TYPE_COMPUTE, settings.macros);

		rawrbox::RENDERER->device()->CreateComputePipelineState(PSOCreateInfo, &pipe);
		if (pipe == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Pipeline] Failed to create pipeline '{}'", name));

		if (settings.signature == nullptr) {
			for (auto& uni : settings.uniforms) {
				if (uni.uniform == nullptr) continue;
				auto var = pipe->GetStaticVariableByName(uni.type, uni.name.c_str());

				if (var == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Pipeline] Failed to create pipeline '{}', could not find variable '{}' on '{}'", name, uni.name, magic_enum::enum_name(uni.type)));
				var->Set(uni.uniform);
			}
		}

		// Bind ----
		if (!settings.bind.empty() && settings.signature == nullptr) {
			pipe->CreateShaderResourceBinding(&_binds[settings.bind], true);
		}
		//-----

		_pipelines[name] = std::move(pipe);
		return _pipelines[name];
	}

	Diligent::IPipelineState* PipelineUtils::createPipeline(const std::string& name, rawrbox::PipeSettings settings) {
		auto fnd = _pipelines.find(name);
		if (fnd != _pipelines.end()) return fnd->second;

		auto& desc = rawrbox::RENDERER->swapChain()->GetDesc();

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
		info.GraphicsPipeline.DepthStencilDesc.DepthEnable = settings.depth == Diligent::COMPARISON_FUNC_UNKNOWN ? false : true;
		info.GraphicsPipeline.DepthStencilDesc.DepthFunc = settings.depth;
		info.GraphicsPipeline.DepthStencilDesc.DepthWriteEnable = settings.depthWrite;
		info.GraphicsPipeline.RasterizerDesc.ScissorEnable = settings.scissors;
		info.GraphicsPipeline.RasterizerDesc.FillMode = settings.fill;

		std::array<Diligent::IPipelineResourceSignature*, 1> signatures = {settings.signature};
		if (settings.signature != nullptr) {
			info.ppResourceSignatures = signatures.data();
			info.ResourceSignaturesCount = 1U;
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
			info.GraphicsPipeline.RTVFormats[0] = desc.ColorBufferFormat;
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
		rawrbox::RENDERER->device()->CreateGraphicsPipelineState(info, &pipe);
		if (pipe == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Pipeline] Failed to create pipeline '{}'", name));

		if (settings.signature == nullptr) {
			for (auto& uni : settings.uniforms) {
				if (uni.uniform == nullptr) continue;

				auto var = pipe->GetStaticVariableByName(uni.type, uni.name.c_str());
				if (var == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Pipeline] Failed to create pipeline '{}', could not find variable '{}' on '{}'", name, uni.name, magic_enum::enum_name(uni.type)));

				var->Set(uni.uniform);
			}
		}

		// Bind ----
		if (!settings.bind.empty() && settings.signature == nullptr) {
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
