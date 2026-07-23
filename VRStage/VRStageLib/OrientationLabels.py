"""R/L/A/P/S/I orientation labels for VR Stage.

The labels are authored directly in RAS/world coordinates (no UserMatrix), so they
track the anatomy's apparent rotation/placement automatically — the PhysicalToWorldMatrix
makes them move with the data, not the room.  ``updateScale`` must be called on every
physical-to-world change to keep their apparent height constant in physical space.
"""

import math

import vtk

from .Constants import (
    ORIENTATION_LABEL_AXES,
    ORIENTATION_LABEL_DEFAULT_RADIUS_MM,
    ORIENTATION_LABEL_FONT_SIZE,
    ORIENTATION_LABEL_HEIGHT_M,
    ORIENTATION_LABEL_MARGIN_MM,
)
from . import FramingMath
from . import Props


class OrientationLabels:
    """Collaborator owned by VRStageLogic — no back-reference.

    All environment reads (physical scale, data bounds) are passed as arguments.
    """

    def __init__(self):
        self.actors = {}

    def build(self, renderer, color, dataBounds, dataCenter) -> None:
        self.actors = {}
        camera = renderer.GetActiveCamera()
        for letter in ORIENTATION_LABEL_AXES:
            actor = Props.orientationLabelActor(camera, letter, color=color)
            renderer.AddViewProp(actor)
            self.actors[letter] = actor
        self.updatePositions(dataBounds, dataCenter)

    def teardown(self, renderer) -> None:
        if renderer is not None:
            for actor in self.actors.values():
                renderer.RemoveViewProp(actor)
        self.actors = {}

    def updatePositions(self, dataBounds, dataCenter) -> None:
        """Reposition labels around *dataBounds* / *dataCenter*."""
        if not self.actors:
            return
        radiusXY = 0.5 * max(
            FramingMath.extentAlongAxis(dataBounds, (1.0, 0.0, 0.0)),
            FramingMath.extentAlongAxis(dataBounds, (0.0, 1.0, 0.0)))
        radiusZ = 0.5 * FramingMath.extentAlongAxis(dataBounds, (0.0, 0.0, 1.0))
        if radiusXY <= 0.0 and radiusZ <= 0.0:
            radiusXY = radiusZ = ORIENTATION_LABEL_DEFAULT_RADIUS_MM
        else:
            radiusXY += ORIENTATION_LABEL_MARGIN_MM
            radiusZ += ORIENTATION_LABEL_MARGIN_MM
        for letter, axis in ORIENTATION_LABEL_AXES.items():
            actor = self.actors.get(letter)
            if actor is None:
                continue
            radius = radiusZ if axis[2] != 0.0 else radiusXY
            actor.SetPosition(
                dataCenter[0] + axis[0] * radius,
                dataCenter[1] + axis[1] * radius,
                dataCenter[2] + axis[2] * radius)

    def updateScale(self, physicalScale) -> None:
        """Keep the letters ORIENTATION_LABEL_HEIGHT_M tall in physical space.

        *physicalScale* is ``renderWindow.GetPhysicalScale()`` (world units per physical
        meter); pass 1000.0 when no VR render window is available (headless tests).
        """
        if not self.actors:
            return
        if not math.isfinite(physicalScale) or physicalScale <= 0.0:
            return
        scale = ORIENTATION_LABEL_HEIGHT_M * physicalScale / float(ORIENTATION_LABEL_FONT_SIZE)
        for actor in self.actors.values():
            actor.SetScale(scale, scale, scale)
