
#include <rawrbox/render/materials/skinnedLit.hpp>

namespace rawrbox {
	// STATIC DATA ----
	bool MaterialSkinnedLit::_built = false;

	Diligent::RefCntAutoPtr<Diligent::IBuffer> MaterialSkinnedLit::_uniforms;
	Diligent::RefCntAutoPtr<Diligent::IBuffer> MaterialSkinnedLit::_uniforms_pixel;
	// ----------------

	void MaterialSkinnedLit::createUniforms() {
		if (this->_uniforms != nullptr || this->_uniforms_pixel != nullptr) return;

		// Uniforms -------
		{
			Diligent::BufferDesc CBDesc;
			CBDesc.Name = "rawrbox::MaterialSkinnedLit::Vertex::Uniforms";
			CBDesc.Size = sizeof(rawrbox::MaterialSkinnedUniforms);
			CBDesc.Usage = Diligent::USAGE_DYNAMIC;
			CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

			rawrbox::RENDERER->device()->CreateBuffer(CBDesc, nullptr, &this->_uniforms);
		}

		{
			Diligent::BufferDesc CBDesc;
			CBDesc.Name = "rawrbox::MaterialSkinnedLit::Pixel::Uniforms";
			CBDesc.Size = sizeof(rawrbox::MaterialLitPixelUniforms);
			CBDesc.Usage = Diligent::USAGE_DYNAMIC;
			CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

			rawrbox::RENDERER->device()->CreateBuffer(CBDesc, nullptr, &this->_uniforms_pixel);
		}
		// ------------
	}

	void MaterialSkinnedLit::init() {
		const std::string id = "Model::Skinned::Lit";

		if (!this->_built) {
			fmt::print("[RawrBox-MaterialSkinnedLit] Building material..\n");

			Diligent::ShaderMacroHelper helper;
			helper.AddShaderMacro("MAX_BONES", rawrbox::MAX_BONES_PER_MODEL);
			helper.AddShaderMacro("NUM_BONES_PER_VERTEX", rawrbox::MAX_BONES_PER_VERTEX);
			helper.AddShaderMacro("SKINNED", true);

			this->createUniforms();
			this->createPipelines(id, vertexBufferType::vLayout(), helper);

			this->_built = true;
		}

		this->setupPipelines(id);
	}
} // namespace rawrbox
