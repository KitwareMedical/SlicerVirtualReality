import logging

import vtk
import qt

import slicer
from slicer.i18n import tr as _
from slicer.i18n import translate
from slicer.ScriptedLoadableModule import *
from slicer.util import VTKObservationMixin

from VRStageLib.Constants import *            # noqa: F401,F403
from VRStageLib.Constants import _rgbF, _WallTile  # noqa: F401
from VRStageLib.ParameterNode import (        # noqa: F401
    VRStageDisplayOptions, VRStageControlBindings, VRStageParameterNode)
from VRStageLib import UserDefaults           # noqa: F401
from VRStageLib.BakedText import (            # noqa: F401
    BakedTextMixin, BakedTextActor, BakedFollowerTextActor)
from VRStageLib import Props                  # noqa: F401
from VRStageLib import FramingMath             # noqa: F401
from VRStageLib import LocomotionMath          # noqa: F401
from VRStageLib.StageLocomotion import StageLocomotion  # noqa: F401
from VRStageLib.MeasurementTool import MeasurementTool  # noqa: F401
from VRStageLib.ReformatTool import ReformatTool  # noqa: F401
from VRStageLib.Logic import VRStageLogic  # noqa: F401


#
# VRStage
#


class VRStage(ScriptedLoadableModule):
    """Uses ScriptedLoadableModule base class, available at:
    https://github.com/Slicer/Slicer/blob/main/Base/Python/slicer/ScriptedLoadableModule.py
    """

    def __init__(self, parent):
        ScriptedLoadableModule.__init__(self, parent)
        self.parent.title = _("VR Stage")
        self.parent.categories = [translate("qSlicerAbstractCoreModule", "Virtual Reality")]
        self.parent.dependencies = ["VirtualReality"]
        self.parent.contributors = ["Kyle Sunderland (PerkLab, Queen's University)"]
        self.parent.helpText = _("""
A grounded, presentation-oriented virtual reality viewer. Instead of flying through empty space,
the user stands in a fixed room with a turntable in front of them. Scene data appears on the
turntable and is rotated with the left thumbstick; the right thumbstick walks the user around
the room (which stays put, and cannot be left); world scale and scene-view navigation are
driven by controller buttons. A grabbable arbitrary reformat plane lets the data be sliced from
any angle, with a floating screen showing the reformatted image live.

The viewer does not modify the user's loaded data: turntable placement, scale and rotation are
applied only to the VR view (via its PhysicalToWorldMatrix), so the desktop 3D and slice views
are left untouched. The Red/Green/Yellow slice planes are not shown.

Controller bindings (Oculus Touch, defaults shown - ten of these are rebindable in the Controls
section below, or via `logic.getParameterNode().controls`):
- Left thumbstick left/right: rotate the turntable (yaw); up/down: pitch. Left grip (hold):
  turns left/right into roll and up/down into table height - see below. (fixed)
- Right thumbstick: walk around the room - up/down moves along the direction the headset is
  facing, left/right strafes. The room, table and data stay put; the headset is kept inside the
  walls (with a small margin), even in combination with physically walking around the playspace.
  (fixed)
- Either grip (hold): the reformat plane follows that controller's position/orientation for as
  long as the grip is held - release to leave it in place. A floating screen beside the plane
  shows the reformatted image live. Hidden until toggled on - the show/hide toggle is unbound by
  default, bind it to a free button in the Controls section. The left grip doubles
  as the roll/table-height modifier above, so holding it does both at once. (fixed)
- Right thumbstick click: return to the center of the room (undoes any right-stick walking; the
  room, table and data stay put)
- B button: increase scale, Y button: decrease scale
- A/X: unbound by default (aim at a right-wall tile and pull the right trigger to pick a scene
  view instead - see below - or rebind next/previous scene view onto A/X in the Controls section)
- Left thumbstick click: recenter the data on the table (at the default scale, see the Behavior
  section's "Default scale" - 1.0 = normal VR size, unless "Fit data to table" is on)
- Left menu button: toggle hands-free auto-spin
- Right trigger: aim the right controller at the anatomy (or the revealed reformat plane, for
  volume-only data) and pull to place a measurement point; pull again to complete the pair into
  a persisted distance measurement - OR, if the aim ray is over a wall tile instead (the left
  wall's scene-library launcher or the right wall's scene-view launcher), activate that tile. Left
  trigger: undo the last point or measurement. (aiming with the right controller is fixed; which
  buttons place/undo is rebindable)

The built-in two-controller A+X free gesture (and the default right-stick fly) are disabled
while the stage is active, and restored on exit.

The left ("library") wall offers one-press scene launchers: either the built-in atlas set, or -
choose "MRB directory" under Behavior > "Left wall shows" and pick a folder - one tile per MRB
file in that folder, thumbnailed from each file's embedded scene screenshot. Aim and pull the
right trigger to load one; the current scene is replaced but the VR session stays on.

Other modules can reuse this room/table setup while customizing its colors and showing/hiding
individual components (walls, signage, orientation labels, table screen, info screen, library
wall, scene view wall), disabling the reformat/measurement tools, or rebinding which button triggers
which action - see VRStageDisplayOptions (`logic.getParameterNode().display`) and
VRStageControlBindings (`logic.getParameterNode().controls`).

"Save as default" stores all current options (Behavior, Display, Controls) in the application
settings so they persist between sessions and are applied automatically on startup - like the
Markups module's "Save as default". "Restore defaults" discards them and returns every option
to its original value.
""")
        self.parent.helpText += self.getDefaultModuleDocumentationLink()
        self.parent.acknowledgementText = _("""
This module is part of the SlicerVirtualReality extension.
""")


