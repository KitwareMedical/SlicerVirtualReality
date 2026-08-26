"""All module-level constants for VRStage, kept in one file because they are deeply
cross-derived (e.g. HELP_PANEL_HEIGHT_M depends on CONTROL_ACTION_ORDER's length,
SCENE_VIEW_WALL_CENTER_Y_M on FLOOR_THICKNESS_M).  Also includes the _rgbF helper
and the _WallTile namedtuple (underscore-prefixed; import them explicitly where needed)."""

import collections
import math

import qt

from slicer.util import TESTING_DATA_URL


def _rgbF(color: "qt.QColor"):
    """Convert a qt.QColor display option to the 0-1 RGB float triple VTK's SetColor()/
    emissive-factor APIs expect."""
    return (color.redF(), color.greenF(), color.blueF())


# Physical-space layout of the room, in meters. The tracking origin is at the user's
# feet; forward is -Z and up is +Y (VTK VR physical convention). Tune in-headset.
FLOOR_RADIUS_M = 2.0
FLOOR_THICKNESS_M = 0.02
TABLE_RADIUS_M = 0.40
TABLE_TOP_THICKNESS_M = 0.05
TABLE_HEIGHT_M = 0.90            # authored height of the table top above the floor (see below)
TABLE_FORWARD_M = -0.60         # distance in front of the user (-Z)

# Runtime table height (left grip + left stick up/down) and its data-aware default.
# TABLE_HEIGHT_M above stays the AUTHORED height all chrome geometry is baked at; the live
# height (VRStageLogic._tableHeightM) is applied as a physical-space Y offset to the
# table-anchored props in _reanchorChrome, so nothing is rebuilt while the table moves. The
# clamp minimum keeps the collar and monitor housing (which occupy the RIM_BAND_HEIGHT_M band
# below the tabletop, see below) above the floor.
TABLE_HEIGHT_MIN_M = 0.30
TABLE_HEIGHT_MAX_M = 1.50
TABLE_MOVE_SPEED_M_PER_S = 0.30   # table travel speed at full stick deflection
# Physical height a framing reset places the data's vertical center at: low enough that even
# small data sits comfortably below eye level, while tall data brings the table down further
# (clamped to TABLE_HEIGHT_MIN_M) instead of towering overhead off a fixed-height table.
TABLE_COMFORT_CENTER_HEIGHT_M = 0.80
COLUMN_RADIUS_M = 0.08
ROOM_SIZE_M = (6.0, 3.0, 6.0)   # width (X), height (Y), depth (Z)
ROOM_CENTER_Y_M = 1.5

# Raised collar/apron band around the table's whole perimeter, sitting directly under the
# tabletop cap (in the space otherwise occupied only by the thin column) - gives the table a
# real, constructed pedestal-table silhouette (post -> collar -> cap) instead of a plain disc,
# and gives the monitor housing (see MONITOR_* below) a wide, sturdy-looking base to be mounted
# into. Inset from TABLE_RADIUS_M so the cap overhangs the collar as a lip.
RIM_BAND_RADIUS_M = TABLE_RADIUS_M * 0.90
RIM_BAND_HEIGHT_M = 0.22          # must clear the monitor housing's footprint, see MONITOR_* below

# Bright "medical sci-fi" palette: cool steel room/floor with a cyan holo-tech accent used for
# every glowing trim/rim/readout, so the room reads as one coherent kit of parts rather than a
# grab-bag of colors.
ACCENT_COLOR = (0.25, 0.85, 1.0)
ACCENT_COLOR_DIM = (0.10, 0.35, 0.42)
FLOOR_BASE_COLOR = (0.62, 0.68, 0.75)
WALL_BASE_COLOR = (0.80, 0.85, 0.90)
COLUMN_COLOR = (0.30, 0.34, 0.40)
TABLE_RING_COLOR = (0.40, 0.46, 0.54)
TABLE_SCREEN_BG_COLOR = (0.03, 0.07, 0.11)
RIM_BAND_COLOR = (0.34, 0.38, 0.44)   # between COLUMN_COLOR and TABLE_RING_COLOR

# Table "screen" (the circular holo-readout inset in the tabletop), as a fraction of
# TABLE_RADIUS_M so it scales if the table size is tuned. Recessed below the surrounding cap's
# top surface (see _annulusActor) rather than sitting proud on it, so it reads as a screen sunk
# into the table rather than a decal stuck on top - must leave enough floor thickness below it
# (TABLE_TOP_THICKNESS_M - RECESS_DEPTH_M) to still read as solid.
TABLE_SCREEN_RADIUS_FRAC = 0.85
TABLE_SCREEN_RECESS_DEPTH_M = 0.015
# Emissive glow for the screen's texture (see _tableScreenTexture) - lets the circuit/ring
# pattern read as self-lit "holo" tech, independent of the overhead light rig, using VTK's PBR
# emissive-texture pipeline rather than the plain ambient-only trick used for the ring/rim glows.
TABLE_SCREEN_EMISSIVE_FACTOR = (1.0, 1.0, 1.0)

