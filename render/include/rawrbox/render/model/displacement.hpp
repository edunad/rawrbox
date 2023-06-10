#pragma once
#include <rawrbox/render/model/material/displacement.hpp>
#include <rawrbox/utils/pack.hpp>

#define BGFX_STATE_DEFAULT_DISPLACEMENT (0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CW)

namespace rawrbox {
	template <typename M = rawrbox::MaterialDisplacement>
	class Displacement {
	protected:
		bgfx::VertexBufferHandle _vbh = BGFX_INVALID_HANDLE; // Vertices - Static
		bgfx::IndexBufferHandle _ibh = BGFX_INVALID_HANDLE;  // Indices - Static

		rawrbox::Matrix4x4 _matrix = {};

		rawrbox::Vector3f _scale = {1, 1, 1};
		rawrbox::Vector3f _pos = {};
		rawrbox::Vector4f _angle = {};

		std::unique_ptr<rawrbox::Mesh<typename M::vertexBufferType>> _mesh = std::make_unique<rawrbox::Mesh<typename M::vertexBufferType>>();
		std::unique_ptr<M> _material = std::make_unique<M>();

		void generate(uint32_t size) {
			this->_mesh->clear();

			auto ps = static_cast<float>(size / 2);
			for (uint32_t y = 0; y < size; y++) {
				for (uint32_t x = 0; x < size; x++) {

					auto xF = static_cast<float>(x);
					auto yF = static_cast<float>(y);

					if constexpr (supportsNormals<typename M::vertexBufferType>) {
						this->_mesh->vertices.push_back(rawrbox::VertexLitData(rawrbox::Vector3f(ps - xF, 0, ps - yF), {(x + 0.5F) / size, (y + 0.5F) / size}, {rawrbox::PackUtils::packNormal(0, 1, 0), 0}, rawrbox::Colors::White));
					} else {
						this->_mesh->vertices.push_back(rawrbox::VertexData(rawrbox::Vector3f(ps - xF, 0, ps - yF), {(x + 0.5F) / size, (y + 0.5F) / size}, rawrbox::Colors::White));
					}
				}
			}

			for (uint16_t y = 0; y < (size - 1); y++) {
				uint16_t y_offset = (y * size);

				for (uint16_t x = 0; x < (size - 1); x++) {
					this->_mesh->indices.push_back(y_offset + x + 1);
					this->_mesh->indices.push_back(y_offset + x + size);
					this->_mesh->indices.push_back(y_offset + x);
					this->_mesh->indices.push_back(y_offset + x + size + 1);
					this->_mesh->indices.push_back(y_offset + x + size);
					this->_mesh->indices.push_back(y_offset + x + 1);
				}
			}
		}

	public:
		Displacement(Displacement&&) = delete;
		Displacement& operator=(Displacement&&) = delete;
		Displacement(const Displacement&) = delete;
		Displacement& operator=(const Displacement&) = delete;

		explicit Displacement(uint32_t size) { this->generate(size + 1); }
		virtual ~Displacement() {
			RAWRBOX_DESTROY(this->_vbh);
			RAWRBOX_DESTROY(this->_ibh);

			this->_mesh.reset();
		}

		void setHeightMap(rawrbox::TextureBase* map, float strength) { this->_mesh->setBumpTexture(map, strength); }
		void setTexture(rawrbox::TextureBase* ptr) { this->_mesh->setTexture(ptr); }

		[[nodiscard]] virtual const rawrbox::Vector3f& getPos() const { return this->_pos; }
		virtual void setPos(const rawrbox::Vector3f& pos) {
			this->_pos = pos;
			this->_matrix.mtxSRT(this->_scale, this->_angle, this->_pos);
		}

		[[nodiscard]] virtual const rawrbox::Vector3f& getScale() const { return this->_scale; }
		virtual void setScale(const rawrbox::Vector3f& scale) {
			this->_scale = scale;
			this->_matrix.mtxSRT(this->_scale, this->_angle, this->_pos);
		}

		[[nodiscard]] virtual const rawrbox::Vector4f& getAngle() const { return this->_angle; }
		virtual void setAngle(const rawrbox::Vector4f& ang) {
			this->_angle = ang;
			this->_matrix.mtxSRT(this->_scale, this->_angle, this->_pos);
		}

		virtual void setEulerAngle(const rawrbox::Vector3f& ang) {
			this->_angle = rawrbox::Vector4f::toQuat(ang);
			this->_matrix.mtxSRT(this->_scale, this->_angle, this->_pos);
		}

		[[nodiscard]] const bool isUploaded() const {
			return bgfx::isValid(this->_ibh) && bgfx::isValid(this->_vbh);
		}

		void upload() {
			if (this->_mesh->vertices.empty() || this->_mesh->indices.empty()) throw std::runtime_error("[RawrBox-Displacement] Static buffer cannot contain empty vertices / indices.");

			const bgfx::Memory* vertMem = bgfx::makeRef(this->_mesh->vertices.data(), static_cast<uint32_t>(this->_mesh->vertices.size()) * M::vertexBufferType::vLayout().m_stride);
			const bgfx::Memory* indexMem = bgfx::makeRef(this->_mesh->indices.data(), static_cast<uint32_t>(this->_mesh->indices.size()) * sizeof(uint16_t));

			this->_vbh = bgfx::createVertexBuffer(vertMem, M::vertexBufferType::vLayout());
			this->_ibh = bgfx::createIndexBuffer(indexMem);

			this->_material->upload();
			this->_material->registerUniforms();
		}

		void draw(const rawrbox::Vector3f& camPos) {
			if (!this->isUploaded()) throw std::runtime_error("[RawrBox-Model] Failed to render model, vertex / index buffer is not uploaded");

			this->_material->preProcess(camPos);
			this->_material->process(*this->_mesh);

			bgfx::setVertexBuffer(0, this->_vbh);
			bgfx::setIndexBuffer(this->_ibh);

			bgfx::setTransform(this->_matrix.data());
			bgfx::setState(BGFX_STATE_DEFAULT_DISPLACEMENT, 0);

			this->_material->postProcess();
		}
	};
} // namespace rawrbox
