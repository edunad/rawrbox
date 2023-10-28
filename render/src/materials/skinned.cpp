#include <rawrbox/render/materials/skinned.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

namespace rawrbox {
	// STATIC DATA ----
	Diligent::RefCntAutoPtr<Diligent::IBuffer> MaterialSkinned::_uniforms;
	// ----------------

	void MaterialSkinned::init() {
		// Uniforms -------
		Diligent::BufferDesc CBDesc;
		CBDesc.Name = "rawrbox::MaterialSkinned::Uniforms";
		CBDesc.Size = sizeof(rawrbox::MaterialSkinnedUniforms);
		CBDesc.Usage = Diligent::USAGE_DYNAMIC;
		CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
		CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

		rawrbox::RENDERER->device()->CreateBuffer(CBDesc, nullptr, &_uniforms);
		// ------------

		// PIPELINE ----
		rawrbox::PipeSettings settings;
		settings.pVS = "unlit_skinned.vsh";
		settings.pPS = "unlit.psh";
		settings.cull = Diligent::CULL_MODE_FRONT;
		settings.layout = rawrbox::VertexBoneData::vLayout();
		settings.immutableSamplers = {false, true};
		settings.resources = {
		    Diligent::ShaderResourceVariableDesc{Diligent::SHADER_TYPE_PIXEL, "g_Texture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
		    Diligent::ShaderResourceVariableDesc{Diligent::SHADER_TYPE_VERTEX, "g_Displacement", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC}};
		settings.uniforms = {{Diligent::SHADER_TYPE_VERTEX, _uniforms}};

		settings.fill = Diligent::FILL_MODE_WIREFRAME;
		rawrbox::PipelineUtils::createPipelines("Model::Skinned::Wireframe", "Model::Skinned", settings);

		settings.fill = Diligent::FILL_MODE_SOLID;
		rawrbox::PipelineUtils::createPipelines("Model::Skinned", "Model::Skinned", settings);

		settings.blending = {Diligent::BLEND_FACTOR_SRC_ALPHA, Diligent::BLEND_FACTOR_INV_SRC_ALPHA};
		rawrbox::PipelineUtils::createPipelines("Model::Skinned::Alpha", "Model::Skinned", settings);

		settings.fill = Diligent::FILL_MODE_SOLID;
		settings.cull = Diligent::CULL_MODE_BACK;
		settings.blending = {};
		rawrbox::PipelineUtils::createPipelines("Model::Skinned::CullBack", "Model::Skinned", settings);

		settings.blending = {Diligent::BLEND_FACTOR_SRC_ALPHA, Diligent::BLEND_FACTOR_INV_SRC_ALPHA};
		rawrbox::PipelineUtils::createPipelines("Model::Skinned::CullBack::Alpha", "Model::Skinned", settings);

		settings.cull = Diligent::CULL_MODE_NONE;
		settings.blending = {};
		rawrbox::PipelineUtils::createPipelines("Model::Skinned::CullNone", "Model::Skinned", settings);

		settings.blending = {Diligent::BLEND_FACTOR_SRC_ALPHA, Diligent::BLEND_FACTOR_INV_SRC_ALPHA};
		rawrbox::PipelineUtils::createPipelines("Model::Skinned::CullNone::Alpha", "Model::Skinned", settings);

		// -----
	}

	void MaterialSkinned::prepareMaterial() {
		// Not a fan, but had to move it away from static, since we want to override them
		if (this->_base == nullptr) this->_base = rawrbox::PipelineUtils::getPipeline("Model::Skinned");
		if (this->_base_alpha == nullptr) this->_base_alpha = rawrbox::PipelineUtils::getPipeline("Model::Skinned::Alpha");

		if (this->_wireframe == nullptr) this->_wireframe = rawrbox::PipelineUtils::getPipeline("Model::Skinned::Wireframe");

		if (this->_cullback == nullptr) this->_cullback = rawrbox::PipelineUtils::getPipeline("Model::Skinned::CullBack");
		if (this->_cullback_alpha == nullptr) this->_cullback_alpha = rawrbox::PipelineUtils::getPipeline("Model::Skinned::CullBack::Alpha");

		if (this->_cullnone == nullptr) this->_cullnone = rawrbox::PipelineUtils::getPipeline("Model::Skinned::CullNone");
		if (this->_cullnone_alpha == nullptr) this->_cullnone_alpha = rawrbox::PipelineUtils::getPipeline("Model::Skinned::CullNone::Alpha");

		if (this->_bind == nullptr) this->_bind = rawrbox::PipelineUtils::getBind("Model::Skinned");
	}

	void MaterialSkinned::bindUniforms(const rawrbox::Mesh& mesh) {
		auto context = rawrbox::RENDERER->context();

		// SETUP UNIFORMS ----------------------------
		Diligent::MapHelper<rawrbox::MaterialSkinnedUniforms> CBConstants(context, this->_uniforms, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
		this->bindBaseUniforms<rawrbox::MaterialSkinnedUniforms>(mesh, CBConstants);
		// ------------

		(*CBConstants).g_bones = {};
	}

	uint32_t MaterialSkinned::supports() const {
		return rawrbox::MaterialBase::supports() | rawrbox::MaterialFlags::BONES;
	}

	void* MaterialSkinned::convert(const std::vector<rawrbox::ModelVertexData>& v) {
		this->_temp.reserve(v.size());
		std::transform(v.begin(), v.end(),
		    std::back_inserter(this->_temp),
		    [](const rawrbox::ModelVertexData& data) -> rawrbox::VertexBoneData { return {data.position, data.uv, data.color, data.bone_indices, data.bone_weights}; });

		return this->_temp.data();
	}

	const uint32_t MaterialSkinned::vLayoutSize() {
		return sizeof(rawrbox::VertexBoneData);
	}
} // namespace rawrbox
