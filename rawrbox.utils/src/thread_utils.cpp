#include <rawrbox/utils/thread_utils.hpp>

#include <tracy/Tracy.hpp>

namespace rawrbox {
	void ThreadUtils::setName(const std::string& name) { tracy::SetThreadName(name.c_str()); }
} // namespace rawrbox
