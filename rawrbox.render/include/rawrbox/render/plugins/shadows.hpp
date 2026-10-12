#pragma once

#include <rawrbox/math/bbox.hpp>
#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/math/rect.hpp>
#include <rawrbox/math/vector4.hpp>
#include <rawrbox/render/plugins/base.hpp>
#include <rawrbox/render/shadows/caster.hpp>

#include <GPUCompletionAwaitQueue.hpp>
#include <RefCntAutoPtr.hpp>
#include <ShaderMacroHelper.hpp>

#include <Buffer.h>
#include <PipelineState.h>
#include <Texture.h>

#include <array>
#include <memory>
#include <vector>

// Based on Sparse Virtual Shadows Maps (but without the sparse, because problems)
// https://ktstephano.github.io/rendering/stratusgfx/svsm
// https://dev.epicgames.com/documentation/unreal-engine/virtual-shadow-maps-in-unreal-engine

namespace rawrbox {
	class LightBase;

	struct ShadowSettings {
		// DEFAULT SETTINGS ----
		uint32_t virtualSize = 2048U;

		uint32_t pageSize = 64U;
		uint32_t poolSize = 4096U;

		uint32_t maxLights = 256U;

		uint32_t pageCache = 120U;
		uint32_t budget = 64U;

		float lodBias = 2.F;

		float normalBias = 1.5F;
		float depthBias = 0.0015F;

		float dither = 2.F;
		float clipmapExtent = 8.F;
		// -------------
	};

#ifdef _DEBUG
	enum class SHADOW_DEBUG : uint32_t {
		NONE = 0,

		MIPS,
		PAGES,
		LIGHTS,

		COUNT
	};
#endif

	struct ShadowConstants {
		rawrbox::Matrix4x4 invView = {};

		rawrbox::Vector4u settings = {};
		rawrbox::Vector4f params = {};
		rawrbox::Vector4u misc = {};
	};

	struct ShadowSlotData {
		rawrbox::Vector4f right = {};
		rawrbox::Vector4f up = {};
		rawrbox::Vector4f forward = {};

		std::array<rawrbox::Vector4i, 3> origins = {};

		rawrbox::Vector4f params = {};
		rawrbox::Vector4f light = {};
	};

	struct ShadowSlot {
		std::weak_ptr<rawrbox::LightBase> light = {};

		rawrbox::Vector3f position = {};
		float radius = 0.F;

		rawrbox::Vector3f direction = {};
		float cosCone = -1.F;

		// DIRECTIONAL LIGHTS ---
		bool directional = false;

		rawrbox::Vector3f right = {};
		rawrbox::Vector3f up = {};

		float depthOrigin = 0.F;
		std::array<rawrbox::Vector2i, 6> origins = {};
		// ----------

		bool invalid = true;
	};

	struct ShadowFaceBatch {
		uint32_t batch = 0;
		uint32_t slot = 0;
		uint32_t face = 0;

		rawrbox::ShadowMode mode = rawrbox::ShadowMode::STATIC;
		rawrbox::Recti rect = {};

		size_t casters = 0;
	};

	struct ShadowReadback {
		Diligent::RefCntAutoPtr<Diligent::IBuffer> buffer;
		uint32_t frame = 0;
	};

	struct ShadowLightCandidate {
		std::shared_ptr<rawrbox::LightBase> light = nullptr;
		bool hidden = false;

		float priority = 0.F;
	};

	struct ShadowBatchCaster {
		rawrbox::ShadowCaster* caster = nullptr;
		uint32_t draws = 0;
	};

	struct ShadowSlotCaster {
		rawrbox::ShadowCaster* caster = nullptr;
		rawrbox::ShadowMode mode = rawrbox::ShadowMode::STATIC;

		rawrbox::BBOX bounds = {};
		uint32_t faces = 0;
	};

	class ShadowPlugin : public rawrbox::RenderPlugin {
	protected:
		rawrbox::ShadowSettings _settings = {};

		// STATS ---
		uint32_t _pages = 0;
		uint32_t _mips = 0;
		uint32_t _faceEntries = 0;

		uint32_t _poolPages = 0;
		uint32_t _totalPages = 0;
		uint32_t _totalFaces = 0;
		// ------------

#ifdef _DEBUG
		rawrbox::SHADOW_DEBUG _debugMode = rawrbox::SHADOW_DEBUG::NONE;
		Diligent::IPipelineState* _debugProgram = nullptr;
#endif

		uint32_t _frame = 0;
		uint32_t _activeSlots = 0;

		std::vector<rawrbox::ShadowSlot> _slots = {};

		std::vector<rawrbox::ShadowLightCandidate> _lightCandidate = {};
		std::vector<bool> _keepSlots = {};
		std::vector<uint32_t> _slotFrames = {};

		// FRAME ---
		std::vector<rawrbox::Recti> _rects = {};
		std::vector<rawrbox::Vector2u> _invalids = {};

