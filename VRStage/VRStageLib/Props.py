"""Stateless VTK actor / texture / polydata factory functions.

Every function here creates and returns a new VTK object with no dependency on
VRStageLogic state.  Converted from ``VRStageLogic`` ``@staticmethod`` /
``@classmethod`` entries; the leading underscores are dropped (public module API).
"""

import functools
import math

import numpy as np
import vtk
from vtk.util import numpy_support

from .BakedText import BakedFollowerTextActor, BakedTextActor
from .Constants import *            # noqa: F401,F403
from .Constants import _HELP_BODY_BLOCK_HEIGHT_M, _rgbF


# ------------------------------------------------------------------ actor factories


def physicalLight(position, focalPoint, color, intensity):
    """A directional (non-positional) light authored in physical meters."""
    light = vtk.vtkLight()
    light.SetLightTypeToSceneLight()
    light.SetPositional(False)
    light.SetPosition(*position)
    light.SetFocalPoint(*focalPoint)
    light.SetColor(*color)
    light.SetIntensity(intensity)
    return light


def discActor(center, radius, height, color):
    """A flat cylinder (axis = Y) used for floor/table/column."""
    source = vtk.vtkCylinderSource()
    source.SetRadius(radius)
    source.SetHeight(height)
    source.SetCenter(center[0], center[1], center[2])
    source.SetResolution(64)
    mapper = vtk.vtkPolyDataMapper()
    mapper.SetInputConnection(source.GetOutputPort())
    actor = vtk.vtkActor()
    actor.SetMapper(mapper)
    actor.GetProperty().SetColor(*color)
    actor.GetProperty().SetAmbient(0.3)
    actor.GetProperty().SetDiffuse(0.7)
    actor.PickableOff()
    return actor


def boxActor(center, size, color):
    """A small solid, flat-shaded box."""
    source = vtk.vtkCubeSource()
    source.SetXLength(size[0])
    source.SetYLength(size[1])
    source.SetZLength(size[2])
    source.SetCenter(center[0], center[1], center[2])
    mapper = vtk.vtkPolyDataMapper()
    mapper.SetInputConnection(source.GetOutputPort())
    actor = vtk.vtkActor()
    actor.SetMapper(mapper)
    actor.GetProperty().SetColor(*color)
    actor.GetProperty().SetAmbient(0.3)
    actor.GetProperty().SetDiffuse(0.7)
    actor.PickableOff()
    return actor


def roomActor(color, texture):
    """A large box seen from the inside, wearing a tiled wall-panel texture."""
    source = vtk.vtkCubeSource()
    source.SetXLength(ROOM_SIZE_M[0])
    source.SetYLength(ROOM_SIZE_M[1])
    source.SetZLength(ROOM_SIZE_M[2])
    source.SetCenter(0.0, ROOM_CENTER_Y_M, 0.0)
    tile = vtk.vtkTransformTextureCoords()
    tile.SetInputConnection(source.GetOutputPort())
    tile.SetScale(10.0, 5.0, 10.0)
    mapper = vtk.vtkPolyDataMapper()
    mapper.SetInputConnection(tile.GetOutputPort())
    actor = vtk.vtkActor()
    actor.SetMapper(mapper)
    actor.SetTexture(texture)
    actor.GetProperty().SetColor(*color)
    actor.GetProperty().FrontfaceCullingOn()
    actor.GetProperty().BackfaceCullingOff()
    actor.GetProperty().SetAmbient(0.5)
    actor.GetProperty().SetDiffuse(0.5)
    actor.PickableOff()
    return actor


