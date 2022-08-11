from conans import CMake
from conans import ConanFile
from conans import tools

class OctoLoggerCPPConan(ConanFile):
    name = "octo-logger-cpp"
    version = "1.0.0"
    url = "https://github.com/ofiriluz/octo-logger-cpp"
    author = "Ofir Iluz"
    generators = "cmake"
    settings = "os", "compiler", "build_type", "arch"

    def requirements(self):
        self.requires("catch2/3.1.0")
        self.requires("fmt/9.0.0")
        self.requires("trompeloeil/42")

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        cmake.install()
        cmake.test()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = tools.collect_libs(self)
