"""Interactive wall-tile galleries for VR Stage.

Two walls of one-press tiles: the left wall is a fixed set of "load this atlas" tiles
(ATLAS_SPECS); the right wall shows one tile per scene view in the scene, paginated.
Both share the same grid-layout math, textured-panel-plus-label construction, and
aim-ray-pick + button-press activation machinery.

Picking: tile panels are the one exception to "every VRStage chrome actor calls
PickableOff()" (see the measurement tool's picking invariant) — so they get their own
dedicated, pick-list-restricted picker instead of relying on that invariant.
"""

import logging
import math
import os

import vtk
import qt

import slicer
from slicer.i18n import tr as _

from .Constants import (
    ACCENT_COLOR,
    ATLAS_SPECS,
    ATLAS_WALL_CENTER_Y_M,
    ATLAS_WALL_CENTER_Z_M,
    ATLAS_WALL_COLUMNS,
    SCENE_VIEW_WALL_CENTER_Y_M,
    SCENE_VIEW_WALL_CENTER_Z_M,
    SCENE_VIEW_WALL_COLUMNS,
    SCENE_VIEW_WALL_NAV_ROW_DV_M,
    SCENE_VIEW_WALL_PAGE_SIZE,
    WALL_TILE_GUTTER_M,
    WALL_TILE_HEIGHT_M,
    WALL_TILE_HOVER_COLOR,
    WALL_TILE_NORMAL_COLOR,
    WALL_TILE_WIDTH_M,
    _WallTile,
    _rgbF,
)
from .MeasurementTool import MeasurementTool
from .SceneViews import sceneViewsLogic
from . import Props