# Seam-line glow ring marking where the cap overhangs the collar (see RIM_BAND_RADIUS_M), as
# fractions of RIM_BAND_RADIUS_M rather than TABLE_RADIUS_M since it now trims the collar, not
# the cap's own edge.
COLLAR_SEAM_RING_INNER_FRAC = 0.97
COLLAR_SEAM_RING_OUTER_FRAC = 1.01

# Floor "landing pad" glow ring drawn around the table's footprint.
FLOOR_RING_INNER_M = TABLE_RADIUS_M + 0.25
FLOOR_RING_OUTER_M = TABLE_RADIUS_M + 0.32

# Rebindable controller buttons: the discrete, click-based (Press/Release) controller events
# that can be freely reassigned to any of this module's button-triggered actions. Deliberately
# EXCLUDES events that are structural/continuous rather than a simple discrete click:
#   - LeftThumbstickEvent/RightThumbstickEvent/Right*ThumbstickTouchEvent (continuous axis -
#     turntable rotation/table-height drive, and the suppress-default-fly observers)
#   - Left/RightGripClickEvent + Left/RightGripPoseEvent (paired continuous pose tracking for
#     the reformat plane and the roll/table-height modifier - "either grip" is a fixed
#     structural affordance, not one of the assignable actions below)
#   - RightAimPoseEvent (continuous aim ray for the measurement reticle)
#   - RightSystemClickEvent (reserved for the platform/system menu on most runtimes)
# Labels use the physical button printed on an Oculus Touch controller where one exists (A/B on
# the right controller, X/Y on the left) - matching the naming the module's help text already used
# before this was made configurable.
CONTROL_BINDING_EVENT_NAMES = {
    "X": "LeftButton1ClickEvent",
    "Y": "LeftButton2ClickEvent",
    "Left Menu": "LeftMenuClickEvent",
    "Left Trigger": "LeftTriggerClickEvent",
    "Left Stick Click": "LeftThumbstickClickEvent",
    "A": "RightButton1ClickEvent",
    "B": "RightButton2ClickEvent",
    "Right Trigger": "RightTriggerClickEvent",
    "Right Stick Click": "RightThumbstickClickEvent",
}
# Sentinel Choice value meaning "no button triggers this action" - a real, selectable option in
# the Controls UI's combo boxes (not just an implementation detail), so an action can be
# deliberately left with nothing bound (see nextSceneView/prevSceneView's defaults below, freed
# up now that the right-wall scene-view tiles cover scene selection). Deliberately NOT a key in
# CONTROL_BINDING_EVENT_NAMES (it has no event) - addAction/_controlSchemeBodyText special-case it.
CONTROL_BINDING_UNBOUND = "Unbound"
CONTROL_BINDING_LABELS = [CONTROL_BINDING_UNBOUND] + list(CONTROL_BINDING_EVENT_NAMES.keys())

# The module's nine button-triggered actions, in the order they're listed in the Controls UI
# section and generated into the back-wall signage's control-scheme text, paired with a short
# human-readable description used only for that signage text (not shown in the Controls UI,
# where the action itself is the row label - see VRStageControlBindings). Defaults (set on the
# parameterPack fields below) reproduce the module's original fixed bindings exactly, so behavior
# is unchanged until a user actually rebinds something in the Controls UI. HELP_BODY_LINE_COUNT
# below is derived from this list's length (+2 for the fixed rotate/grip lines) rather than
# hand-counted, so the back-wall signage panel auto-resizes if an action is ever added/removed.
CONTROL_ACTION_ORDER = [
    ("scaleUp", "scale up"),
    ("scaleDown", "scale down"),
    ("nextSceneView", "next scene view"),
    ("prevSceneView", "previous scene view"),
    ("resetFraming", "reset framing"),
    ("toggleReformatVisible", "show/hide reformat plane"),
    ("placeMeasurementPoint", "place measurement point"),
    ("undoMeasurement", "undo point/measurement"),
    ("toggleAutoSpin", "toggle auto-spin"),
]

