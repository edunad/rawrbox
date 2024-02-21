#include <rawrbox/utils/json.hpp>
#include <rawrbox/utils/string.hpp>

#include <fmt/format.h>

namespace rawrbox {
	std::vector<rawrbox::JSONDiff> JSONUtils::diff(const glz::json_t& a, const glz::json_t& b, const std::string& path) {
		std::vector<rawrbox::JSONDiff> diffs = {};

		if (a.holds<glz::json_t::array_t>() && b.holds<glz::json_t::array_t>()) {
			auto& arrA = a.get<glz::json_t::array_t>();
			auto& arrB = b.get<glz::json_t::array_t>();

			// Compare arrays element by element
			size_t minSize = std::min(arrA.size(), arrB.size());
			for (size_t i = 0; i < minSize; ++i) {
				auto subDiffs = diff(arrA[i], arrB[i], fmt::format("{}/{}", path, std::to_string(i)));
				diffs.insert(diffs.end(), subDiffs.begin(), subDiffs.end());
			}

			// If arrA is longer, add "remove" operations for the extra elements
			if (arrA.size() > arrB.size()) {
				for (size_t i = arrB.size(); i < arrA.size(); ++i) {
					diffs.emplace_back(rawrbox::JSONDiffOp::REMOVE, fmt::format("{}/{}", path, std::to_string(i)), glz::json_t::null_t());
				}
			}

			// If arrB is longer, add "add" operations for the extra elements
			if (arrB.size() > arrA.size()) {
				for (size_t i = arrA.size(); i < arrB.size(); ++i) {
					diffs.emplace_back(rawrbox::JSONDiffOp::ADD, fmt::format("{}/{}", path, std::to_string(i)), arrB[i]);
				}
			}
		} else if (a.holds<glz::json_t::object_t>() && b.holds<glz::json_t::object_t>()) {
			auto& objA = a.get<glz::json_t::object_t>();
			auto& objB = b.get<glz::json_t::object_t>();

			for (const auto& [key, valueA] : objA) {
				auto jsonPath = fmt::format("{}/{}", path, key);

				if (objB.contains(key)) {
					auto& valueB = objB.at(key);
					auto subDiffs = diff(valueA, valueB, jsonPath);
					diffs.insert(diffs.end(), subDiffs.begin(), subDiffs.end());
				} else {
					diffs.emplace_back(rawrbox::JSONDiffOp::REMOVE, jsonPath, glz::json_t::null_t());
				}
			}

			for (const auto& [key, valueB] : objB) {
				if (!objA.contains(key)) {
					diffs.emplace_back(rawrbox::JSONDiffOp::ADD, fmt::format("{}/{}", path, key), valueB);
				}
			}
		} else { // root
			compareAndDiff<double>(a, b, path, diffs);
			compareAndDiff<std::string>(a, b, path, diffs);
			compareAndDiff<bool>(a, b, path, diffs);
		}

		return diffs;
	}

	void JSONUtils::patch(glz::json_t& json, const std::vector<rawrbox::JSONDiff>& diffs) {
		for (const auto& diff : diffs) {
			glz::json_t* current = &json;

			std::vector<std::string> segments = rawrbox::StrUtils::split(diff.path, '/', true);
			for (auto& segment : segments) {
				if (segment.empty()) continue;

				// Check if the segment is an array index
				if (rawrbox::StrUtils::isNumeric(segment)) {
					size_t index = std::stoi(segment);
					if (!current->holds<glz::json_t::array_t>()) *current = glz::json_t::array_t{};

					auto& array = current->get<glz::json_t::array_t>();
					if (index >= array.size()) array.resize(index + 1);
					current = &array[index];

				} else {
					current = &(*current)[segment];
				}
			}

			if (diff.op == rawrbox::JSONDiffOp::REPLACE || diff.op == rawrbox::JSONDiffOp::ADD) {
				*current = diff.value;
			} else if (diff.op == rawrbox::JSONDiffOp::REMOVE) {
				if (current->holds<glz::json_t::object_t>()) { // object
					auto& object = current->get<glz::json_t::object_t>();
					object.erase(segments.back());
				} else if (current->holds<glz::json_t::array_t>()) { // array
					auto& array = current->get<glz::json_t::array_t>();
					array.erase(array.begin() + std::stoi(segments.back()));
				} else { // double, string, etc on root
					if (json.holds<glz::json_t::object_t>()) {
						auto& object = json.get<glz::json_t::object_t>();
						object.erase(segments.back());
					} else if (json.holds<glz::json_t::array_t>()) {
						auto& array = json.get<glz::json_t::array_t>();
						array.erase(array.begin() + std::stoi(segments.back()));
					}
				}
			}
		}
	}
} // namespace rawrbox
