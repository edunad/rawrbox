
#include <rawrbox/render/scripting/wrappers/light/base_wrapper.hpp>

namespace rawrbox {
	LightBaseWrapper::LightBaseWrapper(rawrbox::LightBase* ref) : _ref(ref->getReference()) {}
	LightBaseWrapper::~LightBaseWrapper() { this->_ref.reset(); }

	// UTILS ----
	const rawrbox::Colori LightBaseWrapper::getColor() const {
		if (this->_ref.valid()) throw std::runtime_error("[RawrBox-LightBase] Light reference not set!");
		return this->_ref.get().getColor().cast<int>();
	}

	void LightBaseWrapper::setColor(const rawrbox::Colori& col) {
		if (this->_ref.valid()) throw std::runtime_error("[RawrBox-LightBase] Light reference not set!");
		this->_ref.get().setColor(col.cast<float>());
	}

	const rawrbox::Vector4f LightBaseWrapper::getData() const {
		if (this->_ref.valid()) throw std::runtime_error("[RawrBox-LightBase] Light reference not set!");
		return this->_ref.get().getData();
	}

	void LightBaseWrapper::setRadius(float radius) {
		if (this->_ref.valid()) throw std::runtime_error("[RawrBox-LightBase] Light reference not set!");
		this->_ref.get().setRadius(radius);
	}

	float LightBaseWrapper::getRadius() const {
		if (this->_ref.valid()) throw std::runtime_error("[RawrBox-LightBase] Light reference not set!");
		return this->_ref.get().getRadius();
	}

	size_t LightBaseWrapper::id() const {
		if (this->_ref.valid()) throw std::runtime_error("[RawrBox-LightBase] Light reference not set!");
		return this->_ref.get().id();
	}

	bool LightBaseWrapper::isOn() const {
		if (this->_ref.valid()) throw std::runtime_error("[RawrBox-LightBase] Light reference not set!");
		return this->_ref.get().isOn();
	}

	void LightBaseWrapper::setStatus(bool on) {
		if (this->_ref.valid()) throw std::runtime_error("[RawrBox-LightBase] Light reference not set!");
		this->_ref.get().setStatus(on);
	}

	const rawrbox::Vector3f& LightBaseWrapper::getPos() const {
		if (this->_ref.valid()) throw std::runtime_error("[RawrBox-LightBase] Light reference not set!");
		return this->_ref.get().getPos();
	}

	void LightBaseWrapper::setPos(const rawrbox::Vector3f& pos) {
		if (this->_ref.valid()) throw std::runtime_error("[RawrBox-LightBase] Light reference not set!");
		this->_ref.get().setPos(pos);
	}

	const rawrbox::Vector3f& LightBaseWrapper::getOffsetPos() const {
		if (this->_ref.valid()) throw std::runtime_error("[RawrBox-LightBase] Light reference not set!");
		return this->_ref.get().getOffsetPos();
	}

	void LightBaseWrapper::setOffsetPos(const rawrbox::Vector3f& pos) {
		if (this->_ref.valid()) throw std::runtime_error("[RawrBox-LightBase] Light reference not set!");
		this->_ref.get().setOffsetPos(pos);
	}

	const rawrbox::Vector3f LightBaseWrapper::getWorldPos() const {
		if (this->_ref.valid()) throw std::runtime_error("[RawrBox-LightBase] Light reference not set!");
		return this->_ref.get().getWorldPos();
	}

	rawrbox::LightType LightBaseWrapper::getType() const {
		if (this->_ref.valid()) throw std::runtime_error("[RawrBox-LightBase] Light reference not set!");
		return this->_ref.get().getType();
	}

	const rawrbox::Vector3f& LightBaseWrapper::getDirection() const {
		if (this->_ref.valid()) throw std::runtime_error("[RawrBox-LightBase] Light reference not set!");
		return this->_ref.get().getDirection();
	}

	void LightBaseWrapper::setDirection(const rawrbox::Vector3f& dir) {
		if (this->_ref.valid()) throw std::runtime_error("[RawrBox-LightBase] Light reference not set!");
		this->_ref.get().setDirection(dir);
	}
	// ------

	bool LightBaseWrapper::isValid() const {
		return this->_ref.valid();
	}

	rawrbox::LightBase* LightBaseWrapper::getRef() const { return &_ref.get(); }

	void LightBaseWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<LightBaseWrapper>("LightBase",
		    sol::no_constructor,

		    // UTILS ----
		    "getColor", &LightBaseWrapper::getColor,
		    "setColor", &LightBaseWrapper::setPos,

		    "getData", &LightBaseWrapper::getData,

		    "setRadius", &LightBaseWrapper::setRadius,
		    "getRadius", &LightBaseWrapper::getRadius,

		    "id", &LightBaseWrapper::id,

		    "isOn", &LightBaseWrapper::isOn,
		    "setStatus", &LightBaseWrapper::setStatus,

		    "getPos", &LightBaseWrapper::getPos,
		    "setPos", &LightBaseWrapper::setPos,

		    "getOffsetPos", &LightBaseWrapper::getOffsetPos,
		    "setOffsetPos", &LightBaseWrapper::setOffsetPos,

		    "getWorldPos", &LightBaseWrapper::getWorldPos,
		    "getType", &LightBaseWrapper::getType,

		    "getDirection", &LightBaseWrapper::getDirection,
		    "setDirection", &LightBaseWrapper::setDirection,
		    // --------------

		    "isValid", &LightBaseWrapper::isValid);
	}
} // namespace rawrbox
