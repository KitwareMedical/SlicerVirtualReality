"""VRStageLogic - coordinator for VR Stage behavior.

The viewer is entirely non-destructive to the MRML scene: placement, scale and turntable
rotation are applied ONLY to the VR view, by overriding its PhysicalToWorldMatrix.  The
desktop 3D and 2D views therefore never move.  The math (computePhysicalToWorld and the
geometry helpers) is pure and covered by the headless test; the chrome/observer paths
require an active VR view.

The window's matrix is composed as ``PhysicalToWorld = roomToWorld x L``, where
``roomToWorld`` is the framing matrix (StageFraming — "room" coordinates are the authored
physical frame of Constants.py) and ``L`` is the right-stick locomotion offset
(StageLocomotion, physical->room).  Framing controls act on roomToWorld; room chrome is
anchored to roomToWorld; changing only L therefore walks the user through a world-fixed
room while the room, table and data stay put.

Behavioral clusters are delegated to collaborator classes in this package:
    RoomChrome, WallTileGallery, StageLighting, OrientationLabels,
    StageFraming, StageLocomotion, SceneViewNavigator, MeasurementTool, ReformatTool.
This file retains: lifecycle (enter/exit), options fan-out, the physical-to-world
pipeline, thin public controls, the input timer, observer registration
(VTKObservationMixin), and tool/wall-tile arbitration glue.
"""

import logging
import math

import vtk
import qt

import slicer
from slicer.i18n import tr as _
from slicer.ScriptedLoadableModule import ScriptedLoadableModuleLogic
from slicer.util import VTKObservationMixin

from .Constants import *  # noqa: F403
from .Constants import _rgbF, _WallTile
from .ParameterNode import VRStageParameterNode
from . import Props
from . import FramingMath
from .MeasurementTool import MeasurementTool
from .ReformatTool import ReformatTool
from .SceneViews import sceneViewsLogic, SceneViewNavigator
from .StageLighting import StageLighting
from .OrientationLabels import OrientationLabels
from .WallTiles import WallTileGallery
from .RoomChrome import RoomChrome
from .StageFraming import StageFraming
from .StageLocomotion import StageLocomotion
from . import LocomotionMath


