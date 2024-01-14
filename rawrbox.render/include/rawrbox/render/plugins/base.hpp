#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/enums/draw.hpp>
#include <rawrbox/render/textures/render.hpp>

#include <GraphicsTypes.h>
#include <RenderPass.h>
#include <TextureView.h>

#include <string>

namespace rawrbox {
	class RenderPlugin {
	protected:
		bool _enabled = true;

		// LOGGER ------
		std::unique_ptr<rawrbox::Logger> _logger = std::make_unique<rawrbox::Logger>("RawrBox-RenderPlugin");
		// -------------

	public:
		RenderPlugin() = default;
		RenderPlugin(const RenderPlugin&) = delete;
		RenderPlugin(RenderPlugin&&) = delete;
		RenderPlugin& operator=(const RenderPlugin&) = delete;
		RenderPlugin& operator=(RenderPlugin&&) = delete;
		virtual ~RenderPlugin() = default;

		// UTILS ----
		virtual void setEnabled(bool enabled);
		[[nodiscard]] virtual bool isEnabled() const;
		[[nodiscard]] virtual const std::string getID() const;
		// ----------

		virtual void requirements(Diligent::DeviceFeatures& features);

		virtual void signatures(std::vector<Diligent::PipelineResourceDesc>& sig);
		virtual void bind(Diligent::IPipelineResourceSignature& sig);

		virtual void initialize(const rawrbox::Vector2i& renderSize);
		virtual void resize(const rawrbox::Vector2i& renderSize);
		virtual void upload();

		virtual void preRender();
		virtual void postRender(rawrbox::TextureRender& renderTarget);

		virtual void update();
	};
} // namespace rawrbox
