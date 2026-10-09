#include <rawrbox/render/models/utils/animation.hpp>
#include <rawrbox/utils/logger.hpp>

#include <array>

namespace rawrbox {
	void TransformBlend::add(const rawrbox::Vector3f& _pos, const rawrbox::Vector4f& _rotation, const rawrbox::Vector3f& _scale, float _weight) {
		if (_weight <= 0.F) return;

		rawrbox::Vector4f rot = _rotation;
		if (this->weight > 0.F && (this->rotation.x * rot.x + this->rotation.y * rot.y + this->rotation.z * rot.z + this->rotation.w * rot.w) < 0.F) rot = rot * -1.F;

		this->pos += _pos * _weight;
		this->rotation += rot * _weight;
		this->scale += _scale * _weight;
		this->weight += _weight;
	}

	rawrbox::Matrix4x4 TransformBlend::toMatrix() const {
		if (this->weight <= 0.F) return {};

		const float inv = 1.F / this->weight;
		return rawrbox::Matrix4x4::mtxSRT(this->scale * inv, this->rotation.normalized(), this->pos * inv);
	}

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
} // namespace rawrbox
