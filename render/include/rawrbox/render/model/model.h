#pragma once
#include <rawrbox/render/model/model_mesh.h>
#include <rawrbox/render/texture/flat.h>

#include <array>
#include <memory>
#include <vector>

namespace rawrBox {
	class ModelMesh;

	class Model {

	protected:
		bgfx::VertexLayout _vLayout;

		bgfx::ProgramHandle _handle = BGFX_INVALID_HANDLE;
		bgfx::VertexBufferHandle _vbh = BGFX_INVALID_HANDLE; // Vertices
		bgfx::IndexBufferHandle _ibh = BGFX_INVALID_HANDLE;  // Indices

		bgfx::UniformHandle _texColor = BGFX_INVALID_HANDLE;

		bgfx::UniformHandle _lightsSettings = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle _lightsData = BGFX_INVALID_HANDLE;

		std::vector<std::shared_ptr<rawrBox::ModelMesh>> _meshes;
		std::array<float, 16> _matrix;

		std::vector<rawrBox::ModelVertexData> _vertices;
		std::vector<uint16_t> _indices;

		uint64_t _cull = BGFX_STATE_CULL_CW;
		bool _fullbright = false;

	public:
		static std::shared_ptr<rawrBox::TextureFlat> defaultTexture;

		Model();
		virtual ~Model();

		// UTIL ---
		virtual void setMatrix(const std::array<float, 16>& matrix);
		virtual std::array<float, 16>& getMatrix();

		virtual void addMesh(const std::shared_ptr<rawrBox::ModelMesh>& mesh);
		virtual const std::shared_ptr<rawrBox::ModelMesh>& getMesh(size_t id = 0);

		virtual void setWireframe(bool wireframe, int id = -1);
		virtual void setCulling(uint64_t cull);
		virtual void setFullbright(bool b);
		// ----

		virtual void upload();
		virtual void draw(bgfx::ViewId id = 0);
	};
} // namespace rawrBox