# Back-wall signage: the control-scheme text lives on the wall behind the table (rather than
# crowding the table edge closest to the user), keeping the table itself uncluttered so the
# anatomy on it is easy to read accurately.
# The panel/text are held noticeably proud of the actual wall (tens of cm, not mm) - at the
# ~3m viewing distance out here, the same absolute gap that looks fine on the nearby table (see
# the mm-scale offsets above) resolves to far less usable z-buffer precision, so a small gap
# z-fights. The panel border is baked into its texture (see _signagePanelTexture) rather than
# a second coincident plane, for the same reason.
BACK_WALL_Z_M = -(ROOM_SIZE_M[2] / 2.0) + 0.05
# Baked text sits this far in front of its panel. With vtkTextActor3D the text used to float
# ~4 cm proud (only the glyphs were visible, so the gap never showed); an opaque caption quad
# that far out reads as a slab hovering in front of the panel in stereo. 1 mm is far beyond the
# VR depth buffer's resolution at room distances (microns), and the text mappers additionally get
# a polygon offset toward the viewer (see _BakedTextMixin.initText), so there is no z-fighting.
BAKED_TEXT_PROUD_M = 0.001
BACK_WALL_PANEL_OFFSET_M = 0.20   # panel in front of the wall
BACK_WALL_TEXT_OFFSET_M = BACK_WALL_PANEL_OFFSET_M + BAKED_TEXT_PROUD_M
HELP_PANEL_CENTER_Y_M = ROOM_CENTER_Y_M + 0.35
HELP_PANEL_WIDTH_M = 2.4
HELP_TITLE_HEIGHT_M = 0.14
HELP_BODY_HEIGHT_M = 0.085
HELP_BODY_LINE_COUNT = 1 + len(CONTROL_ACTION_ORDER) + 4  # rotate line + one per action + walk
                                                           # line + grip line + roll line +
                                                           # table-height line - worst
                                                           # case (every action bound); an
                                                           # Unbound one produces no line, see
                                                           # _controlSchemeBodyText
HELP_TITLE_BODY_GAP_M = 0.05      # deliberate breathing room between title and body
HELP_PANEL_TEXT_MARGIN_M = 0.05   # from the usable (border-excluded) interior edge to the text
HELP_PANEL_BORDER_FRAC = 0.05     # must match _signagePanelTexture's default borderFrac

# HELP_PANEL_HEIGHT_M is derived, not hand-tuned: the text renderer's rendered height for N lines at
# heightMeters is always <= N * heightMeters (measured ~0.955x), so budgeting with the nominal
# heightMeters values here is already conservative. Deriving the panel height from that budget -
# instead of picking one by eye - keeps title+body guaranteed to fit inside the border (previously
# they didn't) if the body text or font sizes above are ever edited.
_HELP_BODY_BLOCK_HEIGHT_M = HELP_BODY_LINE_COUNT * HELP_BODY_HEIGHT_M
_HELP_CONTENT_HEIGHT_M = (2.0 * HELP_PANEL_TEXT_MARGIN_M + HELP_TITLE_HEIGHT_M
                           + HELP_TITLE_BODY_GAP_M + _HELP_BODY_BLOCK_HEIGHT_M)
HELP_PANEL_HEIGHT_M = _HELP_CONTENT_HEIGHT_M / (1.0 - 2.0 * HELP_PANEL_BORDER_FRAC)

# Info screen content: the live scale (line 1) and current scene view name (line 2), rendered
# on the monitor housing built into the table's collar (see MONITOR_* below). Text layout is
# unchanged from the module's original standing-sign design - only the housing/mounting geometry
# around it changed.
INFO_SCREEN_LINE_HEIGHT_M = 0.035    # the scale readout ("1.00x") - short, room to stay large
INFO_SCREEN_LINE_GAP_M = 0.015
INFO_SCREEN_TEXT_MARGIN_M = 0.02
INFO_SCREEN_BORDER_FRAC = 0.05    # must match _signagePanelTexture's default borderFrac
# The scene-view name gets its own (smaller) line height, distinct from the scale line above it:
# on the monitor's narrower MONITOR_SCREEN_WIDTH_M, the old shared size overflowed the screen's
# edges for long names. Truncation (see _fitNameToScreenWidth / INFO_SCREEN_NAME_MAX_WIDTH_M
# below, once MONITOR_SCREEN_WIDTH_M is defined) is based on each name's actual rendered width,
# not a fixed character count - a fixed count either truncated ordinary short names that had
# plenty of room left, or would still overflow on unusually wide ones.
INFO_SCREEN_NAME_LINE_HEIGHT_M = 0.0105

_INFO_SCREEN_CONTENT_HEIGHT_M = (2.0 * INFO_SCREEN_TEXT_MARGIN_M + INFO_SCREEN_LINE_HEIGHT_M
                                  + INFO_SCREEN_NAME_LINE_HEIGHT_M + INFO_SCREEN_LINE_GAP_M)
INFO_SCREEN_HEIGHT_M = _INFO_SCREEN_CONTENT_HEIGHT_M / (1.0 - 2.0 * INFO_SCREEN_BORDER_FRAC)

# Monitor housing: a physically-modeled screen module (housing shell + textured screen face +
# live text), mounted into the table's collar near the edge closest to the user, reclined so
# it's legible without standing tall enough to occlude anatomy sitting further back on the
# table. See _buildMonitorAssembly.
MONITOR_SCREEN_WIDTH_M = 0.24        # narrower than the old standing sign - reads as one
                                       # embedded module now, not a sign
# The scene-view name is truncated to whatever actually fits this width (see
# _fitNameToScreenWidth), measured via the live text actor's own rendered bounds rather than a
# fixed character count - matches the screen's interior width, i.e. inside both the texture's
# baked border and the same text margin used for the vertical layout above.
INFO_SCREEN_NAME_MAX_WIDTH_M = (MONITOR_SCREEN_WIDTH_M * (1.0 - 2.0 * INFO_SCREEN_BORDER_FRAC)
                                  - 2.0 * INFO_SCREEN_TEXT_MARGIN_M)
