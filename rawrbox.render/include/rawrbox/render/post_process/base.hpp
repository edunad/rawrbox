#pragma once

#include <rawrbox/math/vector4.hpp>
#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/textures/base.hpp>

namespace rawrbox {
	class PostProcessBase {
	protected:
		Diligent::IBuffer* _buffer = nullptr;
		Diligent::IPipelineState* _pipeline = nullptr;
		bool _enabled = true;

#ifdef UINT_DATA
		std::array<rawrbox::Vector4u, RB_RENDER_MAX_POST_DATA> _data = {};
#else
		std::array<rawrbox::Vector4f, RB_RENDER_MAX_POST_DATA> _data = {};
#endif

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

		virtual void init();
		virtual void applyEffect(const rawrbox::TextureBase& texture);

		virtual void setEnabled(bool enabled);
		[[nodiscard]] virtual bool isEnabled() const;
	};
} // namespace rawrbox
