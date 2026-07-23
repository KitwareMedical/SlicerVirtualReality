"""Scene-view enumeration and navigation for VR Stage."""

import slicer


def sceneViewsLogic():
    """The SceneViews module's logic, or None if unavailable.

    The modern SceneViews module stores scene views inside a sequence browser, not as
    top-level vtkMRMLSceneViewNode nodes, so we go through its logic to enumerate/restore.
    """
    try:
        return slicer.modules.sceneviews.logic()
    except Exception:  # noqa: BLE001
        return None


class SceneViewNavigator:
    """Index bookkeeping for cycling/jumping among scene views.

    No back-reference to VRStageLogic; the post-restore fan-out (recomputeDataBounds,
    readout update) stays in Logic.
    """

    def __init__(self):
        self.currentIndex = -1

    def count(self) -> int:
        logic = sceneViewsLogic()
        return logic.GetNumberOfSceneViews() if logic else 0

    def restore(self, index) -> bool:
        """Restore the scene view at *index*. Returns True on success."""
        logic = sceneViewsLogic()
        if logic is None or not (0 <= index < logic.GetNumberOfSceneViews()):
            return False
        self.currentIndex = index
        logic.RestoreSceneView(index)
        return True

    def wrappedIndex(self, direction) -> int:
        """Next/previous index with wrap-around, or None if there are no scene views."""
        logic = sceneViewsLogic()
        if logic is None:
            return None
        count = logic.GetNumberOfSceneViews()
        if count <= 0:
            return None
        return (self.currentIndex + (1 if direction > 0 else -1)) % count
