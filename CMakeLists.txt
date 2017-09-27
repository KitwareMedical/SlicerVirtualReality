cmake_minimum_required(VERSION 2.8.9)

project(VRExtension)

#-----------------------------------------------------------------------------
# Extension meta-information
set(EXTENSION_HOMEPAGE "http://slicer.org/slicerWiki/index.php/Documentation/Nightly/Extensions/VR")
set(EXTENSION_CATEGORY "Examples")
set(EXTENSION_CONTRIBUTORS "Jean-Baptiste Vimort (Kitware), Jean-Christophe Fillion-Robin (Kitware)")
set(EXTENSION_DESCRIPTION "This extension allow to use a an HTC vive to display data in Virtual Reallity")
set(EXTENSION_ICONURL "http://www.example.com/Slicer/Extensions/VR.png")
set(EXTENSION_SCREENSHOTURLS "http://www.example.com/Slicer/Extensions/VR/Screenshots/1.png")
set(EXTENSION_DEPENDS "NA") # Specified as a space separated string, a list or 'NA' if any

#-----------------------------------------------------------------------------
# Extension dependencies
find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})

# DO CHECK HERE:
# VR enabled

#-----------------------------------------------------------------------------
# Extension modules
add_subdirectory(VR)
## NEXT_MODULE

#-----------------------------------------------------------------------------
include(${Slicer_EXTENSION_GENERATE_CONFIG})
include(${Slicer_EXTENSION_CPACK})
