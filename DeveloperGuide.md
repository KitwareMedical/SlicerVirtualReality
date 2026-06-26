# Information for developers

This Slicer extension is in active development. The API may change from version to version without notice.

> **OpenVR is deprecated.** The legacy OpenVR backend (`SlicerVirtualReality_HAS_OPENVR_SUPPORT`) is still present in the codebase, but is no longer actively maintained, and will eventually be removed. This guide only documents the OpenXR backend.

## Controller event IDs

The tables below are current as of this writing; the `ControllerEvents` doc comment in [`vtkVirtualRealityViewOpenXRInteractorStyle.h`][vtkVirtualRealityViewOpenXRInteractorStyle-h-url] (and `SetupActions()`/`ProcessControllerEvents()` in the [`.cxx`][vtkVirtualRealityViewOpenXRInteractorStyle-cxx-url]) remains the authoritative source if this set changes and these tables fall out of sync.

### Controller Events

One event ID per physical control, always independently observable on the interactor (regardless of whether it is also translated by default to an action event):

| Controller event ID | Action name | Meta Quest control | Notes |
| --- | --- | --- | --- |
| `LeftGripPoseEvent` | `left_grip_pose` | Left grip pose | |
| `RightGripPoseEvent` | `right_grip_pose` | Right grip pose | |
| `LeftAimPoseEvent` | `left_aim_pose` | Left aim/ray pose | |
| `RightAimPoseEvent` | `right_aim_pose` | Right aim/ray pose | |
| `LeftGripValueEvent` | `left_grip_value` | Left grip/squeeze (analog) | Float action; never invoked (see `\warning` in the header) |
| `RightGripValueEvent` | `right_grip_value` | Right grip/squeeze (analog) | Float action; never invoked |
| `LeftGripClickEvent` | `left_grip_click` | Left grip/squeeze (digital) | By default translated to `PositionProp3DEvent` |
| `RightGripClickEvent` | `right_grip_click` | Right grip/squeeze (digital) | By default translated to `PositionProp3DEvent` |
| `LeftTriggerValueEvent` | `left_trigger_value` | Left trigger (analog) | Float action; never invoked |
| `RightTriggerValueEvent` | `right_trigger_value` | Right trigger (analog) | Float action; never invoked |
| `LeftTriggerClickEvent` | `left_trigger_click` | Left trigger (digital) | |
| `RightTriggerClickEvent` | `right_trigger_click` | Right trigger (digital) | |
| `LeftTriggerTouchEvent` | `left_trigger_touch` | Left trigger touch | |
| `RightTriggerTouchEvent` | `right_trigger_touch` | Right trigger touch | |
| `LeftThumbstickEvent` | `left_thumbstick` | Left thumbstick position | |
| `RightThumbstickEvent` | `right_thumbstick` | Right thumbstick position | By default translated to `ViewerMovement3DEvent` |
| `LeftThumbstickClickEvent` | `left_thumbstick_click` | Left thumbstick click | |
| `RightThumbstickClickEvent` | `right_thumbstick_click` | Right thumbstick click | |
| `LeftThumbstickTouchEvent` | `left_thumbstick_touch` | Left thumbstick touch | |
| `RightThumbstickTouchEvent` | `right_thumbstick_touch` | Right thumbstick touch | By default translated to `ViewerMovement3DEvent` |
| `LeftThumbrestTouchEvent` | `left_thumbrest_touch` | Left thumbrest touch | |
| `RightThumbrestTouchEvent` | `right_thumbrest_touch` | Right thumbrest touch | |
| `LeftButton1ClickEvent` | `left_button1_click` | X button | Also bound to `complexgestureaction` |
| `LeftButton1TouchEvent` | `left_button1_touch` | X button touch | |
| `LeftButton2ClickEvent` | `left_button2_click` | Y button | |
| `LeftButton2TouchEvent` | `left_button2_touch` | Y button touch | |
| `LeftMenuClickEvent` | `left_menu_click` | Left menu button | |
| `RightButton1ClickEvent` | `right_button1_click` | A button | Also bound to `complexgestureaction` |
| `RightButton1TouchEvent` | `right_button1_touch` | A button touch | |
| `RightButton2ClickEvent` | `right_button2_click` | B button | |
| `RightButton2TouchEvent` | `right_button2_touch` | B button touch | |
| `RightSystemClickEvent` | `right_system_click` | System button | |

### Action events

High-level actions that any controller even can be mapped to:

| Action event ID | Description |
| --- | --- |
| `vtk.vtkCommand.ViewerMovement3DEvent` | Fly/dolly movement (`vtkVRInteractorStyle::OnViewerMovement3D()` → `Movement3D()`) |
| `vtk.vtkCommand.PositionProp3DEvent` | Grab/move props (`OnPositionProp3D()` → `VTKIS_POSITION_PROP` → `vtkVirtualRealityViewInteractorStyleDelegate::StartPositionProp()`/`PositionProp()`/`EndPositionProp()`) |

[vtkVirtualRealityViewOpenXRInteractorStyle-h-url]: https://github.com/KitwareMedical/SlicerVirtualReality/blob/master/VirtualReality/MRMLDM/vtkVirtualRealityViewOpenXRInteractorStyle.h
[vtkVirtualRealityViewOpenXRInteractorStyle-cxx-url]: https://github.com/KitwareMedical/SlicerVirtualReality/blob/master/VirtualReality/MRMLDM/vtkVirtualRealityViewOpenXRInteractorStyle.cxx

To customize a control's behavior from Python, observe its `ControllerEvents` value directly and, if needed, forward it to a different VTK event — see the "Low-level event handling" snippet below.

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

### Controller event handling

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

# To override a button that is already used for some default action, we can set the abort flag.
# For example, here we take over the right hand joystick that is used for flying by default.

@vtk.calldata_type(vtk.VTK_OBJECT)
def onRightThumbStickEvent(caller, event, calldata):
    print(f"onRightThumbStickEvent received: {event}")
    print(f"WorldPosition: {calldata.GetWorldPosition()}")
    # Prevent further processing (e.g. to override the default grab/move behavior)
    interactor.GetCommand(rightThumbStickEventObserverTag).AbortFlagOn()
    interactor.GetCommand(rightThumbStickTouchEventObserverTag).AbortFlagOn()

rightThumbStickEventObserverTag = interactor.AddObserver(vtkSlicerVirtualRealityModuleMRMLDisplayableManager.vtkVirtualRealityViewOpenXRInteractorStyle.RightThumbstickEvent, onRightThumbStickEvent, highPriority)

rightThumbStickTouchEventObserverTag = interactor.AddObserver(vtkSlicerVirtualRealityModuleMRMLDisplayableManager.vtkVirtualRealityViewOpenXRInteractorStyle.RightThumbstickTouchEvent, onRightThumbStickEvent, highPriority)

# To assign a default action to another button, invoke the appropriate event on the interactor.
# Here we allow flying with the left joystick, in addition to the right one.
#
# Note: calldata cannot be passed as-is to interactor.InvokeEvent() from Python, because
# vtkObject::InvokeEvent()'s call data parameter is a void*, and Python cannot convert a wrapped
# vtkObject (such as our vtkEventDataDevice3D calldata) into a void* argument. Instead, use
# vtkSlicerVirtualRealityLogic.InvokeEvent(), a Python-friendly wrapper whose call data
# parameter is a proper vtkEventData*, avoiding that conversion problem.

@vtk.calldata_type(vtk.VTK_OBJECT)
def onLeftThumbStickEvent(caller, event, calldata):
    vrLogic.InvokeEvent(interactor, vtk.vtkCommand.ViewerMovement3DEvent, calldata)

vrLogic = slicer.modules.virtualreality.logic()
leftThumbStickEventObserverTag = interactor.AddObserver(vtkSlicerVirtualRealityModuleMRMLDisplayableManager.vtkVirtualRealityViewOpenXRInteractorStyle.LeftThumbstickEvent, onLeftThumbStickEvent, highPriority)
leftThumbStickTouchEventObserverTag = interactor.AddObserver(vtkSlicerVirtualRealityModuleMRMLDisplayableManager.vtkVirtualRealityViewOpenXRInteractorStyle.LeftThumbstickTouchEvent, onLeftThumbStickEvent, highPriority)

# Low-level interception also works for default VTK 3D events, not just raw ControllerEvents.
# Here we take over PositionProp3DEvent (grab/move props), driven by default by either grip's click/squeeze:

@vtk.calldata_type(vtk.VTK_OBJECT)
def onPositionProp3DEvent(caller, event, calldata):
    print(f"PositionProp3DEvent received: {event}")
    print(f"WorldPosition: {calldata.GetWorldPosition()}")
    # Prevent further processing (e.g. to override the default grab/move behavior)
    caller.GetCommand(positionPropObserverTag).AbortFlagOn()

