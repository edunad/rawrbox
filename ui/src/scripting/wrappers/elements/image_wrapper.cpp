#include <rawrbox/ui/elements/image.hpp>
#include <rawrbox/ui/scripting/wrappers/elements/image_wrapper.hpp>

namespace rawrbox {
	ImageWrapper::ImageWrapper(const std::shared_ptr<rawrbox::UIContainer>& element, rawrbox::Mod* mod) : rawrbox::UIContainerWrapper(element), _mod(mod) {}

	rawrbox::TextureWrapper ImageWrapper::getTexture() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ImageWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIImage> ptr = std::dynamic_pointer_cast<rawrbox::UIImage>(this->_ref.lock());

		return {ptr.lock()->getTexture()};
	}

	void ImageWrapper::setTexture(const rawrbox::TextureWrapper& texture) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ImageWrapper] Invalid ui reference");

		std::weak_ptr<rawrbox::UIImage> ptr = std::dynamic_pointer_cast<rawrbox::UIImage>(this->_ref.lock());
		ptr.lock()->setTexture(texture.getRef());
	}

	void ImageWrapper::setTexture(const std::string& path) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ImageWrapper] Invalid ui reference");

		std::weak_ptr<rawrbox::UIImage> ptr = std::dynamic_pointer_cast<rawrbox::UIImage>(this->_ref.lock());
		ptr.lock()->setTexture(rawrbox::LuaUtils::getContent(path, this->_mod->getFolder()));
	}

	rawrbox::Colori ImageWrapper::getColor() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ImageWrapper] Invalid ui reference");

		std::weak_ptr<rawrbox::UIImage> ptr = std::dynamic_pointer_cast<rawrbox::UIImage>(this->_ref.lock());
		return ptr.lock()->getColor().cast<int>();
	}

	void ImageWrapper::setColor(const rawrbox::Colori& col) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ImageWrapper] Invalid ui reference");

		std::weak_ptr<rawrbox::UIImage> ptr = std::dynamic_pointer_cast<rawrbox::UIImage>(this->_ref.lock());
		ptr.lock()->setColor(col.cast<float>());
	}

	void ImageWrapper::sizeToContents() {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ImageWrapper] Invalid ui reference");

		std::weak_ptr<rawrbox::UIImage> ptr = std::dynamic_pointer_cast<rawrbox::UIImage>(this->_ref.lock());
		ptr.lock()->sizeToContents();
	}

	void ImageWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<ImageWrapper>("UIImage",
		    sol::no_constructor,

		    "getTexture", &ImageWrapper::getTexture,
		    "setTexture", sol::overload(sol::resolve<void(const std::string&)>(&ImageWrapper::setTexture), sol::resolve<void(const rawrbox::TextureWrapper&)>(&ImageWrapper::setTexture)),

		    "getColor", &ImageWrapper::getColor,
		    "setColor", &ImageWrapper::setColor,

		    "sizeToContents", &ImageWrapper::sizeToContents,
		    sol::base_classes, sol::bases<rawrbox::UIContainerWrapper>());
	}
} // namespace rawrbox
