#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/textures/utils/blit.hpp>

namespace rawrbox {
	TextureBLIT::TextureBLIT(const rawrbox::Vector2u& size) {
		this->_data.size = size;
		this->_data.channels = 4;

		this->_name = "RawrBox::GPU::Blit";
	}

	TextureBLIT::~TextureBLIT() {
		this->_copies.reset();
		this->_callbacks.clear();
	}

	void TextureBLIT::upload(Diligent::TEXTURE_FORMAT /*format*/, uint32_t /*flags*/) {
		if (this->_tex != nullptr) RAWRBOX_CRITICAL("Already uploaded");

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
		this->_copies = std::make_unique<Diligent::GPUCompletionAwaitQueue<Diligent::RefCntAutoPtr<Diligent::ITexture>>>(rawrbox::RENDERER->device());
	}

	void TextureBLIT::copy(Diligent::ITexture* base, Diligent::Box* box, const std::function<void()>& callback) {
		if (base == nullptr) RAWRBOX_CRITICAL("Invalid texture");
		if (callback == nullptr) RAWRBOX_CRITICAL("Invalid callback");

		auto* context = rawrbox::RENDERER->context();

		if (!this->_pending) {
			// Copy texture over to staging ---
			Diligent::CopyTextureAttribs copy;
			copy.pSrcTexture = base;
			copy.pDstTexture = this->_tex;
			copy.pSrcBox = box;
			copy.DstTextureTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY;
			copy.SrcTextureTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY;

			rawrbox::BarrierUtils::barrier({{copy.pSrcTexture, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_COPY_SOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
			context->CopyTexture(copy); // This is an async operation, wait for it on the queue
			//   --------------

			this->_copies->Enqueue(context, Diligent::RefCntAutoPtr<Diligent::ITexture>{this->_tex});
			this->_pending = true;

			rawrbox::BindlessManager::registerUpdateTexture(*this);
		}

		this->_callbacks.push_back(callback);
	}

	void TextureBLIT::blit(Diligent::Box* box, const std::function<void(const uint8_t*, const uint64_t)>& callback) {
		if (callback == nullptr) RAWRBOX_CRITICAL("Invalid callback");
		auto* context = rawrbox::RENDERER->context();

		Diligent::MappedTextureSubresource MappedSubres;
		context->MapTextureSubresource(this->_tex, 0, 0, Diligent::MAP_READ, Diligent::MAP_FLAG_DO_NOT_WAIT | Diligent::MAP_FLAG_DISCARD, box, MappedSubres);
		try { // Slap a try and catch since we need to ensure that the resource is unmapped
			callback(MappedSubres.pData == nullptr ? nullptr : std::bit_cast<uint8_t*>(MappedSubres.pData), MappedSubres.Stride);
		} catch (...) {
		}
		context->UnmapTextureSubresource(this->_tex, 0, 0);
	}

	void TextureBLIT::update() {
		if (!this->_pending || this->_copies->GetFirstCompleted() == nullptr) return;
		for (auto& callback : this->_callbacks) {
			callback();
		}

		this->_callbacks.clear();
		this->_pending = false;
	}

	bool TextureBLIT::requiresUpdate() const {
		return this->_pending;
	}
} // namespace rawrbox