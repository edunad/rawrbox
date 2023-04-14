#include <rawrbox/render/model/model.h>
#include <rawrbox/render/model/model_mesh.h>

#include <rawrbox/render/shader_defines.h>
#include <generated/shaders/render/all.h>

static const bgfx::EmbeddedShader shaders[] = {
	BGFX_EMBEDDED_SHADER(vs_model),
	BGFX_EMBEDDED_SHADER(fs_model),
	BGFX_EMBEDDED_SHADER_END()
};

namespace rawrBox {
	Model::Model() {
		this->_vLayout.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
		.end();
	}

	Model::~Model() {
		bgfx::destroy(this->_vbh);
		bgfx::destroy(this->_ibh);
		bgfx::destroy(this->_handle);
	}

	void Model::setMatrix(const std::array<float, 16>& matrix_) {
		this->_matrix = matrix_;
	}

	std::array<float, 16>& Model::getMatrix() {
		return this->_matrix;
	}

	void Model::addMesh(const std::shared_ptr<ModelMesh>& mesh) {
		this->_meshes.push_back(mesh);

		// Copy vertices over
		this->_vertices.insert(this->_vertices.end(), mesh->getData()->vertices.begin(), mesh->getData()->vertices.end());
		this->_indices.insert(this->_indices.end(), mesh->getData()->indices.begin(), mesh->getData()->indices.end());
	}

	void Model::upload() {
		if(bgfx::isValid(this->_vbh) || bgfx::isValid(this->_ibh)) throw std::runtime_error("ModelMeshData::generate called twice");
		if(this->_vertices.empty() || this->_indices.empty()) return;

		this->_vbh = bgfx::createVertexBuffer(bgfx::makeRef(this->_vertices.data(), static_cast<uint32_t>(this->_vertices.size()) * this->_vLayout.m_stride), this->_vLayout);
		this->_ibh = bgfx::createIndexBuffer(bgfx::makeRef(this->_indices.data(), static_cast<uint32_t>(this->_indices.size()) * sizeof(uint16_t)));

		bgfx::RendererType::Enum type = bgfx::getRendererType();
		bgfx::ShaderHandle vsh = bgfx::createEmbeddedShader(shaders, type, "vs_model");
		bgfx::ShaderHandle fsh = bgfx::createEmbeddedShader(shaders, type, "fs_model");

		this->_handle = bgfx::createProgram(vsh, fsh, true);
	}

	void Model::draw() {
		if(!bgfx::isValid(this->_vbh) || !bgfx::isValid(this->_ibh)) return;

		bgfx::setVertexBuffer(0, this->_vbh);
		bgfx::setIndexBuffer(this->_ibh);

		bgfx::setTransform(this->_matrix.data());
		bgfx::submit(0, this->_handle);
	}
}