def ceilingLightActor(bgColor, panelColor):
    """A flat panel mounted just below the ceiling, textured with a grid of bright
    fixtures and fed to the PBR emissive pipeline."""
    halfWidth, halfDepth = ROOM_SIZE_M[0] / 2.0, ROOM_SIZE_M[2] / 2.0
    y = ROOM_SIZE_M[1] - CEILING_LIGHT_OFFSET_M
    source = vtk.vtkPlaneSource()
    source.SetOrigin(-halfWidth, y, halfDepth)
    source.SetPoint1(halfWidth, y, halfDepth)
    source.SetPoint2(-halfWidth, y, -halfDepth)
    mapper = vtk.vtkPolyDataMapper()
    mapper.SetInputConnection(source.GetOutputPort())
    actor = vtk.vtkActor()
    actor.SetMapper(mapper)
    texture = arrayToTexture(ceilingPanelTexture(bgColor, panelColor))
    texture.UseSRGBColorSpaceOn()
    prop = actor.GetProperty()
    prop.SetInterpolationToPBR()
    prop.SetBaseColorTexture(texture)
    prop.SetEmissiveTexture(texture)
    prop.SetEmissiveFactor(*CEILING_LIGHT_EMISSIVE_FACTOR)
    actor.PickableOff()
    return actor


def texturedDiscActor(center, radius, texture, innerRadius=0.0, color=(1.0, 1.0, 1.0),
                      opacity=1.0, ambient=0.6, diffuse=0.4, resolution=64):
    """A flat disc (normal = +Y) carrying a planar texture."""
    source = vtk.vtkDiskSource()
    source.SetInnerRadius(innerRadius)
    source.SetOuterRadius(radius)
    source.SetRadialResolution(1)
    source.SetCircumferentialResolution(resolution)
    rotation = vtk.vtkTransform()
    rotation.RotateX(-90.0)
    rotate = vtk.vtkTransformPolyDataFilter()
    rotate.SetTransform(rotation)
    rotate.SetInputConnection(source.GetOutputPort())
    tmap = vtk.vtkTextureMapToPlane()
    tmap.SetInputConnection(rotate.GetOutputPort())
    mapper = vtk.vtkPolyDataMapper()
    mapper.SetInputConnection(tmap.GetOutputPort())
    actor = vtk.vtkActor()
    actor.SetMapper(mapper)
    actor.SetTexture(texture)
    actor.SetPosition(center[0], center[1], center[2])
    prop = actor.GetProperty()
    prop.SetColor(*color)
    prop.SetAmbient(ambient)
    prop.SetDiffuse(diffuse)
    prop.SetOpacity(opacity)
    actor.PickableOff()
    return actor


def glowRingActor(center, innerRadius, outerRadius, color=ACCENT_COLOR, opacity=1.0,
                  resolution=96):
    """A flat, self-lit (ambient-only) ring used for neon trim."""
    source = vtk.vtkDiskSource()
    source.SetInnerRadius(innerRadius)
    source.SetOuterRadius(outerRadius)
    source.SetRadialResolution(1)
    source.SetCircumferentialResolution(resolution)
    rotation = vtk.vtkTransform()
    rotation.RotateX(-90.0)
    rotate = vtk.vtkTransformPolyDataFilter()
    rotate.SetTransform(rotation)
    rotate.SetInputConnection(source.GetOutputPort())
    mapper = vtk.vtkPolyDataMapper()
    mapper.SetInputConnection(rotate.GetOutputPort())
    actor = vtk.vtkActor()
    actor.SetMapper(mapper)
    actor.SetPosition(center[0], center[1], center[2])
    prop = actor.GetProperty()
    prop.SetColor(*color)
    prop.SetAmbient(1.0)
    prop.SetDiffuse(0.0)
    prop.SetOpacity(opacity)
    actor.PickableOff()
    return actor


def annulusActor(center, innerRadius, outerRadius, height, color, resolution=64):
    """A solid ring with a real hole through it (flat annulus extruded to real thickness)."""
    source = vtk.vtkDiskSource()
    source.SetInnerRadius(innerRadius)
    source.SetOuterRadius(outerRadius)
    source.SetRadialResolution(1)
    source.SetCircumferentialResolution(resolution)
    rotation = vtk.vtkTransform()
    rotation.RotateX(-90.0)
    rotate = vtk.vtkTransformPolyDataFilter()
    rotate.SetTransform(rotation)
    rotate.SetInputConnection(source.GetOutputPort())
    extrude = vtk.vtkLinearExtrusionFilter()
    extrude.SetInputConnection(rotate.GetOutputPort())
    extrude.SetExtrusionTypeToVectorExtrusion()
    extrude.SetVector(0.0, -height, 0.0)
    extrude.CappingOn()
    mapper = vtk.vtkPolyDataMapper()
    mapper.SetInputConnection(extrude.GetOutputPort())
    actor = vtk.vtkActor()
    actor.SetMapper(mapper)
    actor.SetPosition(center[0], center[1], center[2])
    prop = actor.GetProperty()
    prop.SetColor(*color)
    prop.SetAmbient(0.3)
    prop.SetDiffuse(0.7)
    actor.PickableOff()
    return actor


