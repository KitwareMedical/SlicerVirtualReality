import slicer
from slicer.ScriptedLoadableModule import *

#
# VirtualRealityStub
#

class VirtualRealityStub(ScriptedLoadableModule):
  def __init__(self, parent):
    ScriptedLoadableModule.__init__(self, parent)
    self.parent.title = "Virtual Reality"
    self.parent.categories = ["Virtual Reality"]
    self.parent.dependencies = []
    self.parent.contributors = ["Andras Lasso (PerkLab)"]
    self.parent.helpText = """
This is a placeholder module to tell the user that Virtual Reality extension is not available on the platform.
See more information in the <a href="https://github.com/KitwareMedical/SlicerVirtualReality">extension documentation</a>.
"""
    self.parent.acknowledgementText = ""

#
# VirtualRealityStubWidget
#

class VirtualRealityStubWidget(ScriptedLoadableModuleWidget):
  def __init__(self, parent=None):
    ScriptedLoadableModuleWidget.__init__(self, parent)

  def setup(self):
    ScriptedLoadableModuleWidget.setup(self)

  def enter(self):
    """
    Called each time the user opens this module.
    """
    slicer.util.messageBox("Virtual Reality is not supported on this platform.<br>"
      "See <a href='https://github.com/KitwareMedical/SlicerVirtualReality#setup'>Slicer Virtual Reality extension website</a> for details.")
