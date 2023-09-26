
#include <rawrbox/render/materials/base.hpp>
#include <rawrbox/render/renderers/base.hpp>
#include <rawrbox/render/utils/render.hpp>
#include <rawrbox/utils/pack.hpp>

// NOLINTBEGIN(*)
const bgfx::EmbeddedShader clustered_unlit_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_clustered_unlit_base),
    BGFX_EMBEDDED_SHADER(fs_clustered_unlit_base),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrbox {
	constexpr auto MAX_DATA = 4;

	void MaterialBase::setupUniforms() {
		this->registerUniform("s_albedo", bgfx::UniformType::Sampler);
		this->registerUniform("s_displacement", bgfx::UniformType::Sampler);

		this->registerUniform("u_colorOffset", bgfx::UniformType::Vec4);
		this->registerUniform("u_data", bgfx::UniformType::Vec4, MAX_DATA);
		this->registerUniform("u_tex_flags", bgfx::UniformType::Vec4);
	}

	MaterialBase::~MaterialBase() {
		RAWRBOX_DESTROY(this->_program);
		for (auto& handle : this->_uniforms) {
			RAWRBOX_DESTROY(handle.second);
		}
	}

	void MaterialBase::registerUniform(const std::string& name, bgfx::UniformType::Enum type, uint16_t num) {
		auto handl = bgfx::createUniform(name.c_str(), type, num);
		if (!bgfx::isValid(handl)) throw std::runtime_error(fmt::format("[RawrBox-MaterialBase] Failed to create uniform '{}'", name));
		this->_uniforms[name] = handl;
	}

	bgfx::UniformHandle& MaterialBase::getUniform(const std::string& name) {
		auto fnd = this->_uniforms.find(name);
		if (fnd == this->_uniforms.end()) throw std::runtime_error(fmt::format("[RawrBox-MaterialBase] Failed to find uniform {}, did you call upload()?", name));
		return fnd->second;
	}

	void MaterialBase::setUniformData(const std::string& name, const std::vector<rawrbox::Matrix4x4>& data) {
		auto fnd = this->_uniforms.find(name);
		if (fnd == this->_uniforms.end()) return;

		bgfx::setUniform(fnd->second, &data.front(), static_cast<uint16_t>(data.size()));
	}

	void MaterialBase::process(const rawrbox::Mesh& mesh) {
		if (mesh.texture != nullptr && mesh.texture->isValid() && !mesh.lineMode && !mesh.wireframe) {
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
		// ---
	}

	void MaterialBase::process(const bgfx::TextureHandle& texture) {
		if (bgfx::isValid(texture)) {
			bgfx::setTexture(rawrbox::SAMPLE_MAT_ALBEDO, this->getUniform("s_albedo"), texture);
		} else {
			bgfx::setTexture(rawrbox::SAMPLE_MAT_ALBEDO, this->getUniform("s_albedo"), rawrbox::WHITE_TEXTURE->getHandle());
		}
	}

	void MaterialBase::postProcess() {
		if (!bgfx::isValid(this->_program)) throw std::runtime_error("[RawrBox-MaterialBase] Invalid program, did you call 'upload'?");
		bgfx::submit(rawrbox::CURRENT_VIEW_ID, this->_program);
	}

	void MaterialBase::upload() {
		this->setupUniforms();
		rawrbox::RenderUtils::buildShader(clustered_unlit_shaders, this->_program);
	}

	uint32_t MaterialBase::supports() const {
		return rawrbox::MaterialFlags::NONE;
	}

	const bgfx::VertexLayout MaterialBase::vLayout() const {
		return rawrbox::VertexData::vLayout();
	}
} // namespace rawrbox
