
#include <rawrbox/render/model/base.hpp>
#include <rawrbox/render/model/light/manager.h>
#include <rawrbox/render/model/material/sprite_unlit.hpp>
#include <rawrbox/render/model/material/unlit.hpp>

#include <fmt/format.h>

namespace rawrBox {
	void LightManager::init(int32_t maxLights) {
		this->maxLights = maxLights;

		this->_pointTexture = std::make_shared<rawrBox::TextureImage>("./content/textures/debug/point.png");
		this->_spotTexture = std::make_shared<rawrBox::TextureImage>("./content/textures/debug/spot.png");
		this->_dirTexture = std::make_shared<rawrBox::TextureImage>("./content/textures/debug/dir.png");

		this->_debugMdl = std::make_shared<rawrBox::Sprite>(std::make_shared<rawrBox::MaterialSpriteUnlit>());
	}

	void LightManager::destroy() {
		this->_lights.clear();

		this->_debugMdl = nullptr;
		this->_pointTexture = nullptr;
		this->_spotTexture = nullptr;
		this->_dirTexture = nullptr;
	}

	void LightManager::setEnabled(bool fb) { this->fullbright = fb; }

	// Light utils ----
	void LightManager::addLight(std::shared_ptr<rawrBox::LightBase> light) {
		if (light == nullptr || this->_lights.size() >= this->maxLights) return;

		// Add mesh --
		if (this->_debugMdl != nullptr) {
			auto pos = light->getPosMatrix();

			auto mesh = rawrBox::ModelBase::generatePlane({pos[0], pos[1], pos[2]}, 0.1f);
			mesh->setName(fmt::format("Light-{}", light->getType()));
			// mesh->setColor(light->getDiffuseColor());

			switch (light->getType()) {
				case LightType::LIGHT_SPOT:
					mesh->setTexture(this->_spotTexture);
					break;
				case LightType::LIGHT_DIR:
					mesh->setTexture(this->_dirTexture);
					break;
				default:
				case LightType::LIGHT_POINT:
					mesh->setTexture(this->_pointTexture);
					break;
			}

			this->_debugMdl->addMesh(mesh);
		}

		this->_lights.push_back(std::move(light));
	}

	void LightManager::removeLight(std::shared_ptr<rawrBox::LightBase> light) {
		if (this->_lights.empty()) return;

		for (size_t i = 0; i < this->_lights.size(); i++) {
			if (this->_lights[i] == light) {
				this->_lights.erase(this->_lights.begin() + i);
				this->_debugMdl->removeMesh(i);
				return;
			}
		}
	}

	std::shared_ptr<rawrBox::LightBase> LightManager::getLight(size_t indx) {
		if (indx < 0 || indx >= this->_lights.size()) return nullptr;
		return this->_lights[indx];
	}

	size_t LightManager::count() {
		return this->_lights.size();
	}
	// ---------

	// DEBUG -----
	void LightManager::uploadDebug() {
		this->_pointTexture->upload();
		this->_dirTexture->upload();
		this->_spotTexture->upload();

		this->_debugMdl->upload();
	}

	void LightManager::drawDebug(const rawrBox::Vector3f& camPos) {
		if (this->_debugMdl == nullptr) return;
		this->_debugMdl->draw(camPos);
	}
	// ------

} // namespace rawrBox
