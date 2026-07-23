"""In-VR point-to-point measurement tool (solo review aid).

Completed measurements are real vtkMRMLMarkupsLineNodes left in the scene on exit -
only the reticle is a raw VTK actor (pure aiming feedback, not user data).
"""

import math
import time

import vtk

import slicer

from .Constants import (
    ACCENT_COLOR,
    MEASURE_COLOR,
    MEASURE_FLASH_COLOR,
    MEASURE_FLASH_DURATION_S,
    MEASURE_GESTURE_SUPPRESS_WINDOW_S,
    MEASURE_RETICLE_RADIUS_MM,
)
from . import Props


class MeasurementTool:
    """Collaborator owned by VRStageLogic that encapsulates the measurement picker,
    reticle, pending/completed line nodes, and button1 debounce state."""

    def __init__(self):
        self._picker = None
        self._reticleActor = None
        self.currentHit = None
        self.pendingLineNode = None
        self.measurements = []
        self.flashRemaining = 0.0
        self.button1Held = {"Left": False, "Right": False}
        self.button1PressTime = {"Left": None, "Right": None}

    def setup(self, renderer) -> None:
        self._picker = vtk.vtkCellPicker()
        self._reticleActor = Props.worldGlowDotActor(MEASURE_RETICLE_RADIUS_MM, ACCENT_COLOR)
        renderer.AddViewProp(self._reticleActor)
        self.currentHit = None
        self.pendingLineNode = None
        self.measurements = []
        self.flashRemaining = 0.0
        self.button1Held = {"Left": False, "Right": False}
        self.button1PressTime = {"Left": None, "Right": None}

    def teardown(self, renderer) -> None:
        if renderer is not None and self._reticleActor is not None:
            renderer.RemoveViewProp(self._reticleActor)
        self.cancelPending()
        self._picker = None
        self._reticleActor = None
        self.currentHit = None
        self.measurements = []

    @staticmethod
    def isButton1PressSuppressed(now, otherHeld, otherPressTime, windowSeconds) -> bool:
        """True if the *other* hand's button1 was pressed and is still held within
        windowSeconds of `now` - keeps a deliberate two-hand free-gesture engagement
        (the built-in A+X combo) from also firing a spurious place/undo action."""
        return bool(otherHeld) and otherPressTime is not None and (now - otherPressTime) < windowSeconds

    def updateReticle(self, calldata, renderer) -> None:
        """Per-frame aim-ray pick. While a measurement is pending (first point placed),
        also drags its second control point to the current hit."""
        if renderer is None or self._picker is None or self._reticleActor is None:
            return
        try:
            pos = calldata.GetWorldPosition()
            ori = calldata.GetWorldOrientation()
        except Exception:  # noqa: BLE001
            return
        if not self._isFinitePickRay(pos, ori, renderer):
            return
        hit = self._picker.Pick3DRay(pos, ori, renderer)
        if hit:
            self.currentHit = tuple(self._picker.GetPickPosition())
            self._reticleActor.SetPosition(*self.currentHit)
            self._reticleActor.GetProperty().SetColor(*ACCENT_COLOR)
            self._reticleActor.VisibilityOn()
            if self.pendingLineNode is not None:
                self.pendingLineNode.SetNthControlPointPositionWorld(1, *self.currentHit)
        else:
            self.currentHit = None
            self._reticleActor.VisibilityOff()

    @staticmethod
    def _isFinitePickRay(pos, ori, renderer) -> bool:
        near, far = renderer.GetActiveCamera().GetClippingRange()
        if not (math.isfinite(near) and math.isfinite(far)) or far <= 0.0:
            return False
        return all(math.isfinite(v) for v in pos) and all(math.isfinite(v) for v in ori)

    def commitPoint(self, point) -> None:
        """First press creates a new Line markup; second press finalizes it."""
        if self.pendingLineNode is None:
            lineNode = slicer.mrmlScene.AddNewNodeByClass("vtkMRMLMarkupsLineNode", "VR Measurement")
            lineNode.CreateDefaultDisplayNodes()
            lineNode.AddControlPoint(point[0], point[1], point[2])
            lineNode.AddControlPoint(point[0], point[1], point[2])
            displayNode = lineNode.GetDisplayNode()
            if displayNode is not None:
                displayNode.SetColor(*MEASURE_COLOR)
                displayNode.SetSelectedColor(*MEASURE_COLOR)
                displayNode.PropertiesLabelVisibilityOn()
                displayNode.SetUseGlyphScale(False)
            self.pendingLineNode = lineNode
        else:
            self.pendingLineNode.SetNthControlPointPositionWorld(1, point[0], point[1], point[2])
            self.measurements.append(self.pendingLineNode)
            self.pendingLineNode = None

    def cancelPending(self) -> None:
        if self.pendingLineNode is not None:
            slicer.mrmlScene.RemoveNode(self.pendingLineNode)
            self.pendingLineNode = None

    def undoLast(self) -> None:
        """Undo priority: cancel pending first, then remove last completed, then flash."""
        if self.pendingLineNode is not None:
            self.cancelPending()
            return
        if self.measurements:
            slicer.mrmlScene.RemoveNode(self.measurements.pop())
            return
        self.flash()

    def flash(self) -> None:
        """Brief color flash on the reticle - feedback when there's nothing to act on."""
        self.flashRemaining = MEASURE_FLASH_DURATION_S
        if self._reticleActor is not None:
            self._reticleActor.GetProperty().SetColor(*MEASURE_FLASH_COLOR)
            self._reticleActor.VisibilityOn()

    def trackButton1(self, side, isPress) -> None:
        if isPress:
            self.button1Held[side] = True
            self.button1PressTime[side] = time.time()
        else:
            self.button1Held[side] = False

    def isPlaceSuppressed(self) -> bool:
        return self.isButton1PressSuppressed(
            time.time(), self.button1Held["Left"], self.button1PressTime["Left"],
            MEASURE_GESTURE_SUPPRESS_WINDOW_S)

    def isUndoSuppressed(self) -> bool:
        return self.isButton1PressSuppressed(
            time.time(), self.button1Held["Right"], self.button1PressTime["Right"],
            MEASURE_GESTURE_SUPPRESS_WINDOW_S)

    def hideReticle(self) -> None:
        """Hide the reticle and clear the current hit (used when a wall tile is hovered)."""
        self.currentHit = None
        if self._reticleActor is not None:
            self._reticleActor.VisibilityOff()

    def decayFlash(self, dt) -> None:
        """Decay the flash timer; call from the host's input timer."""
        if self.flashRemaining > 0.0:
            self.flashRemaining = max(0.0, self.flashRemaining - dt)
            if self.flashRemaining == 0.0 and self._reticleActor is not None:
                self._reticleActor.GetProperty().SetColor(*ACCENT_COLOR)
                if self.currentHit is None:
                    self._reticleActor.VisibilityOff()
