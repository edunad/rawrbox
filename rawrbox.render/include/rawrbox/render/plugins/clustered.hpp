#pragma once

#include <rawrbox/render/plugins/base.hpp>
#include <rawrbox/render/render_config.hpp>

#include <RefCntAutoPtr.hpp>
#include <ShaderMacroHelper.hpp>

#include <Buffer.h>
#include <PipelineState.h>

#include <vector>

namespace rawrbox {
	// Improved Culling for Tiled and Clustered Rendering - SIGGRAPH 2017
	class ClusteredPlugin : public rawrbox::RenderPlugin {
	protected:
		Diligent::IPipelineState* _tileCullProgram = nullptr;

		// BUFFERS ---
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _lightTiles;
		Diligent::RefCntAutoPtr<Diligent::IBufferView> _lightTilesWrite;
		Diligent::RefCntAutoPtr<Diligent::IBufferView> _lightTilesRead;

		Diligent::RefCntAutoPtr<Diligent::IBuffer> _decalTiles;
		Diligent::RefCntAutoPtr<Diligent::IBufferView> _decalTilesWrite;
		Diligent::RefCntAutoPtr<Diligent::IBufferView> _decalTilesRead;
		// -----------

		// CAMERAS ---
		std::vector<const rawrbox::CameraBase*> _cameras = {}; // Extra cameras
		// --------------

		// SIGNATURE ---
		Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature> _signature;
		Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> _signatureBind;
		// --------------

		virtual void buildBuffers();
		virtual void buildSignatures();
		virtual void buildPipelines();

	public:
		static constexpr uint32_t BIN_BITS = 32; // Lights / decals per bin
		static constexpr uint32_t TILES = RB_RENDER_TILES_X * RB_RENDER_TILES_Y;
		static constexpr uint32_t LIGHT_BINS = RB_RENDER_MAX_LIGHTS / BIN_BITS; // Per tile
		static constexpr uint32_t DECAL_BINS = RB_RENDER_MAX_DECALS / BIN_BITS; // Per tile

		ClusteredPlugin() = default;
		ClusteredPlugin(const ClusteredPlugin&) = delete;
		ClusteredPlugin(ClusteredPlugin&&) = delete;
		ClusteredPlugin& operator=(const ClusteredPlugin&) = delete;
		ClusteredPlugin& operator=(ClusteredPlugin&&) = delete;
		~ClusteredPlugin() override;

		// CAMERAS ----
		virtual void addCamera(const rawrbox::CameraBase& camera);
		virtual void removeCamera(const rawrbox::CameraBase& camera);
		
		[[nodiscard]] virtual bool isBinned(const rawrbox::CameraBase& camera) const;
		// ----------

		// UTILS ----
		virtual Diligent::ShaderMacroHelper getClusterMacros();

		virtual Diligent::IBufferView* getLightTilesBuffer(bool readOnly = true);
		virtual Diligent::IBufferView* getDecalTilesBuffer(bool readOnly = true);
		// ----------

		void initialize(const rawrbox::Vector2u& size) override;
		void upload() override;

		void signatures(std::vector<Diligent::PipelineResourceDesc>& sig) override;
		void bindStatic(Diligent::IPipelineResourceSignature& sig) override;

		void preRender(const rawrbox::CameraBase& camera) override;

		std::string getID() override;
	};
} // namespace rawrbox
