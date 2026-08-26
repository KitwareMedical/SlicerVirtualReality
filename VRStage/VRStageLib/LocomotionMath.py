"""Pure locomotion math for VR Stage — walking the user around the room.

Every function here is headless-testable (no VR, no renderer, no MRML scene).

Coordinate/sign conventions (all in "room" coordinates — the authored physical frame
of Constants.py, meters): forward is -Z, up is +Y, right is +X, the room footprint is
centered on the origin.  A walker's rightward direction is ``forward x up``
(check: (0,0,-1) x (0,1,0) = (1,0,0)).  Stick +Y walks along the horizontal gaze
direction, stick +X strafes right.
"""

import math

import vtk


def horizontalUnit(v3):
    """The horizontal ([x, 0, z]) unit vector of a 3-vector, or None if the horizontal
    part is degenerate (e.g. gaze straight up/down) or non-finite."""
    x, z = v3[0], v3[2]
    if not (math.isfinite(x) and math.isfinite(z)):
        return None
    norm = math.hypot(x, z)
    if norm < 1e-6:
        return None
    return [x / norm, 0.0, z / norm]


def walkVelocity(stickX, stickY, forwardRoom, deadzone, maxSpeedMPerS):
    """Room-frame horizontal walk velocity (vx, vz) in m/s for a 2D stick input.

    ``forwardRoom`` is the head's horizontal gaze unit vector in room coordinates
    (see horizontalUnit); None yields (0, 0).  Free 2D movement: the stick vector's
    magnitude drives a radial deadzone/speed ramp ((|v| - deadzone) / (1 - deadzone),
    clamped to [0, 1]) so speed scales smoothly with deflection in any direction.
    """
    if forwardRoom is None:
        return (0.0, 0.0)
    if not (math.isfinite(stickX) and math.isfinite(stickY)):
        return (0.0, 0.0)
    magnitude = math.hypot(stickX, stickY)
    if magnitude <= deadzone or deadzone >= 1.0:
        return (0.0, 0.0)
    speed = maxSpeedMPerS * min(1.0, (magnitude - deadzone) / (1.0 - deadzone))
    unitX, unitY = stickX / magnitude, stickY / magnitude
    fx, fz = forwardRoom[0], forwardRoom[2]
    rx, rz = -fz, fx  # right = forward x up(+Y)
    return (speed * (unitY * fx + unitX * rx),
            speed * (unitY * fz + unitX * rz))


def clampWalkDelta(headX, headZ, deltaX, deltaZ, roomSize, marginM):
    """Per-axis clamp of a proposed head displacement so the head stays inside the room.

    Limits are [-roomSize[axis]/2 + marginM, +roomSize[axis]/2 - marginM] per horizontal
    axis, EXPANDED to include the current head position (lo = min(lo, head),
    hi = max(hi, head)): a head already outside the margin (a physical playspace larger
    than the room) is never teleported inward, can always move back in, and can never
    stick-move further out.  Clamping per axis (not radially) lets the user slide along
    a wall.  Returns the allowed (dx, dz).
    """
    allowed = []
    for head, delta, size in ((headX, deltaX, roomSize[0]), (headZ, deltaZ, roomSize[2])):
        lo = min(-size / 2.0 + marginM, head)
        hi = max(size / 2.0 - marginM, head)
        allowed.append(max(lo, min(hi, head + delta)) - head)
    return tuple(allowed)


def locomotionMatrix(offsetX, offsetZ):
    """Materialize the physical->room locomotion transform as a vtkMatrix4x4: a pure
    horizontal translation by (offsetX, 0, offsetZ)."""
    matrix = vtk.vtkMatrix4x4()
    matrix.SetElement(0, 3, offsetX)
    matrix.SetElement(2, 3, offsetZ)
    return matrix
