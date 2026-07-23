import qt
import vtk
import slicer
import VRStage
from VRStageLib import FramingMath, Props
from VRStageLib.MeasurementTool import MeasurementTool

# Headless tests for VRStageLogic: everything that does not require a headset.
#
# WARNING: this test clears the MRML scene. Run it only in an isolated Slicer (ctest),
# never by exec'ing it into a live working session.

logic = VRStage.VRStageLogic()


def _addVisibleModel(name, center):
    sphere = vtk.vtkSphereSource()
    sphere.SetCenter(center)
    sphere.SetRadius(10.0)
    sphere.Update()
    modelNode = slicer.modules.models.logic().AddModel(sphere.GetOutput())
    modelNode.SetName(name)
    modelNode.GetDisplayNode().SetVisibility(True)
    return modelNode


# ---------------------------------------------------------------- pure helpers

# Multiplicative magnification stepping, clamped to [MIN, MAX].
assert abs(logic.steppedMagnification(1.0, +1, 1.25) - 1.25) < 1e-9
assert abs(logic.steppedMagnification(1.0, -1, 1.25) - 0.8) < 1e-9
assert logic.steppedMagnification(VRStage.MAX_MAGNIFICATION, +1, 2.0) == VRStage.MAX_MAGNIFICATION
assert logic.steppedMagnification(VRStage.MIN_MAGNIFICATION, -1, 2.0) == VRStage.MIN_MAGNIFICATION
print("steppedMagnification: OK")

# Extent of an AABB along an axis: a 20x40x60 box has vertical (Z) extent 60.
assert abs(FramingMath.extentAlongAxis([-10, 10, -20, 20, -30, 30], [0, 0, 1]) - 60.0) < 1e-9
assert FramingMath.extentAlongAxis([0, -1, 0, 0, 0, 0], [0, 0, 1]) == 0.0  # empty
print("extentAlongAxis: OK")

# computePhysicalToWorld with M0 = identity: the data center should appear at the table
# physical location, offset by the fixed TABLE_LIFT_BUFFER_MM along "up" (zero-extent data,
# so that's the only offset) - same invariant the in-module VRStageTest.test_VRStageLogic1
# checks.
identity = vtk.vtkMatrix4x4()
dataCenter = [10.0, 20.0, 30.0]
emptyBounds = [0.0, -1.0, 0.0, -1.0, 0.0, -1.0]
up = FramingMath.worldUp(identity)
expectedCenter = [dataCenter[a] - up[a] * VRStage.TABLE_LIFT_BUFFER_MM for a in range(3)]
m = logic.computePhysicalToWorld(identity, 1.0, 0.0, emptyBounds, dataCenter, VRStage.TABLE_PHYSICAL)
mapped = m.MultiplyPoint([VRStage.TABLE_PHYSICAL[0], VRStage.TABLE_PHYSICAL[1], VRStage.TABLE_PHYSICAL[2], 1.0])
for a in range(3):
    assert abs(mapped[a] - expectedCenter[a]) < 1e-4, (a, mapped[a], expectedCenter[a])

# Placement invariant holds at any rotation angle (data center stays on the table point) -
# rotation is about "up", so the same lift-adjusted expectation applies unchanged.
for angleDeg in (37.0, 90.0, 180.0):
    m = logic.computePhysicalToWorld(
        identity, 1.0, vtk.vtkMath.RadiansFromDegrees(angleDeg), emptyBounds, dataCenter, VRStage.TABLE_PHYSICAL)
    mapped = m.MultiplyPoint([VRStage.TABLE_PHYSICAL[0], VRStage.TABLE_PHYSICAL[1], VRStage.TABLE_PHYSICAL[2], 1.0])
    for a in range(3):
        assert abs(mapped[a] - expectedCenter[a]) < 1e-4, (angleDeg, a)

# Scale invariant: at relScale s, a world offset shrinks by 1/s in physical/view space.
m2 = logic.computePhysicalToWorld(identity, 2.0, 0.0, emptyBounds, dataCenter, VRStage.TABLE_PHYSICAL)
inv = vtk.vtkMatrix4x4()
vtk.vtkMatrix4x4.Invert(m2, inv)  # world -> physical
centerPhys = inv.MultiplyPoint([dataCenter[0], dataCenter[1], dataCenter[2], 1.0])
offsetPhys = inv.MultiplyPoint([dataCenter[0] + 100.0, dataCenter[1], dataCenter[2], 1.0])
dist = ((offsetPhys[0] - centerPhys[0]) ** 2 + (offsetPhys[1] - centerPhys[1]) ** 2 + (offsetPhys[2] - centerPhys[2]) ** 2) ** 0.5
assert abs(dist - 200.0) < 1e-3, dist  # 100 world units * relScale 2 = 200 physical units
print("computePhysicalToWorld: OK")

# computeDefaultTableHeightM: inverts computePhysicalToWorld's placement so the data's
# vertical center lands at TABLE_COMFORT_CENTER_HEIGHT_M physical. Build a base matrix at the
# standard magnification-1.0 scale (column length = 1000, world mm per physical m).
s0 = VRStage.UNIT_MAGNIFICATION_SCALE  # 1000.0
baseM = vtk.vtkMatrix4x4()
for i in range(3):
    baseM.SetElement(i, i, s0)
