#pragma once
#include <rawrbox/render/model/base.hpp>

#include <array>
#include <memory>
#include <vector>

namespace rawrBox {
	class Model : public rawrBox::ModelBase {
	public:
		using ModelBase::ModelBase;
		void draw(const rawrBox::Vector3f& camPos) override;
	};
} // namespace rawrBox
