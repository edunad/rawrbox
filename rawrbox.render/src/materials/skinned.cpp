#include <rawrbox/render/materials/skinned.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

namespace rawrbox {
	// STATIC DATA ----
	Diligent::RefCntAutoPtr<Diligent::IBuffer> MaterialSkinned::_uniforms;
	Diligent::RefCntAutoPtr<Diligent::IBuffer> MaterialSkinned::_uniforms_pixel;

	bool MaterialSkinned::_built = false;
	// ----------------

	void MaterialSkinned::createUniforms() {
		if (this->_uniforms != nullptr) return;

		// Uniforms -------
		{
			Diligent::BufferDesc CBDesc;
			CBDesc.Name = "rawrbox::MaterialSkinned::Uniforms";
			CBDesc.Size = sizeof(rawrbox::MaterialSkinnedUniforms);
			CBDesc.Usage = Diligent::USAGE_DYNAMIC;
			CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

			rawrbox::RENDERER->device()->CreateBuffer(CBDesc, nullptr, &this->_uniforms);
		}
		// ------------

		// Pixel Uniforms -------
		{
			Diligent::BufferDesc CBDesc;
			CBDesc.Name = "rawrbox::MaterialSkinned::Uniforms::Pixel";
			CBDesc.Size = sizeof(rawrbox::MaterialBasePixelUniforms);
			CBDesc.Usage = Diligent::USAGE_DYNAMIC;
			CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

			rawrbox::RENDERER->device()->CreateBuffer(CBDesc, nullptr, &this->_uniforms_pixel);
		}
		// ------------
	}

	void MaterialSkinned::init() {
		const std::string id = "Model::Skinned";

		if (!this->_built) {
			fmt::print("[RawrBox-MaterialSkinned] Building material..\n");

			Diligent::ShaderMacroHelper helper;
			helper.AddShaderMacro("MAX_BONES", rawrbox::MAX_BONES_PER_MODEL);
			helper.AddShaderMacro("NUM_BONES_PER_VERTEX", rawrbox::MAX_BONES_PER_VERTEX);
			helper.AddShaderMacro("SKINNED", true);

			this->createUniforms();
			this->createPipelines(id, vertexBufferType::vLayout(), this->_uniforms, this->_uniforms_pixel, helper);

			this->_built = true;
		}

		this->setupPipelines(id);
	}
} // namespace rawrbox