#
# VRStageWidget
#


class VRStageWidget(ScriptedLoadableModuleWidget, VTKObservationMixin):
    """Thin desktop panel: enter/exit the viewer and edit options.
    All behavior lives in VRStageLogic so it can be tested headless.
    """

    def __init__(self, parent=None) -> None:
        ScriptedLoadableModuleWidget.__init__(self, parent)
        VTKObservationMixin.__init__(self)
        self.logic = None
        self._parameterNode = None
        self._parameterNodeGuiTag = None

    def setup(self):
        ScriptedLoadableModuleWidget.setup(self)

        uiWidget = slicer.util.loadUI(self.resourcePath("UI/VRStage.ui"))
        self.layout.addWidget(uiWidget)
        self.ui = slicer.util.childWidgetVariables(uiWidget)
        uiWidget.setMRMLScene(slicer.mrmlScene)

        self.logic = VRStageLogic()

        self.addObserver(slicer.mrmlScene, slicer.mrmlScene.StartCloseEvent, self.onSceneStartClose)
        self.addObserver(slicer.mrmlScene, slicer.mrmlScene.EndCloseEvent, self.onSceneEndClose)

        self.ui.enterButton.clicked.connect(self.onEnterButton)
        self.ui.exitButton.clicked.connect(self.onExitButton)
        self.ui.resetInteractionTransformsButton.clicked.connect(self.onResetInteractionTransformsButton)
        self.ui.saveDefaultsButton.clicked.connect(self.onSaveDefaultsButton)
        self.ui.restoreDefaultsButton.clicked.connect(self.onRestoreDefaultsButton)

        self.initializeParameterNode()
        self.updateGUIFromLogic()

    def cleanup(self) -> None:
        if self.logic:
            self.logic.exitViewerMode()
        self.removeObservers()

    def enter(self) -> None:
        self.initializeParameterNode()
        self.updateGUIFromLogic()

    def exit(self) -> None:
        self.setParameterNode(None)

    def onSceneStartClose(self, caller, event) -> None:
        if self.logic:
            self.logic.exitViewerMode()
        self.setParameterNode(None)

    def onSceneEndClose(self, caller, event) -> None:
        if self.parent.isEntered:
            self.initializeParameterNode()

    def initializeParameterNode(self) -> None:
        self.setParameterNode(self.logic.getParameterNode())

    def setParameterNode(self, inputParameterNode) -> None:
        if self._parameterNode == inputParameterNode:
            return
        if self._parameterNode:
            self._parameterNode.disconnectGui(self._parameterNodeGuiTag)
            self.removeObserver(self._parameterNode, vtk.vtkCommand.ModifiedEvent, self.onParameterNodeModified)
        self._parameterNode = inputParameterNode
        if self._parameterNode:
            self._parameterNodeGuiTag = self._parameterNode.connectGui(self.ui)
            self.addObserver(self._parameterNode, vtk.vtkCommand.ModifiedEvent, self.onParameterNodeModified)

    def onParameterNodeModified(self, caller=None, event=None) -> None:
        if self.logic:
            self.logic.applyOptions()

    def onEnterButton(self) -> None:
        try:
            self.logic.enterViewerMode()
        except Exception as e:  # noqa: BLE001
            slicer.util.errorDisplay(_("Failed to enter VR Stage: {error}").format(error=str(e)))
            import traceback
            traceback.print_exc()
        self.updateGUIFromLogic()

    def onExitButton(self) -> None:
        self.logic.exitViewerMode()
        self.updateGUIFromLogic()

    def onResetInteractionTransformsButton(self) -> None:
        self.logic.resetInteractionTransforms()

    def onSaveDefaultsButton(self) -> None:
        self.logic.saveOptionsAsDefault()
        slicer.util.showStatusMessage(_("VR Stage options saved as default."), 3000)

    def onRestoreDefaultsButton(self) -> None:
        self.logic.restoreDefaultOptions()
        slicer.util.showStatusMessage(_("VR Stage options restored to defaults."), 3000)

    def updateGUIFromLogic(self) -> None:
        active = bool(self.logic and self.logic.isActive)
        self.ui.enterButton.enabled = not active
        self.ui.exitButton.enabled = active
        if active:
            self.ui.statusLabel.text = _("Active - scale {scale:.2f}x").format(scale=self.logic.getMagnification())
        else:
            self.ui.statusLabel.text = _("Not active")

    # ---- module reload (SlicerHeart pattern, ported to importlib for Python 3.12+)

    # Dependency order: each module must come after every module it imports from, so that
    # re-executing it binds the freshly reloaded classes/functions (a missing entry keeps the
    # stale module cached in sys.modules even after Logic reloads).
    _VRSTAGELIB_SUBMODULES = [
        "Constants",
        "ParameterNode",
        "UserDefaults",
        "BakedText",
        "Props",
        "FramingMath",
        "LocomotionMath",
        "MrbLibrary",
        "SceneViews",
        "OrientationLabels",
        "MeasurementTool",
        "ReformatTool",
        "StageLighting",
        "RoomChrome",
        "StageFraming",
        "StageLocomotion",
        "WallTiles",
        "Logic",
    ]

    @staticmethod
    def reloadPackageWithSubmodules(packageName, submoduleNames):
        import importlib
        importlib.invalidate_caches()
        package = importlib.import_module(packageName)
        for submoduleName in submoduleNames:
            submodule = importlib.import_module(f"{packageName}.{submoduleName}")
            importlib.reload(submodule)
        importlib.reload(package)

    def onReload(self):
        logging.debug(f"Reloading {self.moduleName}")
        self.reloadPackageWithSubmodules("VRStageLib", self._VRSTAGELIB_SUBMODULES)
        ScriptedLoadableModuleWidget.onReload(self)



