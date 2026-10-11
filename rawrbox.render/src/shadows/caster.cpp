#include <rawrbox/render/shadows/caster.hpp>
#include <rawrbox/render/shadows/manager.hpp>

namespace rawrbox {
	ShadowCaster::~ShadowCaster() {
		if (this->_shadowMode == rawrbox::ShadowMode::NONE) return;
		rawrbox::SHADOWS::remove(*this);
	}

	void ShadowCaster::setShadowMode(rawrbox::ShadowMode mode) {
		if (this->_shadowMode == mode) return;

		const auto prev = this->_shadowMode;
		this->_shadowMode = mode;

		if (mode == rawrbox::ShadowMode::NONE) {
			rawrbox::SHADOWS::remove(*this);
		} else if (prev == rawrbox::ShadowMode::NONE) {
			rawrbox::SHADOWS::add(*this);
		}
	}

	rawrbox::ShadowMode ShadowCaster::getShadowMode() const { return this->_shadowMode; }
} // namespace rawrbox