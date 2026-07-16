from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps, cmake_layout
from conan.tools.files import copy
import os


class DesignarConan(ConanFile):
    name = "designar"
    version = "2.0.0"
    license = "MIT"
    homepage = "https://github.com/R3mmurd/DeSiGNAR"
    description = "A teaching-oriented C++ data structures and algorithms library."
    topics = ("data-structures", "algorithms", "teaching")
    settings = "os", "compiler", "build_type", "arch"
    package_type = "static-library"

    exports_sources = (
        "CMakeLists.txt",
        "cmake/*",
        "include/*",
        "src/*",
        "LICENSE",
    )

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        # This package only wants the library built: samples/tests pull in
        # Threads and build a couple dozen extra executables for no benefit
        # to a consumer installing Designar as a dependency.
        tc.variables["DESIGNAR_BUILD_SAMPLES"] = False
        tc.variables["DESIGNAR_BUILD_TESTS"] = False
        tc.variables["DESIGNAR_WARNINGS_AS_ERRORS"] = False
        tc.generate()
        CMakeDeps(self).generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build(target="Designar")

    def package(self):
        copy(self, "LICENSE", src=self.source_folder,
             dst=os.path.join(self.package_folder, "licenses"))
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", "Designar")
        self.cpp_info.set_property("cmake_target_name", "Designar::Designar")
        self.cpp_info.libs = ["Designar"]

        if self.settings.os in ("Linux", "FreeBSD"):
            self.cpp_info.system_libs.append("pthread")
