
#include <rawrbox/bass/manager.hpp>
#include <rawrbox/bass/resources/sound.hpp>
#include <rawrbox/render/cameras/orbital.hpp>
#include <rawrbox/render/models/utils/mesh.hpp>
#include <rawrbox/render/resources/font.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/utils/threading.hpp>

#include <bass_test/game.hpp>

namespace bass_test {

	void Game::setupGLFW() {
		auto window = rawrbox::Window::createWindow();
		window->setMonitor(-1);
		window->setTitle("BASS AUDIO TEST");
		window->init(1024, 768, rawrbox::WindowFlags::Window::WINDOWED);
		window->onWindowClose += [this](auto& /*w*/) { this->shutdown(); };
	}

	void Game::init() {
		auto window = rawrbox::Window::getWindow();

		// Setup renderer
		auto render = window->createRenderer();
		render->onIntroCompleted = [this]() { this->loadContent(); };
		render->setDrawCall([this](const rawrbox::DrawPass& pass) {
			if (pass != rawrbox::DrawPass::PASS_OPAQUE) return;
			this->drawWorld();
		});
		// ---------------

		// Setup camera
		auto cam = render->setupCamera<rawrbox::CameraOrbital>(*window);
		cam->setPos({0.F, 5.F, -5.F});
		cam->setAngle({0.F, rawrbox::MathUtils::toRad(-45), 0.F, 0.F});
		// --------------

		// Add loaders
		rawrbox::RESOURCES::addLoader<rawrbox::FontLoader>();
		rawrbox::RESOURCES::addLoader<rawrbox::BASSLoader>();
		// --------------

		render->init();
	}

	void Game::loadContent() {
		std::array initialContentFiles = {
		    std::make_pair<std::string, uint32_t>("./assets/sounds/clownmusic.ogg", 0 | rawrbox::SoundFlags::SOUND_3D)};

		this->_loadingFiles = static_cast<int>(initialContentFiles.size());
		for (auto& f : initialContentFiles) {
			rawrbox::RESOURCES::loadFileAsync(f.first, f.second, [this]() {
				this->_loadingFiles--;
				if (this->_loadingFiles <= 0) {
					rawrbox::runOnRenderThread([this]() { this->contentLoaded(); });
				}
			});
		}
	}

	void Game::contentLoaded() {
		// SETUP SOUNDS -------------------
		// SOUND -----
		// https://i.rawr.dev/Mystery%20Skulls%20-%20Freaking%20Out.mp3
		// https://i.rawr.dev/Just_a_Bit_Crazy.ogg
		this->_sound = rawrbox::BASS::loadHTTPSound("https://i.rawr.dev/Just_a_Bit_Crazy.ogg", rawrbox::SoundFlags::SOUND_3D | rawrbox::SoundFlags::BEAT_DETECTION | rawrbox::SoundFlags::BPM_DETECTION)->createInstance();
		this->_sound.lock()->setVolume(1.F);
		this->_sound.lock()->setLooping(true);
		this->_sound.lock()->set3D(10.F);
		this->_sound.lock()->setPosition({-3.F, 1.F, 0});
		this->_sound.lock()->setTempo(0.8F);
		this->_sound.lock()->play();

		this->_sound.lock()->onBEAT += [this](double) {
			this->_beat = 0.5F;
		};

		this->_sound.lock()->onBPM += [](float bpm) {
			fmt::print("BPM: {}\n", bpm);
		};

		this->_sound2 = rawrbox::BASS::loadSound("./assets/sounds/clownmusic.ogg")->createInstance();
		this->_sound2.lock()->setLooping(true);
		this->_sound2.lock()->set3D(10.F);
		this->_sound2.lock()->setPosition({3.F, 1.F, 0});
		this->_sound2.lock()->setTempo(1.2F);
		this->_sound2.lock()->play();
		// --------------------------------

		// SETUP MODELS -------------------
		{
			auto mesh = rawrbox::MeshUtils::generateGrid(24, {0.F, 0.F, 0.F});
			this->_modelGrid->addMesh(mesh);
			this->_modelGrid->upload();
		}
		// --------------------------------

		// Text test ----
		{
			this->_beatText->addText(*rawrbox::DEBUG_FONT_REGULAR, "BEAT", {-3.F, 1.3F, 0});
			this->_beatText->upload();
		}

		{
			this->_text->addText(*rawrbox::DEBUG_FONT_REGULAR, "HTTP LOADING", {-3.F, 1.1F, 0});
			this->_text->addText(*rawrbox::DEBUG_FONT_REGULAR, "LOCAL LOADING", {3.F, 1.1F, 0});
			this->_text->upload();
		}
		// ------

		this->_ready = true;
	}

	void Game::onThreadShutdown(rawrbox::ENGINE_THREADS thread) {
		if (thread == rawrbox::ENGINE_THREADS::THREAD_INPUT) {
			rawrbox::Window::shutdown();
		} else {
			rawrbox::BASS::shutdown();
			rawrbox::RESOURCES::shutdown();
			rawrbox::ASYNC::shutdown();
		}

		this->_beatText.reset();
		this->_modelGrid.reset();
		this->_text.reset();
	}

	void Game::pollEvents() {
		rawrbox::Window::pollEvents();
	}

	void Game::update() {
		rawrbox::Window::update();

		auto cam = rawrbox::MAIN_CAMERA;
		rawrbox::BASS::setListenerLocation(cam->getPos(), cam->getForward(), cam->getUp());

		this->_beat = std::max(this->_beat - 0.05F, 0.F);
		this->_beatText->setPos({0, this->_beat, 0});
	}

	void Game::drawWorld() {
		if (!this->_ready) return;

		this->_modelGrid->draw();
		this->_beatText->draw();
		this->_text->draw();
	}

	void Game::draw() {
		rawrbox::Window::render(); // Commit primitives
	}
} // namespace bass_test
