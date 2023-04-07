from conan import ConanFile

class Recipe(ConanFile):
	settings = "os", "compiler", "build_type", "arch"
	requires = "fmt/9.1.0", "nlohmann_json/3.11.2", "utfcpp/3.2.3", "stb/cci.20220909", "qhull/8.0.1"
	generators = "CMakeDeps", "CMakeToolchain"

	def requirements(self):
		if self.settings.os == "Linux":
			self.requires("wayland/1.21.0")

