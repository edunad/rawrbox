
#include <rawrbox/render/post_process/manager.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/render.hpp>

#include <fmt/format.h>

#include <stdexcept>

namespace rawrbox {

	PostProcessManager::~PostProcessManager() {
		this->_postProcesses.clear();
	}

	// Post utils ----
	void PostProcessManager::remove(size_t indx) {
		if (this->_postProcesses.empty() || indx >= this->_postProcesses.size()) throw std::runtime_error(fmt::format("[RawrBox-PostProcess] Failed to remove {}!", indx));
		this->_postProcesses.erase(this->_postProcesses.begin() + indx);
	}

	rawrbox::PostProcessBase& PostProcessManager::get(size_t indx) const {
		if (indx >= this->_postProcesses.size()) throw std::runtime_error(fmt::format("[RawrBox-PostProcess] Failed to get {}!", indx));
		return *this->_postProcesses[indx];
	}

	size_t PostProcessManager::count() {
		return this->_postProcesses.size();
	}
	// ----

	void PostProcessManager::upload() {
		auto size = rawrbox::RENDERER->getSize();

		this->_rt = std::make_unique<rawrbox::TextureRender>(size, false);
		this->_rt->upload(Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB);

		Diligent::TextureDesc desc;
		desc.Type = Diligent::RESOURCE_DIM_TEX_2D;
		desc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
		desc.Width = size.x;
		desc.Height = size.y;
		desc.MipLevels = 1;
		desc.Format = Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB;
		desc.Name = "RawrBox::PostProcess::CPY";
		desc.ClearValue.Format = desc.Format;
		desc.ClearValue.Color[0] = 0.F;
		desc.ClearValue.Color[1] = 0.F;
		desc.ClearValue.Color[2] = 0.F;
		desc.ClearValue.Color[3] = 0.F;

		rawrbox::RENDERER->device()->CreateTexture(desc, nullptr, &this->_cpRT);
		this->_cpRTView = this->_cpRT->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);

		for (const auto& _postProcess : this->_postProcesses) {
			_postProcess->upload();
		}
	}

	void PostProcessManager::render(Diligent::ITextureView* renderTexture) {
		if (this->_postProcesses.empty()) {
			rawrbox::RenderUtils::renderQUAD(renderTexture);
			return;
		}

		for (size_t i = 0; i < this->_postProcesses.size(); i++) {
			auto handle = i == 0 ? renderTexture : this->_cpRTView;

			this->_rt->startRecord(false);
			this->_postProcesses[i]->applyEffect(handle);
			this->_rt->stopRecord();

			// Copy texture over
			Diligent::CopyTextureAttribs CopyAttribs{this->_rt->getTexture(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
			    this->_cpRT, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION};
			rawrbox::RENDERER->context()->CopyTexture(CopyAttribs);
		}

		rawrbox::RenderUtils::renderQUAD(this->_cpRTView);
	}

} // namespace rawrbox
