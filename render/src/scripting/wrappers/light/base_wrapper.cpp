
#include <rawrbox/render/scripting/wrappers/light/base_wrapper.hpp>

namespace rawrbox {
	LightBaseWrapper::LightBaseWrapper(const std::shared_ptr<rawrbox::LightBase>& ref) : _ref(ref) {}
	LightBaseWrapper::~LightBaseWrapper() { this->_ref.reset(); }

	// UTILS ----
	const rawrbox::Colori LightBaseWrapper::getColor() const {
		if (this->isValid()) throw std::runtime_error("[RawrBox-LightBase] Light reference not set!");
		return this->_ref.lock()->getColor().cast<int>();
	}

	void LightBaseWrapper::setColor(const rawrbox::Colori& col) {
		if (this->isValid()) throw std::runtime_error("[RawrBox-LightBase] Light reference not set!");
		this->_ref.lock()->setColor(col.cast<float>());
	}

	const rawrbox::Vector4f LightBaseWrapper::getData() const {
		if (this->isValid()) throw std::runtime_error("[RawrBox-LightBase] Light reference not set!");
		return this->_ref.lock()->getData();
	}

	void LightBaseWrapper::setRadius(float radius) {
		if (this->isValid()) throw std::runtime_error("[RawrBox-LightBase] Light reference not set!");
		this->_ref.lock()->setRadius(radius);
	}

	float LightBaseWrapper::getRadius() const {
		if (this->isValid()) throw std::runtime_error("[RawrBox-LightBase] Light reference not set!");
		return this->_ref.lock()->getRadius();
	}

	size_t LightBaseWrapper::id() const {
		if (this->isValid()) throw std::runtime_error("[RawrBox-LightBase] Light reference not set!");
		return this->_ref.lock()->id();
	}

	bool LightBaseWrapper::isOn() const {
		if (this->isValid()) throw std::runtime_error("[RawrBox-LightBase] Light reference not set!");
		return this->_ref.lock()->isOn();
	}

	void LightBaseWrapper::setStatus(bool on) {
		if (this->isValid()) throw std::runtime_error("[RawrBox-LightBase] Light reference not set!");
		this->_ref.lock()->setStatus(on);
	}

	const rawrbox::Vector3f& LightBaseWrapper::getPos() const {
		if (this->isValid()) throw std::runtime_error("[RawrBox-LightBase] Light reference not set!");
		return this->_ref.lock()->getPos();
	}

	void LightBaseWrapper::setPos(const rawrbox::Vector3f& pos) {
		if (this->isValid()) throw std::runtime_error("[RawrBox-LightBase] Light reference not set!");
		this->_ref.lock()->setPos(pos);
	}

	const rawrbox::Vector3f& LightBaseWrapper::getOffsetPos() const {
		if (this->isValid()) throw std::runtime_error("[RawrBox-LightBase] Light reference not set!");
		return this->_ref.lock()->getOffsetPos();
	}

	void LightBaseWrapper::setOffsetPos(const rawrbox::Vector3f& pos) {
		if (this->isValid()) throw std::runtime_error("[RawrBox-LightBase] Light reference not set!");
		this->_ref.lock()->setOffsetPos(pos);
	}

	const rawrbox::Vector3f LightBaseWrapper::getWorldPos() const {
		if (this->isValid()) throw std::runtime_error("[RawrBox-LightBase] Light reference not set!");
		return this->_ref.lock()->getWorldPos();
	}

	rawrbox::LightType LightBaseWrapper::getType() const {
		if (this->isValid()) throw std::runtime_error("[RawrBox-LightBase] Light reference not set!");
		return this->_ref.lock()->getType();
	}

	const rawrbox::Vector3f& LightBaseWrapper::getDirection() const {
		if (this->isValid()) throw std::runtime_error("[RawrBox-LightBase] Light reference not set!");
		return this->_ref.lock()->getDirection();
	}

	void LightBaseWrapper::setDirection(const rawrbox::Vector3f& dir) {
		if (this->isValid()) throw std::runtime_error("[RawrBox-LightBase] Light reference not set!");
		this->_ref.lock()->setDirection(dir);
	}
	// ------

	bool LightBaseWrapper::isValid() const {
		return !this->_ref.expired();
	}

	rawrbox::LightBase* LightBaseWrapper::getRef() const {
		return nullptr;
		// return &_ref.lock();
	}

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
