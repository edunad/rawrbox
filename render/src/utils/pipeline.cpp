#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

#include <fmt/format.h>

namespace rawrbox {
	// STATICS ------
	// PRIVATE ----
	std::unordered_map<std::string, Diligent::RefCntAutoPtr<Diligent::IPipelineState>> PipelineUtils::_pipelines = {};
	std::unordered_map<std::string, Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding>> PipelineUtils::_binds = {};
	std::unordered_map<std::string, Diligent::RefCntAutoPtr<Diligent::IShader>> PipelineUtils::_shaders = {};
	std::unordered_map<uint32_t, Diligent::RefCntAutoPtr<Diligent::ISampler>> PipelineUtils::_samplers = {};
	// -------------

	Diligent::ISampler* PipelineUtils::defaultSampler = nullptr;
	bool PipelineUtils::initialized = false;
	// -----------------

	void PipelineUtils::init() {
		uint32_t id = Diligent::TEXTURE_ADDRESS_WRAP << 6 | Diligent::TEXTURE_ADDRESS_WRAP << 3 | Diligent::TEXTURE_ADDRESS_WRAP;
		Diligent::SamplerDesc desc{
		    Diligent::FILTER_TYPE_POINT, Diligent::FILTER_TYPE_POINT, Diligent::FILTER_TYPE_POINT,
		    Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP};

		defaultSampler = registerSampler(id, desc);
		initialized = true;
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

		auto fnd = _shaders.find(name);
		if (fnd != _shaders.end()) {
			return fnd->second;
		}

		Diligent::ShaderCreateInfo ShaderCI;
		ShaderCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
		ShaderCI.Desc.UseCombinedTextureSamplers = true; // (g_Texture + g_Texture_sampler combination)
		ShaderCI.pShaderSourceStreamFactory = rawrbox::SHADER_FACTORY;

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

		_shaders[name] = std::move(shader);
		return _shaders[name];
	}

	Diligent::IPipelineState* PipelineUtils::createComputePipeline(const std::string& name, const std::string& bindName, const rawrbox::PipeComputeSettings settings) {
		if (settings.pCS.empty()) throw std::runtime_error(fmt::format("[RawrBox-Pipeline] Failed to create shader {}, pCS shader cannot be empty!", name));

		auto fnd = _pipelines.find(name);
		if (fnd != _pipelines.end()) return fnd->second;

		Diligent::RefCntAutoPtr<Diligent::IPipelineState> pipe;

		Diligent::ComputePipelineStateCreateInfo PSOCreateInfo;
		Diligent::PipelineStateDesc& PSODesc = PSOCreateInfo.PSODesc;

		std::string pipeName = fmt::format("RawrBox::COMPUTE::{}", name);

		PSODesc.Name = pipeName.c_str();
		PSODesc.PipelineType = Diligent::PIPELINE_TYPE_COMPUTE;
		PSODesc.ResourceLayout.DefaultVariableType = settings.resourceType;

		if (!settings.resources.empty()) {
			PSODesc.ResourceLayout.Variables = settings.resources.data();
			PSODesc.ResourceLayout.NumVariables = static_cast<uint32_t>(settings.resources.size());
		}

		PSOCreateInfo.pCS = rawrbox::PipelineUtils::compileShader(settings.pCS, Diligent::SHADER_TYPE_COMPUTE, settings.macros);

		rawrbox::RENDERER->device()->CreateComputePipelineState(PSOCreateInfo, &pipe);
		if (pipe == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Pipeline] Failed to create pipeline '{}'", name));

		for (auto& uni : settings.uniforms) {
			if (uni.uniform == nullptr) continue;
			auto var = pipe->GetStaticVariableByName(uni.type, uni.name.c_str());

			if (var == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Pipeline] Failed to create pipeline '{}', could not find variable '{}'", name, uni.name));
			var->Set(uni.uniform);
		}

		// Bind ----
		if (!bindName.empty()) {
			pipe->CreateShaderResourceBinding(&_binds[bindName], true);
		}
		//-----

		_pipelines[name] = std::move(pipe);
		return _pipelines[name];
	}

	Diligent::IPipelineState* PipelineUtils::createPipeline(const std::string& name, const std::string& bindName, const rawrbox::PipeSettings settings) {
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
		std::vector<Diligent::ImmutableSamplerDesc> samplers = {};

		if (!settings.resources.empty()) {
			info.PSODesc.ResourceLayout.Variables = settings.resources.data();
			info.PSODesc.ResourceLayout.NumVariables = static_cast<uint32_t>(settings.resources.size());

			if (!settings.immutableSamplers.empty()) {
				Diligent::SamplerDesc SamLinearClampDesc{
				    Diligent::FILTER_TYPE_POINT, Diligent::FILTER_TYPE_POINT, Diligent::FILTER_TYPE_POINT,
				    Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP};

				for (auto& sampler : settings.immutableSamplers) {
					samplers.emplace_back(sampler.type, sampler.name.c_str(), SamLinearClampDesc);
				}

				info.PSODesc.ResourceLayout.ImmutableSamplers = samplers.data();
				info.PSODesc.ResourceLayout.NumImmutableSamplers = static_cast<uint32_t>(samplers.size());
			}
		}

		// ---------------------
		rawrbox::RENDERER->device()->CreateGraphicsPipelineState(info, &pipe);
		if (pipe == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Pipeline] Failed to create pipeline '{}'", name));

		for (auto& uni : settings.uniforms) {
			if (uni.uniform == nullptr) continue;
			auto var = pipe->GetStaticVariableByName(uni.type, uni.name.c_str());

			if (var == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Pipeline] Failed to create pipeline '{}', could not find variable '{}'", name, uni.name));
			var->Set(uni.uniform);
		}

		if (!bindName.empty()) {
			pipe->CreateShaderResourceBinding(&_binds[bindName], true);
		}

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
