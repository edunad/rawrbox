#pragma once

#include <rawrbox/math/vector4.hpp>
#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/textures/base.hpp>

namespace rawrbox {
	class PostProcessBase {
	protected:
		Diligent::IPipelineState* _pipeline = nullptr;
		std::array<rawrbox::Vector4f, MAX_POST_DATA> _data = {};

		// LOGGER ------
		std::unique_ptr<rawrbox::Logger> _logger = std::make_unique<rawrbox::Logger>("RawrBox-PostProcess");
		// -------------

	public:
		PostProcessBase() = default;
		PostProcessBase(PostProcessBase&&) = delete;
		PostProcessBase& operator=(PostProcessBase&&) = delete;
		PostProcessBase(const PostProcessBase&) = delete;
		PostProcessBase& operator=(const PostProcessBase&) = delete;
		virtual ~PostProcessBase() = default;

		virtual void init() = 0;
		virtual void applyEffect(const rawrbox::TextureBase& texture);
	};
} // namespace rawrbox
