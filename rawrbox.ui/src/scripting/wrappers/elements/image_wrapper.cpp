#include <rawrbox/ui/elements/image.hpp>
#include <rawrbox/ui/scripting/wrappers/elements/image_wrapper.hpp>
#include <rawrbox/utils/memory.hpp>

namespace rawrbox {
	rawrbox::TextureWrapper ImageWrapper::getTexture() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ImageWrapper] Invalid ui reference");
		return {rawrbox::cast<rawrbox::UIImage>(this->_ref).lock()->getTexture()};
	}

	void ImageWrapper::setTexture(const rawrbox::TextureWrapper& texture) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ImageWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIImage>(this->_ref).lock()->setTexture(texture.getRef());
	}

	void ImageWrapper::setTexture(const std::string& path, sol::this_environment modEnv) {
		if (!modEnv.env.has_value()) throw std::runtime_error("[RawrBox-ImageWrapper] MOD not set!");
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ImageWrapper] Invalid ui reference");

		std::string modFolder = modEnv.env.value()["__mod_folder"];
		rawrbox::cast<rawrbox::UIImage>(this->_ref).lock()->setTexture(rawrbox::LuaUtils::getContent(path, modFolder));
	}

	rawrbox::Colori ImageWrapper::getColor() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ImageWrapper] Invalid ui reference");
		return rawrbox::cast<rawrbox::UIImage>(this->_ref).lock()->getColor().cast<int>();
	}

	void ImageWrapper::setColor(const rawrbox::Colori& col) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ImageWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIImage>(this->_ref).lock()->setColor(col.cast<float>());
	}

	void ImageWrapper::sizeToContents() {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ImageWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIImage>(this->_ref).lock()->sizeToContents();
	}

	void ImageWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<ImageWrapper>("UIImage",
		    sol::no_constructor,

		    "getTexture", &ImageWrapper::getTexture,
		    "setTexture", sol::overload(sol::resolve<void(const std::string&, sol::this_environment)>(&ImageWrapper::setTexture), sol::resolve<void(const rawrbox::TextureWrapper&)>(&ImageWrapper::setTexture)),

		    "getColor", &ImageWrapper::getColor,
		    "setColor", &ImageWrapper::setColor,

		    "sizeToContents", &ImageWrapper::sizeToContents,
		    sol::base_classes, sol::bases<rawrbox::UIContainerWrapper>());
	}
} // namespace rawrbox