class WallTileGallery:
    """Collaborator owned by VRStageLogic — no back-reference.

    Activation callbacks are passed at ``build`` time and stored; ``teardown`` clears
    them so no Logic-bound closure outlives a session.
    """

    def __init__(self):
        self.tileByActor = {}
        self.hoveredTile = None
        self.sceneViewPage = 0
        self.sceneViewActors = []
        self._picker = None
        self._onActivateAtlas = None
        self._onRestoreSceneView = None
        self._onPageRequested = None

    def build(self, renderer, display, anchorMatrix, chromeProps,
              onActivateAtlas, onRestoreSceneView, onPageRequested):
        """Build both tile walls and add them to *renderer*.

        *chromeProps* is the shared list owned by RoomChrome; tile actors are appended
        to it so they participate in reanchor ``Modified()`` calls.  *anchorMatrix* is
        the room-space ``vtkMatrix4x4`` identity assigned as ``UserMatrix``.

        *onActivateAtlas(atlasSpec)*, *onRestoreSceneView(index)*, and
        *onPageRequested(page)* are callbacks into VRStageLogic.
        """
        self._onActivateAtlas = onActivateAtlas
        self._onRestoreSceneView = onRestoreSceneView
        self._onPageRequested = onPageRequested

        roomProps = []
        if display.showAtlasWall:
            roomProps.extend(self._buildAtlasWallTiles(display))
        if display.showSceneViewWall:
            self.sceneViewActors = self._buildSceneViewWallTiles(display)
            roomProps.extend(self.sceneViewActors)
        else:
            self.sceneViewActors = []

        for prop in roomProps:
            prop.SetUserMatrix(anchorMatrix)
            renderer.AddViewProp(prop)
        chromeProps.extend(roomProps)

        self._rebuildPicker()

    def teardown(self, renderer) -> None:
        self.tileByActor = {}
        self.hoveredTile = None
        self.sceneViewPage = 0
        self.sceneViewActors = []
        self._picker = None
        self._onActivateAtlas = None
        self._onRestoreSceneView = None
        self._onPageRequested = None

    def _buildAtlasWallTiles(self, display):
        bgColor = _rgbF(display.wallColor)
        borderColor = _rgbF(display.accentColor)
        captionBg = _rgbF(display.tableScreenBackgroundColor)
        offsets = Props.gridTileOffsets(
            len(ATLAS_SPECS), ATLAS_WALL_COLUMNS, WALL_TILE_WIDTH_M, WALL_TILE_HEIGHT_M,
            WALL_TILE_GUTTER_M)
        thumbnailDir = os.path.join(
            os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
            "Resources", "AtlasThumbnails")
        actors = []
        for atlasSpec, (du, dv) in zip(ATLAS_SPECS, offsets):
            x, y, z = Props.wallTileWorldPosition(
                "left", ATLAS_WALL_CENTER_Y_M, ATLAS_WALL_CENTER_Z_M, du, dv)
            thumbnailPath = os.path.join(thumbnailDir, atlasSpec.get("thumbnail", ""))
            if os.path.isfile(thumbnailPath):
                texture = Props.textureFromImageFile(thumbnailPath)
            else:
                texture = Props.arrayToTexture(Props.atlasTileTexture(atlasSpec["kind"], bgColor, borderColor))
            panel = Props.wallTilePanelActor(
                "left", x, y, z, WALL_TILE_WIDTH_M, WALL_TILE_HEIGHT_M, texture)
            label = Props.wallTileLabelActor(
                "left", y, z, WALL_TILE_HEIGHT_M, atlasSpec["name"], captionBg, borderColor)
            self.tileByActor[panel] = _WallTile(
                panel, (lambda spec=atlasSpec: self._activateWallTile(
                    lambda s=spec: self._onActivateAtlas(s))))
            actors.extend([panel, label])
        return actors

    def _buildSceneViewWallTiles(self, display):
        bgColor = _rgbF(display.wallColor)
        borderColor = _rgbF(display.accentColor)
        captionBg = _rgbF(display.tableScreenBackgroundColor)
        logic = sceneViewsLogic()
        totalCount = logic.GetNumberOfSceneViews() if logic is not None else 0

        actors = []
        if totalCount <= 0:
            self.sceneViewPage = 0
            texture = Props.arrayToTexture(Props.signagePanelTexture(bgColor, borderColor))
            x, y, z = Props.wallTileWorldPosition(
                "right", SCENE_VIEW_WALL_CENTER_Y_M, SCENE_VIEW_WALL_CENTER_Z_M, 0.0, 0.0)
            panel = Props.wallTilePanelActor(
                "right", x, y, z, WALL_TILE_WIDTH_M, WALL_TILE_HEIGHT_M, texture)
            panel.PickableOff()
            label = Props.wallTileLabelActor(
                "right", y, z, WALL_TILE_HEIGHT_M, _("No scene views saved"), captionBg, borderColor)
            return [panel, label]

        pageCount = math.ceil(totalCount / SCENE_VIEW_WALL_PAGE_SIZE)
        self.sceneViewPage = max(0, min(self.sceneViewPage, pageCount - 1))
        startIndex = self.sceneViewPage * SCENE_VIEW_WALL_PAGE_SIZE
        count = min(totalCount - startIndex, SCENE_VIEW_WALL_PAGE_SIZE)

        offsets = Props.gridTileOffsets(
            count, SCENE_VIEW_WALL_COLUMNS, WALL_TILE_WIDTH_M, WALL_TILE_HEIGHT_M,
            WALL_TILE_GUTTER_M)
        for i, (du, dv) in enumerate(offsets):
            index = startIndex + i
            x, y, z = Props.wallTileWorldPosition(
                "right", SCENE_VIEW_WALL_CENTER_Y_M, SCENE_VIEW_WALL_CENTER_Z_M, du, dv)
            screenshot = logic.GetNthSceneViewScreenshot(index)
            if screenshot is not None and screenshot.GetDimensions()[0] > 1:
                texture = vtk.vtkTexture()
                texture.SetInputData(screenshot)
                texture.InterpolateOn()
            else:
                texture = Props.arrayToTexture(Props.signagePanelTexture(bgColor, borderColor))
            panel = Props.wallTilePanelActor(
                "right", x, y, z, WALL_TILE_WIDTH_M, WALL_TILE_HEIGHT_M, texture)
            name = logic.GetNthSceneViewName(index) or _("(unnamed)")
            label = Props.wallTileLabelActor(
                "right", y, z, WALL_TILE_HEIGHT_M, name, captionBg, borderColor)
            self.tileByActor[panel] = _WallTile(
                panel, (lambda i=index: self._activateWallTile(
                    lambda idx=i: self._onRestoreSceneView(idx))))
            actors.extend([panel, label])

        if pageCount > 1:
            actors.extend(self._buildSceneViewWallNavTiles(display, pageCount))
        return actors

    def _buildSceneViewWallNavTiles(self, display, pageCount):
        bgColor = _rgbF(display.wallColor)
        accentColor = _rgbF(display.accentColor)
        captionBg = _rgbF(display.tableScreenBackgroundColor)
        navOffsets = Props.gridTileOffsets(
            3, SCENE_VIEW_WALL_COLUMNS, WALL_TILE_WIDTH_M, WALL_TILE_HEIGHT_M, WALL_TILE_GUTTER_M)
        currentPage = self.sceneViewPage
        navSpecs = [
            (currentPage > 0, _("< Prev Page"),
             (lambda p=currentPage - 1: self._onPageRequested(p))),
            (False, _("Page {current} / {total}").format(current=currentPage + 1, total=pageCount), None),
            (currentPage < pageCount - 1, _("Next Page >"),
             (lambda p=currentPage + 1: self._onPageRequested(p))),
        ]
        actors = []
        for (enabled, text, callback), (du, _dv) in zip(navSpecs, navOffsets):
            x, y, z = Props.wallTileWorldPosition(
                "right", SCENE_VIEW_WALL_CENTER_Y_M, SCENE_VIEW_WALL_CENTER_Z_M,
                du, SCENE_VIEW_WALL_NAV_ROW_DV_M)
            borderColor = accentColor if enabled else bgColor
            texture = Props.arrayToTexture(Props.signagePanelTexture(bgColor, borderColor))
            panel = Props.wallTilePanelActor(
                "right", x, y, z, WALL_TILE_WIDTH_M, WALL_TILE_HEIGHT_M, texture)
            label = Props.wallTileLabelActor(
                "right", y, z, WALL_TILE_HEIGHT_M, text, captionBg, borderColor)
            if enabled and callback is not None:
                self.tileByActor[panel] = _WallTile(
                    panel, (lambda cb=callback: self._activateWallTile(cb)))
            else:
                panel.PickableOff()
            actors.extend([panel, label])
        return actors

    def rebuildSceneViewWall(self, renderer, display, anchorMatrix, chromeProps) -> None:
        """Tear down and rebuild just the right wall, in place."""
        if renderer is None:
            return
        self.setHoveredTile(None)
        for actor in self.sceneViewActors:
            renderer.RemoveViewProp(actor)
            if actor in chromeProps:
                chromeProps.remove(actor)
            self.tileByActor.pop(actor, None)
        self.sceneViewActors = self._buildSceneViewWallTiles(display)
        for actor in self.sceneViewActors:
            actor.SetUserMatrix(anchorMatrix)
            renderer.AddViewProp(actor)
        chromeProps.extend(self.sceneViewActors)
        self._rebuildPicker()

    def _rebuildPicker(self) -> None:
        picker = vtk.vtkCellPicker()
        picker.PickFromListOn()
        for actor in self.tileByActor:
            picker.AddPickList(actor)
        self._picker = picker

    def setHoveredTile(self, tile) -> None:
        if tile is self.hoveredTile:
            return
        if self.hoveredTile is not None:
            self.hoveredTile.actor.GetProperty().SetColor(*WALL_TILE_NORMAL_COLOR)
        if tile is not None:
            tile.actor.GetProperty().SetColor(*WALL_TILE_HOVER_COLOR)
        self.hoveredTile = tile

    def pickTile(self, pos, ori, renderer):
        """Per-frame aim-ray pick against the tile-only picker. Returns the hovered tile."""
        if renderer is None or self._picker is None or not self.tileByActor:
            self.setHoveredTile(None)
            return None
        if not MeasurementTool._isFinitePickRay(pos, ori, renderer):
            return self.hoveredTile
        hit = self._picker.Pick3DRay(pos, ori, renderer)
        tile = self.tileByActor.get(self._picker.GetActor()) if hit else None
        self.setHoveredTile(tile)
        return tile

    def markActorsModified(self) -> None:
        """Mark scene-view wall actors modified after a reanchor."""
        for actor in self.sceneViewActors:
            actor.Modified()

    def _activateWallTile(self, callback) -> None:
        """Defer activation by one Qt event-loop tick to avoid mutating observer state
        mid-dispatch (atlas activation calls exitViewerMode reentrantly)."""
        qt.QTimer.singleShot(0, lambda cb=callback: self._runActivation(cb))

    @staticmethod
    def _runActivation(callback) -> None:
        try:
            callback()
        except Exception:  # noqa: BLE001
            logging.exception("VR Stage wall tile activation failed")