#
# VRStageTest
#


class VRStageTest(ScriptedLoadableModuleTest):
    """Runtime self-test mirroring the headless logic assertions (no headset needed)."""

    def setUp(self):
        slicer.mrmlScene.Clear()

    def runTest(self):
        self.setUp()
        self.test_VRStageLogic1()

    def test_VRStageLogic1(self):
        self.delayDisplay("Starting VR Stage logic test")

        logic = VRStageLogic()

        # Magnification stepping is pure and clamped.
        self.assertAlmostEqual(logic.steppedMagnification(1.0, +1, 1.25), 1.25)
        self.assertAlmostEqual(logic.steppedMagnification(1.0, -1, 1.25), 0.8)
        self.assertEqual(logic.steppedMagnification(MAX_MAGNIFICATION, +1, 2.0), MAX_MAGNIFICATION)
        self.assertEqual(logic.steppedMagnification(MIN_MAGNIFICATION, -1, 2.0), MIN_MAGNIFICATION)

        # With M0 = identity and relScale 1, the data center appears TABLE_LIFT_BUFFER_MM above
        # the table location along "up" (zero-extent data, so that's the only offset): M maps
        # the table physical point onto dataCenter - up*liftBuffer.
        identity = vtk.vtkMatrix4x4()
        dataCenter = [10.0, 20.0, 30.0]
        emptyBounds = [0.0, -1.0, 0.0, -1.0, 0.0, -1.0]  # extent 0
        m = logic.computePhysicalToWorld(identity, 1.0, 0.0, emptyBounds, dataCenter, TABLE_PHYSICAL)
        mapped = m.MultiplyPoint([TABLE_PHYSICAL[0], TABLE_PHYSICAL[1], TABLE_PHYSICAL[2], 1.0])
        up = FramingMath.worldUp(identity)
        expected = [dataCenter[a] - up[a] * TABLE_LIFT_BUFFER_MM for a in range(3)]
        for a in range(3):
            self.assertAlmostEqual(mapped[a], expected[a], places=4)

        # Data-aware table height: mid-range data (200mm along up) at mag 1 puts the center at
        # TABLE_COMFORT_CENTER_HEIGHT_M physical (= tabletop 0.94 m); tall data clamps to MIN.
        s0 = UNIT_MAGNIFICATION_SCALE
        baseM = vtk.vtkMatrix4x4()
        for i in range(3):
            baseM.SetElement(i, i, s0)
        midBounds = [-100.0, 100.0, -100.0, 100.0, -100.0, 100.0]
        self.assertAlmostEqual(
            VRStageLogic.computeDefaultTableHeightM(baseM, 1.0, midBounds), 0.59, places=6)
        tallBounds = [-900.0, 900.0, -900.0, 900.0, -900.0, 900.0]
        self.assertEqual(
            VRStageLogic.computeDefaultTableHeightM(baseM, 1.0, tallBounds), TABLE_HEIGHT_MIN_M)
        self.assertEqual(
            VRStageLogic.computeDefaultTableHeightM(baseM, 1.0, emptyBounds), TABLE_HEIGHT_M)

        # Locomotion math is pure and clamped: full-stick forward walks at
        # LOCOMOTION_SPEED_M_PER_S along the gaze, the wall margin blocks outward motion,
        # and an already-outside head is never teleported (mirrors the headless test).
        gazeBack = [0.0, 0.0, -1.0]
        vx, vz = LocomotionMath.walkVelocity(
            0.0, 1.0, gazeBack, THUMBSTICK_DEADZONE, LOCOMOTION_SPEED_M_PER_S)
        self.assertAlmostEqual(vx, 0.0)
        self.assertAlmostEqual(vz, -LOCOMOTION_SPEED_M_PER_S)
        limitX = ROOM_SIZE_M[0] / 2.0 - LOCOMOTION_WALL_MARGIN_M
        dx, dz = LocomotionMath.clampWalkDelta(
            limitX - 0.05, 0.0, 0.2, 0.2, ROOM_SIZE_M, LOCOMOTION_WALL_MARGIN_M)
        self.assertAlmostEqual(dx, 0.05)  # clamped at the wall
        self.assertAlmostEqual(dz, 0.2)   # while sliding along it
        dx, _dz = LocomotionMath.clampWalkDelta(
            limitX + 0.5, 0.0, 0.2, 0.0, ROOM_SIZE_M, LOCOMOTION_WALL_MARGIN_M)
        self.assertEqual(dx, 0.0)
        walker = StageLocomotion()
        self.assertTrue(walker.update(1.0, [0.0, 1.7, 0.0], gazeBack, 0.0, 1.0))
        self.assertAlmostEqual(walker.offsetZM, -LOCOMOTION_SPEED_M_PER_S)
        walker.reset()
        self.assertTrue(walker.isIdentity())

        # Live option application: the snapshot applyOptions diffs against is deterministic and
        # tracks each rebuild-requiring field; applyOptions is a no-op while inactive.
        params = logic.getParameterNode()
        snapshot = VRStageLogic._optionsSnapshot(params)
        self.assertEqual(snapshot, VRStageLogic._optionsSnapshot(params))
        params.display.showWalls = False
        self.assertNotEqual(snapshot, VRStageLogic._optionsSnapshot(params))
        params.display.showWalls = True
        self.assertEqual(snapshot, VRStageLogic._optionsSnapshot(params))
        logic.applyOptions()
        self.assertIsNone(logic._appliedOptions)

        # User defaults ("Save as default"): a save/apply round trip through a throwaway
        # settings file restores the saved values on a brand-new parameter node - exactly once,
        # so a marked (e.g. scene-loaded) node is never clobbered.
        import os
        import tempfile
        testSettings = qt.QSettings(
            os.path.join(tempfile.mkdtemp(), "VRStageUserDefaults.ini"), qt.QSettings.IniFormat)
        savedParams = VRStageParameterNode(
            slicer.mrmlScene.AddNewNodeByClass("vtkMRMLScriptedModuleNode"))
        savedParams.rotationSpeedDegPerSec = 90.0
        savedParams.display.showFloor = False
        UserDefaults.saveUserDefaults(savedParams, testSettings)
        freshParams = VRStageParameterNode(
            slicer.mrmlScene.AddNewNodeByClass("vtkMRMLScriptedModuleNode"))
        self.assertTrue(UserDefaults.applyUserDefaultsOnce(freshParams, testSettings))
        self.assertEqual(freshParams.rotationSpeedDegPerSec, 90.0)
        self.assertFalse(freshParams.display.showFloor)
        freshParams.rotationSpeedDegPerSec = 33.0
        self.assertFalse(UserDefaults.applyUserDefaultsOnce(freshParams, testSettings))
        self.assertEqual(freshParams.rotationSpeedDegPerSec, 33.0)

        # MRB library (left wall's "MRB directory" source): directory listing and the
        # embedded-screenshot selection heuristic (mirrors the headless logic test).
        import zipfile
        from VRStageLib import MrbLibrary
        mrbDir = tempfile.mkdtemp()
        mrbPath = os.path.join(mrbDir, "Sample.mrb")
        with zipfile.ZipFile(mrbPath, "w") as archive:
            archive.writestr("S/S.mrml", "<MRML/>")
            archive.writestr("S/S.png", b"root-png-bytes")
            archive.writestr("S/Data/view.png", b"data-png-bytes")
        self.assertEqual([p.name for p in MrbLibrary.listMrbFiles(mrbDir)], ["Sample.mrb"])
        self.assertEqual(MrbLibrary.mrbDisplayName(mrbPath), "Sample")
        self.assertEqual(MrbLibrary.mrbScreenshotBytes(mrbPath), b"root-png-bytes")
        self.assertIsNone(MrbLibrary.mrbScreenshotBytes(os.path.join(mrbDir, "missing.mrb")))
        self.assertEqual(MrbLibrary.listMrbFiles(""), [])

        self.delayDisplay("Test passed")
