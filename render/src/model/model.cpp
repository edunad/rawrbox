#include <rawrbox/render/model/light/manager.h>
#include <rawrbox/render/model/model.h>
#include <rawrbox/render/model/model_mesh.h>
#include <rawrbox/render/shader_defines.h>
#include <rawrbox/render/static.h>

#include <bx/math.h>
#include <generated/shaders/render/all.h>

#define BGFX_STATE_DEFAULT_3D (0 | BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA) | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LEQUAL)
#define MAX_LIGHTS            12

static const bgfx::EmbeddedShader shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_model),
    BGFX_EMBEDDED_SHADER(fs_model),
    BGFX_EMBEDDED_SHADER_END()};

namespace rawrBox {
	std::shared_ptr<rawrBox::TextureFlat> Model::defaultTexture = nullptr;

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
		RAWRBOX_DESTROY(this->_vbh);
		RAWRBOX_DESTROY(this->_ibh);
		RAWRBOX_DESTROY(this->_program);

		RAWRBOX_DESTROY(this->_texColor);
		RAWRBOX_DESTROY(this->_offsetColor);

		RAWRBOX_DESTROY(this->_lightsSettings);
		RAWRBOX_DESTROY(this->_lightsPosition);
		RAWRBOX_DESTROY(this->_lightsData);
		RAWRBOX_DESTROY(this->_viewPos);

		this->_meshes.clear();
		this->_vertices.clear();
		this->_indices.clear();
	}

	// UTIL ---
	void Model::setMatrix(const std::array<float, 16>& matrix_) {
		this->_matrix = matrix_;
	}

	std::array<float, 16>& Model::getMatrix() {
		return this->_matrix;
	}

	void Model::addMesh(const std::shared_ptr<ModelMesh>& mesh) {
		// Copy vertices over
		auto& data = mesh->getData();
		data->baseIndex = static_cast<uint16_t>(this->_indices.size());

		this->_vertices.insert(this->_vertices.end(), data->vertices.begin(), data->vertices.end());
		auto pos = static_cast<uint16_t>(this->_vertices.size());
		for (auto& in : data->indices)
			this->_indices.push_back(pos - in);

		this->_meshes.push_back(mesh);
	}

	const std::shared_ptr<rawrBox::ModelMesh>& Model::getMesh(size_t id) {
		return this->_meshes[id];
	}

	void Model::setWireframe(bool wireframe, int id) {
		for (size_t i = 0; i < this->_meshes.size(); i++) {
			if (id != -1 && i != id) continue;
			this->_meshes[i]->setWireframe(wireframe);
		}
	}

	void Model::setCulling(uint64_t cull) {
		this->_cull = cull;
	}

	void Model::setFullbright(bool b) {
		this->_fullbright = b;
	}
	// -------

	void Model::upload() {
		if (bgfx::isValid(this->_vbh) || bgfx::isValid(this->_ibh)) throw std::runtime_error("ModelMeshData::generate called twice");
		if (this->_vertices.empty() || this->_indices.empty()) return;

		if (Model::defaultTexture == nullptr) {
			Model::defaultTexture = std::make_shared<rawrBox::TextureFlat>(rawrBox::Vector2i(1, 1), Colors::White);
			Model::defaultTexture->upload();
		}

		// Fix textures to default & wireframe
		for (auto& mesh : this->_meshes) {
			auto& data = mesh->getData();
			if (data->texture != nullptr && !data->wireframe) continue;

			data->texture = Model::defaultTexture;
		}
		// ----

		this->_vbh = bgfx::createVertexBuffer(bgfx::makeRef(this->_vertices.data(), static_cast<uint32_t>(this->_vertices.size()) * this->_vLayout.m_stride), this->_vLayout);
		this->_ibh = bgfx::createIndexBuffer(bgfx::makeRef(this->_indices.data(), static_cast<uint32_t>(this->_indices.size()) * sizeof(uint16_t)));

		// Setup shader -----
		bgfx::RendererType::Enum type = bgfx::getRendererType();
		bgfx::ShaderHandle vsh = bgfx::createEmbeddedShader(shaders, type, "vs_model");
		bgfx::ShaderHandle fsh = bgfx::createEmbeddedShader(shaders, type, "fs_model");

		this->_texColor = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
		this->_offsetColor = bgfx::createUniform("u_colorOffset", bgfx::UniformType::Vec4);

		this->_lightsSettings = bgfx::createUniform("u_lightsSetting", bgfx::UniformType::Vec4);
		this->_lightsPosition = bgfx::createUniform("u_lightsPosition", bgfx::UniformType::Mat4, MAX_LIGHTS);
		this->_lightsData = bgfx::createUniform("u_lightsData", bgfx::UniformType::Mat4, MAX_LIGHTS);
		this->_viewPos = bgfx::createUniform("u_viewPos", bgfx::UniformType::Vec4, 3);

		this->_program = bgfx::createProgram(vsh, fsh, true);
		if (!bgfx::isValid(this->_program)) throw std::runtime_error("[RawrBox-Model] Failed to bind shader");
		// -----------------
	}

	void Model::processLights() {
		auto& lightManager = rawrBox::LightManager::getInstance();
		size_t lightCount = lightManager.count();

		float lightSettings[4] = {lightManager.FULLBRIGHT || this->_fullbright ? 1.f : 0.f, 0, 0, 0};

		std::vector<std::array<float, 16>> lightData(lightCount);
		std::vector<std::array<float, 16>> lightPos(lightCount);

		float totalSpot = 0;
		float totalPoint = 0;
		float totalDir = 0;

		for (size_t i = 0; i < lightCount; i++) {
			auto light = lightManager.getLight(i);
			if (light->getType() == LightType::LIGHT_POINT) lightSettings[1]++;
			if (light->getType() == LightType::LIGHT_SPOT) lightSettings[2]++;
			if (light->getType() == LightType::LIGHT_DIR) lightSettings[3]++;

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

	void Model::draw(rawrBox::Vector3 camPos) {
		if (!bgfx::isValid(this->_vbh) || !bgfx::isValid(this->_ibh)) return;

		// Set camera --
		float cam[3] = {camPos.x, camPos.y, camPos.z};
		bgfx::setUniform(this->_viewPos, cam, 3);
		// ----------

		// LIGHT ----
		this->processLights();
		// -----------------

		for (auto& mesh : this->_meshes) {
			auto& data = mesh->getData();
			if (data->texture != nullptr) bgfx::setTexture(0, this->_texColor, data->texture->getHandle());

			float meshColor[4] = {data->color.r, data->color.g, data->color.b, data->color.a};
			bgfx::setUniform(this->_offsetColor, meshColor);

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
