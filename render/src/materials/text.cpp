
#include <rawrbox/render/materials/text.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

#include <Graphics/GraphicsTools/interface/MapHelper.hpp>
#include <Platforms/Basic/interface/DebugUtilities.hpp>

namespace rawrbox {
	// STATIC DATA ----
	Diligent::RefCntAutoPtr<Diligent::IBuffer> MaterialText3D::_uniforms;

	Diligent::IPipelineState* MaterialText3D::_base = nullptr;
	Diligent::IPipelineState* MaterialText3D::_wireframe = nullptr;

	Diligent::IShaderResourceBinding* MaterialText3D::_bind = nullptr;
	// ----------------

	void MaterialText3D::init() {
		// Uniforms -------
		Diligent::BufferDesc CBDesc;
		CBDesc.Name = "rawrbox::MaterialText3D::Uniforms";
		CBDesc.Size = sizeof(rawrbox::MaterialTextUniforms);
		CBDesc.Usage = Diligent::USAGE_DYNAMIC;
		CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
		CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

		rawrbox::RENDERER->device()->CreateBuffer(CBDesc, nullptr, &_uniforms);
		// ------------

		// PIPELINE ----
		rawrbox::PipeSettings settings;
		settings.pVS = "3dtext_unlit.vsh";
		settings.pPS = "3dtext_unlit.psh";
		settings.cull = Diligent::CULL_MODE_FRONT;
		settings.layout = vLayout().first;
		settings.uniforms = {{Diligent::SHADER_TYPE_VERTEX, _uniforms}};
		settings.resources = {
		    Diligent::ShaderResourceVariableDesc{Diligent::SHADER_TYPE_PIXEL, "g_Texture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC}};

		_base = rawrbox::PipelineUtils::createPipelines("3DText::Base", "3DText", settings);

		settings.fill = Diligent::FILL_MODE_WIREFRAME;
		_wireframe = rawrbox::PipelineUtils::createPipelines("3DText::Base::Wireframe", "3DText", settings);
		// ----

		_bind = rawrbox::PipelineUtils::getBind("3DText");
	}

	void MaterialText3D::bindUniforms(const rawrbox::Mesh& mesh) {
		auto renderer = rawrbox::RENDERER;
		auto context = renderer->context();

		// SETUP UNIFORMS ----------------------------
		Diligent::MapHelper<rawrbox::MaterialTextUniforms> CBConstants(context, this->_uniforms, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
		// Map the buffer and write current world-view-projection matrix

		auto tTransform = rawrbox::TRANSFORM.transpose();
		auto tWorldView = renderer->camera()->getProjViewMtx().transpose();
		auto tInvView = renderer->camera()->getViewMtx();
		tInvView.inverse();

		*CBConstants = {
		    // CAMERA -------
		    tTransform * tWorldView,
		    tInvView,
		    // --------------
		    mesh.getData("billboard_mode")};
	}

	void MaterialText3D::bindPipeline(const rawrbox::Mesh& mesh) {
		auto context = rawrbox::RENDERER->context();
		if (mesh.wireframe) {
			context->SetPipelineState(_wireframe);
		} else {
			context->SetPipelineState(_base);
		}
	}

	void MaterialText3D::bind(const rawrbox::Mesh& mesh) {
		auto context = rawrbox::RENDERER->context();

		if (mesh.texture != nullptr && mesh.texture->isValid() && !mesh.wireframe) {
			mesh.texture->update(); // Update texture
			_bind->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture")->Set(mesh.texture->getHandle());
		} else {
			_bind->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture")->Set(rawrbox::WHITE_TEXTURE->getHandle());
		}

		this->bindPipeline(mesh);
		this->bindUniforms(mesh);

		context->CommitShaderResources(_bind, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	}

	uint32_t MaterialText3D::supports() const {
		return rawrbox::MaterialBase::supports() | rawrbox::MaterialFlags::TEXT;
	}
} // namespace rawrbox
