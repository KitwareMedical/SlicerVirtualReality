SlicerVirtualReality
====================

Extension for 3D slicer that enables user to interact with the 3D scene using virtual reality.

![](SlicerVirtualReality.png)

The extension works with all OpenVR-compatible headsets, such as [HTC Vive](#setup-htc-vive), all [Windows Mixed Reality headsets](#setup-windows-mixed-reality) (by Acer, Lenovo, HP, etc.), and with [some limitations on Oculus Rift](#setup-oculus-rift).

Selected features:
- View all content of any of the 3D viewers in Slicer: volumes (as 2D image slices or using volume rendering), surfaces, points, etc.
- Viewing of 4D data sets (using volume rendering, surface rendering, etc.) using Sequences extension
- Align the headset's view to match viewpoint of the selected 3D view in Slicer
- Fly around using the touchpad of the right controller: direction is specified by orientation of the controller; speed is determined by the position of the finger on the touchpad. Currently not supported on Oculus Rift (#28).
- Grab and reposition objects using the trigger on the right controller. Experimental (#12). Currently not supported on Oculus Rift (#28).
- Advanced volume rendering performance tuning: available in Virtual Reality module, to find good balance between image quality and refresh rate.
- Make position of controllers available as transforms in the Slicer scene. These transforms can be used in custom modules to reslice volumes (using Volume Reslice Driver module in SlicerIGT extension) or transform any nodes in the scene.

Feature set of the extension is continuously improved. You can give us feedback and propose ideas for improvements by submitting them on the [issue tracker](https://github.com/KitwareMedical/SlicerVirtualReality/issues).

Frequently Asked Questions
--------------------------

<a name="setup-htc-vive" />

### How to set up my HTC Vive headset

- Install Steam and SteamVR and set up your headset (you should be able to see SteanVR home application running in your headset).
- Install Slicer and SlicerVirtualReality extension.
- To see content of content of the 3D view in your headset: click "Show scene in virtual reality" button <img src="https://github.com/KitwareMedical/SlicerVirtualReality/raw/master/VirtualReality/Resources/Icons/VirtualRealityHeadset.png" width="24"> on the toolbar in Slicer.

<a name="setup-windows-mixed-reality" />

### How to set up my Windows Mixed Reality headset

- Install Steam and SteamVR and set up your headset.
- Set up [Windows Mixed Reality for SteamVR](https://docs.microsoft.com/en-us/windows/mixed-reality/enthusiast-guide/using-steamvr-with-windows-mixed-reality) (you should be able to see SteanVR home application running in your headset).
- Install Slicer and SlicerVirtualReality extension.
- To see content of content of the 3D view in your headset: click "Show scene in virtual reality" button <img src="https://github.com/KitwareMedical/SlicerVirtualReality/raw/master/VirtualReality/Resources/Icons/VirtualRealityHeadset.png" width="24"> on the toolbar in Slicer.

<a name="setup-oculus-rift" />

### How to set up my Oculus Rift headset

- Install Steam and SteamVR and set up your headset to work with SteamVR.
- Install Slicer and SlicerVirtualReality extension.
- To see content of content of the 3D view in your headset:
  - Start Oculus app (put on the headset for a moment and it will be started)
  - Click the "Show scene in virtual reality" button <img src="https://github.com/KitwareMedical/SlicerVirtualReality/raw/master/VirtualReality/Resources/Icons/VirtualRealityHeadset.png" width="24"> on the toolbar in Slicer.

Currently, a limitation is that [Oculus Rift controllers are not supported](https://github.com/KitwareMedical/SlicerVirtualReality/issues/28), so the user can see and explore the scene in 3D but can only change the viewpoint by physically moving (not by flying using the controllers) and cannot grab and reposition objects.

### How to record virtual reality videos?

Enable screen mirroring in SteamVR and use the free of [OBS Studio](https://obsproject.com/) software to capture VR headset content, application window, webcam, etc.

Note that [OBS Studio](https://obsproject.com/) may crash if NVidia hardware-based compression is used. If this happens, choose CPU-based video compression option for recording.

For developers
--------------

**This Slicer extension is in active development. The API may change from version to version without notice.**

### Build instructions

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

### Useful Python Snippets

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

License
-------

It is covered by the Apache License, Version 2.0:

http://www.apache.org/licenses/LICENSE-2.0