# Mid-range: 200mm-tall data (bounds ±100 along world-up), relScale 1.0.
# halfHeight = 0.5 * 200 * 1.0 = 100 world mm; lift = 60 * 1.0 = 60 mm; centerAboveTop = 160/1000 = 0.16 m
# height = 0.80 - 0.05 - 0.16 = 0.59 m
midBounds = [-100.0, 100.0, -100.0, 100.0, -100.0, 100.0]  # extent along world-up = 200
midHeight = VRStage.VRStageLogic.computeDefaultTableHeightM(baseM, 1.0, midBounds)
assert abs(midHeight - 0.59) < 1e-9, midHeight
# Tall data: 1800 mm along up -> raw height = 0.80 - 0.05 - (900+60)/1000 = -0.21 -> clamped to MIN
tallBounds = [-900.0, 900.0, -900.0, 900.0, -900.0, 900.0]
tallHeight = VRStage.VRStageLogic.computeDefaultTableHeightM(baseM, 1.0, tallBounds)
assert tallHeight == VRStage.TABLE_HEIGHT_MIN_M, tallHeight
# Empty bounds -> authored default
emptyHeight = VRStage.VRStageLogic.computeDefaultTableHeightM(baseM, 1.0, [0.0, -1.0, 0.0, -1.0, 0.0, -1.0])
assert emptyHeight == VRStage.TABLE_HEIGHT_M, emptyHeight
# Degenerate zero matrix -> authored default
zeroM = vtk.vtkMatrix4x4()
zeroM.Zero()
assert VRStage.VRStageLogic.computeDefaultTableHeightM(zeroM, 1.0, midBounds) == VRStage.TABLE_HEIGHT_M
# Consistency lock: the height computeDefaultTableHeightM produces, when fed back into
# computePhysicalToWorld as part of tablePhysical, should place the data center's apparent
# physical Y at TABLE_COMFORT_CENTER_HEIGHT_M.
checkBounds = [-100.0, 100.0, -100.0, 100.0, -100.0, 100.0]
checkCenter = [0.0, 0.0, 0.0]
checkRelScale = 1.0
h = VRStage.VRStageLogic.computeDefaultTableHeightM(baseM, checkRelScale, checkBounds)
tp = (0.0, h + VRStage.TABLE_TOP_THICKNESS_M, VRStage.TABLE_FORWARD_M)
ptw = VRStage.VRStageLogic.computePhysicalToWorld(baseM, checkRelScale, 0.0, checkBounds, checkCenter, tp)
ptwInv = vtk.vtkMatrix4x4()
vtk.vtkMatrix4x4.Invert(ptw, ptwInv)
centerPhysical = ptwInv.MultiplyPoint([checkCenter[0], checkCenter[1], checkCenter[2], 1.0])
assert abs(centerPhysical[1] - VRStage.TABLE_COMFORT_CENTER_HEIGHT_M) < 1e-4, centerPhysical[1]
print("computeDefaultTableHeightM: OK")

# _frontFacingYawRad: rotation about `up` that spins RAS Anterior to face the physical
# "toward user" direction. Build an M0 where physical up -> WORLD_UP_RAS (S), physical
# toward-user -> RAS Right, and physical X -> RAS Anterior (completing a right-handed frame) -
# i.e. a reference view yawed 90 degrees off of facing the user. Anterior should then need a
# -90 degree turn to face Right.
m0 = vtk.vtkMatrix4x4()
for row, col in ((0, 2), (1, 0), (2, 1)):
    m0.SetElement(row, col, 1.0)
for row, col in ((0, 0), (0, 1), (1, 1), (1, 2), (2, 0), (2, 2)):
    m0.SetElement(row, col, 0.0)
yaw = FramingMath.frontFacingYawRad(m0)
assert abs(yaw - (-vtk.vtkMath.Pi() / 2.0)) < 1e-6, yaw

# Confirm it actually does what it claims: rotating ANTERIOR_RAS by `yaw` about the derived
# up axis lands exactly on the (normalized) toward-user world direction.
up = FramingMath.worldUp(m0)
towardUser = list(m0.MultiplyPoint(list(VRStage.PHYSICAL_TOWARD_USER) + [0.0]))[:3]
towardUserNorm = [c / vtk.vtkMath.Norm(towardUser) for c in towardUser]
rot = vtk.vtkTransform()
rot.RotateWXYZ(vtk.vtkMath.DegreesFromRadians(yaw), up[0], up[1], up[2])
rotMatrix = vtk.vtkMatrix4x4()
rot.GetMatrix(rotMatrix)
rotated = list(rotMatrix.MultiplyPoint(list(VRStage.ANTERIOR_RAS) + [0.0]))[:3]
for a in range(3):
    assert abs(rotated[a] - towardUserNorm[a]) < 1e-6, (a, rotated, towardUserNorm)
print("frontFacingYawRad: OK")

# Fit-to-table: with base scale factor sf0 (identity M0 -> sf0 = 1), the fit relScale makes the
# data diagonal span the table diameter. A cube with diagonal D -> fitRelScale = 2*R_table/D.
logic._basePhysicalToWorld = vtk.vtkMatrix4x4()  # identity, sf0 = 1
logic._dataBounds = [-50.0, 50.0, -50.0, 50.0, -50.0, 50.0]  # 100 cube, diagonal = 100*sqrt(3)
diag = (3 ** 0.5) * 100.0
expectedFit = (2.0 * VRStage.TABLE_RADIUS_M) / diag
assert abs(logic._computeFitRelScale() - expectedFit) < 1e-9, logic._computeFitRelScale()
logic._dataBounds = [0.0, -1.0, 0.0, -1.0, 0.0, -1.0]  # empty -> fit 1.0
assert logic._computeFitRelScale() == 1.0
logic._basePhysicalToWorld = None
print("computeFitRelScale: OK")

# Auto-spin toggles.
logic._autoSpin = False
logic.toggleAutoSpin()
assert logic._autoSpin is True
logic.toggleAutoSpin()
assert logic._autoSpin is False
print("toggleAutoSpin: OK")

# ---------------------------------------------------------------- display options
#
# VRStageDisplayOptions (parameter node field "display") exposes colors/visibility so other
# modules can reuse the room/table chrome while customizing it - see VRStage.py's docstring.
# Defaults must match the module's original palette, and _buildChrome (which only needs a
# renderer, not a live VR widget/headset) must honor the visibility flags.

