#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/texture/base.h>

#include <bgfx/bgfx.h>
#include <bx/math.h>

#include <array>
#include <memory>

namespace rawrBox {

	template <typename T>
	struct MeshData {
	public:
		uint16_t baseVertex = 0;
		uint16_t baseIndex = 0;

		std::shared_ptr<rawrBox::TextureBase> texture = nullptr;
		std::shared_ptr<rawrBox::TextureBase> specular_texture = nullptr;

		std::vector<T> vertices = {};
		std::vector<uint16_t> indices = {};

		std::array<float, 16> offsetMatrix = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}; // Identity matrix by default
		bool wireframe = false;
		rawrBox::Color color = rawrBox::Colors::White;

		MeshData() = default;
		virtual ~MeshData() {
			this->texture = nullptr;
			this->specular_texture = nullptr;

			this->vertices.clear();
			this->indices.clear();
		}
	};

	template <typename T>
	class Mesh {
	private:
		std::string _name = "mesh";
		std::shared_ptr<rawrBox::MeshData<T>> _data = nullptr;

	public:
		Mesh() {
			this->_data = std::make_shared<rawrBox::MeshData<T>>();
		}

		virtual ~Mesh() {
			this->_data = nullptr;
		}

		// UTILS ----
		const std::string& getName() {
			return this->_name;
		}

		void setName(const std::string& name) {
			this->_name = name;
		}

		std::shared_ptr<rawrBox::MeshData<T>>& getData() {
			return this->_data;
		}

		std::vector<T>& getVertices() {
			return this->_data->vertices;
		}

		std::vector<uint16_t>& getIndices() {
			return this->_data->indices;
		}

		void setMatrix(const std::array<float, 16>& offset) {
			this->_data->offsetMatrix = offset;
		}

		void setTexture(const std::shared_ptr<rawrBox::TextureBase>& ptr) {
			this->_data->texture = ptr;
		}

		void setSpecularTexture(const std::shared_ptr<rawrBox::TextureBase>& ptr) {
			this->_data->specular_texture = ptr;
		}

		void setWireframe(bool wireframe) {
			this->_data->wireframe = wireframe;
		}

		void setColor(const rawrBox::Color& color) {
			this->_data->color = color;
		}

		// ----
	};
} // namespace rawrBox
