SlicerVirtualReality
====================

Extension for 3D slicer that enables user to interact with the 3D scene using virtual reality.

![](SlicerVirtualReality.png)

The extension works with all OpenVR-compatible headsets, such as HTC Vive, all Windows Mixed Reality headsets (by Acer, Lenovo, HP, etc,), and Oculus Rift. Currently, a limitation is that [Oculus Rift controllers are not supported](https://github.com/KitwareMedical/SlicerVirtualReality/issues/28), so the user can see and explore the scene in 3D but cannot fly or manipulate objects.

Frequently Asked Questions
--------------------------

### How to record virtual reality videos?

Enable screen mirroring in SteamVR and use the free of [OBS Studio](https://obsproject.com/) software to capture VR headset content, application window, webcam, etc.

### How to avoid crashes when recording video of OpenVR with VolumeRendering enabled?

[OBS Studio](https://obsproject.com/) crashes if NVidia hardware-based compression is used. If CPU-based video compression option is chosen then recording works well.

For developers
--------------

**This Slicer extension is in active development. The API may change from version to version without notice.**

Build instructions:
- Build the extension against the newly built Slicer with Qt5 and VTK9 enabled.
- To start Slicer from a build tree and ensure the extension is properly loaded, considering using the ``--launcher-additional-settings`` option:

   ```
   ./Slicer.exe  --launcher-additional-settings C:\path\to\SlicerVirtualReality-build\inner-build\AdditionalLauncherSettings.ini --additional-module-paths C:\path\to\SlicerVirtualReality-build\inner-build\
   ```

Note that specifying the top-level build directory of the extension ensures that Slicer find all types of modules.

Useful Python Snippets
----------------------

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

License
-------

It is covered by the Apache License, Version 2.0:

http://www.apache.org/licenses/LICENSE-2.0
