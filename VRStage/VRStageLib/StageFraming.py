"""Stage framing transform model for VR Stage.

``StageFraming`` computes framing matrices; ``VRStageLogic`` applies them via
``_setRoomToWorld``.  All mutating controls take "current matrix in, new matrix out" so
the collaborator is free of render-window/renderer/fan-out knowledge and every control
is headless-testable.

Since right-stick locomotion was added, the matrices here are room->world: "room"
coordinates are the authored physical frame of Constants.py, and the window's actual
PhysicalToWorldMatrix additionally composes the physical->room locomotion offset
(``PhysicalToWorld = roomToWorld x L``, see StageLocomotion).  Attribute names keep the
original ``...PhysicalToWorld`` spelling because room coordinates ARE the authored
physical coordinates - only the live walked-about offset distinguishes them.
"""

import math

import vtk

from .Constants import (
    DEFAULT_MAGNIFICATION,
    MAX_MAGNIFICATION,
    MIN_MAGNIFICATION,
    TABLE_FORWARD_M,
    TABLE_HEIGHT_M,
    TABLE_HEIGHT_MAX_M,
    TABLE_HEIGHT_MIN_M,
    TABLE_RADIUS_M,
    TABLE_TOP_THICKNESS_M,
    UNIT_MAGNIFICATION_SCALE,
)
from . import FramingMath


