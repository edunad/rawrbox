#pragma once

#include <bgfx/bgfx.h>

#include <memory>

namespace rawrBox {
	class PostProcessBase {
	protected:
	public:
		PostProcessBase() = default;
		virtual ~PostProcessBase() = default;

		virtual void upload() = 0;
		virtual void applyEffect() = 0;
	};
} // namespace rawrBox
