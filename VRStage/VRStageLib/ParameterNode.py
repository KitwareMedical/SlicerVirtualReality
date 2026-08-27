"""VRStageDisplayOptions, VRStageControlBindings, and VRStageParameterNode."""

import pathlib
from typing import Annotated

import qt

from slicer.parameterNodeWrapper import (
    parameterNodeWrapper,
    parameterPack,
    Choice,
    Default,
    WithinRange,
)

from .Constants import *            # noqa: F401,F403
from .Constants import _rgbF


@parameterPack
class VRStageDisplayOptions:
    """Colors and component visibility for the room/table chrome - exposed so other modules can
    reuse VRStage's grounded-room setup while customizing its look, e.g. a module with its own
    branding might set a different accentColor, or hide the back-wall signage/orientation labels
    it doesn't want shown alongside its own content. Access via
    `slicer.util.getModuleLogic('VRStage').getParameterNode().display`.

    Colors default to this module's original "medical sci-fi" palette. Most are baked into
    procedural textures at chrome-build time (table screen, walls, floor grid, ceiling panel,
    signage panels), so changing one while the stage is active rebuilds the room chrome in place
    (see VRStageLogic.applyOptions/_rebuildChrome) - every option here applies live, no
    exit/re-enter needed. overheadLightColor also recolors the light rig directly.

    Visibility flags gate whole prop groups at chrome-build time; flipping one live likewise
    rebuilds the chrome in place.

    enableReformatTool / enableMeasurementTool set up / tear down those tools live (and skip
    them entirely at enterViewerMode() when off) - useful for a module that wants the room/table
    but not these interactions.
    """

    accentColor: Annotated[qt.QColor, Default(qt.QColor.fromRgbF(*ACCENT_COLOR))]
    accentColorDim: Annotated[qt.QColor, Default(qt.QColor.fromRgbF(*ACCENT_COLOR_DIM))]
    floorColor: Annotated[qt.QColor, Default(qt.QColor.fromRgbF(*FLOOR_BASE_COLOR))]
    wallColor: Annotated[qt.QColor, Default(qt.QColor.fromRgbF(*WALL_BASE_COLOR))]
    columnColor: Annotated[qt.QColor, Default(qt.QColor.fromRgbF(*COLUMN_COLOR))]
    tableColor: Annotated[qt.QColor, Default(qt.QColor.fromRgbF(*TABLE_RING_COLOR))]
    rimBandColor: Annotated[qt.QColor, Default(qt.QColor.fromRgbF(*RIM_BAND_COLOR))]
    tableScreenBackgroundColor: Annotated[qt.QColor, Default(qt.QColor.fromRgbF(*TABLE_SCREEN_BG_COLOR))]
    overheadLightColor: Annotated[qt.QColor, Default(qt.QColor.fromRgbF(*OVERHEAD_LIGHT_COLOR))]

    showFloor: bool = True              # floor disc, floor grid, and the accent ring around the table
    showWalls: bool = True              # room walls + ceiling light panel (table is always drawn)
    showBackWallSignage: bool = True    # the control-scheme help text (independent of showWalls)
    showTableScreen: bool = True        # the holo readout inset in the tabletop
    showInfoScreen: bool = True         # the scale/scene-view monitor mounted on the table's collar
    showOrientationLabels: bool = True  # R/L/A/P/S/I billboards
    showLibraryWall: bool = True        # left-wall scene-launcher tiles - atlases or a
                                         # directory of MRB files, per libraryWallSource
    showSceneViewWall: bool = True      # right-wall scene-view-launcher tiles (built from the
                                         # scene's current Scene Views at enter time)

    enableReformatTool: bool = True
    enableMeasurementTool: bool = True


