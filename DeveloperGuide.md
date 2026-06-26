# Information for developers

This Slicer extension is in active development. The API may change from version to version without notice.

> **OpenVR is deprecated.** The legacy OpenVR backend (`SlicerVirtualReality_HAS_OPENVR_SUPPORT`) is still present in the codebase, but is no longer actively maintained, and will eventually be removed. This guide only documents the OpenXR backend.

## Build instructions

- Build the extension against the newly built Slicer using the SuperBuild system.
- To start Slicer from a build tree and ensure the extension is properly loaded, consider running the `SlicerWithVirtualReality` launcher. For more details, see [here](https://slicer.readthedocs.io/en/latest/developer_guide/extensions.html#run-slicer-with-your-custom-modules).

### CMake build options

The top-level `CMakeLists.txt` exposes:

| CMake option | Default (Windows) | Default (macOS) | Description |
| --- | --- | --- | --- |
| `SlicerVirtualReality_HAS_OPENXR_SUPPORT` | `ON` | `OFF` | Build the OpenXR XR backend |
| `SlicerVirtualReality_HAS_OPENXRREMOTING_SUPPORT` | `ON` | `OFF` | Build OpenXR Remoting support (HoloLens 2) |

OpenXR Remoting is automatically disabled if `SlicerVirtualReality_HAS_OPENXR_SUPPORT` is `OFF`. It is only supported on Windows.

### Key classes

| Class | Location | Description |
| --- | --- | --- |
| `vtkMRMLVirtualRealityViewNode` | `VirtualReality/MRML/` | MRML node holding all VR view settings (backend, magnification, controller transforms, etc.) |
| `vtkSlicerVirtualRealityLogic` | `VirtualReality/Logic/` | Main logic class: activates/deactivates VR, manages the active view node, and sets up button bindings |
| `qMRMLVirtualRealityView` | `VirtualReality/Widgets/` | Qt widget that owns the VTK render window and interactor for the VR view |
| `vtkVirtualRealityViewOpenXRInteractorStyle` | `VirtualReality/MRMLDM/` | OpenXR interactor style: registers one event per physical controller action (`ControllerEvents`) and translates a curated subset into default VTK 3D events |
| `vtkVirtualRealityViewInteractorObserver` | `VirtualReality/MRMLDM/` | Bridges VTK VR interactor events to Slicer displayable managers |
| `vtkVirtualRealityViewInteractorStyleDelegate` | `VirtualReality/MRMLDM/` | Delegate implementing scene/object grab and gesture logic |
| `vtkVirtualRealityComplexGestureRecognizer` | `VirtualReality/MRMLDM/` | Slicer-specific two-controller gesture recognition (translate/rotate/scale) |
| `vtkMRMLVirtualRealityViewDisplayableManagerFactory` | `VirtualReality/MRMLDM/` | Singleton factory that registers displayable managers for the VR view |

## Mapping of Controller Action to VTK event

The mapping process consists of several steps: the action manifest JSON file maps a controller-specific interaction path to a named action, the render window interactor maps that action to a VTK event, which is processed by the interactor style and may be further customized by style delegates. For low-level custom processing, it is also possible to intercept VTK events directly on the interactor.

### 1. Mapping from interaction path to action name

This module ships its own OpenXR action manifest, instead of using vtkRenderingOpenXR's stock one:

- [`VirtualReality/Resources/Bindings/vtk_openxr_actions.json`][vtk_openxr_actions_json_url] declares one action per physical control on the Oculus Touch (Meta Quest) controller — grip pose, grip squeeze/click, trigger, thumbstick, all buttons, etc.
- [`VirtualReality/Resources/Bindings/vtk_openxr_binding_oculus_touch_controller.json`][vtk_openxr_binding_oculus_touch_url] binds each of those actions to its physical [OpenXR interaction profile path](https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#semantic-path-interaction-profiles) (e.g. `/user/hand/right/input/squeeze`).

Both files are deployed under this module's own share directory (see `vtkSlicerVirtualRealityLogic::ComputeActionManifestPath()`), not vtkRenderingOpenXR's. Refer to the [Reserved Paths](https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#semantic-path-reserved) and [Interaction Profile Paths](https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#semantic-path-interaction-profiles) sections of the OpenXR spec for background on the path format.

[vtk_openxr_actions_json_url]: https://github.com/KitwareMedical/SlicerVirtualReality/blob/master/VirtualReality/Resources/Bindings/vtk_openxr_actions.json
[vtk_openxr_binding_oculus_touch_url]: https://github.com/KitwareMedical/SlicerVirtualReality/blob/master/VirtualReality/Resources/Bindings/vtk_openxr_binding_oculus_touch_controller.json

### 2. Mapping from action name to VTK event

Every action declared in the manifest is registered with a dedicated event ID in `vtkVirtualRealityViewOpenXRInteractorStyle::ControllerEvents`, via [`vtkVirtualRealityViewOpenXRInteractorStyle::SetupActions()`][vtkVirtualRealityViewOpenXRInteractorStyle-cxx-url]. This means **every** physical control is independently observable on the interactor (e.g. `RightGripClickEvent`), regardless of whether it is also translated into a default VTK 3D event.

A curated subset is additionally translated, by `ProcessControllerEvents()`, into a default VTK 3D event (`ViewerMovement3DEvent`, `PositionProp3DEvent`, ...) to preserve/add the corresponding end-user behavior — e.g. the right thumbstick drives `ViewerMovement3DEvent` (fly/dolly movement), and either grip's click/squeeze drives `PositionProp3DEvent` (grab/move props). See the `ControllerEvents` doc comment in [`vtkVirtualRealityViewOpenXRInteractorStyle.h`][vtkVirtualRealityViewOpenXRInteractorStyle-h-url] for the authoritative, up-to-date list — including which events are deliberately *not* translated (and why, e.g. VTK's built-in 3D menu has no reliable way to dismiss it) — since this set evolves as the module changes and is not duplicated here to avoid drift.

[vtkVirtualRealityViewOpenXRInteractorStyle-h-url]: https://github.com/KitwareMedical/SlicerVirtualReality/blob/master/VirtualReality/MRMLDM/vtkVirtualRealityViewOpenXRInteractorStyle.h
[vtkVirtualRealityViewOpenXRInteractorStyle-cxx-url]: https://github.com/KitwareMedical/SlicerVirtualReality/blob/master/VirtualReality/MRMLDM/vtkVirtualRealityViewOpenXRInteractorStyle.cxx

To remap an action to a different event from Python, use `slicer.modules.virtualreality.logic().AddAction()` — see the "Low-level event handling" snippet below.

#### Complex Gesture Support

Recognition of complex gesture events commences when the two controller buttons mapped to the `complexgestureaction` action are pressed simultaneously, handled by VTK's `vtkVRRenderWindowInteractor::HandleComplexGestureEvents()`. The SlicerVirtualReality implements its own heuristic on top of that by specializing `HandleComplexGestureEvents()` and `RecognizeComplexGesture()` in [`vtkVirtualRealityComplexGestureRecognizer`][vtkVirtualRealityComplexGestureRecognizer-url].

[vtkVirtualRealityComplexGestureRecognizer-url]: https://github.com/KitwareMedical/SlicerVirtualReality/blob/master/VirtualReality/MRMLDM/vtkVirtualRealityComplexGestureRecognizer.cxx

For OpenXR, `complexgestureaction` is bound to the left X button and the right A button in `vtk_openxr_binding_oculus_touch_controller.json` (in addition to those buttons' own `left_button1_click`/`right_button1_click` actions, which OpenXR allows binding to the same physical control), so pressing X and A simultaneously starts a complex gesture.

### Low-level interception of events

For implementing completely custom behavior, the action → event mapping can be customized from Python (via `slicer.modules.virtualreality.logic().AddAction()`), and any VTK event — including the raw, per-control `ControllerEvents` that are always independently observable — can be intercepted on the render window interactor by adding a high-priority observer.

## Useful Python Snippets

### Activate virtual reality view

```python

import logging
import slicer

def isXRBackendInitialized():
    """Determine if XR backend has been initialized."""
    vrLogic = slicer.modules.virtualreality.logic()
    return vrLogic.GetVirtualRealityActive() if vrLogic else False

def vrCamera():
    # Get VR module widget
    if not isXRBackendInitialized():
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


assert isXRBackendInitialized() is False
assert vrCamera() is None

vrLogic = slicer.modules.virtualreality.logic()
vrLogic.SetVirtualRealityActive(True)

assert isXRBackendInitialized() is True
assert vrCamera() is not None

```

### Set virtual reality view background color to black:

```python

color = [0,0,0]
vrView=getNode('VirtualRealityView')
vrView.SetBackgroundColor(color)
vrView.SetBackgroundColor2(color)

```

### Set whether a node can be selected/grabbed/moved:

```python

nodeLocked.SetSelectable(0)
nodeMovable.SetSelectable(1)

```

### Low-level event handling

```python
# Get the render window interactor and its (OpenXR) interactor style
import vtkSlicerVirtualRealityModuleMRMLDisplayableManagerPython as vtkSlicerVirtualRealityModuleMRMLDisplayableManager
vrViewWidget = slicer.modules.virtualreality.viewWidget()
interactor = vrViewWidget.interactor()
interactorStyle = interactor.GetInteractorStyle()

# Use high priority observers to ensure we get to process the event before the interactor style (and we can prevent
# any further processing of the event)
highPriority = 100.0

# Every physical control fires its own raw ControllerEvents value independently, whether or not it is also
# translated into a default VTK 3D event. Observe the right grip's click/squeeze directly, with no remapping needed:

@vtk.calldata_type(vtk.VTK_OBJECT)
def onRightGripClickEvent(caller, event, calldata):
    print(f"RightTriggerClickEvent received, action={calldata.GetAction()}")

interactor.AddObserver(vtkSlicerVirtualRealityModuleMRMLDisplayableManager.vtkVirtualRealityViewOpenXRInteractorStyle.RightTriggerClickEvent, onRightGripClickEvent, highPriority)

# A curated subset of controller events is also translated into default VTK 3D events by
# vtkVirtualRealityViewOpenXRInteractorStyle::ProcessControllerEvents() (see its header for the up-to-date list).
# For example, both grips' click/squeeze are translated into PositionProp3DEvent (grab/move props).
# Here we take over that event:

@vtk.calldata_type(vtk.VTK_OBJECT)
def onPositionProp3DEvent(caller, event, calldata):
    print(f"PositionProp3DEvent received: {event}")
    print(f"WorldPosition: {calldata.GetWorldPosition()}")
    # Prevent further processing (e.g. to override the default grab/move behavior)
    caller.GetCommand(positionPropObserverTag).AbortFlagOn()

positionPropObserverTag = interactor.AddObserver("PositionProp3DEvent", onPositionProp3DEvent, highPriority)

# It is also possible to remap an action to a different event entirely. For example, the right A button
# (independently observable as RightButton1ClickEvent, but not translated into anything by default) can be
# rebound to also grab/move props, in addition to both grips:

slicer.modules.virtualreality.logic().AddAction(interactor, "right_button1_click", vtk.vtkCommand.PositionProp3DEvent, False)
```

## Related VTK modules

* [VTK::RenderingOpenXR](https://docs.vtk.org/en/latest/modules/vtk-modules/Rendering/OpenXR/README.html)
* [VTK::RenderingOpenXRRemoting](https://docs.vtk.org/en/latest/modules/vtk-modules/Rendering/OpenXRRemoting/README.html)