positionPropObserverTag = interactor.AddObserver("PositionProp3DEvent", onPositionProp3DEvent, highPriority)
```

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

## Mapping of Controller action to Action event

The mapping process consists of several steps: the action manifest JSON file maps a controller-specific interaction path to a named action, the render window interactor maps that action to a VTK event, which is processed by the interactor style and may be further customized by style delegates. For low-level custom processing, it is also possible to intercept VTK events directly on the interactor.

### 1. Mapping from interaction path to action name

The OpenXR action manifest maps device-specific controls (joysticks, buttons, etc.) to device-independent control events. This should not be necessary to modify, as each available control is mapped to an event.

This module ships its own OpenXR action manifest, instead of using vtkRenderingOpenXR's stock one:

- [`VirtualReality/Resources/Bindings/vtk_openxr_actions.json`][vtk_openxr_actions_json_url] declares one action per physical control on the Oculus Touch (Meta Quest) controller — grip pose, grip squeeze/click, trigger, thumbstick, all buttons, etc.
- [`VirtualReality/Resources/Bindings/vtk_openxr_binding_oculus_touch_controller.json`][vtk_openxr_binding_oculus_touch_url] binds each of those actions to its physical [OpenXR interaction profile path](https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#semantic-path-interaction-profiles) (e.g. `/user/hand/right/input/squeeze`).

Both files are deployed under this module's own share directory (see `vtkSlicerVirtualRealityLogic::ComputeActionManifestPath()`), not vtkRenderingOpenXR's. Refer to the [Reserved Paths](https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#semantic-path-reserved) and [Interaction Profile Paths](https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#semantic-path-interaction-profiles) sections of the OpenXR spec for background on the path format.

[vtk_openxr_actions_json_url]: https://github.com/KitwareMedical/SlicerVirtualReality/blob/master/VirtualReality/Resources/Bindings/vtk_openxr_actions.json
[vtk_openxr_binding_oculus_touch_url]: https://github.com/KitwareMedical/SlicerVirtualReality/blob/master/VirtualReality/Resources/Bindings/vtk_openxr_binding_oculus_touch_controller.json

### 2. Mapping from action name to controller event

Every action declared in the manifest is registered with a dedicated event ID in `vtkVirtualRealityViewOpenXRInteractorStyle::ControllerEvents`, via [`vtkVirtualRealityViewOpenXRInteractorStyle::SetupActions()`][vtkVirtualRealityViewOpenXRInteractorStyle-cxx-url]. This means **every** physical control is independently observable on the interactor (e.g. `RightGripClickEvent`), regardless of whether it is also translated into a default VTK 3D event.

A subset of events is translated, by `ProcessControllerEvents()`, into a default action event (`ViewerMovement3DEvent`, `PositionProp3DEvent`, ...) to implement default behavior — e.g. the right thumbstick drives `ViewerMovement3DEvent` (fly/dolly movement), and either grip's click/squeeze drives `PositionProp3DEvent` (grab/move props).

#### Complex Gesture Support

Recognition of complex gesture events commences when the two controller buttons mapped to the `complexgestureaction` action are pressed simultaneously, handled by VTK's `vtkVRRenderWindowInteractor::HandleComplexGestureEvents()`. The SlicerVirtualReality implements its own heuristic on top of that by specializing `HandleComplexGestureEvents()` and `RecognizeComplexGesture()` in [`vtkVirtualRealityComplexGestureRecognizer`][vtkVirtualRealityComplexGestureRecognizer-url].

[vtkVirtualRealityComplexGestureRecognizer-url]: https://github.com/KitwareMedical/SlicerVirtualReality/blob/master/VirtualReality/MRMLDM/vtkVirtualRealityComplexGestureRecognizer.cxx

For OpenXR, `complexgestureaction` is bound to the left X button and the right A button in `vtk_openxr_binding_oculus_touch_controller.json` (in addition to those buttons' own `left_button1_click`/`right_button1_click` actions, which OpenXR allows binding to the same physical control), so pressing X and A simultaneously starts a complex gesture.

### Low-level interception of events

For implementing completely custom behavior, any VTK event — including the raw, per-control `ControllerEvents` that are always independently observable — can be intercepted on the render window interactor by adding a high-priority observer.

It is also possible to invoke a VTK event with event data from a Python observer callback, e.g. to assign a default action (like flying) to another control, by calling `slicer.modules.virtualreality.logic().InvokeEvent()`. This wrapper is necessary because `vtkObject::InvokeEvent(unsigned long, void*)`'s call data parameter is a `void*`, which Python cannot pass a wrapped `vtkObject` as; `InvokeEvent()`'s call data parameter is a proper `vtkEventData*`, avoiding that conversion problem.

## Related VTK modules

* [VTK::RenderingOpenXR](https://docs.vtk.org/en/latest/modules/vtk-modules/Rendering/OpenXR/README.html)
* [VTK::RenderingOpenXRRemoting](https://docs.vtk.org/en/latest/modules/vtk-modules/Rendering/OpenXRRemoting/README.html)
