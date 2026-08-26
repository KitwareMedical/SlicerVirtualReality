"""Arbitrary reformat slice tool for VR: a grabbable plane that drives a live
reformatted image on a floating monitor."""

import vtk

import slicer

from .Constants import (
    ACCENT_COLOR,
    DEFAULT_REFORMAT_HANDLE_SIZE_MM,
    REFORMAT_HANDLE_FRAME_FRAC,
    REFORMAT_HANDLE_SIZE_FRAC,
    REFORMAT_MONITOR_GAP_FRAC,
    REFORMAT_PLANE_NODE_NAME,
    REFORMAT_SLICE_LAYOUT_NAME,
)
from . import Props


class ReformatTool:
    """Collaborator owned by VRStageLogic that encapsulates the reformat plane,
    slice pipeline, floating monitor, and grip-tracking state."""

    def __init__(self):
        self.planeModelNode = None
        self.transformNode = None
        self.sliceNode = None
        self.compositeNode = None
        self.sliceLogic = None
        self.monitorActor = None
        self.monitorHalfSize = (0.0, 0.0)
        self.gripHeldSide = None
        self.visible = False

    def setup(self, renderer) -> None:
        scene = slicer.mrmlScene

        backgroundVolume, size, center = self._backgroundVolumeAndGeometry()
        self.monitorHalfSize = (size / 2.0, size / 2.0)

        halfSize = size / 2.0
        modelNode = scene.AddNewNodeByClass("vtkMRMLModelNode", REFORMAT_PLANE_NODE_NAME)
        modelNode.SetAndObservePolyData(
            Props.squareFramePolyData(halfSize, size * REFORMAT_HANDLE_FRAME_FRAC))
        modelNode.SetHideFromEditors(True)
        modelNode.SetSaveWithScene(False)
        modelNode.CreateDefaultDisplayNodes()

        displayNode = modelNode.GetDisplayNode()
        if displayNode is not None:
            displayNode.SetColor(*ACCENT_COLOR)
            displayNode.SetOpacity(1.0)
            displayNode.SetBackfaceCulling(False)
            displayNode.SetAmbient(0.9)
            displayNode.SetDiffuse(0.1)
            displayNode.SetVisibility2D(False)

        transformNode = scene.AddNewNodeByClass(
            "vtkMRMLLinearTransformNode", REFORMAT_PLANE_NODE_NAME + " Transform")
        transformNode.SetHideFromEditors(True)
        transformNode.SetSaveWithScene(False)
        initialMatrix = vtk.vtkMatrix4x4()
        initialMatrix.SetElement(0, 3, center[0])
        initialMatrix.SetElement(1, 3, center[1])
        initialMatrix.SetElement(2, 3, center[2])
        transformNode.SetMatrixTransformToParent(initialMatrix)
        modelNode.SetAndObserveTransformNodeID(transformNode.GetID())

        self.planeModelNode = modelNode
        self.transformNode = transformNode

        sliceLogic = slicer.vtkMRMLSliceLogic()
        sliceLogic.SetMRMLScene(scene)
        sliceNode = sliceLogic.AddSliceNode(REFORMAT_SLICE_LAYOUT_NAME)
        sliceNode.SetHideFromEditors(True)
        sliceNode.SetSaveWithScene(False)
        sliceNode.SetSliceVisible(False)
        sliceNode.SetFieldOfView(size, size, 1.0)

        compositeNode = sliceLogic.GetSliceCompositeNode()
        if compositeNode is not None:
            compositeNode.SetHideFromEditors(True)
            compositeNode.SetSaveWithScene(False)
            if backgroundVolume is not None:
                compositeNode.SetBackgroundVolumeID(backgroundVolume.GetID())

        self.sliceLogic = sliceLogic
        self.sliceNode = sliceNode
        self.compositeNode = compositeNode

        self.monitorActor = Props.buildReformatMonitorActor(self.monitorHalfSize)
        rgb = vtk.vtkImageExtractComponents()
        rgb.SetInputConnection(sliceLogic.GetExtractModelTexture().GetOutputPort())
        rgb.SetComponents(0, 1, 2)
        texture = vtk.vtkTexture()
        texture.SetInputConnection(rgb.GetOutputPort())
        texture.InterpolateOn()
        self.monitorActor.SetTexture(texture)
        renderer.AddViewProp(self.monitorActor)

        self.updateFromPlane()
        self._applyVisibility()

    def teardown(self, renderer) -> None:
        if renderer is not None and self.monitorActor is not None:
            renderer.RemoveViewProp(self.monitorActor)
        self.monitorActor = None

        self.sliceLogic = None
        self.compositeNode = None

        scene = slicer.mrmlScene
        if self.planeModelNode is not None:
            scene.RemoveNode(self.planeModelNode)
        self.planeModelNode = None

        if self.transformNode is not None:
            scene.RemoveNode(self.transformNode)
        self.transformNode = None

        if self.sliceNode is not None:
            scene.RemoveNode(self.sliceNode)
        self.sliceNode = None

    @staticmethod
    def _backgroundVolumeAndGeometry():
        volume = None
        redComposite = slicer.mrmlScene.GetNodeByID("vtkMRMLSliceCompositeNodeRed")
        if redComposite is not None:
            volumeID = redComposite.GetBackgroundVolumeID()
            if volumeID:
                volume = slicer.mrmlScene.GetNodeByID(volumeID)
        if volume is None:
            volumes = slicer.mrmlScene.GetNodesByClass("vtkMRMLScalarVolumeNode")
            volumes.UnRegister(None)
            if volumes.GetNumberOfItems() > 0:
                volume = volumes.GetItemAsObject(0)

        size = DEFAULT_REFORMAT_HANDLE_SIZE_MM
        center = [0.0, 0.0, 0.0]
        if volume is not None:
            bounds = [0.0] * 6
            volume.GetRASBounds(bounds)
            if bounds[0] <= bounds[1]:
                diagonal = ((bounds[1] - bounds[0]) ** 2 + (bounds[3] - bounds[2]) ** 2
                            + (bounds[5] - bounds[4]) ** 2) ** 0.5
                if diagonal > 1e-6:
                    size = diagonal * REFORMAT_HANDLE_SIZE_FRAC
                center = [(bounds[0] + bounds[1]) / 2.0, (bounds[2] + bounds[3]) / 2.0,
                          (bounds[4] + bounds[5]) / 2.0]
        return volume, size, center

    def onGripClick(self, side, isPress, calldata) -> None:
        if isPress:
            self.gripHeldSide = side
            self.trackPlaneToController(calldata)
        elif self.gripHeldSide == side:
            self.gripHeldSide = None

    def trackPlaneToController(self, calldata) -> None:
        if self.transformNode is None:
            return
        try:
            pos = calldata.GetWorldPosition()
            ori = calldata.GetWorldOrientation()
        except Exception:  # noqa: BLE001
            return
        matrix = vtk.vtkMatrix4x4()
        vtk.vtkMatrix4x4.PoseToMatrix(pos, ori, matrix)
        self.transformNode.SetMatrixTransformToParent(matrix)
        self.updateFromPlane()

    def toggleVisible(self) -> None:
        self.visible = not self.visible
        self._applyVisibility()

    def _applyVisibility(self) -> None:
        if self.planeModelNode is not None:
            displayNode = self.planeModelNode.GetDisplayNode()
            if displayNode is not None:
                displayNode.SetVisibility(self.visible)
        if self.monitorActor is not None:
            self.monitorActor.SetVisibility(self.visible)

    def updateFromPlane(self) -> None:
        """Rebuild the reformat slice's SliceToRAS from the plane's transform, and move the
        floating screen to ride alongside it."""
        if self.transformNode is None or self.sliceNode is None:
            return

        matrix = vtk.vtkMatrix4x4()
        self.transformNode.GetMatrixTransformToParent(matrix)

        sliceToRAS = self.sliceNode.GetSliceToRAS()
        sliceToRAS.DeepCopy(matrix)
        self.sliceNode.UpdateMatrices()

        if self.monitorActor is not None:
            halfW, _halfH = self.monitorHalfSize
            offset = 2.0 * halfW + halfW * REFORMAT_MONITOR_GAP_FRAC
            xAxis = [matrix.GetElement(i, 0) for i in range(3)]
            monitorMatrix = vtk.vtkMatrix4x4()
            monitorMatrix.DeepCopy(matrix)
            for i in range(3):
                monitorMatrix.SetElement(i, 3, matrix.GetElement(i, 3) + xAxis[i] * offset)
            self.monitorActor.SetUserMatrix(monitorMatrix)