displayLogic = VRStage.VRStageLogic()
defaultDisplay = displayLogic.getParameterNode().display
assert defaultDisplay.showWalls is True
assert defaultDisplay.showBackWallSignage is True
assert defaultDisplay.showTableScreen is True
assert defaultDisplay.showInfoScreen is True
assert defaultDisplay.showOrientationLabels is True
assert defaultDisplay.enableReformatTool is True
assert defaultDisplay.enableMeasurementTool is True
expectedAccent = qt.QColor.fromRgbF(*VRStage.ACCENT_COLOR)
# Compare via 8-bit hex, not QColor.__eq__: the parameter node round-trips colors through
# name(1)/re-parse (see QColorSerializer), which quantizes to 8-bit and drops the extended-
# precision float spec fromRgbF() uses - so a direct QColor == would spuriously fail even
# though both render to the identical color.
assert defaultDisplay.accentColor.name() == expectedAccent.name(), \
    (defaultDisplay.accentColor.name(), expectedAccent.name())
print("VRStageDisplayOptions defaults: OK")

# _buildChrome only needs a bare renderer (no live VR widget) - same synthetic-renderer approach
# as the Pick3DRay test further below - to exercise the visibility-gating logic headlessly.
disabledLogic = VRStage.VRStageLogic()
disabledDisplay = disabledLogic.getParameterNode().display
disabledDisplay.showOrientationLabels = False
disabledDisplay.showTableScreen = False
disabledDisplay.showInfoScreen = False
disabledLogic._buildChrome(vtk.vtkRenderer())
assert disabledLogic._orientationLabelActors == {}, "orientation labels should be skipped when disabled"
assert disabledLogic._tableScreenActor is None, "table screen actor should be skipped when disabled"
assert disabledLogic._monitorAssembly is None, "info screen should be skipped when disabled"
# Reset so this doesn't leak into anything else sharing the scene's parameter node.
disabledDisplay.showOrientationLabels = True
disabledDisplay.showTableScreen = True
disabledDisplay.showInfoScreen = True

enabledLogic = VRStage.VRStageLogic()
enabledLogic._buildChrome(vtk.vtkRenderer())  # defaults: everything on
assert len(enabledLogic._orientationLabelActors) == 6
assert enabledLogic._tableScreenActor is not None
assert enabledLogic._monitorAssembly is not None
print("VRStageDisplayOptions visibility gating: OK")

# ---------------------------------------------------------------- live option application
#
# applyOptions diffs an _optionsSnapshot of the rebuild-requiring options against the one the live
# state was built from, and _rebuildChrome rebuilds the room in place (carrying the turntable angle
# and scene-view wall page across) - see VRStage.py's applyOptions/_rebuildChrome. Both are
# exercisable headlessly: the snapshot is pure, and _rebuildChrome takes an explicit renderer.

liveLogic = VRStage.VRStageLogic()
liveParams = liveLogic.getParameterNode()
snapA = VRStage.VRStageLogic._optionsSnapshot(liveParams)
assert snapA == VRStage.VRStageLogic._optionsSnapshot(liveParams), "snapshot must be deterministic"
for field in VRStage.VRStageLogic.CHROME_OPTION_FIELDS + VRStage.VRStageLogic.TOOL_OPTION_FIELDS:
    assert "display." + field in snapA, field
for field in VRStage.VRStageLogic.FRAMING_OPTION_FIELDS:
    assert field in snapA, field
liveParams.display.showInfoScreen = False
snapB = VRStage.VRStageLogic._optionsSnapshot(liveParams)
assert snapB != snapA and snapB["display.showInfoScreen"] is False
liveParams.display.showInfoScreen = True
liveParams.display.wallColor = qt.QColor(10, 20, 30)
snapC = VRStage.VRStageLogic._optionsSnapshot(liveParams)
assert snapC["display.wallColor"] == "#0a141e", snapC["display.wallColor"]
liveParams.display.wallColor = qt.QColor.fromRgbF(*VRStage.WALL_BASE_COLOR)
liveParams.defaultScale = 2.5
assert VRStage.VRStageLogic._optionsSnapshot(liveParams)["defaultScale"] == 2.5
liveParams.defaultScale = VRStage.DEFAULT_MAGNIFICATION
assert VRStage.VRStageLogic._optionsSnapshot(liveParams) == snapA, "resets must restore the original snapshot"
print("_optionsSnapshot: OK")

# applyOptions is a no-op while inactive (no renderer, nothing scheduled, no exception).
liveLogic.applyOptions()
assert liveLogic._chromeRebuildPending is False
assert liveLogic._appliedOptions is None

# _rebuildChrome in place: fewer props after hiding the info screen, no leaked props on the
# renderer, and the turntable angle / wall page survive the rebuild.
liveRenderer = vtk.vtkRenderer()
liveLogic._buildChrome(liveRenderer)
assert liveLogic._monitorAssembly is not None
propCountBefore = liveRenderer.GetViewProps().GetNumberOfItems()
assert propCountBefore == len(liveLogic._chromeProps) + len(liveLogic._orientationLabelActors)
liveLogic._turntableAngleRad = 0.7
liveLogic.isActive = True
liveLogic._appliedOptions = VRStage.VRStageLogic._optionsSnapshot(liveParams)
# Stub the VR renderer accessor so _rebuildChrome/_teardownChrome use the synthetic renderer.
liveLogic._vrRenderer = lambda: liveRenderer
liveParams.display.showInfoScreen = False
liveLogic._rebuildChrome(liveRenderer)
assert liveLogic._monitorAssembly is None, "rebuild must honor the new visibility flag"
assert abs(liveLogic._turntableAngleRad - 0.7) < 1e-12, "turntable angle must survive a rebuild"
propCountAfter = liveRenderer.GetViewProps().GetNumberOfItems()
assert propCountAfter == len(liveLogic._chromeProps) + len(liveLogic._orientationLabelActors), \
    "rebuild leaked props on the renderer"
