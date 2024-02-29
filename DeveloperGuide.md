# Information for developers

This Slicer extension is in active development. The API may change from version to version without notice.

## Build instructions

- Build the extension against the newly built Slicer with Qt5 and VTK9 enabled.
- To start Slicer from a build tree and ensure the extension is properly loaded, considering using the ``--launcher-additional-settings`` option:

   ```
   ./Slicer.exe  --launcher-additional-settings C:\path\to\SlicerVirtualReality-build\inner-build\AdditionalLauncherSettings.ini --additional-module-paths C:\path\to\SlicerVirtualReality-build\inner-build\
   ```

Alternatively, to avoid the need to specify additional command-line parameters, virtual reality extension can be set up in Slicer by:
- Copy these files to (SlicerVirtualReality-binary)\inner-build\lib\Slicer-4.9\qt-loadable-modules\Release:
  - (SlicerVirtualReality-binary)\bin\Release\vtkRenderingOpenVR-9.0.dll
  - (SlicerVirtualReality-binary)\OpenVR\bin\win64\openvr_api.dll
- Add (SlicerVirtualReality-binary)\inner-build\lib\Slicer-4.9\qt-loadable-modules\Release folder to additional module paths in Slicer application settings.

Note that specifying the top-level build directory of the extension ensures that Slicer find all types of modules.

## Useful Python Snippets

Activate virtual reality view:

```python

import logging
import slicer

def isVRInitialized():
    """Determine if VR has been initialized
    """
    vrLogic = slicer.modules.virtualreality.logic()
    if (vrLogic is None
        or vrLogic.GetVirtualRealityViewNode() is None
        or not vrLogic.GetVirtualRealityViewNode().GetVisibility()
        or not vrLogic.GetVirtualRealityViewNode().GetActive()):
        return False
    return True

def vrCamera():
    # Get VR module widget
    if not isVRInitialized():
        return None
    # Get VR camera
    vrViewWidget = slicer.modules.virtualreality.viewWidget()
    if vrViewWidget is None:
      return None
    rendererCollection = vrViewWidget.renderWindow().GetRenderers()
    if rendererCollection.GetNumberOfItems() < 1:
        logging.error('Unable to access VR renderers')
        return None
    return rendererCollection.GetItemAsObject(0).GetActiveCamera()


assert isVRInitialized() is False
assert vrCamera() is None

vrLogic = slicer.modules.virtualreality.logic()
vrLogic.SetVirtualRealityActive(True)

assert isVRInitialized() is True
assert vrCamera() is not None


```

Set virtual reality view background color to black:

```python

color = [0,0,0]
vrView=getNode('VirtualRealityView')
vrView.SetBackgroundColor(color)
vrView.SetBackgroundColor2(color)

```

Set whether a node can be selected/grabbed/moved:

```python

nodeLocked.SetSelectable(0)
nodeMovable.SetSelectable(1)

```
