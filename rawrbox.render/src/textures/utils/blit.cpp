#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/textures/utils/blit.hpp>

#include <GraphicsAccessories.hpp>

#include <utility>

namespace rawrbox {
	TextureBLIT::TextureBLIT(const rawrbox::Vector2u& size) {
		this->_data.size = size;
		this->_data.channels = 4;

		this->_name = "RawrBox::GPU::Blit";
	}

	TextureBLIT::~TextureBLIT() {
		this->_copies.reset();
		this->_callbacks.clear();
		this->_reads.clear();
	}

	// PROTECTED ---
	void TextureBLIT::createStaging() {
		Diligent::TextureDesc desc;
		desc.Type = Diligent::RESOURCE_DIM_TEX_2D;
		desc.Width = this->_data.size.x;
		desc.Height = this->_data.size.y;
		desc.MipLevels = 1;
		desc.Usage = Diligent::USAGE_STAGING;
		desc.BindFlags = Diligent::BIND_NONE;
		desc.CPUAccessFlags = Diligent::CPU_ACCESS_READ;
		desc.Format = Diligent::TEX_FORMAT_RGBA8_UNORM;
		desc.Name = this->_name.c_str();

		rawrbox::RENDERER->device()->CreateTexture(desc, nullptr, &this->_tex);
		if (this->_tex == nullptr) RAWRBOX_CRITICAL("Failed to create blit texture {}x{}", this->_data.size.x, this->_data.size.y);
	}

	void TextureBLIT::startRead(Diligent::ITexture& source, Diligent::Box box, ReadCallback callback) {
		const rawrbox::Vector2u size = {box.Width(), box.Height()};

		this->copy(&source, &box, [this, size, callback = std::move(callback)]() {
			this->blit(nullptr, [this, &size, &callback](const uint8_t* pixels, const uint64_t stride) {
				if (pixels == nullptr) {
					this->_logger->warn("Failed to map the blit texture, returning no pixels");
					callback({}, 0, {});
					return;
				}

				const uint64_t bytes = stride * (size.y - 1U) + static_cast<uint64_t>(size.x) * 4U;
				callback(std::span<const uint8_t>(pixels, bytes), stride, size);
			});
		});
	}

	void TextureBLIT::upload(Diligent::TEXTURE_FORMAT /*format*/, uint32_t /*flags*/) {
		if (this->_tex != nullptr) RAWRBOX_CRITICAL("Already uploaded");

		this->createStaging();
		this->_copies = std::make_unique<Diligent::GPUCompletionAwaitQueue<Diligent::RefCntAutoPtr<Diligent::ITexture>>>(rawrbox::RENDERER->device());
	}

	void TextureBLIT::resize(const rawrbox::Vector2u& size) {
		if (size.x == 0U || size.y == 0U) RAWRBOX_CRITICAL("Invalid blit size {}x{}", size.x, size.y);
		if (this->_pending) RAWRBOX_CRITICAL("Cannot resize the blit texture while a copy is pending");
		if (size == this->_data.size) return;

		this->_data.size = size;
		if (this->_tex == nullptr) return;

		RAWRBOX_DESTROY(this->_tex);
		this->createStaging();
	}