MONITOR_BEZEL_MARGIN_M = 0.025       # housing overhang beyond the screen face, per side
MONITOR_HOUSING_DEPTH_M = 0.05
MONITOR_SCREEN_PROUD_M = 0.006       # screen face proud of the housing shell's front face
MONITOR_TEXT_PROUD_M = BAKED_TEXT_PROUD_M  # text proud of the screen face
MONITOR_MOUNT_PROUD_M = 0.015        # housing pulled proud of the collar's tangent radius
# RIM_BAND_HEIGHT_M must exceed the housing's bezel-inclusive footprint
# (INFO_SCREEN_HEIGHT_M + 2*MONITOR_BEZEL_MARGIN_M =~ 0.189m) so it fits inside the collar band
# with clearance top and bottom (checked, not eyeballed): 0.22m leaves ~1.5cm each side.
MONITOR_TILT_FROM_HORIZONTAL_DEG = 27.5   # midpoint of the agreed-on 25-30 degree recline
# The panel/text are authored in a vertical ("standing sign") local frame, same as the module's
# original design, then pivoted back to the shallow recline as one rigid group - see
# _buildMonitorAssembly for why a vtkAssembly pivot is used instead of repositioning each part.
MONITOR_HINGE_ROTATION_DEG = 90.0 - MONITOR_TILT_FROM_HORIZONTAL_DEG

# R/L/A/P/S/I orientation labels are authored directly in RAS/world (not anchored to physical
# space like the rest of the chrome - see _updateOrientationLabels), so they turn with the
# anatomy as the turntable spins and always show which anatomical direction currently faces the
# user. Each is a camera-facing cut-out letter (vtkFollower carrying baked text - see
# _BakedTextMixin) sized relative to the data, rather than a vtkBillboardTextActor3D: billboards
# are translucent quads, and translucent props in the VR view are very expensive (see the
# "baked text" section below).
ORIENTATION_LABEL_AXES = {
    "R": (1.0, 0.0, 0.0),
    "L": (-1.0, 0.0, 0.0),
    "A": (0.0, 1.0, 0.0),
    "P": (0.0, -1.0, 0.0),
    "S": (0.0, 0.0, 1.0),
    "I": (0.0, 0.0, -1.0),
}
ORIENTATION_LABEL_MARGIN_MM = 40.0
ORIENTATION_LABEL_DEFAULT_RADIUS_MM = 150.0
ORIENTATION_LABEL_FONT_SIZE = 96   # raster resolution only - world size is set from the data
                                   # (see _updateOrientationLabels); high so the cut-out edges
                                   # stay smooth when the letters are viewed up close
ORIENTATION_LABEL_HEIGHT_M = 0.035  # letter height in PHYSICAL meters - held constant across
                                    # magnification (see _updateOrientationLabelScale), the way
                                    # the old constant-screen-size billboards read

# Baked text: every piece of text in the room (signage, info screen, tile labels, orientation
# badges) is rendered ONCE by vtkTextRenderer into an opaque RGB texture on a plain quad, instead
# of using vtkTextActor3D / vtkBillboardTextActor3D. Those two are translucent props, and
# profiling in-headset (2064x2272 per eye, depth peeling on, which transparent data needs) showed
# each one costing ~1-2 ms PER FRAME just by existing - 14 of them were ~24 ms of a ~40 ms frame -
# whereas an opaque textured quad costs ~0.04 ms. Opaque means the text is composited over the
# colour of whatever panel it sits on (bgColor), so each quad reads as part of that panel.
BAKED_TEXT_DPI = 72        # vtkTextActor3D's default rendered DPI - keeps the font metrics (and
                           # therefore every layout constant above) identical to before
BAKED_TEXT_FONT_PX = 96    # authored font size; a quad is authored in pixels, then scaled so
                           # BAKED_TEXT_FONT_PX pixels == the requested height in meters

# Extra clearance between the anatomy's bottom and the table surface (see computePhysicalToWorld),
# so the data floats just above the table instead of sitting flush against it. Kept comfortably
# larger than ORIENTATION_LABEL_MARGIN_MM so the I label (which sits that margin below the data's
# bottom) still clears the table surface too, rather than poking into it.
TABLE_LIFT_BUFFER_MM = 60.0

# The physical point the data center is placed at (table top), and the physical "up" direction
# (true gravity) - the room chrome (floor/table/walls) is authored directly in physical meters
# along this axis and is therefore always level, however the world is currently oriented.
TABLE_PHYSICAL = (0.0, TABLE_HEIGHT_M + TABLE_TOP_THICKNESS_M, TABLE_FORWARD_M)
PHYSICAL_UP = (0.0, 1.0, 0.0)

