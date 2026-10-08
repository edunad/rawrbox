#include <rawrbox/render/models/utils/animation.hpp>
#include <rawrbox/utils/logger.hpp>

#include <array>

namespace rawrbox {
	rawrbox::Matrix4x4 AnimationUtils::toMatrix(const ozz::math::Float4x4& mtx) {
		std::array<float, 16> data = {};
		for (size_t col = 0; col < 4; col++) {
			ozz::math::StorePtrU(mtx.cols[col], &data[col * 4]);
		}

		return rawrbox::Matrix4x4(data);
	}

	void AnimationUtils::getTransform(const ozz::math::SoaTransform& transform, size_t indx, rawrbox::Vector3f& pos, rawrbox::Vector4f& rotation, rawrbox::Vector3f& scale) {
		if (indx >= 4) RAWRBOX_CRITICAL("Invalid bone index {}", indx);

		std::array<float, 4> tx = {};
		std::array<float, 4> ty = {};
		std::array<float, 4> tz = {};
		std::array<float, 4> tw = {};

		// TRANSLATION ---
		ozz::math::StorePtrU(transform.translation.x, tx.data());
		ozz::math::StorePtrU(transform.translation.y, ty.data());
		ozz::math::StorePtrU(transform.translation.z, tz.data());

		pos = {tx[indx], ty[indx], tz[indx]};
		// ---------------

		// ROTATION ---
		ozz::math::StorePtrU(transform.rotation.x, tx.data());
		ozz::math::StorePtrU(transform.rotation.y, ty.data());
		ozz::math::StorePtrU(transform.rotation.z, tz.data());
		ozz::math::StorePtrU(transform.rotation.w, tw.data());

		rotation = {tx[indx], ty[indx], tz[indx], tw[indx]};
		// ------------

		// SCALE ---
		ozz::math::StorePtrU(transform.scale.x, tx.data());
		ozz::math::StorePtrU(transform.scale.y, ty.data());
		ozz::math::StorePtrU(transform.scale.z, tz.data());

		scale = {tx[indx], ty[indx], tz[indx]};
		// ---------
	}

	rawrbox::Matrix4x4 AnimationUtils::toMatrix(const ozz::math::SoaTransform& soa, size_t lane) {
		rawrbox::Vector3f pos = {};
		rawrbox::Vector4f rotation = {};
		rawrbox::Vector3f scale = {};

		rawrbox::AnimationUtils::getTransform(soa, lane, pos, rotation, scale);
		return rawrbox::Matrix4x4::mtxSRT(scale, rotation, pos);
	}
} // namespace rawrbox