	void TextureBLIT::copy(Diligent::ITexture* base, Diligent::Box* box, const std::function<void()>& callback) {
		if (base == nullptr) RAWRBOX_CRITICAL("Invalid texture");
		if (callback == nullptr) RAWRBOX_CRITICAL("Invalid callback");

		if (this->_tex == nullptr) RAWRBOX_CRITICAL("Blit texture not uploaded! Did you call upload?");
		if (this->_pending) RAWRBOX_CRITICAL("A copy is already pending, wait until isPending() is false");
		if (this->_mapped) RAWRBOX_CRITICAL("Cannot copy while the blit texture is mapped");

		const auto& source = base->GetDesc();
		const uint32_t width = box != nullptr ? box->Width() : source.Width;
		const uint32_t height = box != nullptr ? box->Height() : source.Height;

		if (box != nullptr && (!box->IsValid() || box->MaxX > source.Width || box->MaxY > source.Height)) RAWRBOX_CRITICAL("Copy region [{}, {}] -> [{}, {}] is outside the {}x{} source", box->MinX, box->MinY, box->MaxX, box->MaxY, source.Width, source.Height);
		if (width > this->_data.size.x || height > this->_data.size.y) RAWRBOX_CRITICAL("Cannot copy {}x{} pixels into a {}x{} blit texture! Resize it first!", width, height, this->_data.size.x, this->_data.size.y);

		auto* context = rawrbox::RENDERER->context();

		// Copy texture over to staging ---
		Diligent::CopyTextureAttribs copy;
		copy.pSrcTexture = base;
		copy.pDstTexture = this->_tex;
		copy.pSrcBox = box;
		copy.DstTextureTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY;
		copy.SrcTextureTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY;

		const auto state = base->GetState();

		rawrbox::BarrierUtils::barrier({{base, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_COPY_SOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		context->CopyTexture(copy); // This is an async operation, wait for it on the queue
		if (state != Diligent::RESOURCE_STATE_UNKNOWN) rawrbox::BarrierUtils::barrier({{base, Diligent::RESOURCE_STATE_UNKNOWN, state, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		//   --------------

		this->_copies->Enqueue(context, Diligent::RefCntAutoPtr<Diligent::ITexture>{this->_tex});
		this->_pending = true;
		this->_callbacks.push_back(callback);

		rawrbox::BindlessManager::registerUpdateTexture(*this);
	}

	void TextureBLIT::blit(Diligent::Box* box, const std::function<void(const uint8_t*, const uint64_t)>& callback) {
		if (callback == nullptr) RAWRBOX_CRITICAL("Invalid callback");
		auto* context = rawrbox::RENDERER->context();

		Diligent::MappedTextureSubresource MappedSubres;
		context->MapTextureSubresource(this->_tex, 0, 0, Diligent::MAP_READ, Diligent::MAP_FLAG_DO_NOT_WAIT | Diligent::MAP_FLAG_DISCARD, box, MappedSubres);
		this->_mapped = true;

		try {
			callback(MappedSubres.pData == nullptr ? nullptr : std::bit_cast<uint8_t*>(MappedSubres.pData), MappedSubres.Stride);
		} catch (...) {
			context->UnmapTextureSubresource(this->_tex, 0, 0);
			this->_mapped = false;
			throw;
		}

		context->UnmapTextureSubresource(this->_tex, 0, 0);
		this->_mapped = false;
	}

	void TextureBLIT::read(const rawrbox::TextureBase& source, ReadCallback callback, std::optional<Diligent::Box> box) {
		auto* texture = source.getTexture();
		if (texture == nullptr) RAWRBOX_CRITICAL("Source texture '{}' not uploaded", source.getName());

		this->read(*texture, std::move(callback), box);
	}

	void TextureBLIT::read(Diligent::ITexture& source, ReadCallback callback, std::optional<Diligent::Box> box) {
		if (callback == nullptr) RAWRBOX_CRITICAL("Invalid callback");

		const auto& desc = source.GetDesc();
		const auto format = Diligent::GetTextureFormatAttribs(desc.Format);
		if (format.ComponentType == Diligent::COMPONENT_TYPE_COMPRESSED || format.GetElementSize() != 4U)
			RAWRBOX_CRITICAL("Cannot read '{}' back, only 4 byte formats are supported", desc.Name == nullptr ? "?" : desc.Name);

		const Diligent::Box region = box.value_or(Diligent::Box{0, desc.Width, 0, desc.Height});
		if (this->_pending || this->_mapped || !this->_reads.empty()) {
			this->_reads.push_back({.source = Diligent::RefCntAutoPtr<Diligent::ITexture>(&source), .box = region, .callback = std::move(callback)});
			return;
		}

		this->startRead(source, region, std::move(callback));
	}

	bool TextureBLIT::isPending() const { return this->_pending; }

	void TextureBLIT::update() {
		if (!this->_pending || this->_copies->GetFirstCompleted() == nullptr) return;

		const auto callbacks = std::exchange(this->_callbacks, {});
		this->_pending = false;

		for (auto& callback : callbacks) {
			callback();
		}

		if (this->_pending || this->_reads.empty()) return;

		auto next = std::move(this->_reads.front());
		this->_reads.pop_front();

		this->startRead(*next.source, next.box, std::move(next.callback));
	}

	bool TextureBLIT::requiresUpdate() const {
		return this->_pending;
	}
} // namespace rawrbox