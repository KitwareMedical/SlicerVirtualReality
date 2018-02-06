SlicerOpenVR
============

A Slicer extension that enables user to interact with a Slicer scene using virtual reality.

![](VR.png)

Building steps
--------------

**This Slicer extension is in active development. The API may change from version to version without notice.**

It currently requires patches not yet integrated in upstream Slicer. See https://github.com/Slicer/Slicer/compare/master...jbvimort:VR

Waiting it is available in the Slicer Extension Manager, follow these instructions:

- Build the extension against the newly built Slicer with Qt5 and VTK9 enabled.

- To start Slicer from a build tree and ensure the extension is properly loaded, considering using the ``--launcher-additional-settings`` option:

   ```
   ./Slicer.exe  --launcher-additional-settings C:\path\to\SlicerOpenVR-build\inner-build\AdditionalLauncherSettings.ini --additional-module-paths C:\path\to\SlicerOpenVR-build\inner-build\
   ```

Note that specifying the top-level build directory of the extension ensures that Slicer find all types of modules.

License
-------

It is covered by the Apache License, Version 2.0:

http://www.apache.org/licenses/LICENSE-2.0