assert propCountAfter < propCountBefore
# Inactive -> no-op, even with an explicit renderer.
liveLogic.isActive = False
liveParams.display.showInfoScreen = True
liveLogic._rebuildChrome(liveRenderer)
assert liveLogic._monitorAssembly is None, "_rebuildChrome must be a no-op while inactive"
liveLogic._teardownChrome()
assert liveRenderer.GetViewProps().GetNumberOfItems() == 0
print("_rebuildChrome: OK")

# ---------------------------------------------------------------- control bindings
#
# VRStageControlBindings (parameter node field "controls") lets a button be reassigned to any
# of the module's nine button-triggered actions - see CONTROL_BINDING_EVENT_NAMES/
# CONTROL_ACTION_ORDER in VRStage.py. Defaults must reproduce the module's original fixed
# bindings exactly (so behavior is unchanged out of the box), every default button label must
# resolve to a real event on the actual interactor style class (catches a typo'd event name that
# would otherwise only surface as a crash deep in _installObservers with a live VR headset), and
# the generated back-wall signage text must have exactly HELP_BODY_LINE_COUNT lines with the
# right button label substituted in.

controlsLogic = VRStage.VRStageLogic()
defaultControls = controlsLogic.getParameterNode().controls
assert defaultControls.scaleUp == "B"
assert defaultControls.scaleDown == "Y"
assert defaultControls.nextSceneView == VRStage.CONTROL_BINDING_UNBOUND
assert defaultControls.prevSceneView == VRStage.CONTROL_BINDING_UNBOUND
assert defaultControls.resetFraming == "Left Stick Click"
assert defaultControls.toggleReformatVisible == "Right Stick Click"
assert defaultControls.toggleAutoSpin == "Left Menu"
assert defaultControls.placeMeasurementPoint == "Right Trigger"
assert defaultControls.undoMeasurement == "Left Trigger"
print("VRStageControlBindings defaults: OK")

# Every configured button must resolve to a real event name, and (where the real VR interactor
# style module is importable in this headless environment) a real attribute on that class.
# CONTROL_BINDING_UNBOUND is a real, selectable Choice value but deliberately has no event (it
# means "nothing bound") - it's excluded from CONTROL_BINDING_EVENT_NAMES on purpose.
try:
    import vtkSlicerVirtualRealityModuleMRMLDisplayableManagerPython as vrDM
    _style = vrDM.vtkVirtualRealityViewOpenXRInteractorStyle
except ImportError:
    _style = None
for label, eventName in VRStage.CONTROL_BINDING_EVENT_NAMES.items():
    assert isinstance(label, str) and isinstance(eventName, str)
    if _style is not None:
        assert hasattr(_style, eventName), f"{eventName} (button {label!r}) is not a real controller event"
assert VRStage.CONTROL_BINDING_UNBOUND not in VRStage.CONTROL_BINDING_EVENT_NAMES
assert set(VRStage.CONTROL_BINDING_LABELS) == set(VRStage.CONTROL_BINDING_EVENT_NAMES.keys()) | {VRStage.CONTROL_BINDING_UNBOUND}
print("CONTROL_BINDING_EVENT_NAMES: OK" + (" (verified against real interactor style)" if _style else " (interactor style module unavailable, name-shape only)"))

# The generated signage text: one line per BOUND action (with its currently-bound button
# substituted), plus the three fixed lines (rotate, grip, roll, table-height) - an Unbound
# action produces no line (see _controlSchemeBodyText), so HELP_BODY_LINE_COUNT (which assumes
# every action is bound) is only an upper bound, not an exact match, once
# nextSceneView/prevSceneView are left Unbound.
bodyText = Props.controlSchemeBodyText(defaultControls)
bodyLines = bodyText.split("\n")
assert len(bodyLines) == VRStage.HELP_BODY_LINE_COUNT - 2, (len(bodyLines), VRStage.HELP_BODY_LINE_COUNT)
assert bodyLines[0] == "L-stick: rotate/pitch turntable"
assert bodyLines[-1] == "Left grip (hold) + L-stick U/D: table height"
assert "B: scale up" in bodyLines
assert "Right Trigger: place measurement point" in bodyLines
assert "Either grip (hold): move reformat plane" in bodyLines
assert "Left grip (hold) + L-stick L/R: roll" in bodyLines
assert not any("next scene view" in line for line in bodyLines), "unbound actions must not get a signage line"
assert not any("previous scene view" in line for line in bodyLines)

# Binding every action (the worst case HELP_BODY_LINE_COUNT is actually sized for) produces
# exactly HELP_BODY_LINE_COUNT lines, with real lines for the two previously-unbound actions.
fullyBoundLogic = VRStage.VRStageLogic()
fullyBoundControls = fullyBoundLogic.getParameterNode().controls
fullyBoundControls.nextSceneView = "A"
fullyBoundControls.prevSceneView = "X"
fullyBoundText = Props.controlSchemeBodyText(fullyBoundControls)
fullyBoundLines = fullyBoundText.split("\n")
assert len(fullyBoundLines) == VRStage.HELP_BODY_LINE_COUNT, (len(fullyBoundLines), VRStage.HELP_BODY_LINE_COUNT)
assert "A: next scene view" in fullyBoundLines
assert "X: previous scene view" in fullyBoundLines
fullyBoundControls.nextSceneView = VRStage.CONTROL_BINDING_UNBOUND  # reset - shared parameter node
fullyBoundControls.prevSceneView = VRStage.CONTROL_BINDING_UNBOUND
print("control-scheme signage line budget: OK")

# Rebinding is reflected immediately in the generated text (this is what makes the in-VR sign
# stay accurate after a user rebinds something, instead of showing stale defaults).
rebindLogic = VRStage.VRStageLogic()
rebindControls = rebindLogic.getParameterNode().controls
rebindControls.scaleUp = "Left Menu"
reboundText = Props.controlSchemeBodyText(rebindControls)
assert "Left Menu: scale up" in reboundText.split("\n")
assert "B: scale up" not in reboundText
rebindControls.scaleUp = "B"  # reset so this doesn't leak into anything else sharing the scene
print("control-scheme signage text generation: OK")

