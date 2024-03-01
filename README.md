# SlicerVirtualReality

Extension for 3D slicer that enables user to interact with the 3D scene using virtual reality.

![](SlicerVirtualReality.png)

The extension works with all OpenVR-compatible headsets, such as [HTC Vive](#setup-htc-vive), all [Windows Mixed Reality headsets](#setup-windows-mixed-reality) (by Acer, Lenovo, HP, etc.), and with [Oculus Rift](#setup-oculus-rift). See the YouTube video below or this [Kitware blog post](https://blog.kitware.com/slicervirtualreality/) for some more background and application examples.

[![Demo: Pedicle screw insertion in virtual reality using Slicer](https://img.youtube.com/vi/F_UBoE4FaoY/0.jpg)](https://www.youtube.com/watch?v=F_UBoE4FaoY)

## Features

Key features:

- View all content of any 3D viewer in Slicer with a single click.
- Display volumes as 2D image slices or volume renderings, render surfaces, points, etc.
- Visualize any 4D datasets using various rendering techniques (including volume rendering) provided by the Sequences extension.
- Align the headset's view to match the viewpoint of the selected 3D view in Slicer.
- Fly around using the touchpad of the right controller: direction is specified by orientation of the controller; speed is determined by the position of the finger on the touchpad (touch at the top to fly forward, touch at the bottom to fly backward).
- Grab and reposition objects using the controller's grab button.
- Translate, rotate, and scale the entire scene by pressing grab buttons on both controllers simultaneously.
- Advanced volume rendering performance tuning available in the Virtual Reality module to balance image quality and refresh rate.
- Make controller positions available as transforms in the Slicer scene. These transforms can be used in custom modules to reslice volumes (using Volume Reslice Driver module in SlicerIGT extension) or transform any nodes in the scene.

Continuous improvements are made to the feature set. Feedback and ideas for improvement can be submitted via the [issue tracker](https://github.com/KitwareMedical/SlicerVirtualReality/issues).

## Setup

**Platform support:** Currently the extension only works on Windows computers. Linux support is experimental: [Steam VR has limited support on linux](https://github.com/ValveSoftware/SteamVR-for-Linux/blob/master/README.md) and the Slicer extension is built for Linux but not tested. The extension is not available on macOS, as currently there are no virtual reality headsets available for macOS. If you wish to use Virtual Reality extension on Linux or macOS and you have virtual reality capable hardware and Steam VR works well on your computer then add a comment in the issue tracker ([macOS](https://github.com/KitwareMedical/SlicerVirtualReality/issues/3) / [Linux](https://github.com/KitwareMedical/SlicerVirtualReality/issues/57)).

**Configuring graphics:** If both integrated display card and high-performance GPU are available in a system (typically this is the case on laptops with NVidia GPUs), then configure the graphics card application settings to use high-performance GPU for `SlicerApp-real.exe` (it is not necessary to use high-performance GPU for the launcher, `Slicer.exe`).

<a name="setup-htc-vive" ></a>

### How to set up my HTC Vive headset

- Install [Steam](http://store.steampowered.com/about/) and [SteamVR](https://store.steampowered.com/steamvr) and set up your headset (you should be able to see SteanVR home application running in your headset).
- Install Slicer and SlicerVirtualReality extension.
- To see content of content of the 3D view in your headset: click "Show scene in virtual reality" button <img src="https://github.com/KitwareMedical/SlicerVirtualReality/raw/master/VirtualReality/Resources/Icons/VirtualRealityHeadset.png" width="24"> on the toolbar in Slicer.

<a name="setup-windows-mixed-reality" ></a>

### How to set up my Windows Mixed Reality headset

- Install Steam and SteamVR and set up your headset.
- Set up [Windows Mixed Reality for SteamVR](https://docs.microsoft.com/en-us/windows/mixed-reality/enthusiast-guide/using-steamvr-with-windows-mixed-reality) (you should be able to see SteanVR home application running in your headset).
- Install Slicer and SlicerVirtualReality extension.
- To see content of content of the 3D view in your headset: click "Show scene in virtual reality" button <img src="https://github.com/KitwareMedical/SlicerVirtualReality/raw/master/VirtualReality/Resources/Icons/VirtualRealityHeadset.png" width="24"> on the toolbar in Slicer.

<a name="setup-oculus-rift" ></a>

### How to set up my Oculus Rift headset

- Install Steam and SteamVR and set up your headset to work with SteamVR.
- Install Slicer and SlicerVirtualReality extension.
- To see content of content of the 3D view in your headset:
  - Start Oculus app (put on the headset for a moment and it will be started)
  - Click the "Show scene in virtual reality" button <img src="https://github.com/KitwareMedical/SlicerVirtualReality/raw/master/VirtualReality/Resources/Icons/VirtualRealityHeadset.png" width="24"> on the toolbar in Slicer.

<a name="controllers" ></a>

## How to use controllers

![HTC Vive Controller](ControllerHtcVive.jpg)

### Fly

Move around in space. Equivalent to physically walking around.

Controls:
- Touchpad forward: fly forward
- Touchpad backwad: fly backward

> [!NOTE]
> - Flying direction is specified by the orientation of the controller.
> - Speed is proportional to distance of the fingertip from the touchpad center.
> - Maximum speed is configurable in Virtual Reality module.

### Transform entire scene

Translate/rotate/scale the entire rendered scene.

Controls: while keeping grip button depressed on both controllers
- Move controllers closer together/farther apart: scale size of the entire scene
- Translate controllers in parallel up/down/left/right/forward/backward: translate the entire scene
- Pivot controllers around: rotate the entire scene

> [!NOTE]
> - Object positions in the scene are not modified.
> - Controllers must be outside of all selectable objects when grip buttons are pressed.

### Transform objects

Translate/rotate a selected object.

Controls: press grip button when a controller is inside a selectable object
- Move controllers closer together/farther apart: scale size of the entire rendered scene
- Translate controllers in parallel up/down/left/right/forward/backward: translate all objects
- Move controller

> [!NOTE]
> - When you grab and move object, a parent transform is automatically created for it (if it has not been under a transform already) and that transform is modified.
> - To move a group of objects together, assign the same parent transform to them. You can do that in _Data_ module's _Transform hierarchy_ tab by drag-and-dropping objects under the same transform (or by double-clicking in the _Applied Transform_ column in  _Data_ module's _Transform hierarchy_ tab and selecting a transform; or by selecting a transform in _Transforms_ module and applying it to all the nodes that must move together).
> - Either left or right controller can be used to grab an object. Each controller can be used to grab an object and move independently.
> - By default all objects are selectable. An object can be made non-selectable (thus non-movable) in Data module / Subject hierarchy tab, right-clicking on the node and unchecking "Toggle Selectable".
> - Moving of segmentation nodes is slow. If you want to move segmentations using controllers then export them to model nodes (in _Data_ module, right-click on the segmentation node and choose _Export visible segments to models_) and transform the model nodes.

## Other features

### Accessing VR transforms (controller, headset, generic trackers) in Slicer

Go to _Virtual Reality_ module and check the desired checkbox to update linear transform nodes with the various devices' positions. *Note:* The magnification factor in advanced settings affects these transforms.

## Frequently asked questions

### How to clip models

3D Slicer can clip models using slice planes. This feature can be used in virtual reality by moving the slice plane using a transform, with the help of _Volume reslice driver_ module.

The models, transforms, etc. only need to be set up once, because the scene can then be saved to file and next time it can be readily used.

#### Simple clipping

Move clipping planes continuously, as the controllers are moved.

- Go to _Models_ module, select the model to clip, and in _Clipping_ section enable _Clip selected model_. By default the _Red_ slice will be chosen for clipping.
- Go to _Virtual Reality_ module and enable making controller transform appear as transforms in the scene.
- Go to _Volume reslice driver_ module (in _SlicerIGT_ extension) to make the controller transform move the _Red_ slice.
- Moved the controller inside the model to clip it

#### Clipping with handles

Grab and move clipping planes using controllers (clipping plane remains in place).

- Go to _Models_ module, select the model to clip, and in _Clipping_ section enable _Clip selected model_. By default the _Red_ slice will be chosen for clipping.
- Add a small model, for example a box to the scene. It can be loaded from STL or created using _Create models_ module (in _SlicerIGT_ extension).
- Move the model using a controller. This creates a parent transform for this model.
- Go to _Volume reslice driver_ module (in _SlicerIGT_ extension) to make the model's parent transform move the _Red_ slice.
- Grab and move the model using the controller to move the clipping plane.

### Rendering is slow

There are several settings that help in increasing the performance of virtual reality rendering:

- If you are using a computer that has two graphics cards (for example, laptops often have an integrated Intel and a high-performance NVidia graphics card), make sure that Slicer is forced to use the high-performance card that the headset is connected to. Most laptops assign applications to the integrated card by default. When you need to select the application executable, choose `SlicerApp-real.exe` (and not the Slicer launcher application `Slicer.exe`).
- Optimize scene for virtual reality button (magic wand icon on toolbar): this switches volume rendering to use GPU, turns off backface culling for all existing models (to see surfaces even when going inside an object), turns off slice intersection visibility for all existing models and segmentations (to make slice view updates faster)
- Settings in virtual reality module panel related to performance (click on wrench icon in toolbar):
  - Update rate: Volume rendering quality is set to produce the highest possible quality while keeping the desired frame per second
  - Motion sensitivity: It is very important to keep rendering smooth when moving. This setting detects head movement and significantly lowers volume rendering quality while it is happening. At value of 0 motion is never detected, at high values a little motion triggers the quality change
- Settings in Volume rendering module: open "Advanced" section / "Techniques" tab, try "Adaptive" setting with different "Interactive speed" values. Also try and "Normal" setting: it disables the automatic mechanism that tries to dynamically adjust rendering quality based on predicted rendering time (in some cases the prediction does not work well and results in sub-optimal image quality).
- Switch layout of desktop Slicer: Any additional 3D view to render decreases virtual reality rendering performance. By switching to a layout that contains no 3D view (e.g. Red slice only), this can be prevented

Some scenes are too complex to render fluently by mid-range graphics cards. If the scene includes volume rendering of high-resolution CT for example, then it may be necessary to upgrade to a high-end GPU.

### How to record virtual reality videos?

Enable screen mirroring in SteamVR and use the free of [OBS Studio](https://obsproject.com/) software to capture VR headset content, application window, webcam, etc.

Note that [OBS Studio](https://obsproject.com/) may crash if NVidia hardware-based compression is used. If this happens, choose CPU-based video compression option for recording.

### How to ask questions, report problems, or suggest new features?

Visit [Slicer forum](https://discourse.slicer.org) and search for similar discussions. If you do not find related topics then createa a new one. Add _virtual-reality_ tag to make sure people who monitor virtual reality related questions get a notification about your question.

If you are certain that you have found a software bug and no similar issue has been reported in the [issue tracker](https://github.com/KitwareMedical/SlicerVirtualReality/issues)) then please submit a new issue.

Please do not use "VR" acronym (you can spell out "virtual reality" instead), because "VR" may mean "volume rendering" just as well as "virtual reality" - you can even do volume rendering in virtual reality in Slicer - and so it becomes confusing very quickly.

## For developers

Information for developers is available in the [Developer Guide](DeveloperGuide.md).

## Contributors

Contributors include:
- Kitware: Jean-Christophe Fillion-Robin, Jean-Baptiste Vimort
- PerkLab (Queen's University): Csaba Pinter, Andras Lasso
- VASST Lab (Robarts Research Insitute): Adam Rankin

## How to cite

Pinter, C., Lasso, A., Choueib, S., Asselin, M., Fillion-Robin, J. C., Vimort, J. B., Martin, K., Jolley, M. A. & Fichtinger, G. (2020). SlicerVR for Medical Intervention Training and Planning in Immersive Virtual Reality. IEEE Transactions on Medical Robotics and Bionics, vol. 2, no. 2, pp. 108-117, May 2020, doi: 10.1109/TMRB.2020.2983199

## License

It is covered by the Apache License, Version 2.0:

http://www.apache.org/licenses/LICENSE-2.0