# World "up" (RAS Superior) that _alignedBaseMatrix calibrates the reference matrix (M0) to at
# entry, so the data starts out standing upright on the table. _worldUp() re-derives the actual
# current world-space up from whichever matrix it's given, falling back to this constant only in
# the degenerate case - see _worldUp for why a live re-derivation (rather than trusting this
# constant everywhere) matters once the built-in free-move/rotate/scale gesture is used.
WORLD_UP_RAS = (0.0, 0.0, 1.0)

# The physical direction from the table back toward the user - the opposite of TABLE_FORWARD_M's
# -Z ("in front of the user"). Combined with ANTERIOR_RAS by _frontFacingYawRad so a reset faces
# the anatomy's Anterior side toward the user instead of whatever yaw the captured reference view
# (M0) happened to have.
PHYSICAL_TOWARD_USER = (0.0, 0.0, 1.0)

# MRML RAS convention: R=+X, A=+Y, S=+Z.
ANTERIOR_RAS = (0.0, 1.0, 0.0)

# Overhead light rig (authored in physical meters, anchored to the room like the chrome).
# The key light hangs near the ceiling above the table, mostly illuminating the top of the
# data; on its own a light straight down grazes vertical/side surfaces at a shallow, nearly
# azimuth-independent angle, leaving a large dim band around the sides no matter how the data
# is rotated on the turntable. The fill lights sit lower (near chest height) and spread evenly
# around the table so every side gets real coverage from at least one of them.
OVERHEAD_LIGHT_HEIGHT_M = ROOM_SIZE_M[1] - 0.2
OVERHEAD_LIGHT_COLOR = (1.0, 0.97, 0.92)  # warm white
OVERHEAD_LIGHT_INTENSITY = 0.9
FILL_LIGHT_HEIGHT_M = TABLE_HEIGHT_M + 0.5
FILL_LIGHT_RADIUS_M = 1.3                  # horizontal distance from the table center
FILL_LIGHT_ANGLES_DEG = (60.0, 180.0, 300.0)  # evenly spaced (120 degrees apart) around the table
FILL_LIGHT_INTENSITY_FACTOR = 0.6  # fraction of the key light's intensity, applied to each fill

# Ceiling light panels: a purely visual fixture (baked emissive texture, casts no actual light
# of its own) mounted just below the room's ceiling so the overhead rig above has a visible
# source, rather than the room appearing lit from nowhere. Only relevant when display.showWalls
# is set, since without walls there's no ceiling surface for it to read as being mounted into.
# Its background uses display.columnColor (the same tone as the column/post) - see _buildChrome.
CEILING_LIGHT_OFFSET_M = 0.01   # just inside the room cube's inner ceiling surface, avoids z-fighting
CEILING_LIGHT_EMISSIVE_FACTOR = (1.0, 1.0, 1.0)

MIN_MAGNIFICATION = 0.01
MAX_MAGNIFICATION = 100.0
DEFAULT_MAGNIFICATION = 1.0
# PhysicalToWorld column length (world mm per physical m) at magnification 1.0 (real-world size).
# SlicerVR convention: magnification = 1000 / physicalScale.
UNIT_MAGNIFICATION_SCALE = 1000.0

THUMBSTICK_DEADZONE = 0.25
INPUT_TIMER_INTERVAL_MS = 33  # ~30 Hz continuous-input update (turntable)
AUTO_SPIN_DEG_PER_SEC = 30.0    # hands-free presentation rotation speed

# Right-stick locomotion: the user walks around the room (head-relative move/strafe) while the
# room, table and data stay put - implemented as a horizontal physical->room offset composed
# into the PhysicalToWorldMatrix (see StageLocomotion/LocomotionMath). All in physical meters,
# so walk speed is real-world speed regardless of data magnification.
LOCOMOTION_SPEED_M_PER_S = 1.0    # walk speed at full stick deflection
# The headset itself (stick offset + wherever the user has physically walked in their playspace)
# is what's clamped to the room footprint, so the head can never pass through a wall.
LOCOMOTION_WALL_MARGIN_M = 0.3    # minimum headset distance from any wall (X/Z clamp)

SLICE_NODE_IDS = ["vtkMRMLSliceNodeRed", "vtkMRMLSliceNodeGreen", "vtkMRMLSliceNodeYellow"]

# Left/right wall launcher tiles: the left wall holds a fixed set of "load this atlas" tiles
# (ATLAS_SPECS, one press downloads+loads that atlas scene - see _buildAtlasWallTiles), the right
# wall holds one tile per Scene View in the current scene, built fresh at chrome-build time
# (_buildSceneViewWallTiles). Both walls share the same tile-panel geometry (_wallTilePanelActor/
# _wallTileLabelActor/_gridTileOffsets/_wallTileWorldPosition) and the same aim-ray-pick +
# button-press activation machinery (_wallTilePicker/_wallTileByActor/_hoveredWallTile) - see the
# "wall tile galleries" section further down for the picking/dispatch design.
WALL_TILE_WIDTH_M = 0.55
WALL_TILE_HEIGHT_M = 0.55
WALL_TILE_GUTTER_M = 0.12
WALL_TILE_LABEL_HEIGHT_M = 0.045
WALL_TILE_LABEL_MARGIN_M = 0.03
WALL_TILE_LABEL_FRAME_PX = 8       # caption-bar outline, in texture pixels at BAKED_TEXT_FONT_PX
# Proud-of-wall offsets for the tile panel/text, same z-fighting reasoning as
# BACK_WALL_PANEL_OFFSET_M/BACK_WALL_TEXT_OFFSET_M (side walls are a comparable ~3m from the user).
WALL_TILE_PANEL_PROUD_M = 0.20
WALL_TILE_TEXT_PROUD_M = WALL_TILE_PANEL_PROUD_M + BAKED_TEXT_PROUD_M
WALL_TILE_HOVER_COLOR = ACCENT_COLOR
WALL_TILE_NORMAL_COLOR = (1.0, 1.0, 1.0)

