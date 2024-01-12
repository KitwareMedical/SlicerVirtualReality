#-----------------------------------------------------------------------------
# Build VTK Rendering OpenXR module, pointing it to Slicer's VTK and the OpenXR
# libraries also built by this extension.

set(proj vtkRenderingOpenXR)

# Set dependency list
set(${proj}_DEPENDS
  OpenXR-SDK
  vtkRenderingVR
  )

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj)

if(${SUPERBUILD_TOPLEVEL_PROJECT}_USE_SYSTEM_${proj})
  message(FATAL_ERROR "Enabling ${SUPERBUILD_TOPLEVEL_PROJECT}_USE_SYSTEM_${proj} is not supported !")
endif()

# Sanity checks
if(DEFINED ${proj}_DIR AND NOT EXISTS ${${proj}_DIR})
  message(FATAL_ERROR "${proj}_DIR [${${proj}_DIR}] variable is defined but corresponds to nonexistent directory")
endif()

if(NOT DEFINED ${proj}_DIR AND NOT ${SUPERBUILD_TOPLEVEL_PROJECT}_USE_SYSTEM_${proj})

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

  if(NOT EXISTS ${VTKExternalModule_SOURCE_DIR})
    message(FATAL_ERROR "VTKExternalModule_SOURCE_DIR [${VTKExternalModule_SOURCE_DIR}] variable is set to a nonexistent directory")
  endif()

  set(VTK_SOURCE_DIR ${VTK_DIR}/../VTK)
  ExternalProject_Message(${proj} "VTK_SOURCE_DIR:${VTK_SOURCE_DIR}")

  set(_module_subdir Rendering/OpenXR)
  set(_module_name RenderingOpenXR)

  # The python module destination directory is set to `bin/Python/vtkmodules` to
  # match the value hard-coded in "CMake/SlicerExtensionCPackBundleFixup.cmake.in"
  set(_module_python_module_dir "${Slicer_INSTALL_THIRDPARTY_BIN_DIR}/Python/vtkmodules")

  set(EP_SOURCE_DIR ${VTK_SOURCE_DIR}/${_module_subdir})
  set(EP_BINARY_DIR ${CMAKE_BINARY_DIR}/${proj}-build)

  # The "vtk_openxr_actions.json" and "vtk_openxr_binding_*.json" files
  # are then installed into "${_manifest_install_dir}/xr_actions/" directory.
  set(_openxr_manifest_install_dir ${Slicer_INSTALL_THIRDPARTY_LIB_DIR})

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    DOWNLOAD_COMMAND ""
    SOURCE_DIR ${VTKExternalModule_SOURCE_DIR}
    BINARY_DIR ${EP_BINARY_DIR}
    INSTALL_COMMAND ""
    CMAKE_CACHE_ARGS
      # VTKExternalModule
      -DVTK_MODULE_NAME:STRING=${_module_name}
      -DVTK_MODULE_SOURCE_DIR:PATH=${EP_SOURCE_DIR}
      -DVTK_MODULE_CMAKE_MODULE_PATH:PATH=${VTK_SOURCE_DIR}/CMake
      -DVTK_MODULE_PYTHON_MODULE_DESTINATION:STRING=${_module_python_module_dir}/.. # "vtkmodules" is implicitly appended
      -DOpenXR_FIND_PACKAGE_VARS:STRING=
      # vtkRenderingOpenXR
      -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
      -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
      -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
      -DBUILD_TESTING:BOOL=OFF
      -DCMAKE_RUNTIME_OUTPUT_DIRECTORY:PATH=${CMAKE_BINARY_DIR}/${Slicer_THIRDPARTY_BIN_DIR}
      -DCMAKE_LIBRARY_OUTPUT_DIRECTORY:PATH=${CMAKE_BINARY_DIR}/${Slicer_THIRDPARTY_LIB_DIR}
      -DCMAKE_INSTALL_BINDIR:STRING=${Slicer_INSTALL_THIRDPARTY_BIN_DIR}
      -DCMAKE_INSTALL_LIBDIR:STRING=${Slicer_INSTALL_THIRDPARTY_LIB_DIR}
      -DCMAKE_INSTALL_DATAROOTDIR:STRING=${_openxr_manifest_install_dir}
      -DCMAKE_MACOSX_RPATH:BOOL=0
      # Required to find VTK
      -DVTK_DIR:PATH=${VTK_DIR}
      # Required to find vtkRenderingVR
      -DvtkRenderingVR_DIR:PATH=${vtkRenderingVR_DIR}
      # Required to find OpenXR
      -DOpenXR_DIR:PATH=${OpenXR_DIR}
      ${EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS}
    DEPENDS
      ${${proj}_DEPENDS}
    )

  ExternalProject_AlwaysConfigure(${proj})

  set(${proj}_DIR ${EP_BINARY_DIR})

  #-----------------------------------------------------------------------------
  # Launcher setting specific to build tree

  # pythonpath
  set(${proj}_PYTHONPATH_LAUNCHER_BUILD
    ${${proj}_DIR}/${_module_python_module_dir}
    )
  mark_as_superbuild(
    VARS ${proj}_PYTHONPATH_LAUNCHER_BUILD
    LABELS "PYTHONPATH_LAUNCHER_BUILD"
    )

  #-----------------------------------------------------------------------------
  # Launcher setting specific to install tree

  # NA

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDS})
endif()

mark_as_superbuild(VARS ${proj}_DIR:PATH)
ExternalProject_Message(${proj} "${proj}_DIR:${${proj}_DIR}")

