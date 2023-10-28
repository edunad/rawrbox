
#include <rawrbox/render/materials/base.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/pipeline.hpp>
#include <rawrbox/utils/pack.hpp>

#include <Graphics/GraphicsTools/interface/MapHelper.hpp>
#include <Platforms/Basic/interface/DebugUtilities.hpp>

namespace rawrbox {

	// STATIC DATA ----
	Diligent::RefCntAutoPtr<Diligent::IBuffer> MaterialBase::_uniforms;
	// ----------------

	void MaterialBase::init() {
		// Uniforms -------
		Diligent::BufferDesc CBDesc;
		CBDesc.Name = "rawrbox::MaterialBase::Uniforms";
		CBDesc.Size = sizeof(rawrbox::MaterialBaseUniforms);
		CBDesc.Usage = Diligent::USAGE_DYNAMIC;
		CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
		CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

		rawrbox::RENDERER->device()->CreateBuffer(CBDesc, nullptr, &_uniforms);
		// ------------

		// PIPELINE ----
		rawrbox::PipeSettings settings;
		settings.pVS = "unlit.vsh";
		settings.pPS = "unlit.psh";
		settings.cull = Diligent::CULL_MODE_FRONT;
		settings.layout = rawrbox::VertexData::vLayout();
		settings.immutableSamplers = {false, true};
		settings.resources = {
		    Diligent::ShaderResourceVariableDesc{Diligent::SHADER_TYPE_PIXEL, "g_Texture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
		    Diligent::ShaderResourceVariableDesc{Diligent::SHADER_TYPE_VERTEX, "g_Displacement", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC}};
		settings.uniforms = {{Diligent::SHADER_TYPE_VERTEX, _uniforms}};

		rawrbox::PipelineUtils::createPipelines("Model::Base", "Model::Base", settings);

		settings.blending = {Diligent::BLEND_FACTOR_SRC_ALPHA, Diligent::BLEND_FACTOR_INV_SRC_ALPHA};
		rawrbox::PipelineUtils::createPipelines("Model::Base::Alpha", "Model::Base", settings);

		settings.blending = {};
		settings.topology = Diligent::PRIMITIVE_TOPOLOGY_LINE_LIST;
		settings.cull = Diligent::CULL_MODE_NONE;
		rawrbox::PipelineUtils::createPipelines("Model::Line", "Model::Base", settings);

		settings.topology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		settings.cull = Diligent::CULL_MODE_BACK;
		rawrbox::PipelineUtils::createPipelines("Model::Base::CullBack", "Model::Base", settings);

		settings.blending = {Diligent::BLEND_FACTOR_SRC_ALPHA, Diligent::BLEND_FACTOR_INV_SRC_ALPHA};
		rawrbox::PipelineUtils::createPipelines("Model::Base::CullBack::Alpha", "Model::Base", settings);

		settings.cull = Diligent::CULL_MODE_NONE;
		rawrbox::PipelineUtils::createPipelines("Model::Base::CullNone::Alpha", "Model::Base", settings);

		settings.blending = {};
		rawrbox::PipelineUtils::createPipelines("Model::Base::CullNone", "Model::Base", settings);

		settings.blending = {Diligent::BLEND_FACTOR_SRC_ALPHA, Diligent::BLEND_FACTOR_INV_SRC_ALPHA};
		rawrbox::PipelineUtils::createPipelines("Model::Base::CullNone::Alpha", "Model::Base", settings);

		settings.blending = {};
		settings.fill = Diligent::FILL_MODE_WIREFRAME;
		rawrbox::PipelineUtils::createPipelines("Model::Base::Wireframe", "Model::Base", settings);
		// -----
	}

	void MaterialBase::prepareMaterial() {
		// Not a fan, but had to move it away from static, since we want to override them
		if (this->_base == nullptr) this->_base = rawrbox::PipelineUtils::getPipeline("Model::Base");
		if (this->_base_alpha == nullptr) this->_base_alpha = rawrbox::PipelineUtils::getPipeline("Model::Base::Alpha");

		if (this->_line == nullptr) this->_line = rawrbox::PipelineUtils::getPipeline("Model::Line");

		if (this->_cullback == nullptr) this->_cullback = rawrbox::PipelineUtils::getPipeline("Model::Base::CullBack");
		if (this->_cullback_alpha == nullptr) this->_cullback_alpha = rawrbox::PipelineUtils::getPipeline("Model::Base::CullBack::Alpha");

		if (this->_wireframe == nullptr) this->_wireframe = rawrbox::PipelineUtils::getPipeline("Model::Base::Wireframe");

		if (this->_cullnone == nullptr) this->_cullnone = rawrbox::PipelineUtils::getPipeline("Model::Base::CullNone");
		if (this->_cullnone_alpha == nullptr) this->_cullnone_alpha = rawrbox::PipelineUtils::getPipeline("Model::Base::CullNone::Alpha");

		if (this->_bind == nullptr) this->_bind = rawrbox::PipelineUtils::getBind("Model::Base");
	}

	void MaterialBase::bindUniforms(const rawrbox::Mesh& mesh) {
		auto renderer = rawrbox::RENDERER;
		auto context = renderer->context();

		// SETUP UNIFORMS ----------------------------
		Diligent::MapHelper<rawrbox::MaterialBaseUniforms> CBConstants(context, this->_uniforms, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
		this->bindBaseUniforms(mesh, CBConstants);
		// ------------
	}

	void MaterialBase::bindPipeline(const rawrbox::Mesh& mesh) {
		auto context = rawrbox::RENDERER->context();

		if (mesh.wireframe) {
			if (this->_line == nullptr) throw std::runtime_error("[RawrBox-Material] Wireframe not supported on material");
			context->SetPipelineState(this->_wireframe);
		} else if (mesh.lineMode) {
			if (this->_line == nullptr) throw std::runtime_error("[RawrBox-Material] Line not supported on material");
			context->SetPipelineState(this->_line);
		} else {
			if (mesh.culling == Diligent::CULL_MODE_NONE) {
				if (this->_cullnone == nullptr) throw std::runtime_error("[RawrBox-Material] Disabled cull not supported on material");
				if (this->_cullnone_alpha == nullptr) throw std::runtime_error("[RawrBox-Material] Disabled alpha cull not supported on material");
				context->SetPipelineState(mesh.alphaBlend ? this->_cullnone_alpha : this->_cullnone);
			} else if (mesh.culling == Diligent::CULL_MODE_BACK) {
				if (this->_cullback == nullptr) throw std::runtime_error("[RawrBox-Material] Cull back not supported on material");
				if (this->_cullback_alpha == nullptr) throw std::runtime_error("[RawrBox-Material] Cull back alpha not supported on material");
				context->SetPipelineState(mesh.alphaBlend ? this->_cullback_alpha : this->_cullback);
			} else {
				if (this->_cullback == nullptr) throw std::runtime_error("[RawrBox-Material] Cull back not supported on material");
				if (this->_cullback_alpha == nullptr) throw std::runtime_error("[RawrBox-Material] Cull back alpha cull not supported on material");
				if (this->_base_alpha == nullptr) throw std::runtime_error("[RawrBox-Material] Cull back alpha cull not supported on material");

				context->SetPipelineState(mesh.culling == Diligent::CULL_MODE_FRONT ? this->_base : this->_cullback);
			}
		}
	}

	void MaterialBase::bindShaderResources() {
		if (this->_bind == nullptr) throw std::runtime_error("[RawrBox-MaterialBase] Bind not set!");

		auto context = rawrbox::RENDERER->context();
		context->CommitShaderResources(this->_bind, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	}

	void MaterialBase::bind(const rawrbox::Mesh& mesh) {
		auto context = rawrbox::RENDERER->context();
		this->prepareMaterial();

		rawrbox::TextureBase* textureColor = rawrbox::WHITE_TEXTURE.get();
		rawrbox::TextureBase* textureDisplacement = rawrbox::BLACK_TEXTURE.get();

		if (mesh.texture != nullptr && mesh.texture->isValid() && !mesh.wireframe) {
			mesh.texture->update(); // Update texture
			textureColor = mesh.texture;
		}

		if (mesh.displacementTexture != nullptr && mesh.displacementTexture->isValid()) {
			mesh.displacementTexture->update(); // Update texture
			textureDisplacement = mesh.displacementTexture;
		}

		auto handle = textureColor->getHandle();
		handle->SetSampler(textureColor->getSampler());

		this->_bind->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture")->Set(handle);
		this->_bind->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "g_Displacement")->Set(textureDisplacement->getHandle());

		this->bindPipeline(mesh);
		this->bindUniforms(mesh);
		this->bindShaderResources();
	}

	uint32_t MaterialBase::supports() const {
		return rawrbox::MaterialFlags::NONE;
	}

	void* MaterialBase::convert(const std::vector<rawrbox::ModelVertexData>& v) {
		this->_temp.reserve(v.size());
		std::transform(v.begin(), v.end(),
		    std::back_inserter(this->_temp),
		    [](const rawrbox::ModelVertexData& data) -> rawrbox::VertexData { return {data.position, data.uv, data.color}; });

		return this->_temp.data();
	}

	const uint32_t MaterialBase::vLayoutSize() {
		return sizeof(rawrbox::VertexData);
	}
} // namespace rawrbox
