#pragma once
#include <rawrbox/render/model/model_mesh.h>
#include <rawrbox/render/texture/flat.h>

#include <array>
#include <memory>
#include <vector>

namespace rawrBox {
	class ModelMesh;

	class Model {
		bgfx::VertexLayout _vLayout;

		bgfx::ProgramHandle _handle = BGFX_INVALID_HANDLE;
		bgfx::VertexBufferHandle _vbh = BGFX_INVALID_HANDLE; // Vertices
		bgfx::IndexBufferHandle _ibh = BGFX_INVALID_HANDLE;  // Indices

		bgfx::UniformHandle _texColor = BGFX_INVALID_HANDLE;

		std::vector<std::shared_ptr<rawrBox::ModelMesh>> _meshes;
		std::array<float, 16> _matrix;

		std::vector<rawrBox::ModelVertexData> _vertices;
		std::vector<uint16_t> _indices;

	public:
		static std::shared_ptr<rawrBox::TextureFlat> defaultTexture;

		Model();
		~Model();

		// UTIL ---
		void setMatrix(const std::array<float, 16>& matrix);
		std::array<float, 16>& getMatrix();

		void addMesh(const std::shared_ptr<rawrBox::ModelMesh>& mesh);
		const std::shared_ptr<rawrBox::ModelMesh>& getMesh(size_t id = 0);
		// ----

		void upload();
		void draw(bgfx::ViewId id = 0);
	};
} // namespace rawrBox
