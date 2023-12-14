#pragma once

#include <rawrbox/math/vector3.hpp>

namespace rawrbox {
	template <class NumberType>
	struct BBOX_t {
	protected:
		using BBOXType = BBOX_t<NumberType>;

	public:
		rawrbox::Vector3f m_min = {};
		rawrbox::Vector3f m_max = {};
		rawrbox::Vector3f m_size = {};

		[[nodiscard]] bool isEmpty() const {
			return this->m_size == 0;
		}

		[[nodiscard]] const rawrbox::Vector3f& size() const {
			return this->m_size;
		}

		void combine(const BBOX_t<NumberType>& b) {
			this->m_min = {std::min(this->m_min.x, b.m_min.x), std::min(this->m_min.y, b.m_min.y), std::min(this->m_min.z, b.m_min.z)};
			this->m_max = {std::max(this->m_max.x, b.m_max.x), std::max(this->m_max.y, b.m_max.y), std::max(this->m_max.z, b.m_max.z)};

			this->m_size = m_min.abs() + m_max.abs();
		}

		bool operator==(const BBOX_t<NumberType>& other) const { return this->m_size == other.m_size; }
		bool operator!=(const BBOX_t<NumberType>& other) const { return !operator==(other); }
	};

	using BBOXd = BBOX_t<double>;
	using BBOXf = BBOX_t<float>;
	using BBOXi = BBOX_t<int>;
	using BBOX = BBOXf;
} // namespace rawrbox
