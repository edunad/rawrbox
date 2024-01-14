#include <rawrbox/render/plugins/post_process.hpp>
#include <rawrbox/render/static.hpp>

namespace rawrbox {
	PostProcessPlugin::~PostProcessPlugin() {
		this->_postProcesses.clear();
	}

	// Post utils ----
	void PostProcessPlugin::remove(size_t indx) {
		if (this->_postProcesses.empty() || indx >= this->_postProcesses.size()) throw this->_logger->error("Failed to remove {}!", indx);
		this->_postProcesses.erase(this->_postProcesses.begin() + indx);
	}

	rawrbox::PostProcessBase& PostProcessPlugin::get(size_t indx) const {
		if (indx >= this->_postProcesses.size()) throw this->_logger->error("Failed to get {}!", indx);
		return *this->_postProcesses[indx];
	}

	size_t PostProcessPlugin::count() {
		return this->_postProcesses.size();
	}
	// ----

	const std::string PostProcessPlugin::getID() const { return "PostProcess"; }

	void PostProcessPlugin::initialize(const rawrbox::Vector2i& size) {
		this->_rt = std::make_unique<rawrbox::TextureRender>(size, false);
		this->_rt->upload(Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB);

		for (const auto& _postProcess : this->_postProcesses) {
			_postProcess->upload();
		}
	}

	void PostProcessPlugin::resize(const rawrbox::Vector2i& /*size*/) {
		// TODO: RESIZE RENDER TEXTURE?
	}

	void PostProcessPlugin::postRender(const rawrbox::TextureRender& renderTexture) {
		if (this->_postProcesses.empty()) return;
		rawrbox::BindlessManager::barrier(renderTexture);

		for (const auto& _postProcesse : this->_postProcesses) {
			this->_rt->startRecord(false);
			_postProcesse->applyEffect(renderTexture);
			this->_rt->stopRecord();

			// Copy texture over
			Diligent::CopyTextureAttribs CopyAttribs;
			CopyAttribs.pSrcTexture = this->_rt->getTexture();
			CopyAttribs.pDstTexture = renderTexture.getTexture();

			rawrbox::RENDERER->context()->CopyTexture(CopyAttribs);
			// -----------------
		}

		rawrbox::BindlessManager::barrier(*this->_rt);
	}

} // namespace rawrbox
