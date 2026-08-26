"""Persisting VRStage options as user defaults - the "Save as default" feature.

Mirrors the Markups module's "Save as default": the current parameter node values are written
to the application settings (QSettings), and re-applied to every freshly created VRStage
parameter node - so a user's preferred colors, visibility flags, behavior options and control
bindings persist between Slicer sessions.

The functions here are generic over the parameterNodeWrapper/parameterPack introspection API
(``allParameters`` + dotted ``getValue``/``setValue``), so new parameters added to
VRStageParameterNode are saved/applied automatically without touching this file.

Settings layout: one key per leaf parameter under the ``VRStage/Defaults`` group, using the
wrapper's dotted names (e.g. ``VRStage/Defaults/display.accentColor``). Every value is stored
as a string; the expected type is inferred from the parameter's current (factory-default)
value when reading back. A stored value that no longer parses or validates (e.g. a renamed
control-binding label from an older version) is skipped with a warning instead of failing the
whole apply.

All functions take an optional ``settings`` (QSettings) argument so tests can use a throwaway
INI file instead of the user's real application settings.
"""

import logging

import qt

import slicer
from slicer.parameterNodeWrapper import isParameterPack, nestedParameterNames

SETTINGS_PREFIX = "VRStage/Defaults"

# Node attribute marking that saved user defaults were already applied to this parameter node -
# set on first wrap so a parameter node restored from a saved scene (which carries the
# attribute, and the user's scene-specific values) is never overwritten by the defaults.
USER_DEFAULTS_APPLIED_ATTRIBUTE = "VRStage.UserDefaultsApplied"


def _userSettings(settings):
    return settings if settings is not None else slicer.app.userSettings()


def flattenedParameterNames(parameters) -> list:
    """Dotted names of every leaf parameter of a parameterNodeWrapper instance,
    recursing through parameterPack members (e.g. ``display.accentColor``)."""
    names = []
    for name in parameters.allParameters:
        value = parameters.getValue(name)
        if isParameterPack(value):
            names += [f"{name}.{subName}" for subName in nestedParameterNames(value)]
        else:
            names.append(name)
    return names


def serializeValue(value) -> str:
    """One leaf parameter value -> settings string."""
    if isinstance(value, qt.QColor):
        return value.name(1)  # 1 = QColor.HexArgb ("#aarrggbb"), same form QColorSerializer uses
    if isinstance(value, bool):
        return "true" if value else "false"
    if isinstance(value, (int, float, str)):
        return str(value)
    raise ValueError(f"Unsupported user-default value type: {type(value).__name__}")


def deserializeValue(text: str, templateValue):
    """Settings string -> value of the same type as templateValue (the parameter's current
    value). Raises ValueError if the text does not parse to that type."""
    if isinstance(templateValue, qt.QColor):
        color = qt.QColor(text)
        if not color.isValid():
            raise ValueError(f"Invalid color: {text!r}")
        return color
    if isinstance(templateValue, bool):  # before int: bool is an int subclass
        lowered = text.strip().lower()
        if lowered in ("true", "1"):
            return True
        if lowered in ("false", "0"):
            return False
        raise ValueError(f"Invalid bool: {text!r}")
    if isinstance(templateValue, int):
        return int(text)
    if isinstance(templateValue, float):
        return float(text)
    if isinstance(templateValue, str):
        return text
    raise ValueError(f"Unsupported user-default value type: {type(templateValue).__name__}")


def saveUserDefaults(parameters, settings=None) -> None:
    """Write every parameter of the wrapper to settings as the new user defaults,
    replacing any previously saved set (so stale keys from older versions are dropped)."""
    settings = _userSettings(settings)
    settings.remove(SETTINGS_PREFIX)
    for name in flattenedParameterNames(parameters):
        settings.setValue(f"{SETTINGS_PREFIX}/{name}", serializeValue(parameters.getValue(name)))
    settings.sync()


def hasUserDefaults(settings=None) -> bool:
    settings = _userSettings(settings)
    settings.beginGroup(SETTINGS_PREFIX)
    hasKeys = len(settings.allKeys()) > 0
    settings.endGroup()
    return hasKeys


def clearUserDefaults(settings=None) -> None:
    settings = _userSettings(settings)
    settings.remove(SETTINGS_PREFIX)
    settings.sync()


def applyUserDefaults(parameters, settings=None) -> None:
    """Apply every saved user default present in settings onto the wrapper's parameters.
    Unsaved parameters keep their factory defaults; a stored value that no longer parses or
    validates is skipped with a warning."""
    settings = _userSettings(settings)
    wasModified = parameters.StartModify()
    try:
        for name in flattenedParameterNames(parameters):
            key = f"{SETTINGS_PREFIX}/{name}"
            if not settings.contains(key):
                continue
            text = str(settings.value(key))
            try:
                parameters.setValue(name, deserializeValue(text, parameters.getValue(name)))
            except (ValueError, TypeError) as e:
                logging.warning(f"VRStage: ignoring saved default {name}={text!r}: {e}")
    finally:
        parameters.EndModify(wasModified)


def applyUserDefaultsOnce(parameters, settings=None) -> bool:
    """Apply saved user defaults to a parameter node exactly once in its lifetime.

    Call whenever a parameter node is (re)wrapped: a brand-new node (fresh session, or fresh
    node after a scene clear) gets the saved defaults; a node loaded from a saved scene
    carries the marker attribute and keeps the values stored in that scene. Returns whether
    defaults were applied."""
    node = parameters.parameterNode
    if node.GetAttribute(USER_DEFAULTS_APPLIED_ATTRIBUTE):
        return False
    wasModified = parameters.StartModify()
    try:
        node.SetAttribute(USER_DEFAULTS_APPLIED_ATTRIBUTE, "true")
        applyUserDefaults(parameters, settings)
    finally:
        parameters.EndModify(wasModified)
    return True


def resetToFactoryDefaults(parameters) -> None:
    """Reset every parameter of the wrapper back to its declared (factory) default value.
    Does not touch saved user defaults - combine with clearUserDefaults() for a full reset."""
    wasModified = parameters.StartModify()
    try:
        for name in parameters.allParameters:
            parameters.setValue(name, parameters.default(name).value)
    finally:
        parameters.EndModify(wasModified)