class StageFraming:
    """Collaborator owned by VRStageLogic — no back-reference.

    Owns the pure framing state and all matrix-math helpers; Logic calls
    ``_setPhysicalToWorld`` to apply the resulting matrices.
    """

    def __init__(self):
        self.basePhysicalToWorld = None
        self.savedPhysicalToWorld = None
        self.magnification = DEFAULT_MAGNIFICATION
        self.fitRelScale = 1.0
        self.dataBounds = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
        self.dataCenter = [0.0, 0.0, 0.0]
        self.tableHeightM = TABLE_HEIGHT_M
        self.turntableAngleRad = 0.0

    def clear(self) -> None:
        self.basePhysicalToWorld = None
        self.savedPhysicalToWorld = None
        self.magnification = DEFAULT_MAGNIFICATION
        self.fitRelScale = 1.0
        self.dataBounds = [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]
        self.dataCenter = [0.0, 0.0, 0.0]
        self.tableHeightM = TABLE_HEIGHT_M
        self.turntableAngleRad = 0.0

    def captureBase(self, capturedMatrix) -> None:
        self.savedPhysicalToWorld = vtk.vtkMatrix4x4()
        self.savedPhysicalToWorld.DeepCopy(capturedMatrix)
        self.basePhysicalToWorld = FramingMath.alignedBaseMatrix(capturedMatrix)

    # ------------------------------------------------------------------ data collection

    def recomputeDataBounds(self) -> None:
        dataNodes = FramingMath.collectVisibleDataNodes()
        self.dataBounds = FramingMath.combinedRASBounds(dataNodes)
        self.dataCenter = FramingMath.combinedRASCenter(dataNodes)

    # ------------------------------------------------------------------ framing

    def computeFitRelScale(self) -> float:
        """World scale factor that makes the data's diagonal span roughly the table diameter."""
        if self.basePhysicalToWorld is None:
            return 1.0
        m = self.basePhysicalToWorld
        sf0 = (m.GetElement(0, 0) ** 2 + m.GetElement(1, 0) ** 2 + m.GetElement(2, 0) ** 2) ** 0.5
        b = self.dataBounds
        if b[0] > b[1] or sf0 < 1e-9:
            return 1.0
        diagonal = ((b[1] - b[0]) ** 2 + (b[3] - b[2]) ** 2 + (b[5] - b[4]) ** 2) ** 0.5
        if diagonal < 1e-6:
            return 1.0
        return (2.0 * TABLE_RADIUS_M * sf0) / diagonal

    def framingRelScale(self, magnification) -> float:
        """World relScale (relative to M0) that yields the given real-world magnification."""
        if self.basePhysicalToWorld is None:
            return 1.0
        return FramingMath.linearScale(self.basePhysicalToWorld) * magnification / UNIT_MAGNIFICATION_SCALE

    def tablePhysical(self):
        """Live table-top physical point — TABLE_PHYSICAL with the runtime height substituted."""
        return (0.0, self.tableHeightM + TABLE_TOP_THICKNESS_M, TABLE_FORWARD_M)

    def tableAxle(self, matrix):
        """(worldUp, axlePoint) for the given PTW matrix — the vertical turntable axle."""
        up = FramingMath.worldUp(matrix)
        p = self.tablePhysical()
        axle = list(matrix.MultiplyPoint([p[0], p[1], p[2], 1.0]))[:3]
        return up, axle

    def currentMagnification(self, currentMatrix) -> float:
        """True world magnification derived from the live matrix (reflects all sources)."""
        if currentMatrix is None:
            return self.magnification
        scale = FramingMath.linearScale(currentMatrix)
        return (UNIT_MAGNIFICATION_SCALE / scale) if scale > 1e-9 else self.magnification

    def resetFramingMatrix(self, fitToTable, defaultScale):
        """Recompute the data bounds, framing scale, table height, and return the
        framing matrix (or None if no base).  Also zeroes ``turntableAngleRad``."""
        self.recomputeDataBounds()
        if fitToTable:
            self.fitRelScale = self.computeFitRelScale()
        else:
            self.fitRelScale = self.framingRelScale(defaultScale)
        if self.basePhysicalToWorld is not None:
            self.tableHeightM = FramingMath.computeDefaultTableHeightM(
                self.basePhysicalToWorld, self.fitRelScale, self.dataBounds)
        if self.basePhysicalToWorld is None:
            return None
        yawRad = FramingMath.frontFacingYawRad(self.basePhysicalToWorld)
        matrix = FramingMath.computePhysicalToWorld(
            self.basePhysicalToWorld, self.fitRelScale, yawRad,
            self.dataBounds, self.dataCenter, self.tablePhysical())
        self.turntableAngleRad = 0.0
        return matrix

    # ------------------------------------------------------------------ incremental transforms

    def turntableMatrix(self, current, deltaRad):
        """Yaw about the table axle.  Accumulates ``turntableAngleRad``.

        Returns the incremental world-space transform (caller applies via
        ``_incrementalWorldTransform``).
        """
        if current is None:
            return None
        up, axle = self.tableAxle(current)
        self.turntableAngleRad += deltaRad
        return self._pivotMatrix(up, deltaRad, axle)

    def pitchMatrix(self, current, deltaRad, renderer):
        """Pitch about the camera's right-horizontal axis at the data center."""
        if current is None:
            return None
        right = self._cameraRightHorizontal(current, renderer)
        if right is None:
            return None
        return self._pivotMatrix(right, deltaRad, self.dataCenter)

    def rollMatrix(self, current, deltaRad, renderer):
        """Roll about the camera's forward-horizontal axis at the data center."""
        if current is None:
            return None
        forward = self._cameraForwardHorizontal(current, renderer)
        if forward is None:
            return None
        return self._pivotMatrix(forward, deltaRad, self.dataCenter)

    def tableVerticalMatrix(self, current, deltaM):
        """Translate the table vertically by *deltaM*, clamped.  Returns the
        incremental transform or None if no actual movement."""
        if current is None:
            return None
        newHeight = max(TABLE_HEIGHT_MIN_M, min(TABLE_HEIGHT_MAX_M, self.tableHeightM + deltaM))
        actualDelta = newHeight - self.tableHeightM
        if abs(actualDelta) < 1e-9:
            return None
        self.tableHeightM = newHeight
        up = FramingMath.worldUp(current)
        worldDelta = actualDelta * FramingMath.linearScale(current)
        t = vtk.vtkTransform()
        t.Translate(up[0] * worldDelta, up[1] * worldDelta, up[2] * worldDelta)
        w = vtk.vtkMatrix4x4()
        t.GetMatrix(w)
        return w

    def magnificationMatrix(self, current, targetMagnification):
        """Scale the world about the table axle to reach *targetMagnification*.

        Returns the incremental transform or None if no actual change.
        """
        if current is None:
            return None
        currentScale = self.currentMagnification(current)
        newScale = max(MIN_MAGNIFICATION, min(MAX_MAGNIFICATION, targetMagnification))
        if currentScale <= 0:
            return None
        factor = newScale / currentScale
        if abs(factor - 1.0) <= 1e-9:
            return None
        up, axle = self.tableAxle(current)
        t = vtk.vtkTransform()
        t.PostMultiply()
        t.Translate(-axle[0], -axle[1], -axle[2])
        t.Scale(factor, factor, factor)
        t.Translate(axle[0], axle[1], axle[2])
        w = vtk.vtkMatrix4x4()
        t.GetMatrix(w)
        return w

    # ------------------------------------------------------------------ internal helpers

    @staticmethod
    def _pivotMatrix(axis, deltaRad, pivot):
        t = vtk.vtkTransform()
        t.PostMultiply()
        t.Translate(-pivot[0], -pivot[1], -pivot[2])
        t.RotateWXYZ(vtk.vtkMath.DegreesFromRadians(deltaRad), axis[0], axis[1], axis[2])
        t.Translate(pivot[0], pivot[1], pivot[2])
        w = vtk.vtkMatrix4x4()
        t.GetMatrix(w)
        return w

    @staticmethod
    def _cameraForwardHorizontal(matrix, renderer):
        camera = renderer.GetActiveCamera() if renderer is not None else None
        if camera is None:
            return None
        position, focalPoint = camera.GetPosition(), camera.GetFocalPoint()
        forward = [focalPoint[i] - position[i] for i in range(3)]
        up = FramingMath.worldUp(matrix)
        d = vtk.vtkMath.Dot(forward, up)
        horizontal = [forward[i] - d * up[i] for i in range(3)]
        norm = vtk.vtkMath.Norm(horizontal)
        return [c / norm for c in horizontal] if norm > 1e-6 else None

    @staticmethod
    def _cameraRightHorizontal(matrix, renderer):
        forward = StageFraming._cameraForwardHorizontal(matrix, renderer)
        if forward is None:
            return None
        up = FramingMath.worldUp(matrix)
        right = [0.0, 0.0, 0.0]
        vtk.vtkMath.Cross(forward, up, right)
        norm = vtk.vtkMath.Norm(right)
        return [c / norm for c in right] if norm > 1e-6 else None
