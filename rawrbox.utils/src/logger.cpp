#include <rawrbox/utils/logger.hpp>

namespace rawrbox {
#ifdef _DEBUG
	spdlog::level::level_enum Logger::LOG_LEVEL = spdlog::level::trace; // Default
#else
	spdlog::level::level_enum Logger::LOG_LEVEL = spdlog::level::warn; // Default
#endif

	bool Logger::ENABLE_FILE_LOGGING = true;
	std::shared_ptr<spdlog::logger> Logger::LOGGER = nullptr;

	Logger::Logger(const std::string& title) : _title(title) {
		if (LOGGER != nullptr) return;

		auto _spdConsole = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

		if (ENABLE_FILE_LOGGING) {
			auto _spdLogFile = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs.log", true);
			LOGGER = std::make_shared<spdlog::logger>("CONSOLE", spdlog::sinks_init_list{_spdConsole, _spdLogFile});
		} else {
			LOGGER = std::make_shared<spdlog::logger>("CONSOLE", spdlog::sinks_init_list{_spdConsole});
		}

		LOGGER->set_pattern("[%^%l%$%v");
		LOGGER->set_level(rawrbox::Logger::LOG_LEVEL);

		spdlog::set_default_logger(LOGGER);
		spdlog::set_error_handler([](const std::string& msg) { fmt::print("*** LOG ERROR: {} ***\n", msg); });
	}
} // namespace rawrbox