# ---------------------------------------------------------------- wall tile galleries
#
# Left-wall atlas launcher + right-wall scene-view launcher tiles - see VRStage.py's "wall tile
# galleries" section. Grid/geometry math is pure; _buildSceneViewWallTiles/_buildAtlasWallTiles
# only need a parameter node + slicer.modules.sceneviews.logic(), not a live VR widget/headset -
# same "no headset needed" property as the _buildChrome visibility-gating tests above.

# _gridTileOffsets: row-major, centered, stable ordering, and a short last row is itself
# centered rather than left-aligned.
offsets3 = Props.gridTileOffsets(3, 3, 1.0, 1.0, 0.0)
assert len(offsets3) == 3
assert [round(u, 6) for u, _v in offsets3] == [-1.0, 0.0, 1.0], offsets3  # single centered row
assert all(v == 0.0 for _u, v in offsets3)

offsets5 = Props.gridTileOffsets(5, 3, 1.0, 1.0, 0.0)
assert len(offsets5) == 5
firstRowU = [round(u, 6) for u, _v in offsets5[:3]]
secondRowU = [round(u, 6) for u, _v in offsets5[3:]]
assert firstRowU == [-1.0, 0.0, 1.0], firstRowU          # full row of 3, centered on 0
assert secondRowU == [-0.5, 0.5], secondRowU              # short row of 2, centered independently
assert offsets5[0][1] > offsets5[3][1], "first row must be above (larger v than) the second row"

assert Props.gridTileOffsets(0, 3, 1.0, 1.0, 0.0) == []
print("gridTileOffsets: OK")

# _wallTileWorldPosition: fixed X per side (wall inner face + WALL_TILE_PANEL_PROUD_M), mirrored
# U-sign (du increases toward the wall's own "right" as seen by a user facing it), matching the
# winding _wallTilePanelActor uses.
leftX, leftY, leftZ = Props.wallTileWorldPosition("left", 1.5, -0.5, 0.2, 0.3)
assert abs(leftX - (-VRStage.ROOM_SIZE_M[0] / 2.0 + VRStage.WALL_TILE_PANEL_PROUD_M)) < 1e-9
assert abs(leftY - 1.8) < 1e-9
assert abs(leftZ - (-0.7)) < 1e-9  # centerZ - du

rightX, rightY, rightZ = Props.wallTileWorldPosition("right", 1.5, -0.5, 0.2, 0.3)
assert abs(rightX - (VRStage.ROOM_SIZE_M[0] / 2.0 - VRStage.WALL_TILE_PANEL_PROUD_M)) < 1e-9
assert abs(rightZ - (-0.3)) < 1e-9  # centerZ + du
assert leftX != rightX
print("wallTileWorldPosition: OK")

# ATLAS_SPECS: exactly the three AtlasTests atlases, each with the keys _buildAtlasWallTiles and
# _activateAtlasTile need, and a distinct pictogram color per kind.
assert len(VRStage.ATLAS_SPECS) == 3
for spec in VRStage.ATLAS_SPECS:
    for key in ("name", "kind", "fileNames", "uris", "checksums"):
        assert key in spec and spec[key], (spec.get("name"), key)
    texture = Props.atlasTileTexture(spec["kind"], (0.5, 0.5, 0.5), (0.2, 0.2, 0.2))
    assert texture.dtype.name == "uint8"
    assert texture.ndim == 3 and texture.shape[2] == 3
assert len({spec["kind"] for spec in VRStage.ATLAS_SPECS}) == 3, "each atlas needs a distinct icon"
print("ATLAS_SPECS: OK")

# _buildAtlasWallTiles: exactly 3 pickable panel actors (+ 3 labels), all registered for picking.
atlasWallLogic = VRStage.VRStageLogic()
atlasWallActors = atlasWallLogic._buildAtlasWallTiles()
assert len(atlasWallActors) == 6, len(atlasWallActors)  # 3 panels + 3 labels
assert len(atlasWallLogic._wallTileByActor) == 3
for panelActor in atlasWallLogic._wallTileByActor:
    assert panelActor.GetPickable(), "atlas tiles must stay pickable"
print("buildAtlasWallTiles: OK")

# _buildSceneViewWallTiles with zero scene views (nothing created yet at this point in the test
# file - see the "scene views" section further below) builds one non-interactive placeholder tile
# instead of leaving the wall blank.
assert slicer.modules.sceneviews.logic().GetNumberOfSceneViews() == 0
emptyWallLogic = VRStage.VRStageLogic()
emptyWallActors = emptyWallLogic._buildSceneViewWallTiles()
assert len(emptyWallActors) == 2, len(emptyWallActors)  # 1 placeholder panel + 1 label
assert len(emptyWallLogic._wallTileByActor) == 0, "the placeholder must not be registered as pickable"
assert emptyWallActors[0].GetPickable() == 0, "the placeholder panel must not be pickable"
print("buildSceneViewWallTiles (zero views): OK")

# Picking isolation: a PickFromListOn() picker restricted to a "tile" actor must never return a
# closer "anatomy" actor on the same ray - locks down the mechanism _wallTilePicker depends on to
# guarantee a wall tile can never be mistaken for anatomy by the (unrestricted) _measurePicker.
isolationRenderer = vtk.vtkRenderer()
isolationRenderer.GetActiveCamera().SetClippingRange(0.01, 1000.0)


def _isolationTestSphereActor(center):
    sphereSource = vtk.vtkSphereSource()
    sphereSource.SetCenter(*center)
    sphereSource.SetRadius(5.0)
    mapper = vtk.vtkPolyDataMapper()
    mapper.SetInputConnection(sphereSource.GetOutputPort())
    actor = vtk.vtkActor()
    actor.SetMapper(mapper)
    return actor


