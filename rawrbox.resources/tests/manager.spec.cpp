#include <rawrbox/resources/loaders/json.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/utils/threading.hpp>

#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <chrono>
#include <future>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace rawrbox {
	static std::atomic<int> slowLoads = 0;

	class ResourceSlow : public rawrbox::Resource {
	public:
		bool load(const std::vector<uint8_t>& /*buffer*/) override {
			++slowLoads;
			std::this_thread::sleep_for(std::chrono::milliseconds(50));

			const auto path = this->filePath.generic_string();
			if (path.find("recursive") != std::string::npos) static_cast<void>(rawrbox::RESOURCES::loadFile<ResourceSlow>(this->filePath));

			return path.find("broken") == std::string::npos;
		}
	};

	class SlowLoader : public rawrbox::Loader {
	public:
		std::unique_ptr<rawrbox::Resource> createEntry() override { return std::make_unique<ResourceSlow>(); }
		bool canLoad(const std::string& fileExtention) override { return fileExtention == ".slow"; }
		bool supportsBuffer(const std::string& /*fileExtention*/) override { return false; }
	};

	class SlowLoaderTest : public rawrbox::RESOURCES {
	public:
		SlowLoaderTest() { rawrbox::RESOURCES::addLoader<SlowLoader>(); }
		SlowLoaderTest(const SlowLoaderTest&) = delete;
		SlowLoaderTest(SlowLoaderTest&&) = delete;
		SlowLoaderTest& operator=(const SlowLoaderTest&) = delete;
		SlowLoaderTest& operator=(SlowLoaderTest&&) = delete;
		~SlowLoaderTest() {
			std::erase_if(this->_loaders, [](const auto& loader) { return dynamic_cast<SlowLoader*>(loader.get()) != nullptr; });
		}
	};

	static std::vector<std::future<rawrbox::ResourceSlow*>> loadFromThreads(const std::string& path, size_t threads) {
		std::vector<std::future<rawrbox::ResourceSlow*>> jobs = {};
		for (size_t i = 0; i < threads; ++i) {
			jobs.push_back(std::async(std::launch::async, [path]() { return rawrbox::RESOURCES::loadFile<rawrbox::ResourceSlow>(path); }));
		}

		return jobs;
	}
} // namespace rawrbox

TEST_CASE("RESOURCES should behave as expected", "[rawrbox::RESOURCES]") {
	SECTION("rawrbox::RESOURCES::getLoaders") {
		REQUIRE(rawrbox::RESOURCES::getLoaders().size() == 1);
	}

	SECTION("rawrbox::RESOURCES::addLoader") {
		REQUIRE(rawrbox::RESOURCES::getLoaders().size() == 1);
		rawrbox::RESOURCES::addLoader<rawrbox::JSONLoader>();
		REQUIRE(rawrbox::RESOURCES::getLoaders().size() == 2);
	}

	SECTION("rawrbox::RESOURCES::preLoadFile") {
		const auto& ld = rawrbox::RESOURCES::getLoaders().back();

		REQUIRE(ld->getPreload().empty());
		rawrbox::RESOURCES::preLoadFile("./assets/json/test.json");
		REQUIRE(ld->getPreload().size() == 1);
	}
}

TEST_CASE("RESOURCES should share loads between threads", "[rawrbox::RESOURCES]") {
	static std::once_flag setup;
	std::call_once(setup, []() { rawrbox::ASYNC::init(4); });

	const rawrbox::SlowLoaderTest slow;

	SECTION("rawrbox::RESOURCES::loadFile waits for the thread") {
		rawrbox::slowLoads = 0;

		auto jobs = rawrbox::loadFromThreads("./shared.slow", 8);
		std::vector<rawrbox::ResourceSlow*> results = {};
		for (auto& job : jobs) {
			results.push_back(job.get());
		}

		REQUIRE(rawrbox::slowLoads == 1);
		REQUIRE(results.front() != nullptr);

		for (auto* result : results) {
			REQUIRE(result == results.front());
			REQUIRE(result->status == rawrbox::LoadStatus::LOADED);
		}

		REQUIRE(rawrbox::RESOURCES::getFile<rawrbox::ResourceSlow>("./SHARED.slow") == results.front());
	}

	SECTION("rawrbox::RESOURCES::loadFile handles failures") {
		auto jobs = rawrbox::loadFromThreads("./broken.slow", 4);
		for (auto& job : jobs) {
			REQUIRE_THROWS(job.get());
		}

		REQUIRE_FALSE(rawrbox::RESOURCES::isLoaded("./broken.slow"));
		REQUIRE_THROWS(rawrbox::RESOURCES::loadFile<rawrbox::ResourceSlow>("./broken.slow"));
	}

	SECTION("rawrbox::RESOURCES::loadFile catches recursive loads") {
		REQUIRE_THROWS(rawrbox::RESOURCES::loadFile<rawrbox::ResourceSlow>("./recursive.slow"));
		REQUIRE_FALSE(rawrbox::RESOURCES::isLoaded("./recursive.slow"));
	}

	SECTION("rawrbox::RESOURCES::startPreLoadQueueAsync") {
		for (const auto& loader : rawrbox::RESOURCES::getLoaders()) {
			static_cast<void>(loader->processPreload());
		}

		rawrbox::RESOURCES::preLoadFile("./a.slow");
		rawrbox::RESOURCES::preLoadFile("./A.slow");
		rawrbox::RESOURCES::preLoadFile("./sub/../a.slow", 4U);
		rawrbox::RESOURCES::preLoadFile("./b.slow");
		rawrbox::RESOURCES::preLoadFile("./c.slow");
		REQUIRE(rawrbox::RESOURCES::getTotalPreload() == 3);

		std::atomic<int> ended = 0;
		std::atomic<uint32_t> flags = 0;
		std::atomic<int> completed = 0;

		std::promise<void> done;
		auto finished = done.get_future();

		const auto onEnd = [&ended, &flags](const std::string& /*path*/, uint32_t loadFlags) {
			flags |= loadFlags;
			++ended;
		};

		const auto onComplete = [&completed, &done]() {
			++completed;
			done.set_value();
		};

		rawrbox::RESOURCES::startPreLoadQueueAsync(nullptr, onEnd, onComplete);

		REQUIRE(rawrbox::RESOURCES::getTotalPreload() == 0);
		REQUIRE(finished.wait_for(std::chrono::seconds(10)) == std::future_status::ready);

		REQUIRE(completed == 1);
		REQUIRE(ended == 3);
		REQUIRE(flags == 0U);

		const auto progress = rawrbox::RESOURCES::getPreloadProgress();

		REQUIRE(progress.loaded == 3);
		REQUIRE(progress.total == 3);

		REQUIRE_FALSE(rawrbox::RESOURCES::isPreloading());

		REQUIRE(rawrbox::RESOURCES::isLoaded("./a.slow"));
		REQUIRE(rawrbox::RESOURCES::isLoaded("./b.slow"));
		REQUIRE(rawrbox::RESOURCES::isLoaded("./c.slow"));
	}
}