#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector4.hpp>
#include <rawrbox/utils/logger.hpp>

#include <RefCntAutoPtr.hpp>

#include <Buffer.h>
#include <PipelineState.h>
#include <ShaderResourceBinding.h>
#include <Texture.h>

#include <array>
#include <memory>
#include <vector>

namespace rawrbox {
	class CameraBase;

	class OcclusionDraw {
	public:
		OcclusionDraw() = default;
		OcclusionDraw(const OcclusionDraw&) = default;
		OcclusionDraw(OcclusionDraw&&) = default;
		OcclusionDraw& operator=(const OcclusionDraw&) = default;
		OcclusionDraw& operator=(OcclusionDraw&&) = default;
		virtual ~OcclusionDraw() = default;

		virtual void drawLate() = 0;
	};

	struct CullConstants {
		std::array<rawrbox::Vector4f, 4> clip = {};

		rawrbox::Vector4f center = {};
		rawrbox::Vector4f extents = {};
		rawrbox::Vector4u settings = {};
		rawrbox::Vector4u hiz = {};
	};

	class Occlusion {
	protected:
		Diligent::IPipelineState* _depthReduceProgram = nullptr;
		Diligent::IPipelineState* _reduceProgram = nullptr;

		Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> _depthBind;
		Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> _reduceBind;

		// HI-Z ----
		Diligent::RefCntAutoPtr<Diligent::ITexture> _hiz;
		std::vector<Diligent::RefCntAutoPtr<Diligent::ITextureView>> _mips = {};

		rawrbox::Vector2u _size = {};

		bool _built = false;
#ifdef _DEBUG
		bool _frozen = false;
#endif
		// -----------

		// CULLING (shared by every model, per dispatch resources are dynamic) ---
		Diligent::IPipelineState* _cullProgram = nullptr;
		Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> _cullBind;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _cullConstants;
		// -----------

		std::vector<rawrbox::OcclusionDraw*> _lateDraw = {};

		bool _enabled = true;
		bool _recording = false;

		// LOGGER ------
		std::unique_ptr<rawrbox::Logger> _logger = std::make_unique<rawrbox::Logger>("RawrBox-Occlusion");
		// -------------

	public:
		static constexpr uint32_t CULL_THREADS = 256U;
		static constexpr uint32_t CULL_MAX_GROUPS = 65535U;

		Occlusion() = default;
		Occlusion(Occlusion&&) = delete;
		Occlusion& operator=(Occlusion&&) = delete;
		Occlusion(const Occlusion&) = delete;
		Occlusion& operator=(const Occlusion&) = delete;
		virtual ~Occlusion();

		virtual void upload(const rawrbox::CameraBase& camera);
		virtual void resize(const rawrbox::Vector2u& size);

		// PASSES ---
		virtual void begin();
		virtual void end(const rawrbox::CameraBase& camera);

		virtual void queue(rawrbox::OcclusionDraw& draw);
		// -----------

		[[nodiscard]] virtual bool isRecording() const;

		[[nodiscard]] virtual Diligent::ITextureView* getHiZ() const;
		[[nodiscard]] virtual const rawrbox::Vector2u& getSize() const;
		[[nodiscard]] virtual uint32_t getMips() const;

		[[nodiscard]] virtual Diligent::IPipelineState* getCullProgram() const;
		[[nodiscard]] virtual Diligent::IShaderResourceBinding* getCullBind() const;
		[[nodiscard]] virtual Diligent::IBuffer* getCullConstants() const;

		virtual void setEnabled(bool enabled);
		[[nodiscard]] virtual bool isEnabled() const;
	};
} // namespace rawrbox