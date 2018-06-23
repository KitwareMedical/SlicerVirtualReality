SlicerVirtualReality
====================

Extension for 3D slicer that enables user to interact with the 3D scene using virtual reality.

![](SlicerVirtualReality.png)

The extension works with all OpenVR-compatible headsets, such as [HTC Vive](#setup-htc-vive), all [Windows Mixed Reality headsets](#setup-windows-mixed-reality) (by Acer, Lenovo, HP, etc.), and with [some limitations on Oculus Rift](#setup-oculus-rift).

Features include:
- View all content of any of the 3D viewers in Slicer, anytime, by a single click.
- Show volumes as 2D image slices or volume rendering, render surfaces, points, etc.
- View any 4D data sets, using any rendering technique (including volume rendering) - provided by Sequences extension
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

Information for developers is available in the [Developer Guide](DeveloperGuide.md).

License
-------

It is covered by the Apache License, Version 2.0:

http://www.apache.org/licenses/LICENSE-2.0
