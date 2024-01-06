#include <rawrbox/render/lights/manager.hpp>
#include <rawrbox/render/materials/lit.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

namespace rawrbox {
	// STATIC DATA ----
	Diligent::RefCntAutoPtr<Diligent::IBuffer> MaterialLit::_uniforms;
	Diligent::RefCntAutoPtr<Diligent::IBuffer> MaterialLit::_uniforms_pixel;

	bool MaterialLit::_built = false;
	// ----------------

	void MaterialLit::createUniforms() {
		if (this->_uniforms != nullptr || this->_uniforms_pixel != nullptr) return;

		// Uniforms -------
		{
			Diligent::BufferDesc CBDesc;
			CBDesc.Name = "rawrbox::MaterialLit::Vertex::Uniforms";
			CBDesc.Size = sizeof(rawrbox::MaterialBaseUniforms);
			CBDesc.Usage = Diligent::USAGE_DYNAMIC;
			CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

			rawrbox::RENDERER->device()->CreateBuffer(CBDesc, nullptr, &this->_uniforms);
		}

		{
			Diligent::BufferDesc CBDesc;
			CBDesc.Name = "rawrbox::MaterialLit::Pixel::Uniforms";
			CBDesc.Size = sizeof(rawrbox::MaterialLitPixelUniforms);
			CBDesc.Usage = Diligent::USAGE_DYNAMIC;
			CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

			rawrbox::RENDERER->device()->CreateBuffer(CBDesc, nullptr, &this->_uniforms_pixel);
		}
		// ------------
	}

	void MaterialLit::init() {
		const std::string id = "Model::Lit";

		if (!this->_built) {
			fmt::print("[RawrBox-MaterialLit] Building material..\n");

			this->createUniforms();
			this->createPipelines(id, vertexBufferType::vLayout());

			this->_built = true;
		}

		this->setupPipelines(id);
	}

	void MaterialLit::createPipelines(const std::string& id, const std::vector<Diligent::LayoutElement>& layout, Diligent::ShaderMacroHelper helper) {
		/*auto cluster = rawrbox::RENDERER->getPlugin<rawrbox::ClusteredPlugin>("Clustered::Light");
		if (cluster == nullptr) throw std::runtime_error("[RawrBox-MaterialLit] This material requires the `ClusteredLightPlugin` renderer plugin");

		// PIPELINE ----
		rawrbox::PipeSettings settings;
		settings.pVS = "lit.vsh";
		settings.pPS = "lit.psh";
		settings.cull = Diligent::CULL_MODE_FRONT;
		settings.macros = cluster->getClusterMacros() + helper;

		settings.layout = layout;
		settings.immutableSamplers = {
		    // TODO : REPLACE WITH BINDLESS ----
		    {Diligent::SHADER_TYPE_VERTEX, "g_Displacement"},

		    {Diligent::SHADER_TYPE_PIXEL, "g_Texture"},
		    {Diligent::SHADER_TYPE_PIXEL, "g_Normal"},
		    {Diligent::SHADER_TYPE_PIXEL, "g_Emission"},
		    {Diligent::SHADER_TYPE_PIXEL, "g_DecalTexture"},
		    {Diligent::SHADER_TYPE_PIXEL, "g_RoughMetal"}
		    //---------------------------------
		};

		settings.resources = {
		    // TODO : REPLACE WITH BINDLESS ----
		    {Diligent::SHADER_TYPE_PIXEL, "g_Texture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
		    {Diligent::SHADER_TYPE_VERTEX, "g_Displacement", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
		    {Diligent::SHADER_TYPE_PIXEL, "g_Normal", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
		    {Diligent::SHADER_TYPE_PIXEL, "g_Emission", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
		    {Diligent::SHADER_TYPE_PIXEL, "g_DecalTexture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
		    {Diligent::SHADER_TYPE_PIXEL, "g_RoughMetal", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
		    // ----------------------------------

		    {Diligent::SHADER_TYPE_PIXEL, "Constants", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
		    {Diligent::SHADER_TYPE_PIXEL, "Camera", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},

		    {Diligent::SHADER_TYPE_VERTEX, "Constants", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
		    {Diligent::SHADER_TYPE_VERTEX, "Camera", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
		};

		settings.uniforms = {
		    {Diligent::SHADER_TYPE_VERTEX, uniforms, "Constants"},
		    {Diligent::SHADER_TYPE_VERTEX, rawrbox::MAIN_CAMERA->uniforms(), "Camera"},

		    {Diligent::SHADER_TYPE_PIXEL, pixelUniforms, "Constants"},
		    {Diligent::SHADER_TYPE_PIXEL, rawrbox::MAIN_CAMERA->uniforms(), "Camera"}};

		// Apply cluster resources
		cluster->applyPipelineSettings(settings, true);
		// -------------

		settings.fill = Diligent::FILL_MODE_SOLID;
		rawrbox::PipelineUtils::createPipeline(id, id, settings);

		settings.blending = {Diligent::BLEND_FACTOR_SRC_ALPHA, Diligent::BLEND_FACTOR_INV_SRC_ALPHA};
		rawrbox::PipelineUtils::createPipeline(id + "::Alpha", id, settings);

		settings.fill = Diligent::FILL_MODE_SOLID;
		settings.cull = Diligent::CULL_MODE_BACK;
		settings.blending = {};
		rawrbox::PipelineUtils::createPipeline(id + "::CullBack", id, settings);

		settings.blending = {Diligent::BLEND_FACTOR_SRC_ALPHA, Diligent::BLEND_FACTOR_INV_SRC_ALPHA};
		rawrbox::PipelineUtils::createPipeline(id + "::CullBack::Alpha", id, settings);

		settings.cull = Diligent::CULL_MODE_NONE;
		settings.blending = {};
		rawrbox::PipelineUtils::createPipeline(id + "::CullNone", id, settings);

		settings.blending = {Diligent::BLEND_FACTOR_SRC_ALPHA, Diligent::BLEND_FACTOR_INV_SRC_ALPHA};
		rawrbox::PipelineUtils::createPipeline(id + "::CullNone::Alpha", id, settings);*/
		// -----
	}
} // namespace rawrbox
