<?php

class FastHessian {
  private static $filter_map = array(
    array(0, 1, 2, 3),
    array(1, 3, 4, 5),
    array(3, 5, 6, 7),
    array(5, 7, 8, 9),
    array(7, 9, 10, 11)
  );

  private $mIntegralImage;
  private /*List<SURFInterestPoint>*/
    $mInterestPoints;
  private $mOctaves;
  private $mInitSample;
  private $mThreshold;
  private $mHeight;
  private $mWidth;

  private $mRecalculateInterestPoints = TRUE;

  /*List<ResponseLayer>*/
  private $mLayers;

  function __construct( /*IntegralImage*/
    $integralImage, $octaves, $initSample, $threshold, $balanceValue) {
    $this->mIntegralImage = $integralImage;
    $this->mOctaves = $octaves;
    $this->mInitSample = $initSample;
    $this->mThreshold = $threshold;

    $this->mWidth = $integralImage->getWidth();
    $this->mHeight = $integralImage->getHeight();
  }

  // List<SURFInterestPoint>
  public function getIPoints() {
    if ($this->mInterestPoints == NULL || $this->mRecalculateInterestPoints) {
      $this->mInterestPoints = array(); /*new LinkedList<SURFInterestPoint>()*/
      $this->buildResponseMap();

      for ($o = 0; $o < $this->mOctaves; $o++) {
        for ($i = 0; $i <= 1; $i++) {
          $b = $this->mLayers[FastHessian::$filter_map[$o][$i]];
          $m = $this->mLayers[FastHessian::$filter_map[$o][$i + 1]];
          $t = $this->mLayers[FastHessian::$filter_map[$o][$i + 2]];

          // loop over middle response layer at density of the most
          // sparse layer (always top), to find maxima across scale and space
          for ($r = 0; $r < $t->getHeight(); $r++) {
            for ($c = 0; $c < $t->getWidth(); $c++) {
              if ($this->isExtremum($r, $c, $t, $m, $b)) {
                /*SURFInterestPoint*/
                $point = $this->interpolateExtremum($r, $c, $t, $m, $b);
                if ($point != NULL) {
                  $this->mInterestPoints[] = $point;
                }
              }
            }
          }
        }
      }
    }
    return $this->mInterestPoints;
  }

  private function buildResponseMap() {
    $this->mLayers = array();

    $w = (int) ($this->mWidth / $this->mInitSample);
    $h = (int) ($this->mHeight / $this->mInitSample);
    $s = (int) $this->mInitSample;
    if ($this->mOctaves >= 1) {
      $this->mLayers[] = new ResponseLayer($w, $h, $s, 9, $this->mIntegralImage);
      $this->mLayers[] = new ResponseLayer($w, $h, $s, 15, $this->mIntegralImage);
      $this->mLayers[] = new ResponseLayer($w, $h, $s, 21, $this->mIntegralImage);
      $this->mLayers[] = new ResponseLayer($w, $h, $s, 27, $this->mIntegralImage);
    }

    if ($this->mOctaves >= 2) {
      $this->mLayers[] = new ResponseLayer((int) ($w / 2), (int) ($h / 2), $s * 2, 39, $this->mIntegralImage);
      $this->mLayers[] = new ResponseLayer((int) ($w / 2), (int) ($h / 2), $s * 2, 51, $this->mIntegralImage);
    }

    if ($this->mOctaves >= 3) {
      $this->mLayers[] = new ResponseLayer((int) ($w / 4), (int) ($h / 4), $s * 4, 75, $this->mIntegralImage);
      $this->mLayers[] = new ResponseLayer((int) ($w / 4), (int) ($h / 4), $s * 4, 99, $this->mIntegralImage);

    }

    if ($this->mOctaves >= 4) {
      $this->mLayers[] = new ResponseLayer((int) ($w / 8), (int) ($h / 8), $s * 8, 147, $this->mIntegralImage);
      $this->mLayers[] = new ResponseLayer((int) ($w / 8), (int) ($h / 8), $s * 8, 195, $this->mIntegralImage);
    }

    if ($this->mOctaves >= 5) {
      $this->mLayers[] = new ResponseLayer((int) ($w / 16), (int) ($h / 16), $s * 16, 291, $this->mIntegralImage);
      $this->mLayers[] = new ResponseLayer((int) ($w / 16), (int) ($h / 16), $s * 16, 387, $this->mIntegralImage);
    }
  }

  private function isExtremum($r, $c, $t, $m, $b) {
    $layerBorder = ($t->getFilter() + 1) / (2 * $t->getStep());

    if ($r <= $layerBorder || $r >= $t->getHeight() - $layerBorder || $c <= $layerBorder || $c >= $t->getWidth() - $layerBorder) {
      return FALSE;
    }

    $candidate = $m->getResponse($r, $c, $t);

    if (abs($candidate) < $this->mThreshold) {
      return FALSE;
    }

    //See if the response in 3x3x3 is greater, then it isn't a local maxima
    for ($rr = -1; $rr <= 1; $rr++) {
      for ($cc = -1; $cc <= 1; $cc++) {
        $tResponse = abs($t->getResponse($r + $rr, $c + $cc));
        $mResponse = abs($m->getResponse($r + $rr, $c + $cc, $t));
        $bResponse = abs($b->getResponse($r + $rr, $c + $cc, $t));
        if ($candidate > 0 && ($tResponse >= $candidate || $bResponse >= $candidate || ($mResponse >= $candidate && ($rr != 0 || $cc != 0)))) {
          return FALSE;
        }
        elseif ($candidate < 0 && ($tResponse <= $candidate || $bResponse <= $candidate || ($mResponse <= $candidate && ($rr != 0 || $cc != 0)))) {
          return FALSE;
        }
      }
    }
    return TRUE;
  }

