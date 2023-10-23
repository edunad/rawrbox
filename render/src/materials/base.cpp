
#include <rawrbox/render/materials/base.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/pipeline.hpp>
#include <rawrbox/utils/pack.hpp>

#include <Graphics/GraphicsTools/interface/MapHelper.hpp>
#include <Platforms/Basic/interface/DebugUtilities.hpp>

namespace rawrbox {
	constexpr auto MAX_DATA = 4;

	// STATIC DATA ----
	Diligent::RefCntAutoPtr<Diligent::IBuffer> MaterialBase::_uniforms;

	Diligent::IPipelineState* MaterialBase::_base = nullptr;
	Diligent::IPipelineState* MaterialBase::_line = nullptr;
	Diligent::IPipelineState* MaterialBase::_cullback = nullptr;
	Diligent::IPipelineState* MaterialBase::_wireframe = nullptr;
	Diligent::IPipelineState* MaterialBase::_cullnone = nullptr;

	Diligent::IShaderResourceBinding* MaterialBase::_bind = nullptr;
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
		settings.layout = rawrbox::MaterialBase::vLayout().first;
		settings.resources = {
		    Diligent::ShaderResourceVariableDesc{Diligent::SHADER_TYPE_PIXEL, "g_Texture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
		    Diligent::ShaderResourceVariableDesc{Diligent::SHADER_TYPE_VERTEX, "g_Displacement", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC}};
		settings.uniforms = {{Diligent::SHADER_TYPE_VERTEX, _uniforms}};

		_base = rawrbox::PipelineUtils::createPipelines("Model::Base", "Model::Base", settings);

		settings.topology = Diligent::PRIMITIVE_TOPOLOGY_LINE_LIST;
		settings.cull = Diligent::CULL_MODE_NONE;
		_line = rawrbox::PipelineUtils::createPipelines("Model::Line", "Model::Base", settings);

		settings.topology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		settings.cull = Diligent::CULL_MODE_BACK;
		_cullback = rawrbox::PipelineUtils::createPipelines("Model::Base::CullBack", "Model::Base", settings);

		settings.fill = Diligent::FILL_MODE_WIREFRAME;
		_wireframe = rawrbox::PipelineUtils::createPipelines("Model::Base::Wireframe", "Model::Base", settings);

		settings.fill = Diligent::FILL_MODE_SOLID;
		settings.topology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		settings.cull = Diligent::CULL_MODE_NONE;
		_cullnone = rawrbox::PipelineUtils::createPipelines("Model::Base::CullNone", "Model::Base", settings);
		// -----

		_bind = rawrbox::PipelineUtils::getBind("Model::Base");
	}

	std::vector<rawrbox::VertexData> MaterialBase::convert(const std::vector<rawrbox::VertexNormBoneData>& v) {
		return {v.begin(), v.end()};
	}

	void MaterialBase::bindUniforms(const rawrbox::Mesh& mesh) {
		auto renderer = rawrbox::RENDERER;
		auto context = renderer->context();

		// SETUP UNIFORMS ----------------------------
		Diligent::MapHelper<rawrbox::MaterialBaseUniforms> CBConstants(context, this->_uniforms, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
		// Map the buffer and write current world-view-projection matrix

		auto size = renderer->getSize().cast<float>();
		auto tTransform = rawrbox::TRANSFORM.transpose();
		auto tProj = renderer->camera()->getProjMtx().transpose();
		auto tView = renderer->camera()->getViewMtx().transpose();
		auto tInvView = renderer->camera()->getViewMtx();
		tInvView.inverse();

		auto tWorldView = renderer->camera()->getProjViewMtx().transpose();

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
		    tView * tProj,
		    tInvView,
		    tTransform * tWorldView,
		    size,
		    // --------------
		    mesh.color,
		    mesh.texture == nullptr ? rawrbox::Vector4f() : mesh.texture->getData(),
		    data};
		// ----------------------------

		/*
		// Bind extra renderer uniforms ---
		rawrbox::RENDERER->bindRenderUniforms();
		// ---*/
	}

	void MaterialBase::bindPipeline(const rawrbox::Mesh& mesh) {
		auto context = rawrbox::RENDERER->context();

		if (mesh.wireframe) {
			context->SetPipelineState(_wireframe);
		} else if (mesh.lineMode) {
			context->SetPipelineState(_line);
		} else {
			if (mesh.culling == Diligent::CULL_MODE_NONE) {
				context->SetPipelineState(_cullnone);
			} else {
				context->SetPipelineState(mesh.culling == Diligent::CULL_MODE_FRONT ? _base : _cullback);
			}
		}
	}

	void MaterialBase::bind(const rawrbox::Mesh& mesh) {
		auto context = rawrbox::RENDERER->context();

		if (mesh.texture != nullptr && mesh.texture->isValid() && !mesh.wireframe) {
			mesh.texture->update(); // Update texture
			_bind->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture")->Set(mesh.texture->getHandle());
		} else {
			_bind->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture")->Set(rawrbox::WHITE_TEXTURE->getHandle());
		}

		if (mesh.displacementTexture != nullptr && mesh.displacementTexture->isValid()) {
			_bind->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "g_Displacement")->Set(mesh.displacementTexture->getHandle());
		} else {
			_bind->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "g_Displacement")->Set(rawrbox::BLACK_TEXTURE->getHandle());
		}

		this->bindPipeline(mesh);
		this->bindUniforms(mesh);

		context->CommitShaderResources(_bind, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	}

	uint32_t MaterialBase::supports() const {
		return rawrbox::MaterialFlags::NONE;
	}

	const std::pair<std::vector<Diligent::LayoutElement>, uint32_t> MaterialBase::vLayout() {
		return {rawrbox::VertexData::vLayout(), rawrbox::VertexData::vLayoutSize()};
	}
} // namespace rawrbox
