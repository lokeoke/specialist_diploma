<?php

class SURFInterestPoint {
  private $mX;
  private $mY;

  private $mScale;

  private $mOrientation;

  private $mLaplacian;

  private $mDescriptor;

  private $mDx;
  private $mDy;

  private $mClusterIndex;

  function __construct($x, $y, $scale, $laplacian) {
    $this->mX = $x;
    $this->mY = $y;
    $this->mScale = $scale;
    $this->mLaplacian = $laplacian;
  }

  public function setClusterIndex($mClusterIndex) {
    $this->mClusterIndex = $mClusterIndex;
  }

  public function getClusterIndex() {
    return $this->mClusterIndex;
  }

  public function setDescriptor($mDescriptor) {
    $this->mDescriptor = $mDescriptor;
  }

  public function getDescriptor() {
    return $this->mDescriptor;
  }

  public function setDx($mDx) {
    $this->mDx = $mDx;
  }

  public function getDx() {
    return $this->mDx;
  }

  public function setDy($mDy) {
    $this->mDy = $mDy;
  }

  public function getDy() {
    return $this->mDy;
  }

  public function setLaplacian($mLaplacian) {
    $this->mLaplacian = $mLaplacian;
  }

  public function getLaplacian() {
    return $this->mLaplacian;
  }

  public function setOrientation($mOrientation) {
    $this->mOrientation = $mOrientation;
  }

  public function getOrientation() {
    return $this->mOrientation;
  }

  public function setScale($mScale) {
    $this->mScale = $mScale;
  }

  public function getScale() {
    return $this->mScale;
  }

  public function setX($mX) {
    $this->mX = $mX;
  }

  public function getX() {
    return $this->mX;
  }

  public function setY($mY) {
    $this->mY = $mY;
  }

  public function getY() {
    return $this->mY;
  }

  public function getDistance($point) {
    $sum = 0;
    if ($point->getLocation() == NULL || $this->mDescriptor == NULL) {
      return INF;
    }
    for ($i = 0; $i < count($this->mDescriptor); $i++) {
      $point_location = $point->getLocation();
      $diff = $this->mDescriptor[$i] - $point_location[$i];
      $sum += $diff * $diff;
    }
    return (double) sqrt($sum);
  }

  public function  getCoord($dimension) {
    return $this->mDescriptor[$dimension];
  }

  public function  getDimensions() {
    return count($this->mDescriptor);
  }

  public function isEquivalentTo($point) {
    $isEquivalent = TRUE;

    $isEquivalent &= $this->mX == $point->getX();
    $isEquivalent &= $this->mY == $point->getY();

    $isEquivalent &= $this->mDx == $point->getDx();
    $isEquivalent &= $this->mDy == $point->getDy();

    $isEquivalent &= $this->mOrientation == $point->getOrientation();

    $isEquivalent &= $this->mScale == $point->getScale();

    $isEquivalent &= $this->mLaplacian == $point->getLaplacian();

    $isEquivalent &= $this->mDescriptor == $point->getDescriptor();

    return $isEquivalent;
  }
}
