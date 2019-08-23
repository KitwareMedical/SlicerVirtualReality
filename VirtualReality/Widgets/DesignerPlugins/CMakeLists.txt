
set(KIT ${PROJECT_NAME}Plugins)

set(${KIT}_EXPORT "Q_SLICER_MODULE_${MODULE_NAME_UPPER}_WIDGETS_PLUGINS_EXPORT")

set(${KIT}_SRCS
  qSlicer${MODULE_NAME}ModuleWidgetsAbstractPlugin.cxx
  qSlicer${MODULE_NAME}ModuleWidgetsAbstractPlugin.h
  qSlicer${MODULE_NAME}ModuleWidgetsPlugin.cxx
  qSlicer${MODULE_NAME}ModuleWidgetsPlugin.h

  qMRML${MODULE_NAME}PlayWidgetPlugin.cxx
  qMRML${MODULE_NAME}PlayWidgetPlugin.h
  qMRML${MODULE_NAME}SeekWidgetPlugin.cxx
  qMRML${MODULE_NAME}SeekWidgetPlugin.h
  qMRML${MODULE_NAME}ToolBarPlugin.cxx
  qMRML${MODULE_NAME}ToolBarPlugin.h
  )

set(${KIT}_MOC_SRCS
  qSlicer${MODULE_NAME}ModuleWidgetsPlugin.h

  qMRML${MODULE_NAME}PlayWidgetPlugin.h
  qMRML${MODULE_NAME}SeekWidgetPlugin.h
  qMRML${MODULE_NAME}ToolBarPlugin.h
  )

set(${KIT}_TARGET_LIBRARIES
  qSlicer${MODULE_NAME}ModuleWidgets
  )

#-----------------------------------------------------------------------------
# Necessary to set CMAKE_LIBRARY_OUTPUT_DIRECTORY. Without that designer plugins
# would be created in /designer directory.
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${Slicer_DIR}/bin)

ctkMacroBuildQtDesignerPlugin(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT}
  FOLDER "Module-${MODULE_NAME}"
  SRCS ${${KIT}_SRCS}
  MOC_SRCS ${${KIT}_MOC_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
