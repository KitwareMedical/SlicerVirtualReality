#-----------------------------------------------------------------------------
# Build VTK Rendering OpenVR module, pointing it to Slicer's VTK and the OpenVR
# libraries also downloaded by this extension.

set(proj VTKRenderingOpenVR)

# Dependencies
set(${proj}_DEPENDENCIES OpenVR)
if(DEFINED Slicer_SOURCE_DIR)
  list(APPEND ${proj}_DEPENDENCIES VTK)
endif()
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

set(VTK_SOURCE_DIR ${VTK_DIR}/../VTK)
set(${proj}_SOURCE_DIR ${VTK_SOURCE_DIR}/Rendering/OpenVR)
ExternalProject_Message(${proj} "VTK_SOURCE_DIR:${VTK_SOURCE_DIR}")
ExternalProject_Message(${proj} "${proj}_SOURCE_DIR:${${proj}_SOURCE_DIR}")

set(${proj}_BINARY_DIR ${CMAKE_BINARY_DIR}/${proj}-build)

set(EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS)
if(VTK_WRAP_PYTHON)
  list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS
    -DPYTHON_EXECUTABLE:FILEPATH=${PYTHON_EXECUTABLE}
    -DPYTHON_INCLUDE_DIR:PATH=${PYTHON_INCLUDE_DIR}
    -DPYTHON_LIBRARY:FILEPATH=${PYTHON_LIBRARY}
    # Required by FindPython3 CMake module used by VTK
    -DPython3_ROOT_DIR:PATH=${Python3_ROOT_DIR}
    -DPython3_INCLUDE_DIR:PATH=${Python3_INCLUDE_DIR}
    -DPython3_LIBRARY:FILEPATH=${Python3_LIBRARY}
    -DPython3_LIBRARY_DEBUG:FILEPATH=${Python3_LIBRARY_DEBUG}
    -DPython3_LIBRARY_RELEASE:FILEPATH=${Python3_LIBRARY_RELEASE}
    -DPython3_EXECUTABLE:FILEPATH=${Python3_EXECUTABLE}
    )
endif()

ExternalProject_Add(${proj}
  ${${proj}_EP_ARGS}
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${VTKExternalModule_SOURCE_DIR}
  BINARY_DIR ${${proj}_BINARY_DIR}
  INSTALL_COMMAND ""
  CMAKE_CACHE_ARGS
    # VTKExternalModule
    -DVTK_MODULE_NAME:STRING=RenderingOpenVR
    -DVTK_MODULE_SOURCE_DIR:PATH=${${proj}_SOURCE_DIR}
    -DVTK_MODULE_CMAKE_MODULE_PATH:PATH=${VTK_SOURCE_DIR}/CMake
    # VTKRenderingOpenVR
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
    -DOpenVR_INCLUDE_DIR:PATH=${OpenVR_INCLUDE_DIR}
    -DOpenVR_LIBRARY:PATH=${OpenVR_LIBRARY}
    ${EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS}
  DEPENDS
    ${${proj}_DEPENDENCIES}
)

set(vtkRenderingOpenVR_DIR ${${proj}_BINARY_DIR})
mark_as_superbuild(VARS vtkRenderingOpenVR_DIR:PATH)

#-----------------------------------------------------------------------------
# Launcher setting specific to build tree

# pythonpath
set(${proj}_PYTHONPATH_LAUNCHER_BUILD
  ${${proj}_DIR}/${Slicer_INSTALL_THIRDPARTY_LIB_DIR}/${PYTHON_SITE_PACKAGES_SUBDIR}/vtkmodules
  )
mark_as_superbuild(
  VARS ${proj}_PYTHONPATH_LAUNCHER_BUILD
  LABELS "PYTHONPATH_LAUNCHER_BUILD"
  )

#-----------------------------------------------------------------------------
# Launcher setting specific to install tree

# NA

ExternalProject_Message(${proj} "vtkRenderingOpenVR_DIR:${vtkRenderingOpenVR_DIR}")