ATLAS_WALL_COLUMNS = 3
ATLAS_WALL_CENTER_Y_M = ROOM_CENTER_Y_M
ATLAS_WALL_CENTER_Z_M = TABLE_FORWARD_M

SCENE_VIEW_WALL_COLUMNS = 3
SCENE_VIEW_WALL_MAX_ROWS = 3
SCENE_VIEW_WALL_PAGE_SIZE = SCENE_VIEW_WALL_COLUMNS * SCENE_VIEW_WALL_MAX_ROWS  # tiles per page
SCENE_VIEW_WALL_CENTER_Z_M = TABLE_FORWARD_M
# Prev/page-indicator/Next row below the content grid, shown only when there's more than one page
# (see _buildSceneViewWallNavTiles). Sized from SCENE_VIEW_WALL_MAX_ROWS (the full page height),
# not the current page's actual row count, so the nav row sits at the same place on every page -
# including a short last page - rather than jumping up to hug a partially-filled grid.
SCENE_VIEW_WALL_CONTENT_HEIGHT_M = (
    SCENE_VIEW_WALL_MAX_ROWS * (WALL_TILE_HEIGHT_M + WALL_TILE_GUTTER_M) - WALL_TILE_GUTTER_M)
SCENE_VIEW_WALL_NAV_GAP_M = WALL_TILE_GUTTER_M
SCENE_VIEW_WALL_NAV_ROW_DV_M = (
    -SCENE_VIEW_WALL_CONTENT_HEIGHT_M / 2.0 - SCENE_VIEW_WALL_NAV_GAP_M - WALL_TILE_HEIGHT_M / 2.0)
# The content grid plus the nav row is ~2.56 m tall in a 3 m room, so unlike the atlas wall it
# cannot simply be centered at ROOM_CENTER_Y_M - that sank the nav row's bottom (and its
# captions) 11 cm into the floor. Anchor it from the floor instead: the nav row's bottom edge
# sits SCENE_VIEW_WALL_FLOOR_CLEARANCE_M above the floor, and the grid's center follows.
SCENE_VIEW_WALL_FLOOR_CLEARANCE_M = 0.15
SCENE_VIEW_WALL_CENTER_Y_M = (FLOOR_THICKNESS_M + SCENE_VIEW_WALL_FLOOR_CLEARANCE_M
                              + WALL_TILE_HEIGHT_M / 2.0 - SCENE_VIEW_WALL_NAV_ROW_DV_M)

# The three atlases from Slicer's own AtlasTests self-test module (Applications/SlicerApp/
# Testing/Python/AtlasTests.py in Slicer core - not part of this extension) - same fixed
# name/download parameters, reused here as one-press "load this atlas" wall tiles instead of
# AtlasTests' plain desktop buttons. "kind" picks which procedural pictogram _atlasTileTexture
# draws; "thumbnail" names a PNG in Resources/AtlasThumbnails/ (cropped from each atlas's
# embedded "General View" scene-view screenshot).
ATLAS_SPECS = [
    {
        "name": "Abdominal Atlas", "kind": "abdominal",
        "thumbnail": "abdominal.png",
        "fileNames": "Abdominal_Atlas_2012.mrb",
        "uris": TESTING_DATA_URL + "SHA256/5d315abf7d303326669c6075f9eea927eeda2e531a5b1662cfa505806cb498ea",
        "checksums": "SHA256:5d315abf7d303326669c6075f9eea927eeda2e531a5b1662cfa505806cb498ea",
    },
    {
        "name": "Brain Atlas", "kind": "brain",
        "thumbnail": "brain.png",
        "fileNames": "BrainAtlas2012.mrb",
        "uris": TESTING_DATA_URL + "SHA256/688ebcc6f45989795be2bcdc6b8b5bfc461f1656d677ed3ddef8c313532687f1",
        "checksums": "SHA256:688ebcc6f45989795be2bcdc6b8b5bfc461f1656d677ed3ddef8c313532687f1",
    },
    {
        "name": "Knee Atlas", "kind": "knee",
        "thumbnail": "knee.png",
        "fileNames": "KneeAtlas2012.mrb",
        "uris": TESTING_DATA_URL + "SHA256/5d5506c07c238918d0c892e7b04c26ad7f43684d89580780bb207d1d860b0b33",
        "checksums": "SHA256:5d5506c07c238918d0c892e7b04c26ad7f43684d89580780bb207d1d860b0b33",
    },
]
ATLAS_ICON_COLORS = {
    "abdominal": (0.85, 0.55, 0.25),  # warm amber
    "brain": (0.65, 0.35, 0.95),      # violet
    "knee": (0.35, 0.85, 0.45),       # green
}

