#pragma once
#include <rawrbox/math/color.hpp>
#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/math/vector4.hpp>

#include <bgfx/bgfx.h>

#include <array>
#include <cstdint>
#include <vector>

namespace rawrbox {
	class UniformUtils {
	public:
		static void setUniform(const bgfx::UniformHandle& handle, double data);
		static void setUniform(const bgfx::UniformHandle& handle, float data);
		static void setUniform(const bgfx::UniformHandle& handle, uint64_t data);
		static void setUniform(const bgfx::UniformHandle& handle, rawrbox::Matrix4x4 data);
		static void setUniform(const bgfx::UniformHandle& handle, std::vector<rawrbox::Matrix4x4> data);
		static void setUniform(const bgfx::UniformHandle& handle, std::array<float, 4> data);
		static void setUniform(const bgfx::UniformHandle& handle, std::vector<std::array<float, 4>> data);
		static void setUniform(const bgfx::UniformHandle& handle, std::vector<std::array<float, 3>> data);
		static void setUniform(const bgfx::UniformHandle& handle, rawrbox::Vector2f data);
		static void setUniform(const bgfx::UniformHandle& handle, rawrbox::Vector3f data);
		static void setUniform(const bgfx::UniformHandle& handle, rawrbox::Vector4f data);
		static void setUniform(const bgfx::UniformHandle& handle, rawrbox::Colorf data);
	};
} // namespace rawrbox