		std::vector<rawrbox::ShadowSlotData> _slotData = {};

		std::vector<float> _receiverDepth = {};
		std::vector<uint32_t> _receiverClear = {};

		std::vector<uint32_t> _facePending = {};
		std::vector<rawrbox::Recti> _faceRects = {};

		std::vector<rawrbox::ShadowFaceBatch> _batches = {};
		std::vector<rawrbox::ShadowSlotCaster> _slotCasters = {};
		std::vector<rawrbox::ShadowBatchCaster> _batchCasters = {};
		std::vector<uint32_t> _batchRects = {};

		std::vector<rawrbox::Vector4u> _drawRecords = {};
		std::vector<uint32_t> _drawArgs = {};

		uint32_t _drawCursor = 0;
		uint32_t _drawLimit = 0;
		bool _drawOverflow = false;

		uint32_t _revision = 0;
		rawrbox::Vector2u _drawView = {};
		// ---------

		// BUFFERS ---
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _constants;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _drawConstants;

		Diligent::RefCntAutoPtr<Diligent::IBuffer> _pageTable;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _requests;

		Diligent::RefCntAutoPtr<Diligent::IBuffer> _physOwner;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _physClear;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _touched;

		Diligent::RefCntAutoPtr<Diligent::IBuffer> _freeStack;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _allocator;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _ages;

		Diligent::RefCntAutoPtr<Diligent::IBuffer> _clearList;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _clearArgs;

		Diligent::RefCntAutoPtr<Diligent::IBuffer> _invalidationBuffer;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _receiverBuffer;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _feedbackBuffer;

		Diligent::RefCntAutoPtr<Diligent::IBuffer> _viewBuffer;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _rectBuffer;

		Diligent::RefCntAutoPtr<Diligent::IBuffer> _batchBuffer;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _batchRectBuffer;

		Diligent::RefCntAutoPtr<Diligent::IBuffer> _faceRectBuffer;

		Diligent::RefCntAutoPtr<Diligent::IBuffer> _drawRecordBuffer;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _drawArgsBuffer;

		Diligent::RefCntAutoPtr<Diligent::IBuffer> _slotBuffer;
		// -----------

		// TEXTURES ---
		Diligent::RefCntAutoPtr<Diligent::ITexture> _pool;
		Diligent::ITextureView* _poolRead = nullptr;
		Diligent::ITextureView* _poolWrite = nullptr;

		Diligent::RefCntAutoPtr<Diligent::ITexture> _canvas;
		Diligent::ITextureView* _canvasRT = nullptr;
		// ------------

		// SIGNATURES ---
		Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature> _signature;
		Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> _signatureBind;

		Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature> _markSignature;
		Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> _markBind;

		Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature> _casterSignature;
		Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> _casterBind;
		// --------------

		// PIPELINES ---
		Diligent::IPipelineState* _invalidateProgram = nullptr;
		Diligent::IPipelineState* _markProgram = nullptr;
		Diligent::IPipelineState* _freeProgram = nullptr;
		Diligent::IPipelineState* _evictProgram = nullptr;
		Diligent::IPipelineState* _allocateProgram = nullptr;
		Diligent::IPipelineState* _selectProgram = nullptr;
		Diligent::IPipelineState* _patchProgram = nullptr;
		Diligent::IPipelineState* _prepareProgram = nullptr;
		Diligent::IPipelineState* _clearProgram = nullptr;
		Diligent::IPipelineState* _resolveProgram = nullptr;
		Diligent::IPipelineState* _discardProgram = nullptr;
		// --------------

		std::unique_ptr<Diligent::GPUCompletionAwaitQueue<rawrbox::ShadowReadback>> _readback;
		uint32_t _readbackPending = 0;

		virtual void buildBuffers();
		virtual void buildTextures();
		virtual void buildSignatures();
		virtual void buildPipelines();

		// FRAME ---
		virtual void readFeedback();
		virtual void updateSlots();

		virtual void resetFeedback(uint32_t slot);

		virtual void updateClipmaps(const rawrbox::CameraBase& camera);
		virtual void updateCasters();

		virtual void updateConstants(const rawrbox::CameraBase& camera);

		virtual void recordCasters();
		virtual void recordBatch(uint32_t slot, uint32_t face, rawrbox::ShadowMode mode, const rawrbox::Recti& rect);
		[[nodiscard]] virtual rawrbox::ShadowDraw getDraw(const rawrbox::ShadowFaceBatch& batch, bool bind);

		virtual void allocatePages();

		virtual void render();
		virtual void resolvePages();

#ifdef _DEBUG
		virtual void drawDebug(const rawrbox::CameraBase& camera);
#endif

		virtual void dispatch(Diligent::IPipelineState* program, uint32_t groups);
		virtual void bindDrawView(uint32_t first, uint32_t offset);
		// ---------

