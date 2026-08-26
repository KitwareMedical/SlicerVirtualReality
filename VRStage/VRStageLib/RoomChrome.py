"""Room chrome (floor, table, walls, monitors, readouts) for VR Stage.

The chrome is a set of raw VTK props (not MRML) anchored to physical space via two
shared ``vtkMatrix4x4`` instances — ``anchorMatrix`` (room-space props) and
``tableAnchorMatrix`` (table-mounted props with a runtime height offset).  These two
matrices are created once in ``__init__`` and **never reassigned** — props and lights
hold live references; mutations happen only via ``DeepCopy`` inside ``reanchor()``.
"""

import vtk

from slicer.i18n import tr as _

from .Constants import (
    COLLAR_SEAM_RING_INNER_FRAC,
    COLLAR_SEAM_RING_OUTER_FRAC,
    COLUMN_RADIUS_M,
    FLOOR_RADIUS_M,
    FLOOR_RING_INNER_M,
    FLOOR_RING_OUTER_M,
    FLOOR_THICKNESS_M,
    INFO_SCREEN_BORDER_FRAC,
    INFO_SCREEN_HEIGHT_M,
    INFO_SCREEN_LINE_GAP_M,
    INFO_SCREEN_LINE_HEIGHT_M,
    INFO_SCREEN_NAME_LINE_HEIGHT_M,
    INFO_SCREEN_NAME_MAX_WIDTH_M,
    INFO_SCREEN_TEXT_MARGIN_M,
    MONITOR_BEZEL_MARGIN_M,
    MONITOR_HINGE_ROTATION_DEG,
    MONITOR_HOUSING_DEPTH_M,
    MONITOR_MOUNT_PROUD_M,
    MONITOR_SCREEN_PROUD_M,
    MONITOR_SCREEN_WIDTH_M,
    MONITOR_TEXT_PROUD_M,
    RIM_BAND_HEIGHT_M,
    RIM_BAND_RADIUS_M,
    TABLE_FORWARD_M,
    TABLE_HEIGHT_M,
    TABLE_HEIGHT_MAX_M,
    TABLE_RADIUS_M,
    TABLE_SCREEN_EMISSIVE_FACTOR,
    TABLE_SCREEN_RADIUS_FRAC,
    TABLE_SCREEN_RECESS_DEPTH_M,
    TABLE_TOP_THICKNESS_M,
    _rgbF,
)
from . import Props