tileActor = _isolationTestSphereActor((0.0, 0.0, -20.0))       # closer to the ray origin
anatomyActor = _isolationTestSphereActor((0.0, 0.0, -100.0))    # farther
isolationRenderer.AddActor(tileActor)
isolationRenderer.AddActor(anatomyActor)

restrictedPicker = vtk.vtkCellPicker()
restrictedPicker.PickFromListOn()
restrictedPicker.AddPickList(tileActor)
isolationRayPos = (0.0, 0.0, 0.0)
isolationOrientation = (0.0, 0.0, 0.0, 1.0)  # WXYZ, angle 0 -> ray points along (0, 0, -1)
isolationHit = restrictedPicker.Pick3DRay(isolationRayPos, isolationOrientation, isolationRenderer)
assert isolationHit
assert restrictedPicker.GetActor() is tileActor, "a pick-list-restricted picker must never return an actor outside its list"

# And the reverse: a picker restricted to the anatomy actor never returns the (closer) tile actor.
restrictedAnatomyPicker = vtk.vtkCellPicker()
restrictedAnatomyPicker.PickFromListOn()
restrictedAnatomyPicker.AddPickList(anatomyActor)
anatomyHit = restrictedAnatomyPicker.Pick3DRay(isolationRayPos, isolationOrientation, isolationRenderer)
assert anatomyHit
assert restrictedAnatomyPicker.GetActor() is anatomyActor
print("wall tile picking isolation: OK")


# No stray measurement on a tile press: with a wall tile hovered, _onPlaceMeasurementPoint must
# activate the tile and return WITHOUT ever reaching _commitMeasurementPoint - the critical
# regression this feature must never break (aiming at a wall tile must never create a real
# vtkMRMLMarkupsLineNode).
class _PressCalldata:
    @staticmethod
    def GetAction():
        return vtk.vtkEventDataAction.Press


dispatchLogic = VRStage.VRStageLogic()
dispatchCalls = []
dispatchLogic._hoveredWallTile = VRStage._WallTile(vtk.vtkActor(), lambda: dispatchCalls.append(1))
dispatchLogic.measurementTool.pendingLineNode = None
lineNodeCountBefore = len(slicer.util.getNodesByClass("vtkMRMLMarkupsLineNode"))

dispatchLogic._onPlaceMeasurementPoint(None, None, _PressCalldata())

assert dispatchCalls == [1], "the hovered tile's activation callback must fire exactly once"
assert dispatchLogic.measurementTool.pendingLineNode is None, "a tile press must never arm a measurement"
lineNodeCountAfter = len(slicer.util.getNodesByClass("vtkMRMLMarkupsLineNode"))
assert lineNodeCountAfter == lineNodeCountBefore, "a tile press must never create a measurement node"
print("no stray measurement on tile press: OK")

# ---------------------------------------------------------------- collection

slicer.mrmlScene.Clear()
visibleModel = _addVisibleModel("VisibleModel", (0.0, 0.0, 0.0))
hiddenModel = _addVisibleModel("HiddenModel", (50.0, 0.0, 0.0))
hiddenModel.GetDisplayNode().SetVisibility(False)

collected = FramingMath.collectVisibleDataNodes()
collectedIds = [n.GetID() for n in collected]
assert visibleModel.GetID() in collectedIds, "visible model should be collected"
assert hiddenModel.GetID() not in collectedIds, "invisible model should be excluded"
print("collectVisibleDataNodes: OK")

bounds = FramingMath.combinedRASBounds([visibleModel])
assert bounds[0] < bounds[1], bounds
center = FramingMath.combinedRASCenter([visibleModel])
assert all(abs(c) < 1e-6 for c in center), center
print("combinedRASBounds/Center: OK")

# ---------------------------------------------------------------- reformat slice
#
# _updateReformatFromPlane is pure MRML/VTK math (no VR/renderer needed): given the reformat
# transform node's pose (what _trackReformatPlaneToController sets via vtkMatrix4x4.PoseToMatrix
# on every grip-held pose update), it rebuilds the slice's SliceToRAS to match it exactly.

reformatTransformNode = slicer.mrmlScene.AddNewNodeByClass(
    "vtkMRMLLinearTransformNode", "VRStageTestReformatTransform")
snapPosition = [5.0, 6.0, 7.0]
snapOrientation = [90.0, 0.0, 0.0, 1.0]  # 90 degree rotation about Z, same [angle, axis] form the pose events get
poseMatrix = vtk.vtkMatrix4x4()
vtk.vtkMatrix4x4.PoseToMatrix(snapPosition, snapOrientation, poseMatrix)
reformatTransformNode.SetMatrixTransformToParent(poseMatrix)

reformatSliceLogic = slicer.vtkMRMLSliceLogic()
reformatSliceLogic.SetMRMLScene(slicer.mrmlScene)
reformatSliceNode = reformatSliceLogic.AddSliceNode("VRStageTestReformat")

logic.reformatTool.transformNode = reformatTransformNode
logic.reformatTool.sliceNode = reformatSliceNode
logic.reformatTool.monitorActor = None
logic.reformatTool.updateFromPlane()

expectedMatrix = reformatTransformNode.GetMatrixTransformToParent()
sliceToRAS = reformatSliceNode.GetSliceToRAS()
for r in range(4):
    for c in range(4):
        assert abs(sliceToRAS.GetElement(r, c) - expectedMatrix.GetElement(r, c)) < 1e-6
print("updateReformatFromPlane: OK")

logic.reformatTool.transformNode = None
logic.reformatTool.sliceNode = None
reformatCompositeNode = reformatSliceLogic.GetSliceCompositeNode()
reformatSliceLogic = None  # release before removing the slice node it observes
slicer.mrmlScene.RemoveNode(reformatTransformNode)
slicer.mrmlScene.RemoveNode(reformatSliceNode)
if reformatCompositeNode is not None:
    slicer.mrmlScene.RemoveNode(reformatCompositeNode)

