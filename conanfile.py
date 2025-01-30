import os
import platform

from conan import ConanFile
from conan.tools.files import copy
from conan.tools.cmake import CMakeToolchain, CMake


class CompressorRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps"

    def generate(self):
        tc = CMakeToolchain(self)
        tc.user_presets_path = 'ConanPresets.json'
        tc.cache_variables["CMAKE_MSVC_DEBUG_INFORMATION_FORMAT"] = "Embedded"
        tc.generator = 'Ninja'
        tc.generate()

    def requirements(self):
        self.requires("glm/cci.20230113")
        self.requires("magic_enum/0.9.6")
        self.requires("msgpack-c/6.1.0")
        self.requires("spdlog/1.15.0", options={
            'use_std_fmt': True,
            'header_only': True
        })
        self.requires("stb/cci.20240531")
        self.requires("xbyak/7.07")
        self.requires("yaml-cpp/0.8.0")

    def layout(self):
        self.folders.generators = os.path.join("build", str(self.settings.build_type), "generators")
        self.folders.build = os.path.join("build", str(self.settings.build_type))

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

        copy(self, "*.dll", src=os.path.join(self.build_folder), dst=self.package_folder)
