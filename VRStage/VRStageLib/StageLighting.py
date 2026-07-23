"""Overhead light rig for VR Stage.

Replaces the VR view's default lights with a room-anchored overhead rig so the table
reads as lit from the room's ceiling fixture.  The rig is anchored to physical space via
a shared *anchorMatrix* (the same vtkMatrix4x4 the chrome props use), and restores the
default lights on teardown.
"""

import math

import vtk

from .Constants import (
    FILL_LIGHT_ANGLES_DEG,
    FILL_LIGHT_HEIGHT_M,
    FILL_LIGHT_INTENSITY_FACTOR,
    FILL_LIGHT_RADIUS_M,
    OVERHEAD_LIGHT_HEIGHT_M,
    OVERHEAD_LIGHT_INTENSITY,
    TABLE_FORWARD_M,
    TABLE_PHYSICAL,
    _rgbF,
)
from . import Props


class StageLighting:
    """Collaborator owned by VRStageLogic — no back-reference.

    ``anchorMatrix`` is the shared vtkMatrix4x4 identity (created and mutated elsewhere);
    lights hold a live reference to it via ``SetTransformMatrix``.
    """

    def __init__(self):
        self.overheadLights = []
        self.defaultLights = []

    def build(self, renderer, anchorMatrix, color, enabled) -> None:
        """Build the overhead rig and capture the renderer's current (default) lights."""
        self.defaultLights = []
        lights = renderer.GetLights()
        if lights is not None:
            lights.InitTraversal()
            light = lights.GetNextItem()
            while light is not None:
                self.defaultLights.append(light)
                light = lights.GetNextItem()

        overheadColor = color if isinstance(color, tuple) else _rgbF(color)
        keyPosition = (0.0, OVERHEAD_LIGHT_HEIGHT_M, TABLE_FORWARD_M)
        key = Props.physicalLight(
            position=keyPosition, focalPoint=TABLE_PHYSICAL,
            color=overheadColor, intensity=OVERHEAD_LIGHT_INTENSITY)

        self.overheadLights = [key]
        for angleDeg in FILL_LIGHT_ANGLES_DEG:
            angleRad = math.radians(angleDeg)
            fillPosition = (
                TABLE_PHYSICAL[0] + FILL_LIGHT_RADIUS_M * math.sin(angleRad),
                FILL_LIGHT_HEIGHT_M,
                TABLE_PHYSICAL[2] + FILL_LIGHT_RADIUS_M * math.cos(angleRad))
            self.overheadLights.append(Props.physicalLight(
                position=fillPosition, focalPoint=TABLE_PHYSICAL,
                color=overheadColor, intensity=OVERHEAD_LIGHT_INTENSITY * FILL_LIGHT_INTENSITY_FACTOR))

        for light in self.overheadLights:
            light.SetTransformMatrix(anchorMatrix)
            renderer.AddLight(light)

        self.applyEnabled(renderer, enabled)

    def teardown(self, renderer) -> None:
        if renderer is not None:
            for light in self.overheadLights:
                renderer.RemoveLight(light)
            liveLights = renderer.GetLights()
            for light in self.defaultLights:
                if liveLights.IsItemPresent(light) == 0:
                    renderer.AddLight(light)
        self.overheadLights = []
        self.defaultLights = []

    def applyEnabled(self, renderer, enabled) -> None:
        """Live-toggle between the overhead rig and the VR view's default lights.

        The default lights are fully detached from the renderer (not just switched off) while
        the overhead rig is active, rather than relying on vtkLight.Switch.  They are
        positioned directly in world/RAS coordinates with no TransformMatrix, unlike our rig
        which is anchored to physical space; detaching avoids light-shift artifacts as the
        turntable rotates.
        """
        if renderer is None:
            return
        for light in self.overheadLights:
            light.SetSwitch(enabled)
        liveLights = renderer.GetLights()
        for light in self.defaultLights:
            isPresent = liveLights.IsItemPresent(light) != 0
            if enabled and isPresent:
                renderer.RemoveLight(light)
            elif not enabled and not isPresent:
                renderer.AddLight(light)

    def setColor(self, color) -> None:
        overheadColor = color if isinstance(color, tuple) else _rgbF(color)
        for light in self.overheadLights:
            light.SetColor(*overheadColor)
