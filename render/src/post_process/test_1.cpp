
#include <rawrbox/render/post_process/test_1.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

namespace rawrbox {
	void Test1::upload(Diligent::IRenderPass* pass, uint32_t passIndex) {

		// const auto UseGLSL =
		//     m_pDevice->GetDeviceInfo().IsVulkanDevice() ||
		//     m_pDevice->GetDeviceInfo().IsMetalDevice();

		Diligent::GraphicsPipelineStateCreateInfo PSOCreateInfo;
		Diligent::PipelineStateDesc& PSODesc = PSOCreateInfo.PSODesc;

		PSODesc.Name = "Ambient light PSO";

		PSOCreateInfo.GraphicsPipeline.pRenderPass = pass;
		PSOCreateInfo.GraphicsPipeline.SubpassIndex = passIndex; // This PSO will be used within the second subpass

		PSOCreateInfo.GraphicsPipeline.PrimitiveTopology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode = Diligent::CULL_MODE_NONE;
		PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = false; // Disable depth

		Diligent::ShaderCreateInfo ShaderCI;
		ShaderCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
		ShaderCI.Desc.UseCombinedTextureSamplers = true;

		ShaderCI.pShaderSourceStreamFactory = rawrbox::render::SHADER_FACTORY;
		// Create a vertex shader
		Diligent::RefCntAutoPtr<Diligent::IShader> pVS;
		{
			ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_VERTEX;
			ShaderCI.EntryPoint = "main";
			ShaderCI.Desc.Name = "Ambient light VS";
			ShaderCI.FilePath = "post_process_base.vsh";
			rawrbox::render::RENDERER->device()->CreateShader(ShaderCI, &pVS);
			VERIFY_EXPR(pVS != nullptr);
		}

		// Create a pixel shader
		Diligent::RefCntAutoPtr<Diligent::IShader> pPS;
		{
			ShaderCI.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
			ShaderCI.Desc.ShaderType = Diligent::SHADER_TYPE_PIXEL;
			ShaderCI.EntryPoint = "main";
			ShaderCI.Desc.Name = "Ambient light PS";
			ShaderCI.FilePath = "test_1.psh";
			rawrbox::render::RENDERER->device()->CreateShader(ShaderCI, &pPS);
			VERIFY_EXPR(pPS != nullptr);
		}

		PSOCreateInfo.pVS = pVS;
		PSOCreateInfo.pPS = pPS;

		PSODesc.ResourceLayout.DefaultVariableType = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

		Diligent::ShaderResourceVariableDesc Vars[] =
		    {
			{Diligent::SHADER_TYPE_PIXEL, "g_SubpassInputColor", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE}};

		PSODesc.ResourceLayout.Variables = Vars;
		PSODesc.ResourceLayout.NumVariables = _countof(Vars);

		rawrbox::render::RENDERER->device()->CreateGraphicsPipelineState(PSOCreateInfo, &test);
		test->CreateShaderResourceBinding(&testsrg, true);

		/*rawrbox::PipeSettings settings;
		settings.depth = Diligent::COMPARISON_FUNC_UNKNOWN;
		settings.cull = Diligent::CULL_MODE_NONE;
		settings.topology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		settings.pVS = "post_process_base.vsh";
		settings.pPS = "test_1.psh";
		settings.renderTargets = 0;
		settings.renderPass = {
		    pass,
		    passIndex};
		settings.resources = {
		    Diligent::ShaderResourceVariableDesc{Diligent::SHADER_TYPE_PIXEL, "g_SubpassInputColor", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE}};

		this->_pipeline = rawrbox::PipelineUtils::createPipelines("PostProcess::Test1", "PostProcess::Test1", settings);

		this->_bind = rawrbox::PipelineUtils::getBind("PostProcess::Test1");*/
	}

	void Test1::setRTTexture(Diligent::ITextureView* texture) {
		if (auto* pInputColor = testsrg->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_SubpassInputColor"))
			pInputColor->Set(texture);
	}

	void Test1::applyEffect() {
		auto context = rawrbox::render::RENDERER->context();

		context->SetPipelineState(test);
		context->CommitShaderResources(testsrg, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);

		// Diligent::StateTransitionDesc Barriers[] =
		//     {
		//	Diligent::StateTransitionDesc(texture->GetTexture(), Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, true)};
		//
		// context->TransitionResourceStates(1, Barriers);

		// Draw quad
		Diligent::DrawAttribs DrawAttrs;
		DrawAttrs.NumVertices = 4;
		DrawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL; // Verify the state of vertex and index buffers
		context->Draw(DrawAttrs);
	}
} // namespace rawrbox
