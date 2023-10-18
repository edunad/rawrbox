
#include <rawrbox/render/materials/base.hpp>
#include <rawrbox/render/renderers/base.hpp>
#include <rawrbox/render/utils/pipeline.hpp>
#include <rawrbox/utils/pack.hpp>

#include <Common/interface/BasicMath.hpp>
#include <Graphics/GraphicsTools/interface/MapHelper.hpp>
#include <Platforms/Basic/interface/DebugUtilities.hpp>

// NOLINTBEGIN(*)
/*const bgfx::EmbeddedShader clustered_unlit_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_clustered_unlit_base),
    BGFX_EMBEDDED_SHADER(fs_clustered_unlit_base),
    BGFX_EMBEDDED_SHADER_END()};*/
// NOLINTEND(*)

namespace rawrbox {
	constexpr auto MAX_DATA = 4;

	void MaterialBase::setupUniforms() {
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
		settings.cull = Diligent::CULL_MODE_NONE;
		settings.layout = this->vLayout();
		settings.resources = {
		    Diligent::ShaderResourceVariableDesc{Diligent::SHADER_TYPE_PIXEL, "g_Texture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC}};

		rawrbox::PipelineUtils::createPipelines("Model::Base", settings, &this->_pipeline);
		this->_pipeline->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "Constants")->Set(this->_uniforms);
		this->_pipeline->CreateShaderResourceBinding(&this->_SRB, true);
		// -----

		/*this->registerUniform("s_albedo", bgfx::UniformType::Sampler);
		this->registerUniform("s_displacement", bgfx::UniformType::Sampler);

		this->registerUniform("u_colorOffset", bgfx::UniformType::Vec4);
		this->registerUniform("u_data", bgfx::UniformType::Vec4, MAX_DATA);
		this->registerUniform("u_tex_flags", bgfx::UniformType::Vec4);*/
	}

	MaterialBase::~MaterialBase() {
		/*RAWRBOX_DESTROY(this->_program);
		for (auto& handle : this->_uniforms) {
			RAWRBOX_DESTROY(handle.second);
		}*/
	}

	/*void MaterialBase::registerUniform(const std::string& name, bgfx::UniformType::Enum type, uint16_t num) {
		auto handl = bgfx::createUniform(name.c_str(), type, num);
		if (!bgfx::isValid(handl)) throw std::runtime_error(fmt::format("[RawrBox-MaterialBase] Failed to create uniform '{}'", name));
		this->_uniforms[name] = handl;
	}

	bgfx::UniformHandle& MaterialBase::getUniform(const std::string& name) {
		auto fnd = this->_uniforms.find(name);
		if (fnd == this->_uniforms.end()) throw std::runtime_error(fmt::format("[RawrBox-MaterialBase] Failed to find uniform {}, did you call upload()?", name));
		return fnd->second;
	}*/

	void MaterialBase::setUniformData(const std::string& name, const std::vector<rawrbox::Matrix4x4>& data) {
		/*auto fnd = this->_uniforms.find(name);
		if (fnd == this->_uniforms.end()) return;

		bgfx::setUniform(fnd->second, &data.front(), static_cast<uint16_t>(data.size()));*/
	}

	Diligent::float4x4 GetAdjustedProjectionMatrix(float FOV, float NearPlane, float FarPlane) {
		const auto& SCDesc = rawrbox::RENDERER->swapChain->GetDesc();

		float AspectRatio = static_cast<float>(SCDesc.Width) / static_cast<float>(SCDesc.Height);
		float XScale = 0, YScale = 0;

		if (SCDesc.PreTransform == Diligent::SURFACE_TRANSFORM_ROTATE_90 ||
		    SCDesc.PreTransform == Diligent::SURFACE_TRANSFORM_ROTATE_270 ||
		    SCDesc.PreTransform == Diligent::SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90 ||
		    SCDesc.PreTransform == Diligent::SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270) {
			// When the screen is rotated, vertical FOV becomes horizontal FOV
			XScale = 1.F / std::tan(FOV / 2.F);
			// Aspect ratio is inversed
			YScale = XScale * AspectRatio;
		} else {
			YScale = 1.F / std::tan(FOV / 2.F);
			XScale = YScale / AspectRatio;
		}

		Diligent::float4x4 Proj;
		Proj._11 = XScale;
		Proj._22 = YScale;
		Proj.SetNearFarClipPlanes(NearPlane, FarPlane, rawrbox::RENDERER->device->GetDeviceInfo().IsGLDevice());
		return Proj;
	}

	void MaterialBase::bind(const rawrbox::Mesh& mesh) {
		auto context = rawrbox::RENDERER->context;

		Diligent::float4x4 CubeModelTransform;

		// Camera is at (0, 0, -5) looking along the Z axis
		Diligent::float4x4 View = Diligent::float4x4::Translation(0.F, 0.0F, 5.0F);

		// Get projection matrix adjusted to the current screen orientation
		auto Proj = GetAdjustedProjectionMatrix(Diligent::PI_F / 4.0F, 0.1F, 100.F);

		// SETUP UNIFORMS ----------------------------
		Diligent::MapHelper<rawrbox::MaterialUniforms> CBConstants(context, this->_uniforms, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
		// Map the buffer and write current world-view-projection matrix

		*CBConstants = {
		    rawrbox::TRANSFORM,
		    rawrbox::MAIN_CAMERA->getProjMtx(),
		    rawrbox::MAIN_CAMERA->getViewMtx(),
		    rawrbox::TRANSFORM * rawrbox::MAIN_CAMERA->getProjViewMtx(),
		};
		// ----------------------------

		if (mesh.texture != nullptr && mesh.texture->isValid() && !mesh.lineMode && !mesh.wireframe) {
			mesh.texture->update(); // Update texture

			this->_SRB->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture")->Set(mesh.texture->getHandle());
			// bgfx::setUniform(this->getUniform("u_tex_flags"), mesh.texture->getData().data());
		} else {
			this->_SRB->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture")->Set(rawrbox::WHITE_TEXTURE->getHandle());
		}

		// Bind extra renderer uniforms ---
		rawrbox::RENDERER->bindRenderUniforms();
		// ---

		context->SetPipelineState(this->_pipeline);
		context->CommitShaderResources(this->_SRB, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

		/*if (mesh.texture != nullptr && mesh.texture->isValid() && !mesh.lineMode && !mesh.wireframe) {
			mesh.texture->update(); // Update texture

			bgfx::setUniform(this->getUniform("u_tex_flags"), mesh.texture->getData().data());
			bgfx::setTexture(rawrbox::SAMPLE_MAT_ALBEDO, this->getUniform("s_albedo"), mesh.texture->getHandle());
		} else {
			bgfx::setTexture(rawrbox::SAMPLE_MAT_ALBEDO, this->getUniform("s_albedo"), rawrbox::WHITE_TEXTURE->getHandle());
		}

		if (mesh.displacementTexture != nullptr && mesh.displacementTexture->isValid()) {
			bgfx::setTexture(rawrbox::SAMPLE_MAT_DISPLACEMENT, this->getUniform("s_displacement"), mesh.displacementTexture->getHandle());
		} else {
			bgfx::setTexture(rawrbox::SAMPLE_MAT_DISPLACEMENT, this->getUniform("s_displacement"), rawrbox::BLACK_TEXTURE->getHandle());
		}

		// Color override
		bgfx::setUniform(this->getUniform("u_colorOffset"), mesh.color.data().data());
		// -------

		// Pass "special" data ---
		std::array<std::array<float, 4>, MAX_DATA>
		    data = {std::array<float, 4>{0.F, 0.F, 0.F, 0.F}, {0.F, 0.F, 0.F, 0.F}, {0.F, 0.F, 0.F, 0.F}, {0.F, 0.F, 0.F, 0.F}};
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

		bgfx::setUniform(this->getUniform("u_data"), data.front().data(), MAX_DATA);
		// ---

		// Bind extra renderer uniforms ---
		rawrbox::RENDERER->bindRenderUniforms();
		// ---*/
	}

	void MaterialBase::bind(Diligent::ITextureView* texture) {
		if (texture != nullptr) {
			this->_SRB->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture")->Set(texture);
		} else {
			this->_SRB->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture")->Set(rawrbox::WHITE_TEXTURE->getHandle());
		}
	}

	void MaterialBase::postProcess() {
		/*if (!bgfx::isValid(this->_program)) throw std::runtime_error("[RawrBox-MaterialBase] Invalid program, did you call 'upload'?");
		bgfx::submit(rawrbox::CURRENT_VIEW_ID, this->_program);*/
	}

	void MaterialBase::upload() {
		this->setupUniforms();
		// rawrbox::RenderUtils::buildShader(clustered_unlit_shaders, this->_program);
	}

	uint32_t MaterialBase::supports() const {
		return rawrbox::MaterialFlags::NONE;
	}

	const std::vector<Diligent::LayoutElement> MaterialBase::vLayout() const {
		return rawrbox::VertexData::vLayout();
	}
} // namespace rawrbox