@parameterPack
class VRStageControlBindings:
    """Which controller button triggers each of this module's ten button-triggered actions -
    see CONTROL_BINDING_EVENT_NAMES for the available buttons and CONTROL_ACTION_ORDER for the
    action list/descriptions. Exposed so a user (or another module) can rebind the default
    layout, e.g. to avoid a clash with a button that module's own tooling also wants to use.

    Rebinding takes effect immediately while the stage is active (see
    VRStageLogic._refreshControlBindings) as well as on the next enterViewerMode() call.

    Nothing prevents two actions from being assigned to the same button: both fire on press,
    which is rarely useful but not prevented, since validating uniqueness across ten
    independent combo boxes was judged not worth the added UI complexity.

    placeMeasurementPoint/undoMeasurement default to the triggers rather than A/X: both are
    "picking" actions - aim and pull to place a markup point on the anatomy, or to activate
    whichever wall tile (library launcher or scene-view launcher) the aim ray is currently over -
    and a trigger pull is the natural gesture for that, on either controller. Rebinding
    placeMeasurementPoint/undoMeasurement onto A/X (no longer the default) engages the debounce
    in _isButton1PressSuppressed, which exists specifically to avoid a spurious place/undo when
    the built-in two-controller free-gesture (always tied to the literal A+X buttons, independent
    of this pack) is engaged - see that method's docstring.

    nextSceneView/prevSceneView are Unbound (CONTROL_BINDING_UNBOUND) by default - freed up now
    that the right-wall scene-view tiles cover scene selection (aim + pick a specific view,
    rather than blindly cycling next/prev). A user who still wants the old cycling behavior can
    rebind either onto any free button in the Controls UI.

    recenterUser (return to the room origin, undoing right-stick walking) takes Right Stick
    Click: clicking the stick you walk with is the natural "stop and snap back" gesture.
    That displaced toggleReformatVisible to Unbound - the reformat plane is a secondary tool,
    and anyone using it can rebind the toggle onto any free button (e.g. A/X).
    """

    scaleUp: Annotated[str, Choice(CONTROL_BINDING_LABELS)] = "B"
    scaleDown: Annotated[str, Choice(CONTROL_BINDING_LABELS)] = "Y"
    nextSceneView: Annotated[str, Choice(CONTROL_BINDING_LABELS)] = CONTROL_BINDING_UNBOUND
    prevSceneView: Annotated[str, Choice(CONTROL_BINDING_LABELS)] = CONTROL_BINDING_UNBOUND
    resetFraming: Annotated[str, Choice(CONTROL_BINDING_LABELS)] = "Left Stick Click"
    recenterUser: Annotated[str, Choice(CONTROL_BINDING_LABELS)] = "Right Stick Click"
    toggleReformatVisible: Annotated[str, Choice(CONTROL_BINDING_LABELS)] = CONTROL_BINDING_UNBOUND
    toggleAutoSpin: Annotated[str, Choice(CONTROL_BINDING_LABELS)] = "Left Menu"
    placeMeasurementPoint: Annotated[str, Choice(CONTROL_BINDING_LABELS)] = "Right Trigger"
    undoMeasurement: Annotated[str, Choice(CONTROL_BINDING_LABELS)] = "Left Trigger"


@parameterNodeWrapper
class VRStageParameterNode:
    """User-facing options for the VR Stage.

    rotationSpeedDegPerSec - angular speed at full thumbstick deflection, shared by all three
        rotation axes (yaw/pitch/roll - see VRStageLogic's "turntable rotation" section).
    magnificationStep - multiplicative factor applied to world scale per +/- button press.
    defaultScale - real-world magnification (1.0 = normal VR size) used to frame the data when
        fitToTable is off. Ignored while fitToTable is on, which computes its own framing scale.
    fitToTable - if true, auto-scale each framing so the data spans the table. Off by default:
        with it on, different scene views (with different data extents) land at very different
        scales; off, every framing uses defaultScale (1.0 = normal VR size, by default).
    libraryWallSource - what the left ("library") wall's launcher tiles offer: the fixed atlas
        set (LIBRARY_WALL_SOURCE_ATLASES) or one tile per *.mrb scene bundle found in
        mrbLibraryDirectory (LIBRARY_WALL_SOURCE_DIRECTORY), thumbnailed from each bundle's
        embedded scene screenshot. Applies live (the wall rebuilds in place).
    mrbLibraryDirectory - the directory scanned for *.mrb files when libraryWallSource is
        LIBRARY_WALL_SOURCE_DIRECTORY. An empty path shows a "no MRB files" placeholder tile.
    overheadLight - if true, the table is lit by a light rig anchored above it (with softer
        fill lights derived from it) instead of the VR view's default lighting.
    display - colors and component show/hide options - see VRStageDisplayOptions.
    controls - which button triggers each action - see VRStageControlBindings.
    """

    rotationSpeedDegPerSec: Annotated[float, WithinRange(1.0, 360.0)] = 180.0
    magnificationStep: Annotated[float, WithinRange(1.01, 4.0)] = 1.25
    defaultScale: Annotated[float, WithinRange(MIN_MAGNIFICATION, MAX_MAGNIFICATION)] = DEFAULT_MAGNIFICATION
    fitToTable: bool = False
    overheadLight: bool = True
    passthrough: bool = False
    libraryWallSource: Annotated[str, Choice(LIBRARY_WALL_SOURCES)] = LIBRARY_WALL_SOURCE_ATLASES
    mrbLibraryDirectory: pathlib.Path = pathlib.Path()
    display: VRStageDisplayOptions = VRStageDisplayOptions()
    controls: VRStageControlBindings = VRStageControlBindings()
