
include(FetchContent)

set(proj VTKExternalModule)
set(EP_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})
FetchContent_Populate(${proj}
  SOURCE_DIR     ${EP_SOURCE_DIR}
  GIT_REPOSITORY git://github.com/KitwareMedical/VTKExternalModule
  GIT_TAG        50f1c5be9c7a29e9aa768dbb632fdf74d13b4361
  QUIET
  )
message(STATUS "Remote - ${proj} [OK]")

set(VTKExternalModule_SOURCE_DIR ${EP_SOURCE_DIR})
message(STATUS "Remote - VTKExternalModule_SOURCE_DIR:${VTKExternalModule_SOURCE_DIR}")

