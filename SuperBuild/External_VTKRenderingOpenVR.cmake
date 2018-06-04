#-----------------------------------------------------------------------------
# Build VTK Rendering OpenVR module, pointing it to Slicer's VTK and the OpenVR
# libraries also downloaded by this extension.

set(proj VTKRenderingOpenVR)

# Dependencies
set(${proj}_DEPENDENCIES OpenVR)
if(DEFINED Slicer_SOURCE_DIR)
  list(APPEND ${proj}_DEPENDENCIES VTKv9)
endif()
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

set(VTK_SOURCE_DIR ${VTK_DIR}/../VTKv${Slicer_VTK_VERSION_MAJOR})
set(${proj}_SOURCE_DIR ${VTK_SOURCE_DIR}/Rendering/OpenVR)
ExternalProject_Message(${proj} "VTK_SOURCE_DIR:${VTK_SOURCE_DIR}")
ExternalProject_Message(${proj} "${proj}_SOURCE_DIR:${${proj}_SOURCE_DIR}")

set(${proj}_BINARY_DIR ${CMAKE_BINARY_DIR}/${proj}-build)

ExternalProject_Add(${proj}
  ${${proj}_EP_ARGS}
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${${proj}_SOURCE_DIR}
  BINARY_DIR ${${proj}_BINARY_DIR}
  INSTALL_COMMAND ""
  CMAKE_CACHE_ARGS
    -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
    -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
    -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
    -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
    -DBUILD_TESTING:BOOL=OFF
    -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${CMAKE_BINARY_DIR}/${Slicer_THIRDPARTY_BIN_DIR}
    -DCMAKE_LIBRARY_OUTPUT_DIRECTORY:PATH=${CMAKE_BINARY_DIR}/${Slicer_THIRDPARTY_LIB_DIR}
    -DVTK_INSTALL_RUNTIME_DIR:STRING=${Slicer_INSTALL_THIRDPARTY_LIB_DIR}
    -DVTK_INSTALL_LIBRARY_DIR:STRING=${Slicer_INSTALL_THIRDPARTY_LIB_DIR}
    -DCMAKE_MACOSX_RPATH:BOOL=0
    # Required to find VTK
    -DVTK_DIR:PATH=${VTK_DIR}
    # Required to find OpenVR
    -DVTK_OPENVR_OBJECT_FACTORY:BOOL=OFF
    -DOPENVR_INCLUDE_DIR:PATH=${OPENVR_INCLUDE_DIR}
    -DOPENVR_LIBRARY:PATH=${OPENVR_LIBRARY}
  DEPENDS ${${proj}_DEPENDENCIES}
)

set(${proj}_DIR ${${proj}_BINARY_DIR})
mark_as_superbuild(VARS ${proj}_DIR:PATH)

ExternalProject_Message(${proj} "${proj}_DIR:${${proj}_DIR}")