class RoomChrome:
    """Collaborator owned by VRStageLogic — no back-reference.

    Invariant: ``anchorMatrix`` and ``tableAnchorMatrix`` are created once and only ever
    mutated in-place (via ``DeepCopy`` in ``reanchor``).  Never reassign them — props and
    lights hold live references.
    """

    def __init__(self):
        self.anchorMatrix = vtk.vtkMatrix4x4()
        self.tableAnchorMatrix = vtk.vtkMatrix4x4()
        self.props = []
        self.tableScreenActor = None
        self.monitorAssembly = None
        self.scaleTextActor = None
        self.sceneViewTextActor = None
        self.signageBodyActor = None

    def build(self, renderer, params) -> None:
        """Create the room/floor/table/text props and add them to *renderer*.

        Wall tiles, orientation labels, and lights are NOT built here — they are
        sequenced by Logic's ``_buildStage``.
        """
        display = params.display
        tableCenterXZ = (0.0, TABLE_FORWARD_M)

        floorColor = _rgbF(display.floorColor)
        wallColor = _rgbF(display.wallColor)
        columnColor = _rgbF(display.columnColor)
        tableColor = _rgbF(display.tableColor)
        rimBandColor = _rgbF(display.rimBandColor)
        accentColor = _rgbF(display.accentColor)
        accentColorDim = _rgbF(display.accentColorDim)
        overheadLightColor = _rgbF(display.overheadLightColor)

        floorProps = []
        if display.showFloor:
            floor = Props.discActor(
                center=(0.0, FLOOR_THICKNESS_M / 2.0, 0.0),
                radius=FLOOR_RADIUS_M, height=FLOOR_THICKNESS_M, color=floorColor)
            floorGrid = Props.texturedDiscActor(
                center=(0.0, FLOOR_THICKNESS_M + 0.001, 0.0),
                radius=FLOOR_RADIUS_M, texture=Props.arrayToTexture(Props.floorPanelTexture(floorColor)),
                ambient=0.55, diffuse=0.35)
            floorRing = Props.glowRingActor(
                center=(tableCenterXZ[0], FLOOR_THICKNESS_M + 0.002, tableCenterXZ[1]),
                innerRadius=FLOOR_RING_INNER_M, outerRadius=FLOOR_RING_OUTER_M,
                color=accentColor)
            floorProps = [floor, floorGrid, floorRing]

        column = Props.discActor(
            center=(0.0, TABLE_HEIGHT_M - TABLE_HEIGHT_MAX_M / 2.0, TABLE_FORWARD_M),
            radius=COLUMN_RADIUS_M, height=TABLE_HEIGHT_MAX_M, color=columnColor)
        columnBand = Props.glowRingActor(
            center=(tableCenterXZ[0], TABLE_HEIGHT_M - RIM_BAND_HEIGHT_M - 0.10, tableCenterXZ[1]),
            innerRadius=0.0, outerRadius=COLUMN_RADIUS_M * 1.02,
            color=accentColorDim)

        collar = Props.discActor(
            center=(0.0, TABLE_HEIGHT_M - RIM_BAND_HEIGHT_M / 2.0, TABLE_FORWARD_M),
            radius=RIM_BAND_RADIUS_M, height=RIM_BAND_HEIGHT_M, color=rimBandColor)
        collar.GetProperty().SetMetallic(0.6)

        tableTopY = TABLE_HEIGHT_M + TABLE_TOP_THICKNESS_M
        tableScreenRadius = TABLE_RADIUS_M * TABLE_SCREEN_RADIUS_FRAC
        tableTopRing = Props.annulusActor(
            center=(0.0, tableTopY, TABLE_FORWARD_M),
            innerRadius=tableScreenRadius, outerRadius=TABLE_RADIUS_M,
            height=TABLE_TOP_THICKNESS_M, color=tableColor)
        tableTopRing.GetProperty().SetMetallic(1.0)
        tableTopRing.GetProperty().SetRoughness(0.2)
        tableTopRing.GetProperty().SetSpecular(0.5)
        tableTopRing.GetProperty().SetInterpolationToPBR()
        wellFloorHeight = TABLE_TOP_THICKNESS_M - TABLE_SCREEN_RECESS_DEPTH_M
        tableWellFloor = Props.discActor(
            center=(0.0, TABLE_HEIGHT_M + wellFloorHeight / 2.0, TABLE_FORWARD_M),
            radius=tableScreenRadius, height=wellFloorHeight, color=tableColor)
        tableWellFloor.GetProperty().SetMetallic(1.0)
        tableWellFloor.GetProperty().SetRoughness(0.5)
        tableWellFloor.GetProperty().SetInterpolationToPBR()

        self.tableScreenActor = None
        tableScreenProps = []
        if display.showTableScreen:
            tableScreenBg = _rgbF(display.tableScreenBackgroundColor)
            tableScreenTexture = Props.arrayToTexture(
                Props.tableScreenTexture(tableScreenBg, accentColorDim))
            self.tableScreenActor = Props.texturedDiscActor(
                center=(tableCenterXZ[0], TABLE_HEIGHT_M + wellFloorHeight + 0.002, tableCenterXZ[1]),
                radius=tableScreenRadius, texture=tableScreenTexture, ambient=0.85, diffuse=0.15)
            screenProp = self.tableScreenActor.GetProperty()
            screenProp.SetInterpolationToPBR()
            tableScreenTexture.UseSRGBColorSpaceOn()
            screenProp.SetBaseColorTexture(tableScreenTexture)
            screenProp.SetEmissiveTexture(tableScreenTexture)
            screenProp.SetEmissiveFactor(*TABLE_SCREEN_EMISSIVE_FACTOR)
            tableScreenProps = [self.tableScreenActor]

        collarSeamRing = Props.glowRingActor(
            center=(tableCenterXZ[0], tableTopY + 0.003, tableCenterXZ[1]),
            innerRadius=RIM_BAND_RADIUS_M * COLLAR_SEAM_RING_INNER_FRAC,
            outerRadius=RIM_BAND_RADIUS_M * COLLAR_SEAM_RING_OUTER_FRAC,
            color=accentColorDim)

        tableProps = [
            column, columnBand, collar,
            tableTopRing, tableWellFloor, collarSeamRing,
        ]
        tableProps.extend(tableScreenProps)
        roomProps = floorProps

        if display.showWalls:
            roomProps.append(Props.roomActor(
                wallColor, Props.arrayToTexture(Props.wallPanelTexture(wallColor))))
            roomProps.append(Props.ceilingLightActor(columnColor, overheadLightColor))
        self.signageBodyActor = None
        if display.showBackWallSignage:
            signage = Props.backWallSignageActors(display, params.controls)
            self.signageBodyActor = signage[-1]
            roomProps.extend(signage)

        if display.showInfoScreen:
            self.monitorAssembly = self._buildMonitorAssembly(display)
            tableProps.append(self.monitorAssembly)
        else:
            self.monitorAssembly = None
            self.scaleTextActor = None
            self.sceneViewTextActor = None

        for prop in tableProps:
            prop.SetUserMatrix(self.tableAnchorMatrix)
        for prop in roomProps:
            prop.SetUserMatrix(self.anchorMatrix)
        self.props = tableProps + roomProps
        for prop in self.props:
            renderer.AddViewProp(prop)

    def _buildMonitorAssembly(self, display):
        halfW = MONITOR_SCREEN_WIDTH_M / 2.0
        halfH = INFO_SCREEN_HEIGHT_M / 2.0
        housingHalfW = halfW + MONITOR_BEZEL_MARGIN_M
        housingHalfH = halfH + MONITOR_BEZEL_MARGIN_M

        hingeX = 0.0
        hingeY = TABLE_HEIGHT_M
        hingeZ = TABLE_FORWARD_M + RIM_BAND_RADIUS_M + MONITOR_MOUNT_PROUD_M

        housingCenterY = hingeY - housingHalfH
        housingCenterZ = hingeZ - MONITOR_HOUSING_DEPTH_M / 2.0
        housingShell = Props.boxActor(
            center=(hingeX, housingCenterY, housingCenterZ),
            size=(2.0 * housingHalfW, 2.0 * housingHalfH, MONITOR_HOUSING_DEPTH_M),
            color=_rgbF(display.rimBandColor))

        centerY = housingCenterY
        screenZ = hingeZ + MONITOR_SCREEN_PROUD_M

        panelSource = vtk.vtkPlaneSource()
        panelSource.SetOrigin(-halfW, -halfH, 0.0)
        panelSource.SetPoint1(halfW, -halfH, 0.0)
        panelSource.SetPoint2(-halfW, halfH, 0.0)
        mapper = vtk.vtkPolyDataMapper()
        mapper.SetInputConnection(panelSource.GetOutputPort())
        screenFace = vtk.vtkActor()
        screenFace.SetMapper(mapper)
        screenFace.SetTexture(Props.arrayToTexture(
            Props.signagePanelTexture(_rgbF(display.tableScreenBackgroundColor), _rgbF(display.accentColor),
                                       borderFrac=INFO_SCREEN_BORDER_FRAC)))
        screenFace.SetPosition(hingeX, centerY, screenZ)
        screenProp = screenFace.GetProperty()
        screenProp.SetColor(1.0, 1.0, 1.0)
        screenProp.SetAmbient(0.9)
        screenProp.SetDiffuse(0.1)
        screenFace.PickableOff()

        interiorHalfH = halfH * (1.0 - 2.0 * INFO_SCREEN_BORDER_FRAC)
        scaleBottomY = centerY + interiorHalfH - INFO_SCREEN_TEXT_MARGIN_M - INFO_SCREEN_LINE_HEIGHT_M
        viewBottomY = scaleBottomY - INFO_SCREEN_LINE_GAP_M - INFO_SCREEN_NAME_LINE_HEIGHT_M
        textZ = screenZ + MONITOR_TEXT_PROUD_M

        screenBgColor = _rgbF(display.tableScreenBackgroundColor)
        self.scaleTextActor = Props.textActor(
            position=(hingeX, scaleBottomY, textZ),
            heightMeters=INFO_SCREEN_LINE_HEIGHT_M, color=_rgbF(display.accentColor),
            bgColor=screenBgColor)

        self.sceneViewTextActor = Props.textActor(
            position=(hingeX, viewBottomY, textZ),
            heightMeters=INFO_SCREEN_NAME_LINE_HEIGHT_M, color=(0.75, 0.90, 0.95),
            bgColor=screenBgColor)

        assembly = vtk.vtkAssembly()
        for part in (housingShell, screenFace, self.scaleTextActor, self.sceneViewTextActor):
            assembly.AddPart(part)
        assembly.SetOrigin(hingeX, hingeY, hingeZ)
        assembly.SetOrientation(-MONITOR_HINGE_ROTATION_DEG, 0.0, 0.0)
        assembly.PickableOff()
        return assembly

    def teardown(self, renderer) -> None:
        if renderer is not None:
            for prop in self.props:
                renderer.RemoveViewProp(prop)
        self.props = []
        self.scaleTextActor = None
        self.sceneViewTextActor = None
        self.tableScreenActor = None
        self.monitorAssembly = None
        self.signageBodyActor = None

    def reanchor(self, physicalToWorldMatrix, tableHeightM) -> None:
        """DeepCopy *physicalToWorldMatrix* into ``anchorMatrix`` and derive
        ``tableAnchorMatrix`` with the runtime table-height offset, then mark
        every chrome prop modified.

        This is the single framing↔chrome seam; it is called from Logic's
        ``_setPhysicalToWorld`` on every matrix change.
        """
        if physicalToWorldMatrix is None:
            return
        self.anchorMatrix.DeepCopy(physicalToWorldMatrix)
        self.tableAnchorMatrix.DeepCopy(physicalToWorldMatrix)
        delta = tableHeightM - TABLE_HEIGHT_M
        for row in range(3):
            self.tableAnchorMatrix.SetElement(row, 3,
                physicalToWorldMatrix.GetElement(row, 3) + delta * physicalToWorldMatrix.GetElement(row, 1))
        for prop in self.props:
            prop.Modified()

    def setTurntableAngle(self, angleRad) -> None:
        """Spin the table screen's texture to match the accumulated turntable angle."""
        if self.tableScreenActor is not None:
            angleDeg = vtk.vtkMath.DegreesFromRadians(angleRad)
            self.tableScreenActor.SetOrientation(0.0, angleDeg, 0.0)

    def showScale(self, magnification) -> None:
        if self.scaleTextActor is not None:
            self.scaleTextActor.SetInput(
                _("{scale:.2f}x").format(scale=magnification))

    def showSceneViewName(self, name) -> None:
        """Show *name* on the info screen, truncating with an ellipsis if needed."""
        actor = self.sceneViewTextActor
        if actor is None:
            return
        scale = actor.GetScale()[0]

        def fits(text) -> bool:
            return actor.MeasureWidthPx(text) * scale <= INFO_SCREEN_NAME_MAX_WIDTH_M

        if fits(name):
            actor.SetInput(name)
            return
        truncated = name
        while len(truncated) > 1:
            truncated = truncated[:-1]
            if fits(truncated + "…"):
                actor.SetInput(truncated + "…")
                return
        actor.SetInput("…")

    def refreshSignageText(self, controls) -> None:
        if self.signageBodyActor is not None:
            self.signageBodyActor.SetInput(Props.controlSchemeBodyText(controls))