def worldGlowDotActor(radius, color, resolution=16):
    """A small self-lit sphere authored directly in RAS/world."""
    source = vtk.vtkSphereSource()
    source.SetRadius(radius)
    source.SetThetaResolution(resolution)
    source.SetPhiResolution(resolution)
    mapper = vtk.vtkPolyDataMapper()
    mapper.SetInputConnection(source.GetOutputPort())
    actor = vtk.vtkActor()
    actor.SetMapper(mapper)
    prop = actor.GetProperty()
    prop.SetColor(*color)
    prop.SetAmbient(1.0)
    prop.SetDiffuse(0.0)
    actor.PickableOff()
    actor.VisibilityOff()
    return actor


# ------------------------------------------------------------------ procedural textures


def arrayToTexture(rgbArray):
    """uint8 HxWx3 numpy array -> vtkTexture."""
    height, width, _channels = rgbArray.shape
    image = vtk.vtkImageData()
    image.SetDimensions(width, height, 1)
    flatRGB = np.flipud(rgbArray).reshape(-1, 3).copy()
    dataArray = numpy_support.numpy_to_vtk(flatRGB, deep=True, array_type=vtk.VTK_UNSIGNED_CHAR)
    image.GetPointData().SetScalars(dataArray)
    texture = vtk.vtkTexture()
    texture.SetInputData(image)
    texture.InterpolateOn()
    texture.MipmapOn()
    texture.RepeatOn()
    return texture


def textureFromImageFile(path):
    """Load an image file (PNG, JPG, etc.) as a vtkTexture via VTK's image reader."""
    reader = vtk.vtkPNGReader()
    reader.SetFileName(path)
    reader.Update()
    texture = vtk.vtkTexture()
    texture.SetInputConnection(reader.GetOutputPort())
    texture.InterpolateOn()
    return texture


def wallPanelTexture(bgColor, size=256):
    """Subtle bright panel-line grid for the room walls."""
    bg = np.array(bgColor) * 255.0
    line = np.array([0.55, 0.75, 0.85]) * 255.0
    img = np.tile(bg.astype(np.uint8), (size, size, 1))
    spacing = size // 4
    for i in range(0, size, spacing):
        img[max(i - 1, 0):i + 1, :, :] = line.astype(np.uint8)
        img[:, max(i - 1, 0):i + 1, :] = line.astype(np.uint8)
    return img


def floorPanelTexture(bgColor, size=512):
    """Bright steel floor grid, matching the wall paneling."""
    bg = np.array(bgColor) * 255.0
    line = np.array([0.45, 0.55, 0.62]) * 255.0
    img = np.tile(bg.astype(np.uint8), (size, size, 1))
    spacing = size // 8
    for i in range(0, size, spacing):
        img[max(i - 1, 0):i + 1, :, :] = line.astype(np.uint8)
        img[:, max(i - 1, 0):i + 1, :] = line.astype(np.uint8)
    return img


def ceilingPanelTexture(bgColor, panelColor, size=512, rows=2, cols=3, marginFrac=0.10):
    """Grid of bright rectangular light-fixture panels on a dark ceiling background."""
    bg = np.array(bgColor) * 255.0
    panel = np.array(panelColor) * 255.0
    img = np.tile(bg.astype(np.uint8), (size, size, 1))
    cellH, cellW = size / rows, size / cols
    for r in range(rows):
        for c in range(cols):
            y0 = int(r * cellH + cellH * marginFrac)
            y1 = int((r + 1) * cellH - cellH * marginFrac)
            x0 = int(c * cellW + cellW * marginFrac)
            x1 = int((c + 1) * cellW - cellW * marginFrac)
            img[y0:y1, x0:x1, :] = panel.astype(np.uint8)
    return img


