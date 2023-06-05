
#include <rawrbox/render/utils/uniforms.hpp>

namespace rawrbox {
	void UniformUtils::setUniform(const bgfx::UniformHandle& handle, double data) {
		std::array<float, 1> d = {static_cast<float>(data)};
		bgfx::setUniform(handle, d.data(), 1);
	}

	void UniformUtils::setUniform(const bgfx::UniformHandle& handle, float data) {
		std::array<float, 1> d = {data};
		bgfx::setUniform(handle, d.data(), 1);
	}

	void UniformUtils::setUniform(const bgfx::UniformHandle& handle, uint64_t data) {
		std::array<float, 1> d = {static_cast<float>(data)};
		bgfx::setUniform(handle, d.data(), 1);
	}

	void UniformUtils::setUniform(const bgfx::UniformHandle& handle, rawrbox::Matrix4x4 data) {
		bgfx::setUniform(handle, data.data());
	}

	void UniformUtils::setUniform(const bgfx::UniformHandle& handle, std::vector<rawrbox::Matrix4x4> data) {
		bgfx::setUniform(handle, data.front().data(), static_cast<uint16_t>(data.size()));
	}

	void UniformUtils::setUniform(const bgfx::UniformHandle& handle, std::array<float, 4> data) {
		bgfx::setUniform(handle, data.data());
	}

	void UniformUtils::setUniform(const bgfx::UniformHandle& handle, std::vector<std::array<float, 4>> data) {
		bgfx::setUniform(handle, data.front().data(), static_cast<uint16_t>(data.size()));
	}

	void UniformUtils::setUniform(const bgfx::UniformHandle& handle, std::vector<std::array<float, 3>> data) {
		bgfx::setUniform(handle, data.front().data(), static_cast<uint16_t>(data.size()));
	}

	void UniformUtils::setUniform(const bgfx::UniformHandle& handle, rawrbox::Vector2f data) {
		std::array<float, 2> d = {data.x, data.y};
		bgfx::setUniform(handle, d.data(), 2);
	}

	void UniformUtils::setUniform(const bgfx::UniformHandle& handle, rawrbox::Vector3f data) {
		std::array<float, 3> d = {data.x, data.y, data.z};
		bgfx::setUniform(handle, d.data(), 3);
	}

	void UniformUtils::setUniform(const bgfx::UniformHandle& handle, rawrbox::Vector4f data) {
		std::array<float, 4> d = {data.x, data.y, data.z, data.w};
		bgfx::setUniform(handle, d.data());
	}

	void UniformUtils::setUniform(const bgfx::UniformHandle& handle, rawrbox::Colorf data) {
		std::array<float, 4> d = {data.r, data.g, data.b, data.a};
		bgfx::setUniform(handle, d.data());
	}
} // namespace rawrbox