# ---------------------------------------------------------------- scene views

svLogic = slicer.modules.sceneviews.logic()
svLogic.CreateSceneView("VRStageTestView1")
svLogic.CreateSceneView("VRStageTestView2")
assert logic.sceneViewCount() >= 2, logic.sceneViewCount()
startIndex = logic._sceneViewIndex
logic.cycleSceneView(+1)
assert logic._sceneViewIndex != startIndex or logic.sceneViewCount() == 1
logic.cycleSceneView(-1)
print("cycleSceneView: OK")

# _restoreSceneViewAtIndex is the shared tail cycleSceneView and the scene-view wall tiles both
# use - jumping to an explicit, valid index restores it and updates _sceneViewIndex; an
# out-of-range index is a no-op (never raises, never corrupts _sceneViewIndex).
restoreLogic = VRStage.VRStageLogic()
restoreLogic._restoreSceneViewAtIndex(1)
assert restoreLogic._sceneViewIndex == 1
restoreLogic._restoreSceneViewAtIndex(999)  # out of range -> ignored
assert restoreLogic._sceneViewIndex == 1
restoreLogic._activateSceneViewTile(0)  # same tail, reached the way a wall-tile press would
assert restoreLogic._sceneViewIndex == 0
print("restoreSceneViewAtIndex: OK")

# _buildSceneViewWallTiles with a handful of real scene views: one pickable tile per view, each
# registered for picking, none of them the zero-view placeholder path.
fewWallLogic = VRStage.VRStageLogic()
fewWallActors = fewWallLogic._buildSceneViewWallTiles()
assert len(fewWallLogic._wallTileByActor) == logic.sceneViewCount()
assert len(fewWallActors) == 2 * logic.sceneViewCount()  # panel + label per tile
for panelActor in fewWallLogic._wallTileByActor:
    assert panelActor.GetPickable(), "scene view tiles must stay pickable"
print("buildSceneViewWallTiles (few views): OK")

# Pagination: create exactly PAGE_SIZE + 2 total scene views, forcing exactly 2 pages (a full
# first page, a short 2-tile second page) - confirms the wall paginates instead of clamping/
# growing unbounded, and that Prev/Next enable state and the "Page X / Y" indicator text track
# the current page correctly.
while svLogic.GetNumberOfSceneViews() < VRStage.SCENE_VIEW_WALL_PAGE_SIZE + 2:
    svLogic.CreateSceneView(f"VRStageTestViewExtra{svLogic.GetNumberOfSceneViews()}")