		// LAYOUT ---
		[[nodiscard]] virtual uint32_t total() const;
		[[nodiscard]] virtual float getMaxBias() const;

		[[nodiscard]] virtual rawrbox::Recti getFullRect() const;
		[[nodiscard]] virtual uint32_t getBatch(uint32_t slot, uint32_t face, rawrbox::ShadowMode mode) const;
		[[nodiscard]] virtual bool isBehindReceivers(const rawrbox::BBOX& bounds, uint32_t slot, uint32_t face) const;

		[[nodiscard]] virtual rawrbox::Recti getFaceRect(const rawrbox::BBOX& bounds, const rawrbox::ShadowSlot& slot, uint32_t face) const;
		[[nodiscard]] virtual rawrbox::Recti getClipmapRect(const rawrbox::BBOX& bounds, const rawrbox::ShadowSlot& slot, uint32_t level) const;

		[[nodiscard]] virtual uint32_t getCasterFaces(const rawrbox::BBOX& bounds, const rawrbox::ShadowSlot& slot) const;
		[[nodiscard]] virtual float getClipmapTexelSize(uint32_t level) const;
		// ---------

		// UTILS ---
		static void createSignature(const char* name, uint8_t index, const std::vector<Diligent::PipelineResourceDesc>& resources, Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature>& signature);

		[[nodiscard]] static bool requestShadow(const rawrbox::LightBase& light);
		[[nodiscard]] static bool sphereIntersects(const rawrbox::BBOX& bounds, const rawrbox::ShadowSlot& slot);
		// ---------

	public:
		static constexpr uint32_t MAX_MIPS = 16U;

		// shadow_uniforms.fxh ---
		static constexpr uint32_t VIEW_FACE_SHIFT = 13U;
		static constexpr uint32_t MAX_SLOTS = (1U << VIEW_FACE_SHIFT) - 1U;
		static constexpr uint32_t PAGE_EMPTY = 0xFFFFFFFFU;
		// ----------

		static constexpr float RECEIVER_MARGIN = 1.1F;
		static constexpr float SLOT_KEEP = 0.75F;

		static constexpr uint32_t READBACK_FRAMES = 3U;
		static constexpr uint32_t MOTION_FRAMES = READBACK_FRAMES + 2U;
		static constexpr uint32_t MAX_DRAWS = 32768U;
		static constexpr uint32_t DRAW_ARGS = 5U;

		static constexpr int MOTION_MARGIN = 4;

		static constexpr uint32_t EMPTY_PACKED_RECT = 0x0000FFFFU;

		static constexpr float CLIPMAP_DEPTH = 2048.F;
		static constexpr float CLIPMAP_REBASE = 1024.F;

		static constexpr uint32_t THREADS = 64U;
		static constexpr uint32_t MARK_THREADS = 8U;
		static constexpr uint32_t ALLOCATOR_ENTRIES = 4U;

		explicit ShadowPlugin(const rawrbox::ShadowSettings& settings = {});
		ShadowPlugin(const ShadowPlugin&) = delete;
		ShadowPlugin(ShadowPlugin&&) = delete;
		ShadowPlugin& operator=(const ShadowPlugin&) = delete;
		ShadowPlugin& operator=(ShadowPlugin&&) = delete;
		~ShadowPlugin() override;

		// SETTINGS ---
		[[nodiscard]] virtual const rawrbox::ShadowSettings& getSettings() const;

		virtual void setLodBias(float bias);
		[[nodiscard]] virtual float getLodBias() const;

		virtual void setNormalBias(float bias);
		[[nodiscard]] virtual float getNormalBias() const;

		virtual void setDepthBias(float bias);
		[[nodiscard]] virtual float getDepthBias() const;

		virtual void setDither(float size);
		[[nodiscard]] virtual float getDither() const;

		virtual void setBudget(uint32_t pages);
		[[nodiscard]] virtual uint32_t getBudget() const;

#ifdef _DEBUG
		virtual void setDebug(rawrbox::SHADOW_DEBUG mode);
		[[nodiscard]] virtual rawrbox::SHADOW_DEBUG getDebug() const;
#endif
		// ------------

		// UTILS ----
		virtual Diligent::ShaderMacroHelper getShadowMacros();
		virtual void createCasterPipeline(const std::string& id, const std::vector<Diligent::LayoutElement>& layout, const Diligent::ShaderMacroHelper& helper, bool normals);
		// ----------

		void setEnabled(bool enabled) override;
		void requirements(Diligent::DeviceFeatures& features) override;

		void initialize(const rawrbox::Vector2u& size) override;
		void upload() override;

		void signatures(std::vector<Diligent::PipelineResourceDesc>& sig) override;
		void bindStatic(Diligent::IPipelineResourceSignature& sig) override;

		void update() override;

		void preRender(const rawrbox::CameraBase& camera) override;
		void postRender(const rawrbox::CameraBase& camera) override;

		std::string getID() override;
	};
} // namespace rawrbox