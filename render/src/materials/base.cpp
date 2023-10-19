
#include <rawrbox/render/materials/base.hpp>
#include <rawrbox/render/renderers/base.hpp>
#include <rawrbox/render/utils/pipeline.hpp>
#include <rawrbox/utils/pack.hpp>

#include <Graphics/GraphicsTools/interface/MapHelper.hpp>
#include <Platforms/Basic/interface/DebugUtilities.hpp>

namespace rawrbox {
	constexpr auto MAX_DATA = 4;

	void MaterialBase::init() {
		Diligent::BufferDesc CBDesc;
		CBDesc.Name = "rawrbox::MaterialBase::Uniforms";
		CBDesc.Size = sizeof(rawrbox::MaterialUniforms);
		CBDesc.Usage = Diligent::USAGE_DYNAMIC;
		CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
		CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

		rawrbox::RENDERER->device->CreateBuffer(CBDesc, nullptr, &this->_uniforms);

		// PIPELINE ----
		rawrbox::PipeSettings settings;
		settings.psh = "unlit.psh";
		settings.vsh = "unlit.vsh";
		settings.cull = Diligent::CULL_MODE_FRONT;
		settings.layout = this->vLayout().first;
		settings.resources = {
		    Diligent::ShaderResourceVariableDesc{Diligent::SHADER_TYPE_PIXEL, "g_Texture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
		    Diligent::ShaderResourceVariableDesc{Diligent::SHADER_TYPE_VERTEX, "g_Displacement", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC}};

		rawrbox::PipelineUtils::createPipelines("Model::Base", settings, &this->_pipelines["base"]);

		settings.topology = Diligent::PRIMITIVE_TOPOLOGY_LINE_LIST;
		settings.cull = Diligent::CULL_MODE_NONE;
		rawrbox::PipelineUtils::createPipelines("Model::Line", settings, &this->_pipelines["line"]);

		settings.topology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		settings.cull = Diligent::CULL_MODE_BACK;
		rawrbox::PipelineUtils::createPipelines("Model::Base::CullBack", settings, &this->_pipelines["base-back-cull"]);

		settings.fill = Diligent::FILL_MODE_WIREFRAME;
		rawrbox::PipelineUtils::createPipelines("Model::Base::Wireframe", settings, &this->_pipelines["base-wireframe"]);

		settings.fill = Diligent::FILL_MODE_SOLID;
		settings.topology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		settings.cull = Diligent::CULL_MODE_NONE;
		rawrbox::PipelineUtils::createPipelines("Model::Base::CullNone", settings, &this->_pipelines["base-no-cull"]);

		for (auto& pipe : this->_pipelines) {
			pipe.second->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "Constants")->Set(this->_uniforms);
			pipe.second->CreateShaderResourceBinding(&this->_SRB, true);
		}
		// -----

		/*this->registerUniform("s_albedo", bgfx::UniformType::Sampler);
		this->registerUniform("s_displacement", bgfx::UniformType::Sampler);

		this->registerUniform("u_colorOffset", bgfx::UniformType::Vec4);
		this->registerUniform("u_data", bgfx::UniformType::Vec4, MAX_DATA);
		this->registerUniform("u_tex_flags", bgfx::UniformType::Vec4);*/
	}

	MaterialBase::~MaterialBase() {
		for (auto& pipes : this->_pipelines) {
			RAWRBOX_DESTROY(pipes.second);
		}

		this->_pipelines.clear();
	}

	void MaterialBase::bindUniforms(const rawrbox::Mesh& mesh) {
		auto context = rawrbox::RENDERER->context;

		// SETUP UNIFORMS ----------------------------
		Diligent::MapHelper<rawrbox::MaterialUniforms> CBConstants(context, this->_uniforms, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
		// Map the buffer and write current world-view-projection matrix

		auto size = rawrbox::RENDERER->getSize().cast<float>();
		auto tTransform = rawrbox::TRANSFORM.transpose();
		auto tProj = rawrbox::MAIN_CAMERA->getProjMtx().transpose();
		auto tView = rawrbox::MAIN_CAMERA->getViewMtx().transpose();
		auto tInvView = rawrbox::MAIN_CAMERA->getViewMtx();
		tInvView.inverse();

		auto tWorldView = rawrbox::MAIN_CAMERA->getProjViewMtx().transpose();

		std::array<rawrbox::Vector4f, MAX_DATA>
		    data = {rawrbox::Vector4f{0.F, 0.F, 0.F, 0.F}, {0.F, 0.F, 0.F, 0.F}, {0.F, 0.F, 0.F, 0.F}, {0.F, 0.F, 0.F, 0.F}};
		if (mesh.hasData("billboard_mode")) {
			data[0] = mesh.getData("billboard_mode").data();
		}

		if (mesh.hasData("vertex_snap")) {
			data[1] = mesh.getData("vertex_snap").data();
		}

		if (mesh.hasData("displacement_strength")) {
			data[2] = mesh.getData("displacement_strength").data();
		}

		if (mesh.hasData("mask")) {
			data[3] = mesh.getData("mask").data();
		}

		*CBConstants = {
		    // CAMERA -------
		    tTransform,
		    tProj,
		    tView,
		    tInvView,
		    tTransform * tWorldView,
		    size,
		    // --------------
		    mesh.color,
		    data};
		// ----------------------------
		/*
		// bgfx::setUniform(this->getUniform("u_tex_flags"), mesh.texture->getData().data());

		// Color override
		bgfx::setUniform(this->getUniform("u_colorOffset"), mesh.color.data().data());
		// -------


		// Bind extra renderer uniforms ---
		rawrbox::RENDERER->bindRenderUniforms();
		// ---*/
	}

	void MaterialBase::bindPipeline(const rawrbox::Mesh& mesh) {
		auto context = rawrbox::RENDERER->context;

		if (mesh.wireframe) {
			context->SetPipelineState(this->_pipelines["base-wireframe"]);
		} else if (mesh.lineMode) {
			context->SetPipelineState(this->_pipelines["line"]);
		} else {
			if (mesh.culling == Diligent::CULL_MODE_NONE) {
				context->SetPipelineState(this->_pipelines["base-no-cull"]);
			} else {
				context->SetPipelineState(mesh.culling == Diligent::CULL_MODE_FRONT ? this->_pipelines["base"] : this->_pipelines["base-back-cull"]);
			}
		}
	}

	void MaterialBase::bind(const rawrbox::Mesh& mesh) {
		auto context = rawrbox::RENDERER->context;

		if (mesh.texture != nullptr && mesh.texture->isValid() && !mesh.wireframe) {
			mesh.texture->update(); // Update texture
			this->_SRB->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture")->Set(mesh.texture->getHandle());
		} else {
			this->_SRB->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture")->Set(rawrbox::WHITE_TEXTURE->getHandle());
		}

		if (mesh.displacementTexture != nullptr && mesh.displacementTexture->isValid()) {
			this->_SRB->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "g_Displacement")->Set(mesh.displacementTexture->getHandle());
		} else {
			this->_SRB->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "g_Displacement")->Set(rawrbox::BLACK_TEXTURE->getHandle());
		}

		this->bindPipeline(mesh);
		this->bindUniforms(mesh);

		context->CommitShaderResources(this->_SRB, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	}

	uint32_t MaterialBase::supports() const {
		return rawrbox::MaterialFlags::NONE;
	}

	const std::pair<std::vector<Diligent::LayoutElement>, uint32_t> MaterialBase::vLayout() const {
		return {rawrbox::VertexData::vLayout(), rawrbox::VertexData::vLayoutSize()};
	}
} // namespace rawrbox
