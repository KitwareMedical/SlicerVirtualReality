# Information for developers

This Slicer extension is in active development. The API may change from version to version without notice.

## Build instructions

- Build the extension against the newly built Slicer.
- To start Slicer from a build tree and ensure the extension is properly loaded, consider running the `SlicerWithVirtualReality` launcher. For more details, see [here](https://slicer.readthedocs.io/en/latest/developer_guide/extensions.html#run-slicer-with-your-custom-modules).

## Mapping of Controller Action to VTK event

The mapping process consists of two main steps:

1. Parsing the `vtk_open<vr|xr>_actions.json` action manifest file to link controller-specific interaction paths with generic event paths. This file references controller-specific binding files, usually named `vtk_open<vr|xr>_binding_<vendor_name>.json`, where each controller interaction path is associated with a VTK-specific event path.

2. Assigning a VTK event path to either a VTK event or a `std::function`. This association of a VTK event path involving a single controller with a VTK event is carried out in `vtkOpen<VR|XR>InteractorStyle::SetupActions()`.

### Action Manifest File

The controller interaction paths are specific to each backend:

* For OpenVR: Refer to the [List of common controller types](https://github.com/ValveSoftware/openvr/wiki/List-of-common-controller-types) and the [SteamVR Input Guide](https://github.com/OpenVR-Advanced-Settings/OpenVR-AdvancedSettings/blob/master/docs/SteamVRInputGuide.md).
* For OpenXR: Refer to the [Reserved Paths](https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#semantic-path-reserved) and the [Interaction Profile Paths](https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#semantic-path-interaction-profiles).

As of [Slicer@c7fe8657c](https://github.com/Slicer/Slicer/commit/c7fe8657c6a4bc0666685349b3222ff3c1b4fa02), the provided `vtk_open<vr|xr>_actions.json` and `vtk_open<vr|xr>_binding_<vendor_name>.json` files in the `vtkRenderingOpenVR` and `vtkRenderingOpenXR` VTK modules are as follow:

|                                 | OpenVR                                           | OpenXR                                                   |
| ------------------------------- | ------------------------------------------------ | -------------------------------------------------------- |
| Action manifest                 | [url][vtk_openvr_actions_json_url]               | [url][vtk_openxr_actions_json_url]                       |
| - HP Motion Controller          | [url][vtk_openvr_binding_hpmotioncontroller_url] | [url][vtk_openxr_binding_hp_mixed_reality_url]           |
| - HTC Vive Controller           | [url][vtk_openvr_binding_vive_controller_url]    | [url][vtk_openxr_binding_htc_vive_controller_url]        |
| - Microsoft Hand Interaction    |                                                  | [url][vtk_openxr_binding_microsoft_hand_interaction_url] |
| - Oculus Touch                  | [url][vtk_openvr_binding_oculus_touch_url]       | [url][vtk_openxr_binding_oculus_touch_url]               |
| - Valve Knuckles                | [url][vtk_openvr_binding_knuckles_url]           | [url][vtk_openxr_binding_knuckles_url]                   |
| - Khronos Simple Controller[^1] |                                                  | [url][vtk_openxr_binding_khr_simple_url]                 |

[^1]: https://registry.khronos.org/OpenXR/specs/1.0/html/xrspec.html#_khronos_simple_controller_profile

These files serve as essential references for mapping controller actions to VTK events.

<!-- vtkRenderingOpenVR -->
[vtk_openvr_actions_json_url]: https://github.com/Slicer/VTK/blob/slicer-v9.2.20230607-1ff325c54-2/Rendering/OpenVR/vtk_openvr_actions.json
[vtk_openvr_binding_hpmotioncontroller_url]: https://github.com/Slicer/VTK/blob/slicer-v9.2.20230607-1ff325c54-2/Rendering/OpenVR/vtk_openvr_binding_hpmotioncontroller.json
[vtk_openvr_binding_vive_controller_url]: https://github.com/Slicer/VTK/blob/slicer-v9.2.20230607-1ff325c54-2/Rendering/OpenVR/vtk_openvr_binding_vive_controller.json
[vtk_openvr_binding_knuckles_url]: https://github.com/Slicer/VTK/blob/slicer-v9.2.20230607-1ff325c54-2/Rendering/OpenVR/vtk_openvr_binding_knuckles.json
[vtk_openvr_binding_oculus_touch_url]: https://github.com/Slicer/VTK/blob/slicer-v9.2.20230607-1ff325c54-2/Rendering/OpenVR/vtk_openvr_binding_oculus_touch.json

<!-- vtkRenderingOpenXR -->
[vtk_openxr_actions_json_url]: https://github.com/Slicer/VTK/blob/slicer-v9.2.20230607-1ff325c54-2/Rendering/OpenXR/vtk_openxr_actions.json
[vtk_openxr_binding_hp_mixed_reality_url]: https://github.com/Slicer/VTK/blob/slicer-v9.2.20230607-1ff325c54-2/Rendering/OpenXR/vtk_openxr_binding_hp_mixed_reality.json
[vtk_openxr_binding_htc_vive_controller_url]: https://github.com/Slicer/VTK/blob/slicer-v9.2.20230607-1ff325c54-2/Rendering/OpenXR/vtk_openxr_binding_htc_vive_controller.json
[vtk_openxr_binding_microsoft_hand_interaction_url]: https://github.com/Slicer/VTK/blob/slicer-v9.2.20230607-1ff325c54-2/Rendering/OpenXR/vtk_openxr_binding_microsoft_hand_interaction.json
[vtk_openxr_binding_oculus_touch_url]: https://github.com/Slicer/VTK/blob/slicer-v9.2.20230607-1ff325c54-2/Rendering/OpenXR/vtk_openxr_binding_oculus_touch_controller.json
[vtk_openxr_binding_knuckles_url]: https://github.com/Slicer/VTK/blob/slicer-v9.2.20230607-1ff325c54-2/Rendering/OpenXR/vtk_openxr_binding_knuckles.json
[vtk_openxr_binding_khr_simple_url]: https://github.com/Slicer/VTK/blob/slicer-v9.2.20230607-1ff325c54-2/Rendering/OpenXR/vtk_openxr_binding_khr_simple_controller.json

### Mapping of VTK event path

The association of VTK event paths to VTK events hardcoded in each VTK modules is as follow:

* For OpenVR, refer to [vtkOpenVRInteractorStyle::SetupActions()][vtkOpenVRInteractorStyle-url]
* For OpenXR, refer to [vtkOpenXRInteractorStyle::SetupActions()][vtkOpenXRInteractorStyle-url]

[vtkOpenVRInteractorStyle-url]: https://github.com/Slicer/VTK/blob/slicer-v9.2.20230607-1ff325c54-2/Rendering/OpenXR/vtkOpenVRInteractorStyle.cxx
[vtkOpenXRInteractorStyle-url]: https://github.com/Slicer/VTK/blob/slicer-v9.2.20230607-1ff325c54-2/Rendering/OpenXR/vtkOpenXRInteractorStyle.cxx

### Complex Gesture Support

Recognition of complex gesture events commences when the two controller buttons mapped to the ComplexGesture action are pressed.

The SlicerVirtualReality implements its own heuristic by specializing the `HandleComplexGestureEvents()` and `RecognizeComplexGesture()` in the [vtkVirtualRealityComplexGestureRecognizer][vtkVirtualRealityComplexGestureRecognizer-url]  class.

[vtkVirtualRealityComplexGestureRecognizer-url]: https://github.com/KitwareMedical/SlicerVirtualReality/blob/master/VirtualReality/MRMLDM/vtkVirtualRealityComplexGestureRecognizer.cxx

Limitations:

* The selected controller buttons are exclusively mapped to the ComplexGesture action and cannot be associated with a regular action.

* To workaround an OpenVR specific [limitation](https://gitlab.kitware.com/vtk/vtk/-/merge_requests/10778), each button expected to be involved in the complex gesture needs to be respectively associated with `/actions/vtk/in/ComplexGestureAction` and `/actions/vtk/in/ComplexGestureAction_Event2`.

## Useful Python Snippets

Activate virtual reality view:

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

## Related VTK modules

* [VTK::RenderingOpenXR](https://docs.vtk.org/en/latest/modules/vtk-modules/Rendering/OpenXR/README.html)
* [VTK::RenderingOpenXRRemoting](https://docs.vtk.org/en/latest/modules/vtk-modules/Rendering/OpenXRRemoting/README.html)
* [VTK::RenderingOpenVR](https://docs.vtk.org/en/latest/modules/vtk-modules/Rendering/OpenVR/README.html)
