#pragma once
#include <rawrbox/utils/logger.hpp>
namespace rawrbox {
	class BASSUtils {
	protected:
		static std::unique_ptr<rawrbox::Logger> _logger;

	public:
		static void checkBASSError();
	};

} // namespace rawrbox
