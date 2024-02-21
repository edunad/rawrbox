#pragma once

#include <glaze/glaze.hpp>

#include <string>

namespace rawrbox {
	enum class JSONDiffOp {
		ADD = 0,
		REMOVE,
		REPLACE
	};

	struct JSONDiff {
		rawrbox::JSONDiffOp op;
		std::string path;
		glz::json_t value;
	};

	class JSONUtils {
	protected:
		template <typename T = int>
			requires(std::is_same_v<T, double> || std::is_same_v<T, std::string> || std::is_same_v<T, bool>)
		static void compareAndDiff(const glz::json_t& a, const glz::json_t& b, const std::string& path, std::vector<rawrbox::JSONDiff>& diffs) {
			if (a.holds<T>() && b.holds<T>()) {
				if (a.get<T>() != b.get<T>()) {
					diffs.emplace_back(rawrbox::JSONDiffOp::REPLACE, path, b);
				}
			}
		}

	public:
		// These are just a temp fix until it's supported by glaze (https://github.com/stephenberry/glaze/issues/769), they are far from performant, and might have bugs
		static std::vector<rawrbox::JSONDiff> diff(const glz::json_t& a, const glz::json_t& b, const std::string& path = "");
		static void patch(glz::json_t& json, const std::vector<rawrbox::JSONDiff>& diffs);
	};
} // namespace rawrbox
