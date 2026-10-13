#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/render/cameras/base.hpp>
#include <rawrbox/render/occlusion.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

#include <algorithm>
#include <bit>

namespace rawrbox {
	Occlusion::~Occlusion() {
		this->_lateDraw.clear();
		this->_mips.clear();

		RAWRBOX_DESTROY(this->_hiz);
		RAWRBOX_DESTROY(this->_depthBind);
		RAWRBOX_DESTROY(this->_reduceBind);
		RAWRBOX_DESTROY(this->_cullBind);
		RAWRBOX_DESTROY(this->_cullConstants);

		this->_depthReduceProgram = nullptr;
		this->_reduceProgram = nullptr;
		this->_cullProgram = nullptr;
	}

	void Occlusion::upload(const rawrbox::CameraBase& camera) {
		if (this->_depthReduceProgram != nullptr) RAWRBOX_CRITICAL("Occlusion already uploaded!");

		rawrbox::PipeComputeSettings settings;
		settings.pCS = "hiz.csh";
		settings.resourceType = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC;
		settings.macros.AddShaderMacro("REDUCE_THREADS", RB_RENDER_OCCLUSION_THREADS);

		settings.entry = "DepthReduce";
		this->_depthReduceProgram = rawrbox::PipelineUtils::createComputePipeline("Occlusion::DepthReduce", settings);

		settings.entry = "Reduce";
		this->_reduceProgram = rawrbox::PipelineUtils::createComputePipeline("Occlusion::Reduce", settings);

		this->_depthReduceProgram->CreateShaderResourceBinding(&this->_depthBind, true);
		this->_reduceProgram->CreateShaderResourceBinding(&this->_reduceBind, true);
		if (this->_depthBind == nullptr || this->_reduceBind == nullptr) RAWRBOX_CRITICAL("Failed to create occlusion bindings");

		// CULLING ---
		rawrbox::PipeComputeSettings cull;
		cull.pCS = "instance_cull.csh";
		cull.resourceType = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE;
		cull.resources = {
		    {Diligent::SHADER_TYPE_COMPUTE, "Instances", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
		    {Diligent::SHADER_TYPE_COMPUTE, "VisibleInstances", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
		    {Diligent::SHADER_TYPE_COMPUTE, "Visibility", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
		    {Diligent::SHADER_TYPE_COMPUTE, "DrawArgs", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
		    {Diligent::SHADER_TYPE_COMPUTE, "HiZ", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC}};

		cull.macros.AddShaderMacro("CULL_THREADS", CULL_THREADS);

		this->_cullProgram = rawrbox::PipelineUtils::createComputePipeline("Model::Cull", cull);
		this->_cullProgram->CreateShaderResourceBinding(&this->_cullBind, true);
		if (this->_cullBind == nullptr) RAWRBOX_CRITICAL("Failed to create culling binding");

		Diligent::BufferDesc constantsDesc;
		constantsDesc.Name = "RawrBox::Buffer::Cull::Constants";
		constantsDesc.Usage = Diligent::USAGE_DYNAMIC;
		constantsDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
		constantsDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
		constantsDesc.Size = sizeof(rawrbox::CullConstants);

		rawrbox::RENDERER->device()->CreateBuffer(constantsDesc, nullptr, &this->_cullConstants);
		if (this->_cullConstants == nullptr) RAWRBOX_CRITICAL("Failed to create culling constants");

		this->_cullBind->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "CullConstants")->Set(this->_cullConstants);
		// -----------

		this->resize(camera.getRenderTarget()->getSize());
		this->_logger->info("Initializing");
	}

	void Occlusion::resize(const rawrbox::Vector2u& size) {
		// HI-Z  ----
		this->_mips.clear();
		RAWRBOX_DESTROY(this->_hiz);

		const uint32_t width = std::max(size.x >> 1U, 1U);
		const uint32_t height = std::max(size.y >> 1U, 1U);

		Diligent::TextureDesc desc;
		desc.Name = "RawrBox::Occlusion::HiZ";
		desc.Type = Diligent::RESOURCE_DIM_TEX_2D;
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = static_cast<uint32_t>(std::bit_width(std::max(width, height)));
		desc.Format = Diligent::TEX_FORMAT_R32_FLOAT;
		desc.BindFlags = Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_UNORDERED_ACCESS;
		desc.Usage = Diligent::USAGE_DEFAULT;

		rawrbox::RENDERER->device()->CreateTexture(desc, nullptr, &this->_hiz);
		if (this->_hiz == nullptr) RAWRBOX_CRITICAL("Failed to create hi-z texture");

		for (uint32_t mip = 0; mip < desc.MipLevels; mip++) {
			Diligent::TextureViewDesc view;

			view.ViewType = Diligent::TEXTURE_VIEW_UNORDERED_ACCESS;
			view.TextureDim = Diligent::RESOURCE_DIM_TEX_2D;
			view.MostDetailedMip = mip;
			view.NumMipLevels = 1;

			auto& handle = this->_mips.emplace_back();
			this->_hiz->CreateView(view, &handle);

			if (handle == nullptr) RAWRBOX_CRITICAL("Failed to create hi-z mip {} view", mip);
		}

		this->_size = size;
		this->_built = false;
		// -----------
	}

	void Occlusion::begin() {
		this->_lateDraw.clear();
		this->_recording = this->_enabled;
	}

	void Occlusion::end(const rawrbox::CameraBase& camera) {
		this->_recording = false;
		if (this->_lateDraw.empty()) return;

		// REGEN HI-Z ---
#ifdef _DEBUG
		const bool frozen = camera.isCullFrozen();
		const bool rebuild = !frozen || !this->_frozen || !this->_built;
		this->_frozen = frozen;
#else
		constexpr bool rebuild = true;
#endif

		if (rebuild) {
			auto* context = rawrbox::RENDERER->context();

			for (uint32_t mip = 0; mip < this->_mips.size(); mip++) {
				auto* bind = mip == 0 ? this->_depthBind.RawPtr() : this->_reduceBind.RawPtr();
				if (mip == 0) {
					bind->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "Depth")->Set(camera.getDepth());
				} else {
					bind->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "Source")->Set(this->_mips[mip - 1]);
				}

				bind->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "Target")->Set(this->_mips[mip]);

				context->SetPipelineState(mip == 0 ? this->_depthReduceProgram : this->_reduceProgram);
				context->CommitShaderResources(bind, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

				Diligent::DispatchComputeAttribs attribs;
				attribs.ThreadGroupCountX = rawrbox::MathUtils::divideRound(std::max(this->_size.x >> (mip + 1U), 1U), RB_RENDER_OCCLUSION_THREADS);
				attribs.ThreadGroupCountY = rawrbox::MathUtils::divideRound(std::max(this->_size.y >> (mip + 1U), 1U), RB_RENDER_OCCLUSION_THREADS);
				context->DispatchCompute(attribs);
			}

			this->_built = true;
		}
		// -----------

		// LATE PASS ----
		auto* target = camera.getRenderTarget();
		target->startRecord(false);

		for (auto* draw : this->_lateDraw) {
			draw->drawLate();
		}

		target->stopRecord();
		this->_lateDraw.clear();
		// -----------
	}

	void Occlusion::queue(rawrbox::OcclusionDraw& draw) {
		if (!this->_recording) RAWRBOX_CRITICAL("Occlusion not in record mode, switch to recording first!");
		this->_lateDraw.push_back(&draw);
	}

	bool Occlusion::isRecording() const { return this->_recording; }

	Diligent::ITextureView* Occlusion::getHiZ() const {
		if (this->_hiz == nullptr) return nullptr;
		return this->_hiz->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
	}

	const rawrbox::Vector2u& Occlusion::getSize() const { return this->_size; }

	Diligent::IPipelineState* Occlusion::getCullProgram() const { return this->_cullProgram; }
	Diligent::IShaderResourceBinding* Occlusion::getCullBind() const { return this->_cullBind; }
	Diligent::IBuffer* Occlusion::getCullConstants() const { return this->_cullConstants; }

	uint32_t Occlusion::getMips() const { return this->_built ? static_cast<uint32_t>(this->_mips.size()) : 0U; }

	void Occlusion::setEnabled(bool enabled) { this->_enabled = enabled; }
	bool Occlusion::isEnabled() const { return this->_enabled; }
} // namespace rawrbox