  // SURFInterestPoint
  private function interpolateExtremum($r, $c, $t, $m, $b) {
    //should check to make sure that m's filter value is less than t's and greater than b's
    $filterStep = $m->getFilter() - $b->getFilter();

    $values = $this->interpolateStep($r, $c, $t, $m, $b);
    $xi = $values[0];
    $xr = $values[1];
    $xc = $values[2];

    if (abs($xi) < 0.5 && abs($xr) < 0.5 && abs($xc) < 0.5) {
      //WE"VE GOT AN INTEREST POINT HERE
      $x = (double) ($c + $xc) * $t->getStep();
      $y = (double) ($r + $xr) * $t->getStep();
      $scale = (double) (0.1333 * ($m->getFilter() + $xi * $filterStep));
      $laplacian = (int) $m->getLaplacian($r, $c, $t);
      return new SURFInterestPoint($x, $y, $scale, $laplacian);
    }
    return NULL;
  }

  /**
   * Implements gaussian elimination solver with partial pivoting.
   *
   * @param double[][] $A coefficient matrix
   * @param double[]   $b output vector
   * @return double[]  $x solution vector
   */
  public static function solve($A, $b) {

    /**
     * Smallest deviation allowed in floating point comparisons.
     */
    $EPSILON = 1e-10;

    // number of rows
    $N  = count($b);

    // forward elimination
    for ($p=0; $p<$N; $p++) {

      // find pivot row and swap
      $max = $p;
      for ($i = $p+1; $i < $N; $i++)
        if (abs($A[$i][$p]) > abs($A[$max][$p]))
          $max = $i;
      $temp = $A[$p]; $A[$p] = $A[$max]; $A[$max] = $temp;
      $t    = $b[$p]; $b[$p] = $b[$max]; $b[$max] = $t;

      // check if matrix is singular
      if (abs($A[$p][$p]) <= $EPSILON) die("Matrix is singular or nearly singular");

      // pivot within A and b
      for ($i = $p+1; $i < $N; $i++) {
        $alpha = $A[$i][$p] / $A[$p][$p];
        $b[$i] -= $alpha * $b[$p];
        for ($j = $p; $j < $N; $j++)
          $A[$i][$j] -= $alpha * $A[$p][$j];
      }
    }

    // zero the solution vector
    $x = array_fill(0, $N-1, 0);

    // back substitution
    for ($i = $N - 1; $i >= 0; $i--) {
      $sum = 0.0;
      for ($j = $i + 1; $j < $N; $j++)
        $sum += $A[$i][$j] * $x[$j];
      $x[$i] = ($b[$i] - $sum) / $A[$i][$i];
    }

    return $x;

  }

  private function interpolateStep($r, $c, $t, $m, $b) {
    $values = array_fill(0, 3, 0.0);

    $partialDerivs = $this->getPartialDerivativeMatrix($r,$c,$t,$m,$b);
    $hessian3D = $this->getHessian3DMatrix($r,$c,$t,$m,$b);

    $values = FastHessian::solve($hessian3D, $partialDerivs);

    $values = array_reverse($values);

    foreach ($values as &$value) {
      $value = -$value;
    }

    return $values;
  }

  private function getPartialDerivativeMatrix($r, $c, $t, $m, $b) {
    //deriv[0] = dx, deriv[1] = dy, deriv[2] = ds
    $derivs = array();

    $derivs[0] = ($m->getResponse($r, $c + 1, $t) - $m->getResponse($r, $c - 1, $t)) / 2;
    $derivs[1]= ($m->getResponse($r + 1, $c, $t) - $m->getResponse($r - 1, $c, $t)) / 2;
    $derivs[2] = ($t->getResponse($r, $c) - $b->getResponse($r, $c, $t)) / 2.0;

    return $derivs;
  }

  private function getHessian3DMatrix($r, $c, $t, $m, $b) {
    //Layout:
    //  [dxx][dxy][dxs]
    //  [dxy][dyy][dys]
    //  [dxs][dys][dss]
    $hessian = array_fill(0, 3, array_fill(0, 3, 0));;

    $v = $m->getResponse($r, $c, $t);

    //dxx
    $hessian[0][0] = $m->getResponse($r,$c+1,$t) +
      $m->getResponse($r,$c-1,$t) - 2 * $v;
		
		//dyy
		$hessian[1][1] = $m->getResponse($r+1,$c,$t) +
      $m->getResponse($r-1,$c,$t) - 2 * $v;
		
		//dss
		$hessian[2][2] = $t->getResponse($r,$c) +
      $b->getResponse($r,$c,$t) - 2 * $v;
		
		//dxy
		$hessian[0][1] = $hessian[1][0] = ($m->getResponse($r + 1, $c + 1, $t) -
        $m->getResponse($r + 1, $c - 1, $t) -
        $m->getResponse($r - 1, $c + 1, $t) +
        $m->getResponse($r - 1, $c - 1, $t)) / 4.0;
		
		//dxs
		$hessian[0][2] = $hessian[2][0] = ($t->getResponse($r, $c + 1) -
        $t->getResponse($r, $c - 1) -
        $b->getResponse($r, $c + 1, $t) +
        $b->getResponse($r, $c - 1, $t)) / 4.0;
		
		//dys
		$hessian[1][2] = $hessian[2][1] = ($t->getResponse($r + 1, $c) -
        $t->getResponse($r - 1, $c) -
        $b->getResponse($r + 1, $c, $t) +
        $b->getResponse($r - 1, $c, $t)) / 4.0;

    return $hessian;
  }
}
