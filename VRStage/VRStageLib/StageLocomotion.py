"""Locomotion state for VR Stage — the user's walked-to position within the room.

``StageLocomotion`` owns the physical->room offset that lets the user walk around the
room (right thumbstick) while the room, table and data stay put.  The offset composes
into the render window's matrix as ``PhysicalToWorld = roomToWorld x L`` (see
VRStageLogic's physical-to-world pipeline); room chrome stays anchored to roomToWorld,
so changing only L moves the user through a world-fixed room.

Collaborator owned by VRStageLogic — no back-reference, no renderer/render-window
access; head pose and gaze are passed in already converted to room coordinates.  The
offset is stored as two scalars (not an accumulated matrix), so 30 Hz updates cannot
drift and the clamp math stays pure (see LocomotionMath).  A yaw term (artificial
turning) can be added here later without changing the Logic seam.
"""

from .Constants import (
    LOCOMOTION_SPEED_M_PER_S,
    LOCOMOTION_WALL_MARGIN_M,
    ROOM_SIZE_M,
    THUMBSTICK_DEADZONE,
)
from . import LocomotionMath


class StageLocomotion:
    """The user's horizontal physical->room offset, in meters."""

    def __init__(self):
        self.offsetXM = 0.0
        self.offsetZM = 0.0

    def reset(self) -> None:
        self.offsetXM = 0.0
        self.offsetZM = 0.0

    def isIdentity(self) -> bool:
        return abs(self.offsetXM) < 1e-12 and abs(self.offsetZM) < 1e-12

    def matrix(self):
        """The physical->room transform L as a vtkMatrix4x4."""
        return LocomotionMath.locomotionMatrix(self.offsetXM, self.offsetZM)

    def inverseMatrix(self):
        return LocomotionMath.locomotionMatrix(-self.offsetXM, -self.offsetZM)

    def roomFromPhysicalPoint(self, p3):
        return [p3[0] + self.offsetXM, p3[1], p3[2] + self.offsetZM]

    def roomFromPhysicalDirection(self, d3):
        # Identity rotation today; the seam point where a future yaw term would apply.
        return [d3[0], d3[1], d3[2]]

    def update(self, dt, headRoom, forwardRoom, stickX, stickY) -> bool:
        """Advance the offset for one input tick.  ``headRoom`` is the headset position
        and ``forwardRoom`` the horizontal gaze unit vector, both in room coordinates
        (gaze may be None — no walk that tick).  Returns True iff the offset changed."""
        vx, vz = LocomotionMath.walkVelocity(
            stickX, stickY, forwardRoom, THUMBSTICK_DEADZONE, LOCOMOTION_SPEED_M_PER_S)
        if vx == 0.0 and vz == 0.0:
            return False
        dx, dz = LocomotionMath.clampWalkDelta(
            headRoom[0], headRoom[2], vx * dt, vz * dt, ROOM_SIZE_M, LOCOMOTION_WALL_MARGIN_M)
        if abs(dx) < 1e-9 and abs(dz) < 1e-9:
            return False
        self.offsetXM += dx
        self.offsetZM += dz
        return True
