#pragma once
#include <rawrbox/render/model/model_mesh.h>

#include <vector>
#include <array>
#include <memory>

namespace rawrBox {
	class ModelMesh;

	class Model {
		bgfx::VertexLayout _vLayout;

		bgfx::ProgramHandle _handle = BGFX_INVALID_HANDLE;
		bgfx::VertexBufferHandle _vbh = BGFX_INVALID_HANDLE; // Vertices
		bgfx::IndexBufferHandle _ibh = BGFX_INVALID_HANDLE; // Indices

		std::vector<std::shared_ptr<rawrBox::ModelMesh>> _meshes;
		std::array<float, 16> _matrix;

		std::vector<rawrBox::ModelVertexData> _vertices;
		std::vector<uint16_t> _indices;

	public:
		Model();
		~Model();

		void setMatrix(const std::array<float, 16>& matrix);
		std::array<float, 16>& getMatrix();

		void addMesh(const std::shared_ptr<rawrBox::ModelMesh>& mesh);

		void upload();
		void draw();
	};
}