# A built wall tile: its pickable panel actor (registered in _wallTileByActor/_wallTilePicker)
# and the zero-arg callback its activation runs (see _onPlaceMeasurementPoint/_activateWallTile).
_WallTile = collections.namedtuple("_WallTile", ["actor", "onActivate"])

# Arbitrary reformat slice: a plain model-node plane that follows a controller's pose for as
# long as its grip is held (_trackReformatPlaneToController), driving a dedicated, non-layout
# slice node's SliceToRAS. The reformatted image is shown on a floating screen that rides
# alongside the plane (see _updateReformatFromPlane), rather than coincident with it, so the
# handle and the crisp image never occupy the same surface. The handle is an opaque square FRAME
# (not a translucent filled plane, which - like any translucent prop - costs milliseconds per
# frame under depth peeling) so the anatomy it cuts through stays visible inside it.
REFORMAT_SLICE_LAYOUT_NAME = "VRReformat"
REFORMAT_PLANE_NODE_NAME = "VR Reformat Plane"
REFORMAT_HANDLE_FRAME_FRAC = 0.03  # frame bar width, as a fraction of the handle's side length
REFORMAT_HANDLE_SIZE_FRAC = 0.6   # handle side length, as a fraction of the background volume's
                                   # RAS bounding-box diagonal
DEFAULT_REFORMAT_HANDLE_SIZE_MM = 150.0  # fallback if no volume is loaded yet
REFORMAT_MONITOR_GAP_FRAC = 0.12  # gap between the handle's edge and the screen's edge, as a
                                   # fraction of the handle's half-width

# In-VR measurement tool: point-to-point distance markers for a solo review session, each a real
# vtkMRMLMarkupsLineNode (see the "measurement tool" section for why). MEASURE_COLOR is
# deliberately a different hue from ACCENT_COLOR so measurements read as a distinct layer of
# content from the chrome/orientation labels; MEASURE_RETICLE_RADIUS_MM sizes the raw-VTK aiming
# reticle, the only part of this tool that isn't a MRML node.
MEASURE_COLOR = (1.0, 0.65, 0.15)              # warm amber - tune in-headset
MEASURE_FLASH_COLOR = (1.0, 0.25, 0.2)         # "nothing to act on" feedback - tune in-headset
MEASURE_RETICLE_RADIUS_MM = 4.0
MEASURE_FLASH_DURATION_S = 0.3
MEASURE_GESTURE_SUPPRESS_WINDOW_S = 0.25       # tune in-headset - see _isButton1PressSuppressed