@functools.lru_cache(maxsize=4)
def tableScreenTexture(bgColor, ringColor, size=2048):
    """Concentric rings + radial spokes on a dark background - a circuit/targeting-pad
    look for the holo-readout inset in the tabletop."""
    bg = np.array(bgColor) * 255.0
    ring = np.array(ringColor) * 255.0
    img = np.tile(bg.astype(np.uint8), (size, size, 1))
    yy, xx = np.mgrid[0:size, 0:size]
    cx = cy = (size - 1) / 2.0
    r = np.sqrt((xx - cx) ** 2 + (yy - cy) ** 2) / (size / 2.0)
    theta = np.arctan2(yy - cy, xx - cx)
    ringMask = (np.abs(np.sin(r * math.pi * 6.0)) > 0.97) & (r < 0.96)
    spokeMask = (np.abs(np.sin(theta * 8.0)) < 0.02) & (r > 0.12) & (r < 0.96)
    edgeMask = (r > 0.93) & (r < 0.97)
    img[ringMask | spokeMask | edgeMask] = ring.astype(np.uint8)
    return img


def signagePanelTexture(bgColor, borderColor, size=512, borderFrac=0.05):
    """Dark background with an accent border baked in."""
    bg = np.array(bgColor) * 255.0
    border = np.array(borderColor) * 255.0
    img = np.tile(bg.astype(np.uint8), (size, size, 1))
    edge = int(size * borderFrac)
    img[:edge, :, :] = border.astype(np.uint8)
    img[-edge:, :, :] = border.astype(np.uint8)
    img[:, :edge, :] = border.astype(np.uint8)
    img[:, -edge:, :] = border.astype(np.uint8)
    return img


def atlasTileTexture(kind, bgColor, borderColor, size=512):
    """Atlas tile background: signagePanelTexture's bordered panel, plus a simple colored
    pictogram (a circle) so the three atlas tiles are visually distinct at a glance."""
    img = signagePanelTexture(bgColor, borderColor, size=size)
    iconColor = np.array(ATLAS_ICON_COLORS.get(kind, ACCENT_COLOR)) * 255.0
    center = size // 2
    radius = int(size * 0.22)
    yy, xx = np.ogrid[:size, :size]
    mask = (xx - center) ** 2 + (yy - int(size * 0.4)) ** 2 <= radius ** 2
    img[mask] = iconColor.astype(np.uint8)
    return img


# ------------------------------------------------------------------ text / signage factories


def textActor(position, heightMeters, color=(0.9, 0.95, 1.0), orientationDeg=(0.0, 0.0, 0.0),
              bgColor=(0.0, 0.0, 0.0), ambient=0.9, diffuse=0.1):
    """An opaque baked-text quad authored in physical meters, facing +Z by default."""
    actor = BakedTextActor()
    actor.initText(BAKED_TEXT_FONT_PX, color, bgColor)
    actor.GetTextProperty().SetJustificationToCentered()
    actor.GetTextProperty().SetVerticalJustificationToBottom()
    actor.SetInput(" ")
    prop = actor.GetProperty()
    prop.SetAmbient(ambient)
    prop.SetDiffuse(diffuse)
    scale = heightMeters / float(BAKED_TEXT_FONT_PX)
    actor.SetScale(scale, scale, scale)
    actor.SetPosition(position[0], position[1], position[2])
    actor.SetOrientation(*orientationDeg)
    return actor


def orientationLabelActor(camera, text, color=ACCENT_COLOR, fontSize=ORIENTATION_LABEL_FONT_SIZE):
    """A camera-facing cut-out letter anchored at a world/RAS point."""
    actor = BakedFollowerTextActor()
    actor.initText(fontSize, color)
    tprop = actor.GetTextProperty()
    tprop.SetBold(True)
    tprop.ShadowOn()
    tprop.SetJustificationToCentered()
    tprop.SetVerticalJustificationToCentered()
    actor.SetInput(text)
    actor.SetCamera(camera)
    actor.GetProperty().LightingOff()
    return actor