totalViews = svLogic.GetNumberOfSceneViews()
assert totalViews == VRStage.SCENE_VIEW_WALL_PAGE_SIZE + 2, totalViews
pageCount = -(-totalViews // VRStage.SCENE_VIEW_WALL_PAGE_SIZE)  # ceil division
assert pageCount == 2, pageCount


def _navTileTexts(actors, contentTileCount):
    """The 3 nav tiles (Prev, Page indicator, Next) immediately follow the content tiles, panel
    then label each - see _buildSceneViewWallNavTiles/_buildSceneViewWallTiles."""
    navStart = 2 * contentTileCount
    prevPanel, prevLabel, pagePanel, pageLabel, nextPanel, nextLabel = actors[navStart:navStart + 6]
    return prevPanel, prevLabel, pagePanel, pageLabel, nextPanel, nextLabel


# Page 0 (default): full page of content, Prev disabled, Next enabled.
page0Logic = VRStage.VRStageLogic()
page0Actors = page0Logic._buildSceneViewWallTiles()
assert page0Logic._sceneViewWallPage == 0
content0 = min(totalViews, VRStage.SCENE_VIEW_WALL_PAGE_SIZE)
assert len(page0Actors) == 2 * content0 + 2 * 3, len(page0Actors)  # content + 3 nav tiles
# Registered/pickable: every content tile, plus only whichever of Prev/Next is enabled.
assert len(page0Logic._wallTileByActor) == content0 + 1, len(page0Logic._wallTileByActor)
prevPanel0, _prevLabel0, pagePanel0, pageLabel0, nextPanel0, _nextLabel0 = _navTileTexts(page0Actors, content0)
assert not prevPanel0.GetPickable(), "Prev must be disabled on the first page"
assert nextPanel0.GetPickable(), "Next must be enabled when a later page exists"
assert not pagePanel0.GetPickable(), "the page indicator is never interactive"
assert pageLabel0.GetInput() == "Page 1 / 2", pageLabel0.GetInput()
print("buildSceneViewWallTiles (page 1 of 2): OK")

# Page 1 (last, short): remaining content, Prev enabled, Next disabled.
page1Logic = VRStage.VRStageLogic()
page1Logic._sceneViewWallPage = 1
page1Actors = page1Logic._buildSceneViewWallTiles()
content1 = totalViews - VRStage.SCENE_VIEW_WALL_PAGE_SIZE
assert content1 == 2, content1
assert len(page1Actors) == 2 * content1 + 2 * 3, len(page1Actors)
assert len(page1Logic._wallTileByActor) == content1 + 1, len(page1Logic._wallTileByActor)
prevPanel1, _prevLabel1, pagePanel1, pageLabel1, nextPanel1, _nextLabel1 = _navTileTexts(page1Actors, content1)
assert prevPanel1.GetPickable(), "Prev must be enabled once off the first page"
assert not nextPanel1.GetPickable(), "Next must be disabled on the last page"
assert pageLabel1.GetInput() == "Page 2 / 2", pageLabel1.GetInput()
print("buildSceneViewWallTiles (page 2 of 2): OK")

# _activateSceneViewWallPage updates the tracked page even with no live VR renderer to rebuild
# into (_rebuildSceneViewWall is a documented no-op outside VR) - this is what a Next/Prev tile
# press ultimately calls.
pageActivateLogic = VRStage.VRStageLogic()
pageActivateLogic._buildSceneViewWallTiles()
assert pageActivateLogic._sceneViewWallPage == 0
pageActivateLogic._activateSceneViewWallPage(1)
assert pageActivateLogic._sceneViewWallPage == 1
pageActivateLogic._rebuildSceneViewWall()  # no renderer -> must not raise
print("scene view wall pagination: OK")

# ---------------------------------------------------------------- measurement tool

# Gesture-suppress debounce: suppressed only when the other hand was pressed and is still held
# within the window; not suppressed once the window elapses or if the other hand isn't held -
# this is the mechanism that keeps the built-in two-controller A+X free-gesture from also firing
# a spurious place/undo when the user deliberately holds both.
suppress = MeasurementTool.isButton1PressSuppressed
window = VRStage.MEASURE_GESTURE_SUPPRESS_WINDOW_S
assert suppress(100.0, True, 100.0 - window / 2.0, window) is True
assert suppress(100.0, True, 100.0 - window * 2.0, window) is False  # window elapsed
assert suppress(100.0, False, 100.0 - window / 2.0, window) is False  # other hand not held
assert suppress(100.0, True, None, window) is False  # no recorded press time
print("isButton1PressSuppressed: OK")

# Bookkeeping: two placements complete a measurement into a real vtkMRMLMarkupsLineNode left in
# the scene; a third arms a new pending line; undo priority cancels the pending line (removing it
# from the scene) before removing a completed measurement (also from the scene, not just the
# session list) - measurements are real content, unlike the rest of this module's transient state.
mt = MeasurementTool()

mt.commitPoint((0.0, 0.0, 0.0))
pendingNode = mt.pendingLineNode
assert pendingNode is not None
assert pendingNode.GetNumberOfControlPoints() == 2
assert len(mt.measurements) == 0

mt.commitPoint((3.0, 4.0, 0.0))
assert mt.pendingLineNode is None
assert len(mt.measurements) == 1
completedNode = mt.measurements[0]
assert completedNode is pendingNode
lengthMeasurement = completedNode.GetMeasurement("length")
assert abs(lengthMeasurement.GetValue() - 5.0) < 1e-6, lengthMeasurement.GetValue()
assert slicer.mrmlScene.GetNodeByID(completedNode.GetID()) is completedNode

mt.commitPoint((1.0, 0.0, 0.0))
pendingNode2 = mt.pendingLineNode
assert pendingNode2 is not None
assert len(mt.measurements) == 1

mt.undoLast()
assert mt.pendingLineNode is None
assert slicer.mrmlScene.GetNodeByID(pendingNode2.GetID()) is None
assert len(mt.measurements) == 1

mt.undoLast()
assert len(mt.measurements) == 0
assert slicer.mrmlScene.GetNodeByID(completedNode.GetID()) is None

mt.undoLast()
assert len(mt.measurements) == 0
assert mt.flashRemaining > 0.0
print("measurement bookkeeping: OK")

# Synthetic-renderer test of the actual vtkCellPicker.Pick3DRay call - locks down the mechanism
# the volume-rendering fallback depends on (a PickableOff() actor is excluded from a default,
# unrestricted pick, so the ray passes through it to whatever's pickable behind it) without
# needing a headset or even a rendered frame - vtkCellPicker does a geometric ray/cell
# intersection, not a GPU/z-buffer pick, so a bare vtkRenderer with actors added is sufficient.
pickRenderer = vtk.vtkRenderer()
pickRenderer.GetActiveCamera().SetClippingRange(0.01, 1000.0)


def _pickTestSphereActor(center, pickable):
    sphereSource = vtk.vtkSphereSource()
    sphereSource.SetCenter(*center)
    sphereSource.SetRadius(5.0)
    mapper = vtk.vtkPolyDataMapper()
    mapper.SetInputConnection(sphereSource.GetOutputPort())
    actor = vtk.vtkActor()
    actor.SetMapper(mapper)
    actor.SetPickable(pickable)
    return actor


dataActor = _pickTestSphereActor((0.0, 0.0, -100.0), True)
chromeActor = _pickTestSphereActor((0.0, 0.0, -20.0), False)  # closer, but PickableOff
pickRenderer.AddActor(dataActor)
pickRenderer.AddActor(chromeActor)

pickTestPicker = vtk.vtkCellPicker()
rayPos = (0.0, 0.0, 0.0)
identityOrientation = (0.0, 0.0, 0.0, 1.0)  # WXYZ, angle 0 -> ray points along (0, 0, -1)
hit = pickTestPicker.Pick3DRay(rayPos, identityOrientation, pickRenderer)
assert hit, "a ray aimed straight through both spheres should hit the pickable one"
assert pickTestPicker.GetActor() is dataActor, "the closer PickableOff sphere must be excluded"
hitPos = pickTestPicker.GetPickPosition()
assert abs(hitPos[2] - (-95.0)) < 1.0, hitPos  # near surface of the far (pickable) sphere

missOrientation = (90.0, 0.0, 1.0, 0.0)  # 90 degrees about Y -> ray points along +X, away from both
missHit = pickTestPicker.Pick3DRay(rayPos, missOrientation, pickRenderer)
assert not missHit, "a ray aimed away from both spheres should miss"
print("Pick3DRay picking mechanics: OK")

# exitViewerMode() idempotency - the headless stand-in for the reentrancy contract
# _activateAtlasTile relies on (VRStageWidget's own StartCloseEvent observer calls
# exitViewerMode() a second time, reentrantly, from inside slicer.util.loadScene(); that second
# call must be a safe no-op). The real network download/scene load is out of scope for a ctest.
idempotentLogic = VRStage.VRStageLogic()
idempotentLogic.exitViewerMode()
assert idempotentLogic.isActive is False
idempotentLogic.exitViewerMode()  # second call, never having been active - must not raise
assert idempotentLogic.isActive is False
print("exitViewerMode idempotency: OK")

slicer.mrmlScene.Clear()
print("VRStageLogicTest: ALL PASSED")