__all__ = [
    # Room geometry
    "FLOOR_RADIUS_M", "FLOOR_THICKNESS_M", "TABLE_RADIUS_M", "TABLE_TOP_THICKNESS_M",
    "TABLE_HEIGHT_M", "TABLE_FORWARD_M", "TABLE_HEIGHT_MIN_M", "TABLE_HEIGHT_MAX_M",
    "TABLE_MOVE_SPEED_M_PER_S", "TABLE_COMFORT_CENTER_HEIGHT_M",
    "COLUMN_RADIUS_M", "ROOM_SIZE_M", "ROOM_CENTER_Y_M",
    "RIM_BAND_RADIUS_M", "RIM_BAND_HEIGHT_M",
    # Palette
    "ACCENT_COLOR", "ACCENT_COLOR_DIM", "FLOOR_BASE_COLOR", "WALL_BASE_COLOR",
    "COLUMN_COLOR", "TABLE_RING_COLOR", "TABLE_SCREEN_BG_COLOR", "RIM_BAND_COLOR",
    # Table screen
    "TABLE_SCREEN_RADIUS_FRAC", "TABLE_SCREEN_RECESS_DEPTH_M", "TABLE_SCREEN_EMISSIVE_FACTOR",
    "COLLAR_SEAM_RING_INNER_FRAC", "COLLAR_SEAM_RING_OUTER_FRAC",
    "FLOOR_RING_INNER_M", "FLOOR_RING_OUTER_M",
    # Control bindings
    "CONTROL_BINDING_EVENT_NAMES", "CONTROL_BINDING_UNBOUND", "CONTROL_BINDING_LABELS",
    "CONTROL_ACTION_ORDER",
    # Signage / help panel
    "BACK_WALL_Z_M", "BAKED_TEXT_PROUD_M", "BACK_WALL_PANEL_OFFSET_M", "BACK_WALL_TEXT_OFFSET_M",
    "HELP_PANEL_CENTER_Y_M", "HELP_PANEL_WIDTH_M", "HELP_TITLE_HEIGHT_M", "HELP_BODY_HEIGHT_M",
    "HELP_BODY_LINE_COUNT", "HELP_TITLE_BODY_GAP_M", "HELP_PANEL_TEXT_MARGIN_M",
    "HELP_PANEL_BORDER_FRAC", "HELP_PANEL_HEIGHT_M",
    # Info screen / monitor
    "INFO_SCREEN_LINE_HEIGHT_M", "INFO_SCREEN_LINE_GAP_M", "INFO_SCREEN_TEXT_MARGIN_M",
    "INFO_SCREEN_BORDER_FRAC", "INFO_SCREEN_NAME_LINE_HEIGHT_M", "INFO_SCREEN_HEIGHT_M",
    "MONITOR_SCREEN_WIDTH_M", "INFO_SCREEN_NAME_MAX_WIDTH_M", "MONITOR_BEZEL_MARGIN_M",
    "MONITOR_HOUSING_DEPTH_M", "MONITOR_SCREEN_PROUD_M", "MONITOR_TEXT_PROUD_M",
    "MONITOR_MOUNT_PROUD_M", "MONITOR_TILT_FROM_HORIZONTAL_DEG", "MONITOR_HINGE_ROTATION_DEG",
    # Orientation labels
    "ORIENTATION_LABEL_AXES", "ORIENTATION_LABEL_MARGIN_MM", "ORIENTATION_LABEL_DEFAULT_RADIUS_MM",
    "ORIENTATION_LABEL_FONT_SIZE", "ORIENTATION_LABEL_HEIGHT_M",
    # Baked text
    "BAKED_TEXT_DPI", "BAKED_TEXT_FONT_PX",
    # Framing / axes
    "TABLE_LIFT_BUFFER_MM", "TABLE_PHYSICAL", "PHYSICAL_UP", "WORLD_UP_RAS",
    "PHYSICAL_TOWARD_USER", "ANTERIOR_RAS",
    # Lighting
    "OVERHEAD_LIGHT_HEIGHT_M", "OVERHEAD_LIGHT_COLOR", "OVERHEAD_LIGHT_INTENSITY",
    "FILL_LIGHT_HEIGHT_M", "FILL_LIGHT_RADIUS_M", "FILL_LIGHT_ANGLES_DEG",
    "FILL_LIGHT_INTENSITY_FACTOR",
    "CEILING_LIGHT_OFFSET_M", "CEILING_LIGHT_EMISSIVE_FACTOR",
    # Magnification / input
    "MIN_MAGNIFICATION", "MAX_MAGNIFICATION", "DEFAULT_MAGNIFICATION", "UNIT_MAGNIFICATION_SCALE",
    "THUMBSTICK_DEADZONE", "INPUT_TIMER_INTERVAL_MS", "AUTO_SPIN_DEG_PER_SEC",
    "LOCOMOTION_SPEED_M_PER_S", "LOCOMOTION_WALL_MARGIN_M",
    "SLICE_NODE_IDS",
    # Wall tiles
    "WALL_TILE_WIDTH_M", "WALL_TILE_HEIGHT_M", "WALL_TILE_GUTTER_M", "WALL_TILE_LABEL_HEIGHT_M",
    "WALL_TILE_LABEL_MARGIN_M", "WALL_TILE_LABEL_FRAME_PX",
    "WALL_TILE_PANEL_PROUD_M", "WALL_TILE_TEXT_PROUD_M",
    "WALL_TILE_HOVER_COLOR", "WALL_TILE_NORMAL_COLOR",
    "ATLAS_WALL_COLUMNS", "ATLAS_WALL_CENTER_Y_M", "ATLAS_WALL_CENTER_Z_M",
    "SCENE_VIEW_WALL_COLUMNS", "SCENE_VIEW_WALL_MAX_ROWS", "SCENE_VIEW_WALL_PAGE_SIZE",
    "SCENE_VIEW_WALL_CENTER_Z_M", "SCENE_VIEW_WALL_CONTENT_HEIGHT_M",
    "SCENE_VIEW_WALL_NAV_GAP_M", "SCENE_VIEW_WALL_NAV_ROW_DV_M",
    "SCENE_VIEW_WALL_FLOOR_CLEARANCE_M", "SCENE_VIEW_WALL_CENTER_Y_M",
    "ATLAS_SPECS", "ATLAS_ICON_COLORS",
    # Reformat
    "REFORMAT_SLICE_LAYOUT_NAME", "REFORMAT_PLANE_NODE_NAME",
    "REFORMAT_HANDLE_FRAME_FRAC", "REFORMAT_HANDLE_SIZE_FRAC",
    "DEFAULT_REFORMAT_HANDLE_SIZE_MM", "REFORMAT_MONITOR_GAP_FRAC",
    # Measurement
    "MEASURE_COLOR", "MEASURE_FLASH_COLOR", "MEASURE_RETICLE_RADIUS_MM",
    "MEASURE_FLASH_DURATION_S", "MEASURE_GESTURE_SUPPRESS_WINDOW_S",
]
