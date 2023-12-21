#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/enums/draw.hpp>

#include <GraphicsTypes.h>
#include <RenderPass.h>

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace rawrbox {
	struct RenderPassAttachments {
		std::string id;
		Diligent::RenderPassAttachmentDesc desc;
	};

	class RenderPass {
	protected:
		bool _enabled = true;

		std::vector<std::string> _dependencies;
		std::vector<rawrbox::RenderPassAttachments> _passes;

	public:
		RenderPass() = default;
		RenderPass(const RenderPass&) = default;
		RenderPass(RenderPass&&) = delete;
		RenderPass& operator=(const RenderPass&) = default;
		RenderPass& operator=(RenderPass&&) = delete;
		virtual ~RenderPass() = default;

		// UTILS ----
		virtual void setEnabled(bool enabled);
		[[nodiscard]] virtual bool isEnabled() const;
		[[nodiscard]] virtual const std::string getID() const;
		// ----------

		// PASS ---
		[[nodiscard]] virtual const std::vector<rawrbox::RenderPassAttachments>& getPasses() const;
		virtual void addPass(const rawrbox::RenderPassAttachments& attach);

		[[nodiscard]] virtual const std::vector<std::string>& getDependencies() const;
		virtual void addDependency(const std::string& id);
		// --------

		virtual void initialize(const rawrbox::Vector2i& renderSize);
		virtual void resize(const rawrbox::Vector2i& renderSize);

		virtual void render(const rawrbox::DrawPass& pass);
		virtual void update();
	};
} // namespace rawrbox