def controlSchemeBodyText(controls):
    """The back-wall signage's control-scheme text, generated from the current button
    bindings rather than hardcoded."""
    lines = ["L-stick: rotate/pitch turntable"]
    for fieldName, description in CONTROL_ACTION_ORDER:
        binding = getattr(controls, fieldName)
        if binding == CONTROL_BINDING_UNBOUND:
            continue
        lines.append(f"{binding}: {description}")
    lines.append("Either grip (hold): move reformat plane")
    lines.append("Left grip (hold) + L-stick L/R: roll")
    lines.append("Left grip (hold) + L-stick U/D: table height")
    return "\n".join(lines)


def backWallSignageActors(display, controls):
    """A signage panel on the back wall, behind the table, holding the control-scheme
    text."""
    from slicer.i18n import tr as _

    bgColor = _rgbF(display.tableScreenBackgroundColor)
    accentColor = _rgbF(display.accentColor)
    halfW = HELP_PANEL_WIDTH_M / 2.0
    halfH = HELP_PANEL_HEIGHT_M / 2.0
    panelSource = vtk.vtkPlaneSource()
    panelSource.SetOrigin(-halfW, -halfH, 0.0)
    panelSource.SetPoint1(halfW, -halfH, 0.0)
    panelSource.SetPoint2(-halfW, halfH, 0.0)
    mapper = vtk.vtkPolyDataMapper()
    mapper.SetInputConnection(panelSource.GetOutputPort())
    panel = vtk.vtkActor()
    panel.SetMapper(mapper)
    panel.SetTexture(arrayToTexture(signagePanelTexture(bgColor, accentColor)))
    panel.SetPosition(0.0, HELP_PANEL_CENTER_Y_M, BACK_WALL_Z_M + BACK_WALL_PANEL_OFFSET_M)
    panelProp = panel.GetProperty()
    panelProp.SetColor(1.0, 1.0, 1.0)
    panelProp.SetAmbient(0.9)
    panelProp.SetDiffuse(0.1)
    panel.PickableOff()

    interiorHalfH = halfH * (1.0 - 2.0 * HELP_PANEL_BORDER_FRAC)
    titleBottomY = HELP_PANEL_CENTER_Y_M + interiorHalfH - HELP_PANEL_TEXT_MARGIN_M - HELP_TITLE_HEIGHT_M
    bodyBottomY = titleBottomY - HELP_TITLE_BODY_GAP_M - _HELP_BODY_BLOCK_HEIGHT_M

    textZ = BACK_WALL_Z_M + BACK_WALL_TEXT_OFFSET_M
    title = textActor(
        position=(0.0, titleBottomY, textZ),
        heightMeters=HELP_TITLE_HEIGHT_M, color=accentColor, bgColor=bgColor)
    title.SetInput(_("VR VIEWER CONTROLS"))

    body = textActor(
        position=(0.0, bodyBottomY, textZ),
        heightMeters=HELP_BODY_HEIGHT_M, color=(0.75, 0.90, 0.95), bgColor=bgColor)
    body.SetInput(controlSchemeBodyText(controls))

    return [panel, title, body]


# ------------------------------------------------------------------ wall tile factories


def gridTileOffsets(count, columns, tileWidth, tileHeight, gutter):
    """Row-major grid of `count` tiles, `columns` wide, centered on (0, 0)."""
    if count <= 0:
        return []
    rows = (count + columns - 1) // columns
    cellW, cellH = tileWidth + gutter, tileHeight + gutter
    gridHeight = rows * cellH - gutter
    topV = gridHeight / 2.0
    offsets = []
    for i in range(count):
        row, col = divmod(i, columns)
        colsInRow = min(columns, count - row * columns)
        rowWidth = colsInRow * cellW - gutter
        rowLeftU = -rowWidth / 2.0
        u = rowLeftU + col * cellW + tileWidth / 2.0
        v = topV - row * cellH - tileHeight / 2.0
        offsets.append((u, v))
    return offsets


def wallTileWorldPosition(side, centerY, centerZ, du, dv):
    """Maps a grid-local (du, dv) offset to a world (x, y, z) tile center."""
    y = centerY + dv
    if side == "left":
        return (-ROOM_SIZE_M[0] / 2.0 + WALL_TILE_PANEL_PROUD_M, y, centerZ - du)
    return (ROOM_SIZE_M[0] / 2.0 - WALL_TILE_PANEL_PROUD_M, y, centerZ + du)


