#include <rawrbox/render/model/light/manager.h>
#include <rawrbox/render/model/model.h>
#include <rawrbox/render/shader_defines.h>
#include <rawrbox/render/static.h>
#include <rawrbox/render/util/uniforms.hpp>

#include <bx/math.h>
#include <generated/shaders/render/all.h>

#define BGFX_STATE_DEFAULT_3D (0 | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA) | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LEQUAL)

static const bgfx::EmbeddedShader shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_model),
    BGFX_EMBEDDED_SHADER(fs_model),
    BGFX_EMBEDDED_SHADER_END()};

namespace rawrBox {
	Model::Model() {
		this->_vLayout.begin()
		    .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
		    .add(bgfx::Attrib::Normal, 4, bgfx::AttribType::Uint8, true, true)
		    .add(bgfx::Attrib::Tangent, 4, bgfx::AttribType::Uint8, true, true)
		    .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
		    .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
		    .end();
	}

	Model::~Model() {
		ModelBase::~ModelBase();
		RAWRBOX_DESTROY(this->_texSpecularColor);
	}

	void Model::upload() {
		ModelBase::upload();

		// Setup shader -----
		bgfx::RendererType::Enum type = bgfx::getRendererType();
		bgfx::ShaderHandle vsh = bgfx::createEmbeddedShader(shaders, type, "vs_model");
		bgfx::ShaderHandle fsh = bgfx::createEmbeddedShader(shaders, type, "fs_model");

		this->_texSpecularColor = bgfx::createUniform("s_texSpecularColor", bgfx::UniformType::Sampler);

		this->_lightsSettings = bgfx::createUniform("u_lightsSetting", bgfx::UniformType::Vec4, 2);
		this->_lightsPosition = bgfx::createUniform("u_lightsPosition", bgfx::UniformType::Vec4, rawrBox::LightManager::getInstance().maxLights);
		this->_lightsData = bgfx::createUniform("u_lightsData", bgfx::UniformType::Mat4, rawrBox::LightManager::getInstance().maxLights);

		this->_program = bgfx::createProgram(vsh, fsh, true);
		if (!bgfx::isValid(this->_program)) throw std::runtime_error("[RawrBox-Model] Failed to bind shader");
		// -----------------
	}

	void Model::processLights() {
		auto& lightManager = rawrBox::LightManager::getInstance();
		size_t lightCount = lightManager.count();

		float lightSettings[2] = {lightManager.fullbright || this->_fullbright ? 1.f : 0.f, static_cast<float>(lightCount)};

		std::vector<std::array<float, 16>> lightData(lightCount);
		std::vector<std::array<float, 4>> lightPos(lightCount);

		for (size_t i = 0; i < lightCount; i++) {
			auto light = lightManager.getLight(i);

			lightPos[i] = light->getPosMatrix();
			lightData[i] = light->getDataMatrix();
		}

		if (lightPos.size() != lightData.size()) throw std::runtime_error("[RawrBox-MODEL] LightPos and LightData do not match!");

		bgfx::setUniform(this->_lightsSettings, lightSettings);
		if (lightSettings[0] == 1.f) return; // Fullbright

		if (lightCount > 0) {
			bgfx::setUniform(this->_lightsPosition, lightPos.front().data(), static_cast<uint16_t>(lightCount));
			bgfx::setUniform(this->_lightsData, lightData.front().data(), static_cast<uint16_t>(lightCount));
		}
	}

	void Model::draw(const rawrBox::Vector3f& camPos) {
		ModelBase::draw(camPos);

		// Set camera --
		float cam[3] = {camPos.x, camPos.y, camPos.z};
		bgfx::setUniform(this->_viewPos, cam, 3);
		// ----------

		this->processLights();

		for (auto& mesh : this->_meshes) {
			auto& data = mesh->getData();
			if (data->texture != nullptr && !data->wireframe) {
				bgfx::setTexture(0, this->_texColor, data->texture->getHandle());
			} else {
				bgfx::setTexture(0, this->_texColor, Model::defaultTexture()->getHandle());
			}

			if (data->specular_texture != nullptr) bgfx::setTexture(1, this->_texSpecularColor, data->specular_texture->getHandle());

			UniformUtils::setUniform(this->_offsetColor, data->color);

			bgfx::setVertexBuffer(0, this->_vbh, data->baseVertex, static_cast<uint32_t>(data->vertices.size()));
			bgfx::setIndexBuffer(this->_ibh, data->baseIndex, static_cast<uint32_t>(data->indices.size()));

			float matrix[16];
			bx::mtxMul(matrix, data->offsetMatrix.data(), this->_matrix.data());
			bgfx::setTransform(matrix);

			uint64_t flags = BGFX_STATE_DEFAULT_3D | this->_cull;
			if (data->wireframe) flags |= BGFX_STATE_PT_LINES;

			bgfx::setState(flags, 0);
			bgfx::submit(rawrBox::CURRENT_VIEW_ID, this->_program);
		}
	}
} // namespace rawrBox
