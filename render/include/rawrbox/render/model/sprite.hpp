#pragma once
#include <rawrbox/render/model/base.hpp>

#include <cstdint>

namespace rawrBox {
	class Sprite : public rawrBox::ModelBase {
	public:
		using ModelBase::ModelBase;
		void draw(const rawrBox::Vector3f& camPos) override;
	};
} // namespace rawrBox
