"""Directory-of-MRB-files enumeration and thumbnail extraction for VR Stage.

Backs the left wall's "MRB directory" library source: the user points the module at a folder
and each ``*.mrb`` scene bundle in it becomes a launcher tile, using the scene screenshot
embedded in the bundle as the tile thumbnail.

Screenshot location inside an MRB (a zip archive) varies by the Slicer version that saved it:
- Modern Slicer writes the save-dialog scene screenshot as ``<root>/<name>.png`` next to
  ``<root>/<name>.mrml``.
- Older bundles (e.g. the 2012 atlases) have no root PNG, but carry scene-view screenshots as
  ``<root>/Data/*.png``.
``mrbScreenshotBytes`` prefers the root PNG matching the ``.mrml`` stem, then any root-level
PNG, then the first (sorted) ``Data`` PNG.

Pure Python (no VTK/Qt/Slicer imports) so it is headless-testable.
"""

import logging
import zipfile
from pathlib import Path, PurePosixPath


def isLibraryDirectorySet(directory) -> bool:
    """Whether *directory* names an actual choice ("" and "." are the unset Path defaults)."""
    return str(directory) not in ("", ".")


def listMrbFiles(directory) -> list:
    """Top-level ``*.mrb`` files of *directory* (no recursion), case-insensitively sorted by
    name. An unset, missing, or unreadable directory yields an empty list."""
    if not isLibraryDirectorySet(directory):
        return []
    libraryDir = Path(directory)
    if not libraryDir.is_dir():  # common, not an error: e.g. a saved default naming a removed folder
        return []
    try:
        files = [p for p in libraryDir.iterdir()
                 if p.is_file() and p.suffix.lower() == ".mrb"]
    except OSError:
        logging.exception("VR Stage: cannot list MRB library directory %s", directory)
        return []
    return sorted(files, key=lambda p: p.name.lower())


def mrbDisplayName(mrbPath) -> str:
    return Path(mrbPath).stem


def _screenshotEntryName(names) -> str:
    """The archive entry to use as the thumbnail, or None. Pure selection logic over the
    archive's entry name list. "Root" here means directly inside the bundle's single root
    folder (or at the archive top level, for a bundle without one)."""
    pngs = sorted(n for n in names if n.lower().endswith(".png"))
    if not pngs:
        return None
    mrmlStems = {PurePosixPath(n).stem for n in names if n.lower().endswith(".mrml")}
    rootPngs = [n for n in pngs if len(PurePosixPath(n).parts) <= 2
                and "Data" not in PurePosixPath(n).parts[:-1]]
    for name in rootPngs:
        if PurePosixPath(name).stem in mrmlStems:
            return name
    if rootPngs:
        return rootPngs[0]
    dataPngs = [n for n in pngs if "Data" in PurePosixPath(n).parts[:-1]]
    if dataPngs:
        return dataPngs[0]
    return None


def mrbScreenshotBytes(mrbPath):
    """PNG bytes of the bundle's scene screenshot, or None (no PNG, or unreadable archive)."""
    try:
        with zipfile.ZipFile(mrbPath) as archive:
            entryName = _screenshotEntryName(archive.namelist())
            if entryName is None:
                return None
            return archive.read(entryName)
    except (OSError, zipfile.BadZipFile, KeyError):
        logging.exception("VR Stage: cannot read scene screenshot from %s", mrbPath)
        return None
