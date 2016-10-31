<?php

class IntegralImage {

  private $mIntImage = array();
  private $width = -1;
  private $height = -1;

  function  __construct($image) {
    $this->width = imagesx($image->resource);
    $this->height = imagesy($image->resource);
    $this->mIntImage = array_fill(0, $this->width, array_fill(0, $this->height, 0));

    for ($y = 0; $y < $this->height; $y++) {
      $sum = 0;
      for ($x = 0; $x < $this->width; $x++) {
        $pixel = imagecolorsforindex($image->resource, imagecolorat($image->resource, $x, $y));
        $intensity = IntegralImage::calculateIntensity($pixel);
        $sum += $intensity;
        if ($y == 0) {
          $this->mIntImage[$x][$y] = (double) $sum;
        }
        else {
          $this->mIntImage[$x][$y] = (double) ($sum + $this->mIntImage[$x][$y - 1]);
        }
      }
    }
  }

  public function getIntegralValue($row, $col, $rows, $cols) {
    $r1 = $row - 1;
    $c1 = $col - 1;
    $r2 = $row + $rows - 1;
    $c2 = $col + $cols - 1;

    $A = $this->isInBounds($r1, $c1) ? $this->mIntImage[$r1][$c1] : 0;
    $B = $this->isInBounds($r1, $c2) ? $this->mIntImage[$r1][$c2] : 0;
    $C = $this->isInBounds($r2, $c1) ? $this->mIntImage[$r2][$c1] : 0;
    $D = $this->isInBounds($r2, $c2) ? $this->mIntImage[$r2][$c2] : 0;

    return max(0, $A - $B - $C + $D);
  }

  private function  isInBounds($x, $y) {
    return $x >= 0 && $x < $this->width && $y >= 0 && $y < $this->height;
  }

  static function calculateIntensity($pixel) {
    $weightedIntensity = 0.299 * $pixel['red'] + 0.587 * $pixel['green'] + 0.114 * $pixel['blue'];
    return (double) ($weightedIntensity / 255);
  }

  public function getWidth() {
    return $this->width;
  }

  public function  getHeight() {
    return $this->height;
  }

  function getTableValue($column, $row) {
    return $this->mIntImage[$column][$row];
  }
}