from conan import ConanFile

class Recipe(ConanFile):
	settings = "os", "compiler", "build_type", "arch"
	requires = "fmt/9.1.0", "utfcpp/3.2.3", "qhull/8.0.1", "glm/cci.20230113", "freetype/2.13.0"
	generators = "CMakeDeps", "CMakeToolchain"

	def requirements(self):
		if self.settings.os == "Linux":
			self.requires("wayland/1.21.0")

