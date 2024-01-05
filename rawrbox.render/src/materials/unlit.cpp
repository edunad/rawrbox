
#include <rawrbox/render/materials/unlit.hpp>

namespace rawrbox {

	// STATIC DATA ----
	Diligent::RefCntAutoPtr<Diligent::IBuffer> MaterialUnlit::_uniforms;
	Diligent::RefCntAutoPtr<Diligent::IBuffer> MaterialUnlit::_uniforms_pixel;

	bool MaterialUnlit::_built = false;
	// ----------------

	void MaterialUnlit::init() {
		const std::string id = "Model::Unlit";

		if (!this->_built) {
			fmt::print("[RawrBox-MaterialUnlit] Building material..\n");

			this->createUniforms();
			this->createPipelines(id, vertexBufferType::vLayout(), this->_uniforms, this->_uniforms_pixel);

			this->_built = true;
		}

		this->setupPipelines(id);
	}

	void MaterialUnlit::createPipelines(const std::string& id, const std::vector<Diligent::LayoutElement>& layout, Diligent::IBuffer* uniforms, Diligent::IBuffer* pixelUniforms, Diligent::ShaderMacroHelper helper) {
		// PIPELINE ----
		rawrbox::PipeSettings settings;
		settings.pVS = "unlit.vsh";
		settings.pPS = "unlit.psh";
		settings.cull = Diligent::CULL_MODE_FRONT;
		settings.macros = helper;
		settings.layout = layout;
		settings.immutableSamplers = {
		    {Diligent::SHADER_TYPE_PIXEL, "g_Textures"},
		    // {Diligent::SHADER_TYPE_PIXEL, "g_DecalTexture"},
		    //{Diligent::SHADER_TYPE_VERTEX, "g_Displacement"}
		};

		/*settings.resources = {
		    {Diligent::SHADER_TYPE_PIXEL, "g_Textures", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},

		    //{Diligent::SHADER_TYPE_PIXEL, "g_DecalTexture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
		    //{Diligent::SHADER_TYPE_PIXEL, "g_Decals", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},

		    //{Diligent::SHADER_TYPE_VERTEX, "g_Displacement", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
		};*/

		settings.uniforms = {
		    {Diligent::SHADER_TYPE_VERTEX, rawrbox::MAIN_CAMERA->uniforms(), "Camera"},

		    //   {Diligent::SHADER_TYPE_PIXEL, rawrbox::DECALS::uniforms, "Decals"},
		    // {Diligent::SHADER_TYPE_PIXEL, rawrbox::DECALS::getBuffer(), "g_Decals"},

		    {Diligent::SHADER_TYPE_VERTEX, uniforms, "Constants"},
		    {Diligent::SHADER_TYPE_PIXEL, pixelUniforms, "Constants"}

		};

		// Create signatures ---
		rawrbox::PipeSignatureSettings signature = {};
		signature.immutableSamplers = settings.immutableSamplers;
		signature.desc = {
		    Diligent::PipelineResourceDesc{Diligent::SHADER_TYPE_VERTEX, "Camera", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
		    Diligent::PipelineResourceDesc{Diligent::SHADER_TYPE_VERTEX, "Constants", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},

		    Diligent::PipelineResourceDesc{Diligent::SHADER_TYPE_PIXEL, "Constants", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},

		    Diligent::PipelineResourceDesc{Diligent::SHADER_TYPE_PIXEL, "g_Textures", rawrbox::RENDERER->MAX_TEXTURES, Diligent::SHADER_RESOURCE_TYPE_TEXTURE_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE, Diligent::PIPELINE_RESOURCE_FLAG_RUNTIME_ARRAY},
		    Diligent::PipelineResourceDesc{Diligent::SHADER_TYPE_PIXEL, "g_Textures_sampler", 1, Diligent::SHADER_RESOURCE_TYPE_SAMPLER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
		};

		settings.signature = rawrbox::PipelineUtils::createSignature(id, signature);
		// ---------------------

		rawrbox::PipelineUtils::createPipeline(id, id, settings);

		settings.blending = {Diligent::BLEND_FACTOR_SRC_ALPHA, Diligent::BLEND_FACTOR_INV_SRC_ALPHA};
		rawrbox::PipelineUtils::createPipeline(id + "::Alpha", id, settings);

		settings.blending = {};
		settings.topology = Diligent::PRIMITIVE_TOPOLOGY_LINE_LIST;
		settings.cull = Diligent::CULL_MODE_NONE;
		rawrbox::PipelineUtils::createPipeline(id + "::Line", id, settings);

		settings.topology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		settings.cull = Diligent::CULL_MODE_BACK;
		rawrbox::PipelineUtils::createPipeline(id + "::CullBack", id, settings);

		settings.blending = {Diligent::BLEND_FACTOR_SRC_ALPHA, Diligent::BLEND_FACTOR_INV_SRC_ALPHA};
		rawrbox::PipelineUtils::createPipeline(id + "::CullBack::Alpha", id, settings);

		settings.cull = Diligent::CULL_MODE_NONE;
		rawrbox::PipelineUtils::createPipeline(id + "::CullNone::Alpha", id, settings);

		settings.blending = {};
		rawrbox::PipelineUtils::createPipeline(id + "::CullNone", id, settings);

		settings.blending = {};
		settings.fill = Diligent::FILL_MODE_WIREFRAME;
		rawrbox::PipelineUtils::createPipeline(id + "::Wireframe", id, settings);
		// -----
	}

	void MaterialUnlit::createUniforms() {
		if (this->_uniforms != nullptr || this->_uniforms_pixel != nullptr) return;

		// Uniforms -------
		{
			Diligent::BufferDesc CBDesc;
			CBDesc.Name = "rawrbox::MaterialUnlit::Vertex::Uniforms";
			CBDesc.Size = sizeof(rawrbox::MaterialBaseUniforms);
			CBDesc.Usage = Diligent::USAGE_DYNAMIC;
			CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

			rawrbox::RENDERER->device()->CreateBuffer(CBDesc, nullptr, &this->_uniforms);
		}
		// ------------

		// Pixel Uniforms -------
		{
			Diligent::BufferDesc CBDesc;
			CBDesc.Name = "rawrbox::MaterialUnlit::Pixel::Uniforms";
			CBDesc.Size = sizeof(rawrbox::MaterialBasePixelUniforms);
			CBDesc.Usage = Diligent::USAGE_DYNAMIC;
			CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

			rawrbox::RENDERER->device()->CreateBuffer(CBDesc, nullptr, &this->_uniforms_pixel);
		}
		// ------------
	}

	void MaterialUnlit::bindShaderResources() const {
		if (this->_bind == nullptr) throw std::runtime_error("[RawrBox-MaterialUnlit] Bind not set!");

		auto context = rawrbox::RENDERER->context();
		context->CommitShaderResources(this->_bind, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	}
} // namespace rawrbox
