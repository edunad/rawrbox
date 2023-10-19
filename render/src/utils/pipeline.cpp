#include <rawrbox/render/renderers/base.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

#include <fmt/format.h>

namespace rawrbox {
	void PipelineUtils::createPipelines(const std::string& name, const rawrbox::PipeSettings settings, Diligent::IPipelineState** pipe) {
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

		Diligent::RefCntAutoPtr<Diligent::IShader> pVS;
		if (!settings.vsh.empty()) {
			ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
			ShaderCI.EntryPoint = "main";
			ShaderCI.Desc.Name = fmt::format("RawrBox::{}::VS", name).c_str();
			ShaderCI.FilePath = settings.vsh.c_str();

			rawrbox::RENDERER->device->CreateShader(ShaderCI, &pVS);
			if (pVS == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Pipeline] Failed to create VERTEX shader '{}'", settings.vsh));
			info.pVS = pVS;
		}

		// Create a pixel shader
		Diligent::RefCntAutoPtr<Diligent::IShader> pPS;
		if (!settings.psh.empty()) {
			ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
			ShaderCI.EntryPoint = "main";
			ShaderCI.Desc.Name = fmt::format("RawrBox::{}::PS", name).c_str();
			ShaderCI.FilePath = settings.psh.c_str();

			rawrbox::RENDERER->device->CreateShader(ShaderCI, &pPS);
			if (pPS == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Pipeline] Failed to create PIXEL shader '{}'", settings.psh));
			info.pPS = pPS;
		}
		// ----------------------

		// Create a geometry shader
		Diligent::RefCntAutoPtr<Diligent::IShader> pGEO;
		if (!settings.gsh.empty()) {
			ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_GEOMETRY;
			ShaderCI.EntryPoint = "main";
			ShaderCI.Desc.Name = fmt::format("RawrBox::{}::GEO", name).c_str();
			ShaderCI.FilePath = settings.gsh.c_str();

			rawrbox::RENDERER->device->CreateShader(ShaderCI, &pGEO);
			if (pGEO == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Pipeline] Failed to create GEOMETRY shader '{}'", settings.gsh));
			info.pGS = pGEO;
		}
		// ----------------------

		// Layout
		info.GraphicsPipeline.InputLayout.LayoutElements = settings.layout.data();
		info.GraphicsPipeline.InputLayout.NumElements = static_cast<uint32_t>(settings.layout.size());
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
		rawrbox::RENDERER->device->CreateGraphicsPipelineState(info, pipe);
		if (pipe == nullptr) throw std::runtime_error(fmt::format("[RawrBox-Pipeline] Failed to create pipeline '{}'", name));
	}
} // namespace rawrbox
