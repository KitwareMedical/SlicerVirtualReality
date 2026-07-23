"""Pure transform / fit / bounds math extracted from VRStageLogic.

Every function here is headless-testable (no VR, no renderer, no MRML scene
dependency beyond the optional slicer import in collectVisibleDataNodes).
"""

import math

import vtk

import slicer

from .Constants import (
    ANTERIOR_RAS,
    PHYSICAL_TOWARD_USER,
    PHYSICAL_UP,
    TABLE_COMFORT_CENTER_HEIGHT_M,
    TABLE_HEIGHT_M,
    TABLE_HEIGHT_MAX_M,
    TABLE_HEIGHT_MIN_M,
    TABLE_LIFT_BUFFER_MM,
    TABLE_TOP_THICKNESS_M,
    WORLD_UP_RAS,
)


def extentAlongAxis(bounds, axis):
    """Extent (max-min projection) of an RAS AABB onto an axis. 0 for empty bounds."""
    if bounds[0] > bounds[1]:
        return 0.0
    projections = []
    for xi in (bounds[0], bounds[1]):
        for yi in (bounds[2], bounds[3]):
            for zi in (bounds[4], bounds[5]):
                projections.append(xi * axis[0] + yi * axis[1] + zi * axis[2])
    return max(projections) - min(projections)


def alignedBaseMatrix(baseMatrix):
    """Rotate the captured reference matrix (M0) about its own origin so that physical
    up (true gravity) maps exactly onto WORLD_UP_RAS.

    This just sets the data's *starting* orientation (Superior pointing at the ceiling);
    worldUp() re-derives the actual axis afterwards on every rotate/scale, so this
    alignment isn't required for correctness, only so the data starts upright.
    """
    physicalUp = list(baseMatrix.MultiplyPoint([0.0, 1.0, 0.0, 0.0]))[:3]
    norm = vtk.vtkMath.Norm(physicalUp)
    if norm < 1e-9:
        return baseMatrix
    physicalUp = [c / norm for c in physicalUp]

    target = list(WORLD_UP_RAS)
    axis = [0.0, 0.0, 0.0]
    vtk.vtkMath.Cross(physicalUp, target, axis)
    axisNorm = vtk.vtkMath.Norm(axis)
    if axisNorm < 1e-9:
        return baseMatrix
    axis = [c / axisNorm for c in axis]
    angleDeg = vtk.vtkMath.DegreesFromRadians(vtk.vtkMath.AngleBetweenVectors(physicalUp, target))

    rotation = vtk.vtkTransform()
    rotation.RotateWXYZ(angleDeg, axis[0], axis[1], axis[2])
    rotationMatrix = vtk.vtkMatrix4x4()
    rotation.GetMatrix(rotationMatrix)

    corrected = vtk.vtkMatrix4x4()
    vtk.vtkMatrix4x4.Multiply4x4(rotationMatrix, baseMatrix, corrected)
    for i in range(3):
        corrected.SetElement(i, 3, baseMatrix.GetElement(i, 3))
    return corrected


def worldUp(matrix):
    """The world/RAS direction that `matrix` currently maps physical up to. Falls back to
    WORLD_UP_RAS in the degenerate (zero-length) case."""
    up = list(matrix.MultiplyPoint([PHYSICAL_UP[0], PHYSICAL_UP[1], PHYSICAL_UP[2], 0.0]))[:3]
    norm = vtk.vtkMath.Norm(up)
    return [c / norm for c in up] if norm > 1e-9 else list(WORLD_UP_RAS)


def frontFacingYawRad(baseMatrix):
    """The angle to rotate about worldUp(baseMatrix) that spins RAS Anterior to face
    the physical front of the table (toward the user), so a reset/scene-view-change
    presents the anatomy front-on."""
    up = worldUp(baseMatrix)
    towardUser = list(baseMatrix.MultiplyPoint(
        [PHYSICAL_TOWARD_USER[0], PHYSICAL_TOWARD_USER[1], PHYSICAL_TOWARD_USER[2], 0.0]))[:3]

    def projectPerpendicular(v):
        d = vtk.vtkMath.Dot(v, up)
        projected = [v[i] - d * up[i] for i in range(3)]
        norm = vtk.vtkMath.Norm(projected)
        return [c / norm for c in projected] if norm > 1e-9 else None

    anterior = projectPerpendicular(list(ANTERIOR_RAS))
    towardUser = projectPerpendicular(towardUser)
    if anterior is None or towardUser is None:
        return 0.0

    cross = [0.0, 0.0, 0.0]
    vtk.vtkMath.Cross(anterior, towardUser, cross)
    return math.atan2(vtk.vtkMath.Dot(cross, up), vtk.vtkMath.Dot(anterior, towardUser))


