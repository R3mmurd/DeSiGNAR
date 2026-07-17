# This port is maintained in-tree (vcpkg/ports/designar) rather than in the
# vcpkg curated registry, since this library does not (yet) have a tagged
# release. To use it locally: `vcpkg install designar --overlay-ports=vcpkg/ports`.
#
# Once a real tag exists upstream, replace REF/SHA512 below with the
# tagged commit and its hash (`vcpkg hash <tarball>` after downloading the
# corresponding GitHub release/source archive), and this port can move to
# the vcpkg curated registry unmodified.

vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO R3mmurd/DeSiGNAR
  REF "v${VERSION}" # expects a tag named e.g. v2.0.0 once one is cut
  SHA512 0 # placeholder: fill in with `vcpkg hash` once REF is a real tag
  HEAD_REF main
)

vcpkg_cmake_configure(
  SOURCE_PATH "${SOURCE_PATH}"
  OPTIONS
    -DDESIGNAR_WARNINGS_AS_ERRORS=OFF
    -DDESIGNAR_BUILD_SAMPLES=OFF
    -DDESIGNAR_BUILD_TESTS=OFF
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(PACKAGE_NAME Designar CONFIG_PATH lib/cmake/Designar)

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
