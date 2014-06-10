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

  function __construct($x, $y, $scale, $laplacian){
    $this->mX = $x;
    $this->mY = $y;
    $this->mScale = $scale;
    $this->mLaplacian = $laplacian;
  }

  /**
   * @param mixed $mClusterIndex
   */
  public function setClusterIndex($mClusterIndex) {
    $this->mClusterIndex = $mClusterIndex;
  }

  /**
   * @return mixed
   */
  public function getClusterIndex() {
    return $this->mClusterIndex;
  }

  /**
   * @param mixed $mDescriptor
   */
  public function setDescriptor($mDescriptor) {
    $this->mDescriptor = $mDescriptor;
  }

  /**
   * @return mixed
   */
  public function getDescriptor() {
    return $this->mDescriptor;
  }

  /**
   * @param mixed $mDx
   */
  public function setDx($mDx) {
    $this->mDx = $mDx;
  }

  /**
   * @return mixed
   */
  public function getDx() {
    return $this->mDx;
  }

  /**
   * @param mixed $mDy
   */
  public function setDy($mDy) {
    $this->mDy = $mDy;
  }

  /**
   * @return mixed
   */
  public function getDy() {
    return $this->mDy;
  }

  /**
   * @param mixed $mLaplacian
   */
  public function setLaplacian($mLaplacian) {
    $this->mLaplacian = $mLaplacian;
  }

  /**
   * @return mixed
   */
  public function getLaplacian() {
    return $this->mLaplacian;
  }

  /**
   * @param mixed $mOrientation
   */
  public function setOrientation($mOrientation) {
    $this->mOrientation = $mOrientation;
  }

  /**
   * @return mixed
   */
  public function getOrientation() {
    return $this->mOrientation;
  }

  /**
   * @param mixed $mScale
   */
  public function setScale($mScale) {
    $this->mScale = $mScale;
  }

  /**
   * @return mixed
   */
  public function getScale() {
    return $this->mScale;
  }

  /**
   * @param mixed $mX
   */
  public function setX($mX) {
    $this->mX = $mX;
  }

  /**
   * @return mixed
   */
  public function getX() {
    return $this->mX;
  }

  /**
   * @param mixed $mY
   */
  public function setY($mY) {
    $this->mY = $mY;
  }

  /**
   * @return mixed
   */
  public function getY() {
    return $this->mY;
  }

  public function getDistance(/*InterestPoint*/ $point){
    $sum = 0;
		if ( $point->getLocation() == null || $this->mDescriptor == null ) return INF;
		for ( $i = 0; $i < count($this->mDescriptor); $i++ ){
      $point_location = $point->getLocation();
      $diff = $this->mDescriptor[$i] - $point_location[$i];
			$sum += $diff*$diff;
		}
		return (double)sqrt($sum);
  }

  public function  getCoord($dimension) {
    return $this->mDescriptor[$dimension];
  }

  public function  getDimensions() {
		return count($this->mDescriptor);
	}

  public function isEquivalentTo(/*SURFInterestPoint*/ $point){
    $isEquivalent = true;

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
