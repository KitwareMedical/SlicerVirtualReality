"""Baked-text VTK props: opaque textured quads that replace the expensive translucent
vtkTextActor3D/vtkBillboardTextActor3D.  See BAKED_TEXT_DPI in Constants for the perf rationale."""

import logging

import vtk

from .Constants import BAKED_TEXT_DPI


class BakedTextMixin:
    """Shared implementation for the two baked-text props below (see BAKED_TEXT_DPI for why
    text is baked at all). The text is rasterized by vtkTextRenderer - the same engine behind
    vtkTextActor3D, so metrics match - over an OPAQUE background (bgColor), its alpha channel is
    dropped (vtkImageExtractComponents -> 3-component RGB, which vtkTexture::IsTranslucent
    treats as opaque without even scanning pixels), and the result is mapped onto a quad whose
    corners are the rasterized image's extent in PIXELS. vtkTextRenderer already offsets that
    extent by the text property's justification (centered -> x in [-w/2, w/2], bottom -> y in
    [0, h]), so the prop's origin sits exactly where vtkTextActor3D's did; callers scale the
    prop to turn pixels into meters/mm. Only SetInput/Rebake rasterize - never rendering."""

    def initText(self, fontSizePx, color, bgColor=None) -> None:
        """bgColor: the panel colour to composite over (opaque RGB texture). None: a "cut-out"
        label instead - no background at all, the RGBA texture is kept but the prop is forced
        into the OPAQUE pass and a fragment-shader replacement discards pixels under 50% alpha.
        Still no translucent geometry (so no depth-peeling cost), at the price of hard-edged
        glyphs - fine for big bold letters floating in space (the orientation labels)."""
        self._text = None
        self._cutout = bgColor is None
        self._tprop = vtk.vtkTextProperty()
        self._tprop.SetFontSize(fontSizePx)
        self._tprop.SetColor(*color)
        if not self._cutout:
            self._tprop.SetBackgroundColor(*bgColor)
            self._tprop.SetBackgroundOpacity(1.0)
        self._image = vtk.vtkImageData()
        # vtkTextRenderer allocates a power-of-two image and draws the text into just its
        # bounding box (the rest stays black/transparent) - vtkTextActor3D hides that padding
        # with a display extent; here the image is clipped to the bbox instead (see Rebake).
        self._clip = vtk.vtkImageClip()
        self._clip.SetInputData(self._image)
        self._clip.ClipDataOn()
        texture = vtk.vtkTexture()
        if self._cutout:
            # A transparent margin around the glyphs (the rasterized image has none below/left
            # of the bbox) so the alpha test never lands on the quad's clamped edge texels -
            # that showed as a ragged line along the bottom where the shadow ends.
            self._pad = vtk.vtkImageConstantPad()
            self._pad.SetInputConnection(self._clip.GetOutputPort())
            self._pad.SetConstant(0.0)
            texture.SetInputConnection(self._pad.GetOutputPort())
        else:
            self._rgb = vtk.vtkImageExtractComponents()
            self._rgb.SetInputConnection(self._clip.GetOutputPort())
            self._rgb.SetComponents(0, 1, 2)
            texture.SetInputConnection(self._rgb.GetOutputPort())
        texture.InterpolateOn()
        # Mipmaps smear alpha across levels, which the cut-out alpha test then turns into
        # crawling edges; the cut-out glyphs are baked large instead (ORIENTATION_LABEL_FONT_SIZE).
        texture.SetMipmap(not self._cutout)
        texture.EdgeClampOn()
        self._plane = vtk.vtkPlaneSource()
        mapper = vtk.vtkPolyDataMapper()
        mapper.SetInputConnection(self._plane.GetOutputPort())
        # Text sits only BAKED_TEXT_PROUD_M off its panel; bias it toward the viewer so it wins
        # the depth test against the panel even where precision gets marginal.
        mapper.SetRelativeCoincidentTopologyPolygonOffsetParameters(-2.0, -2.0)
        if self._cutout:
            # Runs after the texture has been multiplied into gl_FragData[0] (TCoord::Impl),
            # so its alpha is the glyph coverage: drop everything that isn't glyph/shadow.
            self.GetShaderProperty().AddFragmentShaderReplacement(
                "//VTK::Picking::Impl", True,
                "  if (gl_FragData[0].a < 0.5) { discard; }\n//VTK::Picking::Impl\n", False)
            self.ForceOpaqueOn()
        self.SetMapper(mapper)
        self.SetTexture(texture)
        self.GetProperty().SetColor(1.0, 1.0, 1.0)  # texture carries the colours
        self.PickableOff()

    def GetTextProperty(self):
        """Edits to the returned property only take effect after Rebake()."""
        return self._tprop

    def GetInput(self):
        return self._text

    def SetInput(self, text) -> None:
        if text == self._text:
            return
        self._text = text
        self.Rebake()

    def MeasureWidthPx(self, text) -> int:
        """Rendered width of text in pixels (same units as the quad), without rasterizing."""
        bbox = [0, 0, 0, 0]
        vtk.vtkTextRenderer.GetInstance().GetBoundingBox(self._tprop, text, bbox, BAKED_TEXT_DPI)
        return bbox[1] - bbox[0] + 1

    def Rebake(self) -> None:
        text = self._text if self._text else " "
        if not vtk.vtkTextRenderer.GetInstance().RenderString(
                self._tprop, text, self._image, [0, 0], BAKED_TEXT_DPI):
            logging.warning("VRStage: failed to rasterize text %r", text)
            return
        bbox = [0, 0, 0, 0]
        vtk.vtkTextRenderer.GetInstance().GetBoundingBox(self._tprop, text, bbox, BAKED_TEXT_DPI)
        x0, x1, y0, y1 = bbox
        self._clip.SetOutputWholeExtent(x0, x1, y0, y1, 0, 0)
        if self._cutout:
            pad = 4
            x0, x1, y0, y1 = x0 - pad, x1 + pad, y0 - pad, y1 + pad
            self._pad.SetOutputWholeExtent(x0, x1, y0, y1, 0, 0)
        self._plane.SetOrigin(x0, y0, 0.0)
        self._plane.SetPoint1(x1 + 1, y0, 0.0)
        self._plane.SetPoint2(x0, y1 + 1, 0.0)
        self._image.Modified()


class BakedTextActor(BakedTextMixin, vtk.vtkActor):
    """Baked text on a fixed-orientation quad - signage, info screen, tile labels."""


class BakedFollowerTextActor(BakedTextMixin, vtk.vtkFollower):
    """Baked text on a camera-facing quad - the orientation badges."""