class VRStageLogic(ScriptedLoadableModuleLogic, VTKObservationMixin):
    """Coordinator for VR Stage — lifecycle, options, observers, public controls.

    Collaborators are exposed as public attributes so tests can access them directly.
    """

    def __init__(self) -> None:
        ScriptedLoadableModuleLogic.__init__(self)
        VTKObservationMixin.__init__(self)
        self._parameterNode = None

        self.isActive = False

        # Runtime VR handles (only valid while active).
        self._interactor = None
        self._rightStickPosTag = None
        self._rightStickTouchTag = None
        self._physicalToWorldConnected = False
        self._installedBindings = None

        # Collaborators.
        self.roomChrome = RoomChrome()
        self.wallTiles = WallTileGallery()
        self.lighting = StageLighting()
        self.orientationLabels = OrientationLabels()
        self.framing = StageFraming()
        self.locomotion = StageLocomotion()
        self.sceneViews = SceneViewNavigator()
        self.reformatTool = ReformatTool()
        self.measurementTool = MeasurementTool()

        # Continuous inputs.
        self._leftStickX = 0.0
        self._leftStickY = 0.0
        self._rightStickX = 0.0
        self._rightStickY = 0.0
        self._autoSpin = False
        self._inputTimer = qt.QTimer()
        self._inputTimer.setInterval(INPUT_TIMER_INTERVAL_MS)
        self._inputTimer.timeout.connect(self._onInputTimer)

        # Saved VR navigation state, restored on exit.
        self._savedDolly = None
        self._savedGrab = None

        # Live option application.
        self._appliedOptions = None
        self._chromeRebuildPending = False

    def getParameterNode(self):
        parameterNode = super().getParameterNode()
        if not self._parameterNode or self._parameterNode.parameterNode != parameterNode:
            self._parameterNode = VRStageParameterNode(parameterNode)
        return self._parameterNode

    # ------------------------------------------------------------------ VR access

    @staticmethod
    def _vrLogic():
        try:
            return slicer.modules.virtualreality.logic()
        except Exception:  # noqa: BLE001
            return None

    @staticmethod
    def _vrViewWidget():
        try:
            return slicer.modules.virtualreality.viewWidget()
        except Exception:  # noqa: BLE001
            return None

    def _renderWindow(self):
        widget = self._vrViewWidget()
        return widget.renderWindow() if widget is not None else None

    def _vrViewNode(self):
        vrLogic = self._vrLogic()
        return vrLogic.GetVirtualRealityViewNode() if vrLogic else None

    def _vrRenderer(self):
        renderWindow = self._renderWindow()
        if renderWindow is None:
            return None
        renderers = renderWindow.GetRenderers()
        if renderers.GetNumberOfItems() < 1:
            return None
        return renderers.GetItemAsObject(0)

    # ------------------------------------------------------------------ enter/exit

    def enterViewerMode(self) -> None:
        """Activate VR (if needed), build the room, and install controls."""
        if self.isActive:
            vrLogic = self._vrLogic()
            if vrLogic and vrLogic.GetVirtualRealityActive() and self.roomChrome.props:
                return
            self.exitViewerMode()

        vrLogic = self._vrLogic()
        if vrLogic is None:
            raise RuntimeError(_("The VirtualReality module is not available."))

        self._applyPassthroughOption()
        vrLogic.SetVirtualRealityActive(True)

        widget = self._vrViewWidget()
        renderer = self._vrRenderer()
        viewNode = self._vrViewNode()
        if widget is None or renderer is None or viewNode is None:
            raise RuntimeError(_("VR view is not available. Is a headset connected?"))

        self._savedDolly = widget.isDolly3DEnabled()
        self._savedGrab = widget.isGrabObjectsEnabled()
        widget.setDolly3DEnabled(False)
        try:
            widget.setGestureButtonToNone()
        except Exception:  # noqa: BLE001
            pass

        slicer.app.processEvents()
        # L = identity before capturing, so the captured window matrix IS roomToWorld.
        self.locomotion.reset()
        capturedPhysicalToWorld = vtk.vtkMatrix4x4()
        widget.renderWindow().GetPhysicalToWorldMatrix(capturedPhysicalToWorld)
        self.framing.captureBase(capturedPhysicalToWorld)

        self.framing.magnification = DEFAULT_MAGNIFICATION
        self._autoSpin = False
        self._leftStickX = 0.0
        self._leftStickY = 0.0
        self._rightStickX = 0.0
        self._rightStickY = 0.0
        self.reformatTool.gripHeldSide = None
        self.reformatTool.visible = False
        self.framing.tableHeightM = TABLE_HEIGHT_M
        self.framing.turntableAngleRad = 0.0
        self.wallTiles.sceneViewPage = 0

        self._buildStage(renderer)

        params = self.getParameterNode()
        parameterNode = params
        matrix = self.framing.resetFramingMatrix(parameterNode.fitToTable, parameterNode.defaultScale)
        if matrix is not None:
            self._setRoomToWorld(matrix)
            self.roomChrome.setTurntableAngle(self.framing.turntableAngleRad)
            self._updateScaleReadout()

        for sliceId in SLICE_NODE_IDS:
            sliceNode = slicer.mrmlScene.GetNodeByID(sliceId)
            if sliceNode is not None:
                sliceNode.SetSliceVisible(False)

        display = params.display
        if display.enableReformatTool:
            self._setupReformatSlice(renderer)
        if display.enableMeasurementTool:
            self._setupMeasurements(renderer)

        self._installObservers(widget)
        self._inputTimer.start()

        self._appliedOptions = self._optionsSnapshot(params)
        self.isActive = True

    def exitViewerMode(self) -> None:
        """Tear everything down and restore the VR view. Safe when not active; idempotent."""
        if not self.isActive and not self.roomChrome.props and self.framing.basePhysicalToWorld is None:
            return

        self._inputTimer.stop()
        self._leftStickX = 0.0
        self._leftStickY = 0.0
        self._rightStickX = 0.0
        self._rightStickY = 0.0
        self.locomotion.reset()

        self._removeObservers()

        widget = self._vrViewWidget()
        if widget is not None:
            renderWindow = self._renderWindow()
            if renderWindow is not None and self.framing.savedPhysicalToWorld is not None:
                try:
                    renderWindow.SetPhysicalToWorldMatrix(self.framing.savedPhysicalToWorld)
                except Exception:  # noqa: BLE001
                    pass
            if self._savedDolly is not None:
                widget.setDolly3DEnabled(self._savedDolly)
            if self._savedGrab is not None:
                widget.setGrabObjectsEnabled(self._savedGrab)
        self._savedDolly = None
        self._savedGrab = None

        self._teardownReformatSlice()
        self._teardownMeasurements()
        self._teardownStage()

        self.framing.clear()
        self._appliedOptions = None
        self._chromeRebuildPending = False
        self.isActive = False

    def _buildStage(self, renderer) -> None:
        """Build all visual components: chrome, tiles, labels, lighting."""
        params = self.getParameterNode()
        display = params.display

        self.roomChrome.build(renderer, params)

        self.wallTiles.build(
            renderer, display,
            self.roomChrome.anchorMatrix, self.roomChrome.props,
            onActivateAtlas=self._activateAtlasTile,
            onRestoreSceneView=self._activateSceneViewTile,
            onPageRequested=self._activateSceneViewWallPage)

        accentColor = _rgbF(display.accentColor)
        if display.showOrientationLabels:
            self.orientationLabels.build(
                renderer, accentColor, self.framing.dataBounds, self.framing.dataCenter)

        overheadColor = _rgbF(display.overheadLightColor)
        self.lighting.build(
            renderer, self.roomChrome.anchorMatrix, overheadColor,
            params.overheadLight)

    def _teardownStage(self) -> None:
        renderer = self._vrRenderer()
        self.wallTiles.teardown(renderer)
        self.orientationLabels.teardown(renderer)
        self.roomChrome.teardown(renderer)
        self.lighting.teardown(renderer)

    # ------------------------------------------------------------------ parameter persistence

    _PARAM_FIELDS = (
        "rotationSpeedDegPerSec", "magnificationStep", "defaultScale",
        "fitToTable", "overheadLight", "passthrough",
    )
    _DISPLAY_FIELDS = (
        "accentColor", "accentColorDim", "floorColor", "wallColor", "columnColor",
        "tableColor", "rimBandColor", "tableScreenBackgroundColor", "overheadLightColor",
        "showFloor", "showWalls", "showBackWallSignage", "showTableScreen", "showInfoScreen",
        "showOrientationLabels", "showAtlasWall", "showSceneViewWall",
        "enableReformatTool", "enableMeasurementTool",
    )
    _CONTROL_FIELDS = (
        "scaleUp", "scaleDown", "nextSceneView", "prevSceneView", "resetFraming",
        "toggleReformatVisible", "toggleAutoSpin", "placeMeasurementPoint", "undoMeasurement",
    )

    def _snapshotUserSettings(self):
        """Capture all user-facing parameter values so they survive a scene clear."""
        p = self.getParameterNode()
        saved = {f: getattr(p, f) for f in self._PARAM_FIELDS}
        d = p.display
        saved["display"] = {f: (qt.QColor(getattr(d, f)) if isinstance(getattr(d, f), qt.QColor)
                                else getattr(d, f)) for f in self._DISPLAY_FIELDS}
        c = p.controls
        saved["controls"] = {f: getattr(c, f) for f in self._CONTROL_FIELDS}
        return saved

    def _restoreUserSettings(self, saved) -> None:
        """Restore user-facing parameters after a scene clear created a fresh parameter node."""
        p = self.getParameterNode()
        for f in self._PARAM_FIELDS:
            setattr(p, f, saved[f])
        d = p.display
        for f, v in saved["display"].items():
            setattr(d, f, v)
        c = p.controls
        for f, v in saved["controls"].items():
            setattr(c, f, v)

    # ------------------------------------------------------------------ options

    CHROME_OPTION_FIELDS = (
        "accentColor", "accentColorDim", "floorColor", "wallColor", "columnColor", "tableColor",
        "rimBandColor", "tableScreenBackgroundColor", "overheadLightColor",
        "showFloor", "showWalls", "showBackWallSignage", "showTableScreen", "showInfoScreen",
        "showOrientationLabels", "showAtlasWall", "showSceneViewWall",
    )
    TOOL_OPTION_FIELDS = ("enableReformatTool", "enableMeasurementTool")
    FRAMING_OPTION_FIELDS = ("fitToTable", "defaultScale")

    @classmethod
    def _optionsSnapshot(cls, params):
        """Pure helper (headless-testable): a plain dict of every option that needs an
        explicit rebuild/re-setup step to take effect."""
        display = params.display
        snapshot = {}
        for fieldName in cls.CHROME_OPTION_FIELDS + cls.TOOL_OPTION_FIELDS:
            value = getattr(display, fieldName)
            snapshot["display." + fieldName] = value.name() if isinstance(value, qt.QColor) else value
        for fieldName in cls.FRAMING_OPTION_FIELDS:
            snapshot[fieldName] = getattr(params, fieldName)
        return snapshot

    def applyOptions(self) -> None:
        """Apply every option live while the stage is active."""
        if not self.isActive:
            return
        self._refreshControlBindings()
        self._applyPassthroughOption()
        self._applyLightingOption()
        params = self.getParameterNode()
        overheadColor = _rgbF(params.display.overheadLightColor)
        self.lighting.setColor(overheadColor)

        snapshot = self._optionsSnapshot(params)
        previous = self._appliedOptions if self._appliedOptions is not None else {}
        changed = {key for key, value in snapshot.items() if previous.get(key) != value}
        self._appliedOptions = snapshot
        if not changed:
            return

        if any("display." + f in changed for f in self.CHROME_OPTION_FIELDS):
            self._scheduleChromeRebuild()

        renderer = self._vrRenderer()
        if "display.enableReformatTool" in changed and renderer is not None:
            if params.display.enableReformatTool:
                self._setupReformatSlice(renderer)
            else:
                self._teardownReformatSlice()
                self.reformatTool.visible = False
                self.reformatTool.gripHeldSide = None
        if "display.enableMeasurementTool" in changed and renderer is not None:
            if params.display.enableMeasurementTool:
                self._setupMeasurements(renderer)
            else:
                self._teardownMeasurements()

        if any(f in changed for f in self.FRAMING_OPTION_FIELDS):
            self._resetFraming()

    def _scheduleChromeRebuild(self) -> None:
        if self._chromeRebuildPending:
            return
        self._chromeRebuildPending = True
        qt.QTimer.singleShot(0, self._rebuildChrome)

    def _rebuildChrome(self, renderer=None) -> None:
        """Tear down and rebuild the room chrome in place from the current display options."""
        self._chromeRebuildPending = False
        if not self.isActive:
            return
        if renderer is None:
            renderer = self._vrRenderer()
        if renderer is None:
            return
        sceneViewWallPage = self.wallTiles.sceneViewPage
        self._teardownStage()
        self.wallTiles.sceneViewPage = sceneViewWallPage
        self._buildStage(renderer)
        self.roomChrome.reanchor(self._currentRoomToWorld(), self.framing.tableHeightM)
        self.wallTiles.markActorsModified()
        self._physicalScale(update=True)
        self.roomChrome.setTurntableAngle(self.framing.turntableAngleRad)
        self._updateScaleReadout()
        self._updateSceneViewReadout()

    # ------------------------------------------------------------------ physical-to-world pipeline

    @staticmethod
    def computePhysicalToWorld(baseMatrix, relScale, angleRad, dataBounds, dataCenter, tablePhysical):
        """Delegator — see FramingMath.computePhysicalToWorld."""
        return FramingMath.computePhysicalToWorld(
            baseMatrix, relScale, angleRad, dataBounds, dataCenter, tablePhysical)

    @staticmethod
    def computeDefaultTableHeightM(baseMatrix, relScale, dataBounds):
        """Delegator — see FramingMath.computeDefaultTableHeightM."""
        return FramingMath.computeDefaultTableHeightM(baseMatrix, relScale, dataBounds)

    def _currentPhysicalToWorld(self):
        """The raw window matrix (roomToWorld x L)."""
        renderWindow = self._renderWindow()
        if renderWindow is None:
            return None
        matrix = vtk.vtkMatrix4x4()
        renderWindow.GetPhysicalToWorldMatrix(matrix)
        return matrix

    def _currentRoomToWorld(self):
        """The framing matrix R = PhysicalToWorld x L^-1, recomputed on demand (never
        cached) so external PhysicalToWorld writers stay consistent with locomotion."""
        current = self._currentPhysicalToWorld()
        if current is None or self.locomotion.isIdentity():
            return current
        room = vtk.vtkMatrix4x4()
        vtk.vtkMatrix4x4.Multiply4x4(current, self.locomotion.inverseMatrix(), room)
        return room

    def _setWindowPhysicalToWorld(self, matrix) -> bool:
        """Set the raw window matrix (finiteness-gated). Returns False when rejected.

        The camera clipping range is reset after every change (not only scale changes):
        locomotion translates the camera relative to the world geometry, which can
        invalidate the range just as a scale change can.
        """
        renderWindow = self._renderWindow()
        if renderWindow is None:
            return False
        if not all(math.isfinite(matrix.GetElement(r, c)) for r in range(4) for c in range(4)):
            logging.warning("VRStage: ignoring non-finite PhysicalToWorld matrix")
            return False
        try:
            renderWindow.SetPhysicalToWorldMatrix(matrix)
        except Exception:  # noqa: BLE001
            logging.warning("VRStage: unable to set PhysicalToWorldMatrix")
        renderer = self._vrRenderer()
        if renderer is not None:
            renderer.ResetCameraClippingRange()
        return True

    def _setRoomToWorld(self, roomMatrix) -> None:
        """Apply a new framing matrix: compose with the locomotion offset into the
        window, then re-glue the room chrome to the room->world matrix."""
        window = vtk.vtkMatrix4x4()
        vtk.vtkMatrix4x4.Multiply4x4(roomMatrix, self.locomotion.matrix(), window)
        if not self._setWindowPhysicalToWorld(window):
            return
        self.roomChrome.reanchor(roomMatrix, self.framing.tableHeightM)
        self.wallTiles.markActorsModified()
        self._physicalScale(update=True)

    def _physicalScale(self, update=False) -> float:
        """Read the VR physical scale; optionally update orientation labels."""
        renderWindow = self._renderWindow()
        physicalScale = renderWindow.GetPhysicalScale() if renderWindow is not None else 1000.0
        if update:
            self.orientationLabels.updateScale(physicalScale)
        return physicalScale

    def _incrementalWorldTransform(self, worldMatrix) -> None:
        current = self._currentRoomToWorld()
        if current is None:
            return
        inverse = vtk.vtkMatrix4x4()
        vtk.vtkMatrix4x4.Invert(worldMatrix, inverse)
        newMatrix = vtk.vtkMatrix4x4()
        vtk.vtkMatrix4x4.Multiply4x4(inverse, current, newMatrix)
        self._setRoomToWorld(newMatrix)

    def _onPhysicalToWorldModified(self, caller=None, event=None) -> None:
        # Re-glue chrome to the room->world matrix (NOT the raw window matrix): this
        # signal also fires for external writers (e.g. the VirtualReality module's
        # desktop magnification control), which move room+data together relative to the
        # user - the walked-to offset must be preserved through that.
        self.roomChrome.reanchor(self._currentRoomToWorld(), self.framing.tableHeightM)
        self.wallTiles.markActorsModified()
        self._updateScaleReadout()
        self._physicalScale(update=True)

    def _resetFraming(self) -> None:
        # Recentering the user is part of the reset: cheap recovery after walking away.
        self.locomotion.reset()
        parameterNode = self.getParameterNode()
        matrix = self.framing.resetFramingMatrix(parameterNode.fitToTable, parameterNode.defaultScale)
        self.orientationLabels.updatePositions(self.framing.dataBounds, self.framing.dataCenter)
        if matrix is not None:
            self._setRoomToWorld(matrix)
            self.roomChrome.setTurntableAngle(self.framing.turntableAngleRad)
            self._updateScaleReadout()

    # ------------------------------------------------------------------ readout helpers

    def _updateScaleReadout(self) -> None:
        self.framing.magnification = self.framing.currentMagnification(self._currentRoomToWorld())
        self.roomChrome.showScale(self.framing.magnification)

    def _updateSceneViewReadout(self) -> None:
        logic = sceneViewsLogic()
        name = None
        idx = self.sceneViews.currentIndex
        if logic is not None and 0 <= idx < logic.GetNumberOfSceneViews():
            name = logic.GetNthSceneViewName(idx)
        self.roomChrome.showSceneViewName(name if name else _("(live scene)"))

    # ------------------------------------------------------------------ turntable / input

    def rotateTurntable(self, deltaRad) -> None:
        current = self._currentRoomToWorld()
        w = self.framing.turntableMatrix(current, deltaRad)
        if w is not None:
            self._incrementalWorldTransform(w)
            self.roomChrome.setTurntableAngle(self.framing.turntableAngleRad)

    def pitchTable(self, deltaRad) -> None:
        current = self._currentRoomToWorld()
        renderer = self._vrRenderer()
        w = self.framing.pitchMatrix(current, deltaRad, renderer)
        if w is not None:
            self._incrementalWorldTransform(w)

    def rollTable(self, deltaRad) -> None:
        current = self._currentRoomToWorld()
        renderer = self._vrRenderer()
        w = self.framing.rollMatrix(current, deltaRad, renderer)
        if w is not None:
            self._incrementalWorldTransform(w)

    def moveTableVertical(self, deltaM) -> None:
        current = self._currentRoomToWorld()
        w = self.framing.tableVerticalMatrix(current, deltaM)
        if w is not None:
            self._incrementalWorldTransform(w)

    def toggleAutoSpin(self) -> None:
        self._autoSpin = not self._autoSpin

    # ------------------------------------------------------------------ locomotion

    def _headPoseRoom(self):
        """(headRoom, forwardRoom): the headset position and horizontal gaze unit vector
        in room coordinates, or (None, None) when no finite head pose is available.

        Primary source is the HMD's device-to-physical matrix (the same API
        qMRMLVirtualRealityView uses to drive the HMD transform node), converted
        physical->room by the locomotion offset.  Fallback is the active camera (which
        tracks the HMD), mapped world->room by the inverse room->world matrix.
        """
        try:
            renderWindow = self._renderWindow()
            if renderWindow is not None:
                handle = renderWindow.GetDeviceHandleForDevice(
                    vtk.vtkEventDataDevice.HeadMountedDisplay)
                matrix = renderWindow.GetDeviceToPhysicalMatrixForDeviceHandle(handle)
                if matrix is not None:
                    position = [matrix.GetElement(i, 3) for i in range(3)]
                    forward = [-matrix.GetElement(i, 2) for i in range(3)]  # device -Z
                    if all(math.isfinite(c) for c in position + forward):
                        forwardRoom = LocomotionMath.horizontalUnit(
                            self.locomotion.roomFromPhysicalDirection(forward))
                        return self.locomotion.roomFromPhysicalPoint(position), forwardRoom
        except Exception:  # noqa: BLE001
            pass
        try:
            renderer = self._vrRenderer()
            roomToWorld = self._currentRoomToWorld()
            if renderer is None or roomToWorld is None:
                return None, None
            camera = renderer.GetActiveCamera()
            if camera is None:
                return None, None
            worldToRoom = vtk.vtkMatrix4x4()
            vtk.vtkMatrix4x4.Invert(roomToWorld, worldToRoom)
            position = list(worldToRoom.MultiplyPoint(list(camera.GetPosition()) + [1.0]))[:3]
            forward = list(worldToRoom.MultiplyPoint(
                list(camera.GetDirectionOfProjection()) + [0.0]))[:3]
            if not all(math.isfinite(c) for c in position + forward):
                return None, None
            return position, LocomotionMath.horizontalUnit(forward)
        except Exception:  # noqa: BLE001
            return None, None

    def _updateLocomotion(self, dt) -> None:
        """Advance the right-stick walk one input tick.

        Only the locomotion offset changes and the window matrix is recomposed;
        roomToWorld is untouched, so the chrome is deliberately NOT reanchored - the
        room, table and data stay world-fixed while the user moves through them.
        """
        if not self.isActive:
            return
        if math.hypot(self._rightStickX, self._rightStickY) <= THUMBSTICK_DEADZONE:
            return
        roomToWorld = self._currentRoomToWorld()  # before mutating the offset
        if roomToWorld is None:
            return
        headRoom, forwardRoom = self._headPoseRoom()
        if headRoom is None:
            return
        if not self.locomotion.update(dt, headRoom, forwardRoom,
                                      self._rightStickX, self._rightStickY):
            return
        window = vtk.vtkMatrix4x4()
        vtk.vtkMatrix4x4.Multiply4x4(roomToWorld, self.locomotion.matrix(), window)
        self._setWindowPhysicalToWorld(window)

    def _onInputTimer(self) -> None:
        dt = INPUT_TIMER_INTERVAL_MS / 1000.0

        x = self._leftStickX if abs(self._leftStickX) >= THUMBSTICK_DEADZONE else 0.0
        y = self._leftStickY if abs(self._leftStickY) >= THUMBSTICK_DEADZONE else 0.0
        if abs(x) >= abs(y):
            y = 0.0
        else:
            x = 0.0
        if x != 0.0 or y != 0.0:
            speedRad = vtk.vtkMath.RadiansFromDegrees(self.getParameterNode().rotationSpeedDegPerSec)
            if self.reformatTool.gripHeldSide == "Left":
                if x != 0.0:
                    self.rollTable(speedRad * x * dt)
                if y != 0.0:
                    self.pitchTable(speedRad * y * dt)
            else:
                if x != 0.0:
                    self.rotateTurntable(speedRad * x * dt)
                if y != 0.0:
                    self.moveTableVertical(TABLE_MOVE_SPEED_M_PER_S * y * dt)
        elif self._autoSpin:
            self.rotateTurntable(vtk.vtkMath.RadiansFromDegrees(AUTO_SPIN_DEG_PER_SEC) * dt)

        self._updateLocomotion(dt)

        self.measurementTool.decayFlash(dt)

    # ------------------------------------------------------------------ magnification

    def getMagnification(self) -> float:
        return self.framing.currentMagnification(self._currentRoomToWorld())

    @staticmethod
    def steppedMagnification(current, direction, stepFactor):
        """Delegator — see FramingMath.steppedMagnification."""
        return FramingMath.steppedMagnification(current, direction, stepFactor)

    def setMagnification(self, value) -> None:
        current = self._currentRoomToWorld()
        if current is None:
            self.framing.magnification = value
            return
        w = self.framing.magnificationMatrix(current, value)
        if w is not None:
            self._incrementalWorldTransform(w)
        self._updateScaleReadout()

    def stepMagnification(self, direction) -> None:
        stepFactor = self.getParameterNode().magnificationStep
        currentMag = self.framing.currentMagnification(self._currentRoomToWorld())
        self.setMagnification(self.steppedMagnification(currentMag, direction, stepFactor))

    def resetMagnification(self) -> None:
        self._resetFraming()

    @staticmethod
    def resetInteractionTransforms() -> None:
        """Reset all VR interaction transforms to identity.

        Iterates all transform nodes in the scene marked with the
        VirtualReality.InteractionTransform attribute and sets linear ones back
        to identity, returning grabbed objects to their original position.
        Non-linear transforms carrying the attribute are skipped.
        """
        scene = slicer.mrmlScene
        identityMatrix = vtk.vtkMatrix4x4()
        nodes = scene.GetNodesByClass("vtkMRMLTransformNode")
        nodes.UnRegister(None)
        for i in range(nodes.GetNumberOfItems()):
            transformNode = nodes.GetItemAsObject(i)
            if transformNode.GetAttribute("VirtualReality.InteractionTransform") is None:
                continue
            if not transformNode.IsLinear():
                logging.warning(
                    f"resetInteractionTransforms: Skipping non-linear transform node"
                    f" '{transformNode.GetName()}'")
                continue
            transformNode.SetMatrixTransformToParent(identityMatrix)

    # ------------------------------------------------------------------ arbitrary reformat slice

    def _setupReformatSlice(self, renderer) -> None:
        self.reformatTool.setup(renderer)

    def _teardownReformatSlice(self) -> None:
        self.reformatTool.teardown(self._vrRenderer())

    @vtk.calldata_type(vtk.VTK_OBJECT)
    def _onLeftGripClick(self, caller, event, calldata):
        self.reformatTool.onGripClick("Left", self._isPress(calldata), calldata)

    @vtk.calldata_type(vtk.VTK_OBJECT)
    def _onRightGripClick(self, caller, event, calldata):
        self.reformatTool.onGripClick("Right", self._isPress(calldata), calldata)

    @vtk.calldata_type(vtk.VTK_OBJECT)
    def _onLeftGripPose(self, caller, event, calldata):
        if self.reformatTool.gripHeldSide == "Left":
            self.reformatTool.trackPlaneToController(calldata)

    @vtk.calldata_type(vtk.VTK_OBJECT)
    def _onRightGripPose(self, caller, event, calldata):
        if self.reformatTool.gripHeldSide == "Right":
            self.reformatTool.trackPlaneToController(calldata)

    def toggleReformatVisible(self) -> None:
        self.reformatTool.toggleVisible()

    @vtk.calldata_type(vtk.VTK_OBJECT)
    def _onToggleReformatVisible(self, caller, event, calldata):
        if self._isPress(calldata):
            self.reformatTool.toggleVisible()

    # ------------------------------------------------------------------ measurement tool glue

    def _setupMeasurements(self, renderer) -> None:
        self.measurementTool.setup(renderer)

    def _teardownMeasurements(self) -> None:
        self.measurementTool.teardown(self._vrRenderer())

    def undoLastMeasurementAction(self) -> None:
        self.measurementTool.undoLast()

    @vtk.calldata_type(vtk.VTK_OBJECT)
    def _onRightAimPose(self, caller, event, calldata):
        try:
            pos, ori = calldata.GetWorldPosition(), calldata.GetWorldOrientation()
        except Exception:  # noqa: BLE001
            self.wallTiles.setHoveredTile(None)
            self.measurementTool.updateReticle(calldata, self._vrRenderer())
            return
        if self.wallTiles.pickTile(pos, ori, self._vrRenderer()) is not None:
            self.measurementTool.hideReticle()
            return
        self.measurementTool.updateReticle(calldata, self._vrRenderer())

    @vtk.calldata_type(vtk.VTK_OBJECT)
    def _onPlaceMeasurementPoint(self, caller, event, calldata):
        mt = self.measurementTool
        mt.trackButton1("Right", self._isPress(calldata))
        if not self._isPress(calldata):
            return
        if mt.isPlaceSuppressed():
            return
        if self.wallTiles.hoveredTile is not None:
            self.wallTiles.hoveredTile.onActivate()
            return
        if mt.currentHit is None:
            mt.flash()
            return
        mt.commitPoint(mt.currentHit)

    @vtk.calldata_type(vtk.VTK_OBJECT)
    def _onUndoMeasurement(self, caller, event, calldata):
        mt = self.measurementTool
        mt.trackButton1("Left", self._isPress(calldata))
        if not self._isPress(calldata):
            return
        if mt.isUndoSuppressed():
            return
        mt.undoLast()

    # ------------------------------------------------------------------ scene views

    def sceneViewCount(self) -> int:
        return self.sceneViews.count()

    def cycleSceneView(self, direction) -> None:
        nextIndex = self.sceneViews.wrappedIndex(direction)
        if nextIndex is not None:
            self._restoreSceneViewAtIndex(nextIndex)

    def _restoreSceneViewAtIndex(self, index) -> None:
        if self.sceneViews.restore(index):
            self.framing.recomputeDataBounds()
            self.orientationLabels.updatePositions(self.framing.dataBounds, self.framing.dataCenter)
            self._updateSceneViewReadout()

    def _activateSceneViewTile(self, index) -> None:
        self._restoreSceneViewAtIndex(index)

    def _activateSceneViewWallPage(self, page) -> None:
        self.wallTiles.sceneViewPage = page
        renderer = self._vrRenderer()
        if renderer is not None:
            display = self.getParameterNode().display
            self.wallTiles.rebuildSceneViewWall(
                renderer, display, self.roomChrome.anchorMatrix, self.roomChrome.props)

    def _activateAtlasTile(self, atlasSpec) -> None:
        """Download and load an atlas, keeping the VR experience across the swap."""
        wasActive = self.isActive
        savedSettings = self._snapshotUserSettings()
        if wasActive:
            self.exitViewerMode()
        try:
            import SampleData
            filenames = SampleData.downloadFromURL(
                fileNames=atlasSpec["fileNames"], uris=atlasSpec["uris"],
                checksums=atlasSpec["checksums"], loadFiles=False)
            slicer.util.loadScene(filenames[0], properties={"clear": True})
        except Exception:  # noqa: BLE001
            logging.exception("Failed to load atlas %s", atlasSpec.get("name"))
            slicer.util.errorDisplay(
                _("Failed to load {name}. Check your network connection and try again.")
                .format(name=atlasSpec["name"]))
        finally:
            self._restoreUserSettings(savedSettings)
            if wasActive:
                try:
                    self.enterViewerMode()
                except Exception:  # noqa: BLE001
                    logging.exception("Failed to re-enter VR Stage after atlas activation")

    # ------------------------------------------------------------------ lighting option

    def _applyPassthroughOption(self) -> None:
        viewNode = self._vrViewNode()
        if viewNode is None:
            return
        passthrough = self.getParameterNode().passthrough
        if viewNode.GetPassthrough() == passthrough:
            return
        if self.isActive:
            self.exitViewerMode()
            vrLogic = self._vrLogic()
            if vrLogic is not None:
                vrLogic.SetVirtualRealityActive(False)
            self.enterViewerMode()
        else:
            viewNode.SetPassthrough(passthrough)

    def _applyLightingOption(self) -> None:
        renderer = self._vrRenderer()
        if renderer is None:
            return
        enabled = self.getParameterNode().overheadLight
        self.lighting.applyEnabled(renderer, enabled)

    # ------------------------------------------------------------------ control bindings / observers

    def _refreshControlBindings(self) -> None:
        controls = self.getParameterNode().controls
        self.roomChrome.refreshSignageText(controls)
        if self._installedBindings is None or self._bindingSnapshot(controls) == self._installedBindings:
            return
        widget = self._vrViewWidget()
        if widget is None:
            return
        self._removeObservers()
        self._installObservers(widget)

    def _installObservers(self, widget) -> None:
        try:
            import vtkSlicerVirtualRealityModuleMRMLDisplayableManagerPython as vrDM
        except ImportError as e:
            raise RuntimeError(_("Could not import VR interactor style bindings.")) from e

        style = vrDM.vtkVirtualRealityViewOpenXRInteractorStyle
        interactor = widget.interactor()
        self._interactor = interactor
        highPriority = 100.0

        controls = self.getParameterNode().controls
        self._installedBindings = self._bindingSnapshot(controls)

        def addAction(fieldName, callback):
            label = getattr(controls, fieldName)
            if label == CONTROL_BINDING_UNBOUND:
                return
            eventName = CONTROL_BINDING_EVENT_NAMES[label]
            self.addObserver(interactor, getattr(style, eventName), callback, priority=highPriority)

        self.addObserver(interactor, style.LeftThumbstickEvent, self._onLeftThumbstick, priority=highPriority)
        self.addObserver(interactor, style.RightThumbstickEvent, self._onRightThumbstick, priority=highPriority)
        self.addObserver(interactor, style.RightThumbstickTouchEvent, self._onRightThumbstickTouch, priority=highPriority)
        _, self._rightStickPosTag, _ = self.getObserver(interactor, style.RightThumbstickEvent, self._onRightThumbstick)
        _, self._rightStickTouchTag, _ = self.getObserver(interactor, style.RightThumbstickTouchEvent, self._onRightThumbstickTouch)
        addAction("scaleUp", self._onScaleUp)
        addAction("scaleDown", self._onScaleDown)
        addAction("nextSceneView", self._onNextSceneView)
        addAction("prevSceneView", self._onPrevSceneView)
        addAction("resetFraming", self._onResetScale)
        addAction("toggleReformatVisible", self._onToggleReformatVisible)
        addAction("toggleAutoSpin", self._onToggleAutoSpin)
        self.addObserver(interactor, style.LeftGripClickEvent, self._onLeftGripClick, priority=highPriority)
        self.addObserver(interactor, style.RightGripClickEvent, self._onRightGripClick, priority=highPriority)
        self.addObserver(interactor, style.LeftGripPoseEvent, self._onLeftGripPose, priority=highPriority)
        self.addObserver(interactor, style.RightGripPoseEvent, self._onRightGripPose, priority=highPriority)
        self.addObserver(interactor, style.RightAimPoseEvent, self._onRightAimPose, priority=highPriority)
        addAction("placeMeasurementPoint", self._onPlaceMeasurementPoint)
        addAction("undoMeasurement", self._onUndoMeasurement)

        widget.connect("physicalToWorldMatrixModified()", self._onPhysicalToWorldModified)
        self._physicalToWorldConnected = True

    @staticmethod
    def _bindingSnapshot(controls):
        return {fieldName: getattr(controls, fieldName) for fieldName, _description in CONTROL_ACTION_ORDER}

    def _removeObservers(self) -> None:
        self.removeObservers()
        self._rightStickPosTag = None
        self._rightStickTouchTag = None
        self._interactor = None
        self._installedBindings = None

        widget = self._vrViewWidget()
        if widget is not None and self._physicalToWorldConnected:
            try:
                widget.disconnect("physicalToWorldMatrixModified()", self._onPhysicalToWorldModified)
            except Exception:  # noqa: BLE001
                pass
        self._physicalToWorldConnected = False

    @staticmethod
    def _isPress(calldata) -> bool:
        try:
            return calldata.GetAction() == vtk.vtkEventDataAction.Press
        except Exception:  # noqa: BLE001
            return True

    @vtk.calldata_type(vtk.VTK_OBJECT)
    def _onLeftThumbstick(self, caller, event, calldata):
        try:
            pos = calldata.GetTrackPadPosition()
            self._leftStickX = float(pos[0])
            self._leftStickY = float(pos[1])
        except Exception:  # noqa: BLE001
            self._leftStickX = 0.0
            self._leftStickY = 0.0

    def _abort(self, tag):
        if self._interactor is not None:
            command = self._interactor.GetCommand(tag)
            if command is not None:
                command.AbortFlagOn()

    @vtk.calldata_type(vtk.VTK_OBJECT)
    def _onRightThumbstick(self, caller, event, calldata):
        try:
            pos = calldata.GetTrackPadPosition()
            self._rightStickX = float(pos[0])
            self._rightStickY = float(pos[1])
        except Exception:  # noqa: BLE001
            self._rightStickX = 0.0
            self._rightStickY = 0.0
        self._abort(self._rightStickPosTag)

    @vtk.calldata_type(vtk.VTK_OBJECT)
    def _onRightThumbstickTouch(self, caller, event, calldata):
        self._abort(self._rightStickTouchTag)

    @vtk.calldata_type(vtk.VTK_OBJECT)
    def _onScaleUp(self, caller, event, calldata):
        if self._isPress(calldata):
            self.stepMagnification(+1)

    @vtk.calldata_type(vtk.VTK_OBJECT)
    def _onScaleDown(self, caller, event, calldata):
        if self._isPress(calldata):
            self.stepMagnification(-1)

    @vtk.calldata_type(vtk.VTK_OBJECT)
    def _onResetScale(self, caller, event, calldata):
        if self._isPress(calldata):
            self.resetMagnification()

    @vtk.calldata_type(vtk.VTK_OBJECT)
    def _onNextSceneView(self, caller, event, calldata):
        if self._isPress(calldata):
            self.cycleSceneView(+1)

    @vtk.calldata_type(vtk.VTK_OBJECT)
    def _onPrevSceneView(self, caller, event, calldata):
        if self._isPress(calldata):
            self.cycleSceneView(-1)

    @vtk.calldata_type(vtk.VTK_OBJECT)
    def _onToggleAutoSpin(self, caller, event, calldata):
        if self._isPress(calldata):
            self.toggleAutoSpin()
