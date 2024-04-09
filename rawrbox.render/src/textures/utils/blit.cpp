#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/textures/utils/blit.hpp>

namespace rawrbox {
	TextureBLIT::TextureBLIT(const rawrbox::Vector2u& size) {
		this->_size = size;
		this->_name = "RawrBox::GPU::Blit";
	}

	TextureBLIT::~TextureBLIT() {
		RAWRBOX_DESTROY(this->_copyFence);
		this->_callbacks.clear();
	}

	void TextureBLIT::upload(Diligent::TEXTURE_FORMAT /*format*/, bool /*dynamic*/) {
		if (this->_tex != nullptr) throw _logger->error("Already uploaded");

		Diligent::TextureDesc desc;
		desc.Type = Diligent::RESOURCE_DIM_TEX_2D;
		desc.Width = this->_size.x;
		desc.Height = this->_size.y;
		desc.MipLevels = 1;
		desc.Usage = Diligent::USAGE_STAGING;
		desc.BindFlags = Diligent::BIND_NONE;
		desc.CPUAccessFlags = Diligent::CPU_ACCESS_READ;
		desc.Format = Diligent::TEX_FORMAT_RGBA8_UNORM;
		desc.Name = this->_name.c_str();

		rawrbox::RENDERER->device()->CreateTexture(desc, nullptr, &this->_tex);
	}

	void TextureBLIT::copy(Diligent::ITexture* base, Diligent::Box* box, const std::function<void()>& callback) {
		if (base == nullptr) throw _logger->error("Invalid texture");
		if (callback == nullptr) throw _logger->error("Invalid callback");

		auto* device = rawrbox::RENDERER->device();
		auto* context = rawrbox::RENDERER->context();

		if (this->_copyFence == nullptr) { // Create a fence only if the frame wasn't already requested already
			// Create fence ----
			Diligent::FenceDesc desc;
			desc.Type = Diligent::FENCE_TYPE_GENERAL;
			desc.Name = "Rawrbox::Fence::Copy";

			device->CreateFence(desc, &this->_copyFence);
			// -----------------

			// Copy texture over to staging ---
			Diligent::CopyTextureAttribs copy;
			copy.pSrcTexture = base;
			copy.pDstTexture = this->_tex;
			copy.pSrcBox = box;
			copy.DstTextureTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY;
			copy.SrcTextureTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY;

			rawrbox::BarrierUtils::barrier({{copy.pSrcTexture, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_COPY_SOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
			context->CopyTexture(copy); // This is an async operation, enqueue the fence
			//   --------------

			context->EnqueueSignal(this->_copyFence, 1);
			rawrbox::BindlessManager::registerUpdateTexture(*this);
		}

		this->_callbacks.push_back(callback);
	}

	void TextureBLIT::blit(Diligent::Box* box, const std::function<void(const uint8_t*, const uint64_t)>& callback) {
		if (callback == nullptr) throw _logger->error("Invalid callback");
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
		if (this->_copyFence == nullptr || this->_copyFence->GetCompletedValue() <= 0) return;
		for (auto& callback : this->_callbacks) {
			callback();
		}

		this->_callbacks.clear();
		RAWRBOX_DESTROY(this->_copyFence);
	}

	bool TextureBLIT::requiresUpdate() const {
		return this->_copyFence != nullptr;
	}
} // namespace rawrbox