def wallTilePanelActor(side, x, y, z, width, height, texture):
    """The tile's pickable background panel in the Y-Z plane (side walls)."""
    halfW, halfH = width / 2.0, height / 2.0
    source = vtk.vtkPlaneSource()
    if side == "left":
        source.SetOrigin(x, y - halfH, z + halfW)
        source.SetPoint1(x, y - halfH, z - halfW)
        source.SetPoint2(x, y + halfH, z + halfW)
    else:
        source.SetOrigin(x, y - halfH, z - halfW)
        source.SetPoint1(x, y - halfH, z + halfW)
        source.SetPoint2(x, y + halfH, z - halfW)
    mapper = vtk.vtkPolyDataMapper()
    mapper.SetInputConnection(source.GetOutputPort())
    actor = vtk.vtkActor()
    actor.SetMapper(mapper)
    actor.SetTexture(texture)
    prop = actor.GetProperty()
    prop.SetColor(*WALL_TILE_NORMAL_COLOR)
    prop.SetAmbient(0.9)
    prop.SetDiffuse(0.1)
    prop.BackfaceCullingOff()
    return actor


def wallTileLabelActor(side, y, z, height, text, captionBg, frameColor, color=(0.9, 0.95, 1.0)):
    """The tile's name label, held proud of the wall and rotated to face into the room."""
    if side == "left":
        labelX = -ROOM_SIZE_M[0] / 2.0 + WALL_TILE_TEXT_PROUD_M
        orientationDeg = (0.0, 90.0, 0.0)
    else:
        labelX = ROOM_SIZE_M[0] / 2.0 - WALL_TILE_TEXT_PROUD_M
        orientationDeg = (0.0, -90.0, 0.0)
    actor = textActor(
        (labelX, y - height / 2.0 + WALL_TILE_LABEL_MARGIN_M, z),
        WALL_TILE_LABEL_HEIGHT_M, color=color, orientationDeg=orientationDeg, bgColor=captionBg)
    tprop = actor.GetTextProperty()
    tprop.SetFrame(True)
    tprop.SetFrameColor(*frameColor)
    tprop.SetFrameWidth(WALL_TILE_LABEL_FRAME_PX)
    actor.SetInput(text)
    return actor


# ------------------------------------------------------------------ reformat tool polydata


def squareFramePolyData(halfSize, barWidth):
    """A flat square frame in the XY plane (+Z normal), centered on the origin."""
    outer, inner = halfSize, halfSize - barWidth
    points = vtk.vtkPoints()
    for x, y in ((-outer, -outer), (outer, -outer), (outer, outer), (-outer, outer),
                 (-inner, -inner), (inner, -inner), (inner, inner), (-inner, inner)):
        points.InsertNextPoint(x, y, 0.0)
    quads = vtk.vtkCellArray()
    for a, b in ((0, 1), (1, 2), (2, 3), (3, 0)):
        quads.InsertNextCell(4, [a, b, b + 4, a + 4])
    polyData = vtk.vtkPolyData()
    polyData.SetPoints(points)
    polyData.SetPolys(quads)
    return polyData


def buildReformatMonitorActor(halfSize):
    """The floating screen: a plain textured quad authored directly in RAS/world
    coordinates, repositioned each update to ride alongside the reformat plane."""
    halfW, halfH = halfSize
    source = vtk.vtkPlaneSource()
    source.SetOrigin(-halfW, -halfH, 0.0)
    source.SetPoint1(halfW, -halfH, 0.0)
    source.SetPoint2(-halfW, halfH, 0.0)
    mapper = vtk.vtkPolyDataMapper()
    mapper.SetInputConnection(source.GetOutputPort())
    actor = vtk.vtkActor()
    actor.SetMapper(mapper)
    prop = actor.GetProperty()
    prop.SetColor(1.0, 1.0, 1.0)
    prop.SetAmbient(0.9)
    prop.SetDiffuse(0.1)
    prop.BackfaceCullingOff()
    actor.PickableOff()
    return actor
