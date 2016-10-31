<?php

class ResponseLayer {
  private $mWidth;
  private $mHeight;
  private $mStep;
  private $mFilter;

  private $mLaplacian;
  private $mResponses;

  function __construct($width, $height, $step, $filter, $integralImage) {
    $this->mWidth = $width;
    $this->mHeight = $height;
    $this->mStep = $step;
    $this->mFilter = $filter;

    $this->mLaplacian = array();
    $this->mResponses = array();

    $this->buildResponseLayer($integralImage);
  }

  private function buildResponseLayer($img) {
    $b = (int) (($this->mFilter - 1) / 2);
    $l = (int) ($this->mFilter / 3);
    $w = (int) $this->mFilter;
    $inverse_area = (double) (1 / ($w * $w));
    for ($ar = 0, $index = 0; $ar < $this->mHeight; ++$ar) {
      for ($ac = 0; $ac < $this->mWidth; ++$ac, $index++) {
        $r = (int) ($ar * $this->mStep);
        $c = (int) ($ac * $this->mStep);

        // Compute response components
        $Dxx = $img->getIntegralValue($r - $l + 1, $c - $b, 2 * $l - 1, $w)
          - $img->getIntegralValue($r - $l + 1, $c - (int) ($l / 2), 2 * $l - 1, $l) * 3;

        $Dyy = $img->getIntegralValue($r - $b, $c - $l + 1, $w, 2 * $l - 1)
          - $img->getIntegralValue($r - (int) ($l / 2), $c - $l + 1, $l, 2 * $l - 1) * 3;

        $Dxy = $img->getIntegralValue($r - $l, $c + 1, $l, $l)
          + $img->getIntegralValue($r + 1, $c - $l, $l, $l)
          - $img->getIntegralValue($r - $l, $c - $l, $l, $l)
          - $img->getIntegralValue($r + 1, $c + 1, $l, $l);

        // Normalise the filter responses with respect to their size
        $Dxx *= $inverse_area;
        $Dyy *= $inverse_area;
        $Dxy *= $inverse_area;

        // Get the determinant of hessian response & laplacian sign
        $this->mResponses[$ac][$ar] = (double) ($Dxx * $Dyy - 0.81 * $Dxy * $Dxy);
        $this->mLaplacian[$ac][$ar] = ($Dxx + $Dyy >= 0 ? 1 : 0);
      }
    }
  }

  public function getResponse($row, $col, $src = NULL) {
    if ($src) {
      $scale = $this->getWidth() / $src->getWidth();
      return $this->mResponses[$col * $scale][$row * $scale];
    }
    else {
      return $this->mResponses[$col][$row];
    }
  }

  public function getLaplacian($row, $col, $src = NULL) {
    if ($src) {
      $scale = $this->getWidth() / $src->getWidth();
      return $this->mLaplacian[$col * $scale][$row * $scale];
    }
    else {
      return $this->mLaplacian[$col][$row];
    }
  }

  public function  getWidth() {
    return $this->mWidth;
  }

  public function  getHeight() {
    return $this->mHeight;
  }

  public function  getFilter() {
    return $this->mFilter;
  }

  public function  getStep() {
    return $this->mStep;
  }
}
