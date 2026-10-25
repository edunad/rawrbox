
#include <rawrbox/render/post_process/skybox.hpp>

#include <bit>

namespace rawrbox {
	PostProcessSkybox::PostProcessSkybox(const rawrbox::Colorf& start, const rawrbox::Colorf& end) {
		this->setStartColor(start);
		this->setEndColor(end);

		this->setPSXBanding(64);
	}

	void PostProcessSkybox::setStartColor(const rawrbox::Colorf& col) { this->_data[0].x = std::bit_cast<float>(col.pack()); }
	void PostProcessSkybox::setEndColor(const rawrbox::Colorf& col) { this->_data[0].y = std::bit_cast<float>(col.pack()); }
	void PostProcessSkybox::setPSXBanding(uint32_t banding) { this->_data[0].z = static_cast<float>(banding); }

	void PostProcessSkybox::init() {
		rawrbox::PostProcessBase::init();

		this->_pipeline = createPipeline("PostProcess::Skybox", "post_process/skybox.psh");
	}
} // namespace rawrbox