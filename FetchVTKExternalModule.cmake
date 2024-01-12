
include(FetchContent)

set(proj VTKExternalModule)
set(EP_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})
FetchContent_Populate(${proj}
  SOURCE_DIR     ${EP_SOURCE_DIR}
  GIT_REPOSITORY https://github.com/KitwareMedical/VTKExternalModule
  GIT_TAG        3a290ec10d3e8326cd009759d87d8d219c551b47
  QUIET
  )
message(STATUS "Remote - ${proj} [OK]")

set(VTKExternalModule_SOURCE_DIR ${EP_SOURCE_DIR})
message(STATUS "Remote - VTKExternalModule_SOURCE_DIR:${VTKExternalModule_SOURCE_DIR}")

