
#include <rawrbox/render/materials/text.hpp>
#include <rawrbox/render/renderers/base.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

#include <Graphics/GraphicsTools/interface/MapHelper.hpp>
#include <Platforms/Basic/interface/DebugUtilities.hpp>

namespace rawrbox {
	void MaterialText3D::init() {
		Diligent::BufferDesc CBDesc;
		CBDesc.Name = "rawrbox::MaterialText::Uniforms";
		CBDesc.Size = sizeof(rawrbox::MaterialTextUniforms);
		CBDesc.Usage = Diligent::USAGE_DYNAMIC;
		CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
		CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

		rawrbox::RENDERER->device->CreateBuffer(CBDesc, nullptr, &this->_uniforms);

		// PIPELINE ----
		rawrbox::PipeSettings settings;
		settings.psh = "3dtext_unlit.psh";
		settings.vsh = "3dtext_unlit.vsh";
		settings.cull = Diligent::CULL_MODE_FRONT;
		settings.layout = this->vLayout();
		settings.resources = {
		    Diligent::ShaderResourceVariableDesc{Diligent::SHADER_TYPE_PIXEL, "g_Texture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC}};

		rawrbox::PipelineUtils::createPipelines("3DText::Base", settings, &this->_pipelines["base"]);

		settings.fill = Diligent::FILL_MODE_WIREFRAME;
		rawrbox::PipelineUtils::createPipelines("3DText::Base::Wireframe", settings, &this->_pipelines["base-wireframe"]);
		// ----

		for (auto& pipe : this->_pipelines) {
			pipe.second->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "Constants")->Set(this->_uniforms);
			pipe.second->CreateShaderResourceBinding(&this->_SRB, true);
		}
	}

	void MaterialText3D::bindUniforms(const rawrbox::Mesh& mesh) {
		auto context = rawrbox::RENDERER->context;

		// SETUP UNIFORMS ----------------------------
		Diligent::MapHelper<rawrbox::MaterialTextUniforms> CBConstants(context, this->_uniforms, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
		// Map the buffer and write current world-view-projection matrix

		auto tTransform = rawrbox::TRANSFORM.transpose();
		auto tWorldView = rawrbox::MAIN_CAMERA->getProjViewMtx().transpose();
		auto tInvView = rawrbox::MAIN_CAMERA->getViewMtx();
		tInvView.inverse();

		*CBConstants = {
		    // CAMERA -------
		    tTransform * tWorldView,
		    tInvView,
		    // --------------
		    mesh.getData("billboard_mode")};
	}

	void MaterialText3D::bindPipeline(const rawrbox::Mesh& mesh) {
		auto context = rawrbox::RENDERER->context;
		if (mesh.wireframe) {
			context->SetPipelineState(this->_pipelines["base-wireframe"]);
		} else {
			context->SetPipelineState(this->_pipelines["base"]);
		}
	}

	void MaterialText3D::bind(const rawrbox::Mesh& mesh) {
		auto context = rawrbox::RENDERER->context;

		if (mesh.texture != nullptr && mesh.texture->isValid() && !mesh.wireframe) {
			mesh.texture->update(); // Update texture
			this->_SRB->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture")->Set(mesh.texture->getHandle());
		} else {
			this->_SRB->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture")->Set(rawrbox::WHITE_TEXTURE->getHandle());
		}

		this->bindPipeline(mesh);
		this->bindUniforms(mesh);

		context->CommitShaderResources(this->_SRB, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	}

	uint32_t MaterialText3D::supports() const {
		return rawrbox::MaterialBase::supports() | rawrbox::MaterialFlags::TEXT;
	}
} // namespace rawrbox
