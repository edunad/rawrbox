#include <rawrbox/render/renderers/base.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

#include <fmt/format.h>

namespace rawrbox {
	// STATICS ------
	std::unordered_map<std::string, Diligent::RefCntAutoPtr<Diligent::IPipelineState>> PipelineUtils::_pipelines = {};
	std::unordered_map<std::string, Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding>> PipelineUtils::_binds = {};
	std::unordered_map<std::string, Diligent::RefCntAutoPtr<Diligent::IShader>> PipelineUtils::_shaders = {};
	// -----------------

	Diligent::IShader* PipelineUtils::compileShader(const std::string& name, Diligent::SHADER_TYPE type) {
		if (name.empty()) return nullptr;

		auto fnd = _shaders.find(name);
		if (fnd != _shaders.end()) {
			return fnd->second;
		}

		Diligent::ShaderCreateInfo ShaderCI;
		ShaderCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
		ShaderCI.Desc.UseCombinedTextureSamplers = true; // (g_Texture + g_Texture_sampler combination)
		ShaderCI.pShaderSourceStreamFactory = rawrbox::SHADER_FACTORY;

		ShaderCI.Desc.ShaderType = type;
		ShaderCI.EntryPoint = "main";
		ShaderCI.Desc.Name = fmt::format("RawrBox::{}", name).c_str();
		ShaderCI.FilePath = name.c_str();

		Diligent::RefCntAutoPtr<Diligent::IShader> shader;
		Diligent::RefCntAutoPtr<Diligent::IDataBlob> output;
		rawrbox::RENDERER->device->CreateShader(ShaderCI, &shader, &output);
		if (shader == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Pipeline] Failed to create shader '{}'", name));

		std::string_view compilerOutput = output != nullptr ? std::bit_cast<const char*>(output->GetConstDataPtr()) : "";
		fmt::print("[RawrBox-Shader] Compiled shader '{}'\n", name);
		if (!compilerOutput.empty()) fmt::print("  └── {}\n", compilerOutput);

		_shaders[name] = std::move(shader);
		return _shaders[name];
	}

	Diligent::IPipelineState* PipelineUtils::createPipelines(const std::string& name, const std::string& bindName, const rawrbox::PipeSettings settings, Diligent::IBuffer* uniforms) {
		auto fnd = _pipelines.find(name);
		if (fnd != _pipelines.end()) return fnd->second;

		Diligent::RefCntAutoPtr<Diligent::IPipelineState> pipe;

		// Create pipe info ----
		Diligent::GraphicsPipelineStateCreateInfo info;
		info.PSODesc.Name = fmt::format("RawrBox::{}", name).c_str();
		info.PSODesc.PipelineType = Diligent::PIPELINE_TYPE_GRAPHICS;
		info.GraphicsPipeline.NumRenderTargets = settings.renderTargets;

		auto desc = rawrbox::RENDERER->swapChain->GetDesc();
		info.GraphicsPipeline.RTVFormats[0] = desc.ColorBufferFormat;
		info.GraphicsPipeline.DSVFormat = desc.DepthBufferFormat;

		info.GraphicsPipeline.PrimitiveTopology = settings.topology;
		info.GraphicsPipeline.RasterizerDesc.CullMode = settings.cull;
		info.GraphicsPipeline.DepthStencilDesc.DepthEnable = settings.depth == Diligent::COMPARISON_FUNC_UNKNOWN ? false : true;
		info.GraphicsPipeline.DepthStencilDesc.DepthFunc = settings.depth;
		info.GraphicsPipeline.DepthStencilDesc.DepthWriteEnable = settings.depthWrite;
		info.GraphicsPipeline.RasterizerDesc.ScissorEnable = settings.scissors;
		info.GraphicsPipeline.RasterizerDesc.FillMode = settings.fill;

		Diligent::BlendStateDesc BlendState;
		BlendState.RenderTargets[0].BlendEnable = true;
		BlendState.RenderTargets[0].SrcBlend = Diligent::BLEND_FACTOR_SRC_ALPHA;
		BlendState.RenderTargets[0].DestBlend = Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
		info.GraphicsPipeline.BlendDesc = BlendState;
		// -----

		// Create the shaders (move this to a loader) ----
		Diligent::ShaderCreateInfo ShaderCI;
		ShaderCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
		ShaderCI.Desc.UseCombinedTextureSamplers = true; // (g_Texture + g_Texture_sampler combination)
		ShaderCI.pShaderSourceStreamFactory = rawrbox::SHADER_FACTORY;

		// SHADERS ----
		info.pVS = rawrbox::PipelineUtils::compileShader(settings.pVS, Diligent::SHADER_TYPE_VERTEX);
		info.pPS = rawrbox::PipelineUtils::compileShader(settings.pPS, Diligent::SHADER_TYPE_PIXEL);
		info.pGS = rawrbox::PipelineUtils::compileShader(settings.pGS, Diligent::SHADER_TYPE_PIXEL);
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

			Diligent::SamplerDesc SamLinearClampDesc{
			    Diligent::FILTER_TYPE_POINT, Diligent::FILTER_TYPE_POINT, Diligent::FILTER_TYPE_POINT,
			    Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP, Diligent::TEXTURE_ADDRESS_WRAP};

			for (auto& res : settings.resources) {
				samplers.emplace_back(res.ShaderStages, res.Name, SamLinearClampDesc);
			}

			info.PSODesc.ResourceLayout.ImmutableSamplers = samplers.data();
			info.PSODesc.ResourceLayout.NumImmutableSamplers = static_cast<uint32_t>(samplers.size());
			info.PSODesc.ResourceLayout.DefaultVariableType = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;
		}
		// ---------------------
		rawrbox::RENDERER->device->CreateGraphicsPipelineState(info, &pipe);
		if (pipe == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Pipeline] Failed to create pipeline '{}'", name));

		if (uniforms != nullptr) {
			pipe->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "Constants")->Set(uniforms);
		}

		pipe->CreateShaderResourceBinding(&_binds[bindName], true);
		_pipelines[name] = std::move(pipe);
		return _pipelines[name];
	}

	Diligent::IShaderResourceBinding* PipelineUtils::getBind(const std::string& bindName) {
		auto fnd = _binds.find(bindName);
		if (fnd == _binds.end()) return nullptr;

		return fnd->second;
	}
} // namespace rawrbox
