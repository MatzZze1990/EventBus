from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps


class eventbusRecipe(ConanFile):
    name = "eventbus"
    version = "1.0"
    package_type = "library"
    build_policy = "never"

    # Optional metadata
    license = "MIT LICENSE"
    author = "MatzZze <coding@matzzze.de>"
    url = "https://github.com/MatzZze1990/EventBus"
    description = "A simple EventBus for c++"
    topics = "event bus"

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    requires = "gtest/1.14.0"
    default_options = {"shared": False, "fPIC": True}

    # Sources are located in the same place as this recipe, copy them to the recipe
    #exports_sources = "conanfile.py", "conan_provider.cmake", "CMakeLists.txt", "src/*", "include/*", "test/*"

    def config_options(self):
        if self.settings.os == "Windows":
            self.options.rm_safe("fPIC")

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")

    def layout(self):
        cmake_layout(self)
        self.folders.build = "build"

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generate()

    # def build(self):
    #     cmake = CMake(self)
    #     cmake.configure()
    #     cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        if self.settings.os == "Windows":
            self.cpp_info.libs = ["EventBus.lib"]
        elif self.settings.os == "Linux":
            self.cpp_info.libs = ["libEventBus.so"]
        self.cpp_info.includedirs = ["include"]

    def deploy(self):
        self.copy("*.dll", dst="bin", src="bin") # From bin to bin
        self.copy("*.lib*", dst="bin", src="lib") # From lib to bin
    

    

