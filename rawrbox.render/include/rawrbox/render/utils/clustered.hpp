#pragma once

#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/math/vector4.hpp>

#include <cstdint>
#include <vector>

namespace rawrbox {
	struct ZBinEntry {
		float minZ = 0.F;
		float maxZ = 0.F;

		uint32_t index = 0;
	};

	class ClusteredUtils {
	public:
		static constexpr uint32_t EMPTY_BIN = 0x0000FFFF;
		static rawrbox::Vector4f toViewSphere(const rawrbox::Matrix4x4& view, const rawrbox::Vector4f& sphere);

		static rawrbox::ZBinEntry getEntry(const rawrbox::Vector4f& viewSphere, uint32_t index);
		static void build(std::vector<rawrbox::ZBinEntry>& entries, uint32_t offset, float zNear, float zFar, std::vector<uint32_t>& bins);
	};
} // namespace rawrbox