def computePhysicalToWorld(baseMatrix, relScale, angleRad, dataBounds, dataCenter, tablePhysical):
    """Build the VR PhysicalToWorldMatrix that makes the data appear placed on the table,
    scaled by world factor `relScale`, and spun by `angleRad`, while keeping the
    reference-view orientation in `baseMatrix` (M0)."""
    up = worldUp(baseMatrix)
    tableWorld = list(baseMatrix.MultiplyPoint(
        [tablePhysical[0], tablePhysical[1], tablePhysical[2], 1.0]))[:3]

    halfHeight = 0.5 * extentAlongAxis(dataBounds, up) * relScale
    liftBuffer = TABLE_LIFT_BUFFER_MM * relScale
    target = [tableWorld[i] + up[i] * (halfHeight + liftBuffer) for i in range(3)]

    w = vtk.vtkTransform()
    w.PostMultiply()
    w.Translate(-dataCenter[0], -dataCenter[1], -dataCenter[2])
    w.Scale(relScale, relScale, relScale)
    w.RotateWXYZ(vtk.vtkMath.DegreesFromRadians(angleRad), up[0], up[1], up[2])
    w.Translate(target[0], target[1], target[2])
    wMatrix = vtk.vtkMatrix4x4()
    w.GetMatrix(wMatrix)

    wInverse = vtk.vtkMatrix4x4()
    vtk.vtkMatrix4x4.Invert(wMatrix, wInverse)
    result = vtk.vtkMatrix4x4()
    vtk.vtkMatrix4x4.Multiply4x4(wInverse, baseMatrix, result)
    return result


def computeDefaultTableHeightM(baseMatrix, relScale, dataBounds):
    """Tabletop height (meters) that places the data's vertical center at
    TABLE_COMFORT_CENTER_HEIGHT_M physical, clamped to [TABLE_HEIGHT_MIN_M,
    TABLE_HEIGHT_MAX_M]."""
    if dataBounds[0] > dataBounds[1]:
        return TABLE_HEIGHT_M
    s0 = linearScale(baseMatrix)
    if s0 < 1e-9:
        return TABLE_HEIGHT_M
    up = worldUp(baseMatrix)
    halfHeightWorld = 0.5 * extentAlongAxis(dataBounds, up) * relScale
    liftWorld = TABLE_LIFT_BUFFER_MM * relScale
    centerAboveTopM = (halfHeightWorld + liftWorld) / s0
    height = TABLE_COMFORT_CENTER_HEIGHT_M - TABLE_TOP_THICKNESS_M - centerAboveTopM
    return max(TABLE_HEIGHT_MIN_M, min(TABLE_HEIGHT_MAX_M, height))


def matrixScale(matrix):
    """Uniform scale of a rigid+scale 4x4 (length of its first column), or None."""
    if matrix is None:
        return None
    return vtk.vtkMath.Norm([matrix.GetElement(0, 0), matrix.GetElement(1, 0), matrix.GetElement(2, 0)])


def linearScale(matrix):
    """World-per-physical scale factor of a PhysicalToWorld matrix (length of a column)."""
    return (matrix.GetElement(0, 0) ** 2 + matrix.GetElement(1, 0) ** 2
            + matrix.GetElement(2, 0) ** 2) ** 0.5


def collectVisibleDataNodes():
    """Displayable data nodes the user would consider 'on the table': visible models,
    segmentations, markups, fiber bundles, and volumes shown via volume rendering."""
    nodes = []
    scene = slicer.mrmlScene
    for className in ("vtkMRMLModelNode", "vtkMRMLSegmentationNode", "vtkMRMLMarkupsNode", "vtkMRMLFiberBundleNode"):
        collection = scene.GetNodesByClass(className)
        collection.UnRegister(None)
        for i in range(collection.GetNumberOfItems()):
            node = collection.GetItemAsObject(i)
            if node.GetHideFromEditors():
                continue
            displayNode = node.GetDisplayNode()
            if displayNode is None or not displayNode.GetVisibility():
                continue
            nodes.append(node)

    volumes = scene.GetNodesByClass("vtkMRMLVolumeNode")
    volumes.UnRegister(None)
    for i in range(volumes.GetNumberOfItems()):
        volume = volumes.GetItemAsObject(i)
        if volume.GetHideFromEditors():
            continue
        for j in range(volume.GetNumberOfDisplayNodes()):
            displayNode = volume.GetNthDisplayNode(j)
            if displayNode and displayNode.IsA("vtkMRMLVolumeRenderingDisplayNode") and displayNode.GetVisibility():
                nodes.append(volume)
                break
    return nodes


def combinedRASBounds(nodes):
    """Union of the RAS bounding boxes of the displayable nodes. Empty -> zeros."""
    combined = None
    for node in nodes:
        if not node.IsA("vtkMRMLDisplayableNode"):
            continue
        bounds = [0.0] * 6
        node.GetRASBounds(bounds)
        if bounds[0] > bounds[1]:
            continue
        if combined is None:
            combined = list(bounds)
        else:
            for a in range(3):
                combined[2 * a] = min(combined[2 * a], bounds[2 * a])
                combined[2 * a + 1] = max(combined[2 * a + 1], bounds[2 * a + 1])
    return combined if combined is not None else [0.0, 0.0, 0.0, 0.0, 0.0, 0.0]


def combinedRASCenter(nodes):
    b = combinedRASBounds(nodes)
    return [(b[0] + b[1]) / 2.0, (b[2] + b[3]) / 2.0, (b[4] + b[5]) / 2.0]


def steppedMagnification(current, direction, stepFactor):
    """Pure helper: multiplicative scale step, clamped to [MIN, MAX]."""
    from .Constants import MAX_MAGNIFICATION, MIN_MAGNIFICATION
    value = current * stepFactor if direction > 0 else current / stepFactor
    return max(MIN_MAGNIFICATION, min(MAX_MAGNIFICATION, value))
