#pragma once
#include <rawrbox/math/color.hpp>
#include <rawrbox/math/quaternion.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>

#include <bgfx/bgfx.h>

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace rawrBox {
	class UniformUtils {
	public:
		static void setUniform(const bgfx::UniformHandle& handle, double data) {
			std::array<float, 1> d = {static_cast<float>(data)};
			bgfx::setUniform(handle, d.data(), 1);
		}

		static void setUniform(const bgfx::UniformHandle& handle, float data) {
			std::array<float, 1> d = {data};
			bgfx::setUniform(handle, d.data(), 1);
		}

		static void setUniform(const bgfx::UniformHandle& handle, uint64_t data) {
			std::array<float, 1> d = {static_cast<float>(data)};
			bgfx::setUniform(handle, d.data(), 1);
		}

		static void setUniform(const bgfx::UniformHandle& handle, std::array<float, 16> data) {
			bgfx::setUniform(handle, data.data());
		}

		static void setUniform(const bgfx::UniformHandle& handle, std::vector<std::array<float, 16>> data) {
			bgfx::setUniform(handle, data.front().data(), static_cast<uint16_t>(data.size()));
		}

		static void setUniform(const bgfx::UniformHandle& handle, std::array<float, 4> data) {
			bgfx::setUniform(handle, data.data());
		}

		static void setUniform(const bgfx::UniformHandle& handle, std::vector<std::array<float, 4>> data) {
			bgfx::setUniform(handle, data.front().data(), static_cast<uint16_t>(data.size()));
		}

		static void setUniform(const bgfx::UniformHandle& handle, std::vector<std::array<float, 3>> data) {
			bgfx::setUniform(handle, data.front().data(), static_cast<uint16_t>(data.size()));
		}

		static void setUniform(const bgfx::UniformHandle& handle, rawrBox::Vector2f data) {
			std::array<float, 2> d = {data.x, data.y};
			bgfx::setUniform(handle, d.data(), 2);
		}

		static void setUniform(const bgfx::UniformHandle& handle, rawrBox::Vector3f data) {
			std::array<float, 3> d = {data.x, data.y, data.z};
			bgfx::setUniform(handle, d.data(), 3);
		}

		static void setUniform(const bgfx::UniformHandle& handle, rawrBox::Quaternion data) {
			std::array<float, 4> d = {data.x, data.y, data.z, data.w};
			bgfx::setUniform(handle, d.data());
		}

		static void setUniform(const bgfx::UniformHandle& handle, rawrBox::Colorf data) {
			std::array<float, 4> d = {data.r, data.g, data.b, data.a};
			bgfx::setUniform(handle, d.data());
		}
	};
} // namespace rawrBox
