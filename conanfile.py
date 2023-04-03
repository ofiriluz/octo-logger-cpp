from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps, cmake_layout
from conan.tools.build import check_min_cppstd
from conan.errors import ConanInvalidConfiguration
from conan.tools.scm import Version

class OctoLoggerCPPConan(ConanFile):
    name = "octo-logger-cpp"
    url = "https://github.com/ofiriluz/octo-logger-cpp"
    author = "Ofir Iluz"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "with_aws": [True, False],
        "with_json_formatting": [True, False]
    }
    default_options = {
        "with_aws": False,
        "with_json_formatting" : False
    }

    def set_version(self):
        from os import path
        version_file_path = path.join(self.recipe_folder, 'VERSION')
        with open(version_file_path) as fd:
            contents = fd.readline()
        self.version = contents.rstrip().lstrip()

    @property
    def _compilers_minimum_version(self):
        return {
            "gcc": "8",
            "clang": "9",
            "apple-clang": "11",
            "Visual Studio": "16",
        }

    def configure(self):
        if self.options.with_aws:
            self.options["aws-sdk-cpp"].logs = True
            self.options.with_json_formatting = True

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()
        cd = CMakeDeps(self)
        cd.generate()

    def layout(self):
        cmake_layout(self)

    def validate(self):
        if self.info.settings.compiler.cppstd:
            check_min_cppstd(self, "17")

        minimum_version = self._compilers_minimum_version.get(str(self.info.settings.compiler), False)
        if not minimum_version: # Unknown compiler
            self.output.warn(f"{self.name} requires C++17. Your compiler is unknown. Assuming it supports C++17.")
        elif Version(self.info.settings.compiler.version) < minimum_version:
            raise ConanInvalidConfiguration(
                f"{self.name} requires C++17, which your compiler does not support."
            )
        
        if self.settings.compiler == "clang" and self.settings.compiler.get_safe("libcxx") == "libc++":
            raise ConanInvalidConfiguration(f"{self.name} does not support clang with libc++. Use libstdc++ instead.")
        if self.settings.compiler == "Visual Studio" and self.settings.compiler.runtime in ["MTd", "MT"]:
            raise ConanInvalidConfiguration(f"{self.name} does not support MSVC MT/MTd configurations, only MD/MDd is supported")

    def requirements(self):
        self.requires("catch2/3.1.0")
        self.requires("fmt/9.0.0")
        self.requires("trompeloeil/42")
        if self.options.with_json_formatting:
            self.requires("nlohmann_json/3.11.2")
        if self.options.with_aws:
            self.requires("aws-sdk-cpp/1.9.234")

    def build(self):
        cmake = CMake(self)
        cmake.configure(variables={
            "WITH_AWS": self.options.with_aws,
            "WITH_JSON_FORMATTING" : self.options.with_json_formatting
        })
        cmake.build()
        if str(self.settings.os) != "Windows":
            cmake.test()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        cpp_info = self.cpp_info
        cpp_info.set_property("cmake_file_name", "octo-logger-cpp")
        cpp_info.set_property("cmake_target_name", "octo::octo-logger-cpp")
        cpp_info.set_property("pkg_config_name", "octo-logger-cpp")

        component = self.cpp_info.components["libocto-logger-cpp"]
        component.libs = ["octo-logger-cpp"]
        component.requires = ["fmt::fmt"]
        if self.options.with_json_formatting:
            component.requires.extend([
                "nlohmann_json::nlohmann_json",
            ])
            component.defines.append('OCTO_LOGGER_WITH_JSON_FORMATTING')
            cpp_info.defines.append('OCTO_LOGGER_WITH_JSON_FORMATTING')

        if self.options.with_aws:
            component.requires.extend([
                "aws-sdk-cpp::monitoring"
            ])
            component.defines.append('OCTO_LOGGER_WITH_AWS')
            cpp_info.defines.append('OCTO_LOGGER_WITH_AWS')
        cpp_info.filenames["cmake_find_package"] = "octo-logger-cpp"
        cpp_info.filenames["cmake_find_package_multi"] = "octo-logger-cpp"
        cpp_info.names["cmake_find_package"] = "octo-logger-cpp"
        cpp_info.names["cmake_find_package_multi"] = "octo-logger-cpp"
        cpp_info.names["pkg_config"] = "octo-logger-cpp"
        component.names["cmake_find_package"] = "octo-logger-cpp"
        component.names["cmake_find_package_multi"] = "octo-logger-cpp"
        component.set_property("cmake_target_name", "octo::octo-logger-cpp")
        component.set_property("pkg_config_name", "octo-logger-cpp")
