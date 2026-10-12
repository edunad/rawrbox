#pragma once

#include <rawrbox/math/vector4.hpp>
#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/textures/base.hpp>
#include <rawrbox/render/textures/render.hpp>

namespace rawrbox {
	class PostProcessBase {
	protected:
		Diligent::IBuffer* _buffer = nullptr;
		Diligent::IPipelineState* _pipeline = nullptr;
		bool _enabled = true;

		std::array<rawrbox::Vector4f, RB_RENDER_MAX_POST_DATA> _data = {};

		// LOGGER ------
		std::unique_ptr<rawrbox::Logger> _logger = std::make_unique<rawrbox::Logger>("RawrBox-PostProcess");
		// -------------

		virtual void draw(Diligent::IPipelineState* pipeline, const rawrbox::TextureRender& target, const rawrbox::Vector4u& textureIDs);

	public:
		PostProcessBase() = default;
		PostProcessBase(PostProcessBase&&) = delete;
		PostProcessBase& operator=(PostProcessBase&&) = delete;
		PostProcessBase(const PostProcessBase&) = delete;
		PostProcessBase& operator=(const PostProcessBase&) = delete;
		virtual ~PostProcessBase() = default;

		virtual void init();
		virtual void applyEffect(const rawrbox::TextureBase& source, const rawrbox::TextureRender& target, uint32_t depthID);

		virtual void setEnabled(bool enabled);
		[[nodiscard]] virtual bool isEnabled() const;
	};
} // namespace rawrbox