#include <rawrbox/render/model/model.h>
#include <rawrbox/render/model/model_mesh.h>
#include <rawrbox/render/shader_defines.h>

#include <generated/shaders/render/all.h>

#include <bx/math.h>

#define BGFX_STATE_DEFAULT_3D (0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS)

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
		    .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
		    .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
		    .end();
	}

	Model::~Model() {
		bgfx::destroy(this->_vbh);
		bgfx::destroy(this->_ibh);
		bgfx::destroy(this->_handle);
		bgfx::destroy(this->_texColor);

		this->_meshes = {};
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
		this->_handle = bgfx::createProgram(vsh, fsh, true);
		// -----------------
	}

	void Model::draw(bgfx::ViewId id) {
		if (!bgfx::isValid(this->_vbh) || !bgfx::isValid(this->_ibh)) return;

		for (auto& mesh : this->_meshes) {
			auto& data = mesh->getData();
			if (data->texture != nullptr) bgfx::setTexture(0, this->_texColor, data->texture->getHandle());

			bgfx::setVertexBuffer(0, this->_vbh, data->baseVertex, static_cast<uint32_t>(data->vertices.size()));
			bgfx::setIndexBuffer(this->_ibh, data->baseIndex, static_cast<uint32_t>(data->indices.size()));

			float a[16];
			bx::mtxMul(a, data->offsetMatrix.data(), this->_matrix.data());
			bgfx::setTransform(a);

			uint64_t flags = BGFX_STATE_DEFAULT_3D;
			if (data->wireframe) flags |= BGFX_STATE_PT_LINES;

			bgfx::setState(flags | this->_cull);
			bgfx::submit(id, this->_handle);
		}
	}
} // namespace rawrBox
