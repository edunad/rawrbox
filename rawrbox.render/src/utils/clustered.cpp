#include <rawrbox/render/render_config.hpp>
#include <rawrbox/render/utils/clustered.hpp>

#include <algorithm>

namespace rawrbox {
	rawrbox::Vector4f ClusteredUtils::toViewSphere(const rawrbox::Matrix4x4& view, const rawrbox::Vector4f& sphere) {
		const auto center = view.mulVec(sphere.xyz());
		return {center.x, center.y, center.z, sphere.w};
	}

	rawrbox::ZBinEntry ClusteredUtils::getEntry(const rawrbox::Vector4f& viewSphere, uint32_t index) {
		float depth = viewSphere.z;
		if (rawrbox::Matrix4x4::MTX_RIGHT_HANDED) depth = -depth;
		return {depth - viewSphere.w, depth + viewSphere.w, index};
	}

	void ClusteredUtils::build(std::vector<rawrbox::ZBinEntry>& entries, uint32_t offset, float zNear, float zFar, std::vector<uint32_t>& bins) {
		bins.assign(RB_RENDER_ZBINS, EMPTY_BIN);
		std::ranges::sort(entries, [](const rawrbox::ZBinEntry& a, const rawrbox::ZBinEntry& b) { return a.minZ < b.minZ; });
		if (zFar <= zNear) return; // Invalid camera

		const float scale = static_cast<float>(RB_RENDER_ZBINS) / (zFar - zNear);
		constexpr auto lastBin = static_cast<float>(RB_RENDER_ZBINS - 1);

		for (size_t i = 0; i < entries.size(); i++) {
			const auto& entry = entries[i];
			if (entry.maxZ < zNear || entry.minZ > zFar) continue; // Outside

			const auto index = static_cast<uint32_t>(i) + offset;
			const auto firstBin = static_cast<uint32_t>(std::clamp((entry.minZ - zNear) * scale, 0.F, lastBin));
			const auto endBin = static_cast<uint32_t>(std::clamp((entry.maxZ - zNear) * scale, 0.F, lastBin));

			for (uint32_t bin = firstBin; bin <= endBin; bin++) {
				const uint32_t first = std::min(bins[bin] & 0xFFFFU, index);
				const uint32_t last = std::max(bins[bin] >> 16U, index);

				bins[bin] = first | (last << 16U);
			}
		}
	}
} // namespace rawrbox
