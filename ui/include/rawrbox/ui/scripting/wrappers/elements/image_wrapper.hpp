#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/render/scripting/wrappers/textures/base_wrapper.hpp>
#include <rawrbox/scripting/mod.hpp>
#include <rawrbox/ui/scripting/wrappers/ui_container_wrapper.hpp>

#include <sol/sol.hpp>

namespace rawrbox {
	class ImageWrapper : public rawrbox::UIContainerWrapper {
	protected:
		rawrbox::Mod* _mod = nullptr;

	public:
		ImageWrapper(const std::shared_ptr<rawrbox::UIContainer>& element, rawrbox::Mod* mod);

		[[nodiscard]] virtual rawrbox::TextureWrapper getTexture() const;
		virtual void setTexture(const rawrbox::TextureWrapper& texture);
		virtual void setTexture(const std::string& path);

		[[nodiscard]] virtual rawrbox::Colori getColor() const;
		virtual void setColor(const rawrbox::Colori& col);

		void sizeToContents();

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
