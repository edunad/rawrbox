#include <rawrbox/utils/logger.hpp>

namespace rawrbox {
	// NOLINTBEGIN(*)
	Logger::Logger(const std::string& title) : _title(title) {}
	// NOLINTEND(*)

	void Logger::setAutoNewLine(bool set) { this->_autoNewLine = set; }
} // namespace rawrbox
