<?php

/**
 * A class to calculate the upright or free oriented interest points of an image lazily (will not calculate until you ask for them)
 * @author astromberg
 *
 */

define('HESSIAN_OCTAVES', 5);
define('HESSIAN_INTERVALS', 4);
define('HESSIAN_INIT_SAMPLE', 2);
define('HESSIAN_THRESHOLD', 0.0004);
define('HESSIAN_BALANCE_VALUE', 0.81);

class Surf {

  private $mOriginalImage;
  private $mHessian;
  private $mFreeOrientedPoints;
  private $mUprightPoints;
  private $mDescriptorFreeInterestPoints;

  //octaves = steps
  private $mNumOctaves = HESSIAN_OCTAVES;
  //private mNumIntervals = HESSIAN_INTERVALS;
  private $mThreshold = HESSIAN_THRESHOLD;
  private $mBalanceValue = HESSIAN_BALANCE_VALUE;

  // IntegralImage
  private $mIntegralImage;

  function __construct($image, $balanceValue = HESSIAN_BALANCE_VALUE, $threshold = HESSIAN_THRESHOLD, $octaves = HESSIAN_OCTAVES) {
    $this->mOriginalImage = $image;
    $this->mNumOctaves = $octaves;
    $this->mBalanceValue = $balanceValue;
    $this->mThreshold = $threshold;

    //Calculate the integral image
    $this->mIntegralImage = new IntegralImage($this->mOriginalImage);

    //Calculate the fast hessian
    $this->mHessian = new FastHessian($this->mIntegralImage, $this->mNumOctaves, HESSIAN_INIT_SAMPLE, $this->mThreshold, $this->mBalanceValue);

    //Calculate the descriptor and orientation free interest points
    $this->mDescriptorFreeInterestPoints = $this->mHessian->getIPoints();

    var_dump($this->mDescriptorFreeInterestPoints);
    die();
  }

  // List<SURFInterestPoint >
  public function getUprightInterestPoints() {
    return $this->getPoints(TRUE);
  }

  // List<SURFInterestPoint >
  public function getFreeOrientedInterestPoints() {
    return $this->getPoints(FALSE);
  }

  // List<SURFInterestPoint >
  private function getPoints($upright) {
    $points = $upright ? $this->mUprightPoints : $this->mFreeOrientedPoints;
    if ($points == NULL) {
      $points = $this->getDescriptorFreeInterestPoints();
      // cache for next time through
      if ($upright) {
        $this->mUprightPoints = $points;
      }
      else {
        $this->mFreeOrientedPoints = $points;
      }
      // SURFInterestPoint
      foreach ($points as $point) {
        $this->getOrientation($point);
        $this->getMDescriptor($point, $upright);
      }
    }
    return $points;
  }

  // List<SURFInterestPoint >
  private function getDescriptorFreeInterestPoints() {
    $points = array(); //mDescriptorFreeInterestPoints . size()
    foreach ($this->mDescriptorFreeInterestPoints as $point) {
      $points[] = clone $point; // (SURFInterestPoint)point . clone()
      /*catch (CloneNotSupportedException e) {
        e . printStackTrace();
      }*/
    }
    return $points;
  }

  //TODO: Why the multiply by 1?
  private function haarX($row, $column, $s) {
    return $this->mIntegralImage->getIntegralValue($row - $s / 2, $column, $s, $s / 2)
    - 1 * $this->mIntegralImage->getIntegralValue($row - $s / 2, $column - $s / 2, $s, $s / 2);
  }

  //TODO: Why the multiply by 1?
  private function haarY($row, $column, $s) {
    return $this->mIntegralImage->getIntegralValue($row, $column - $s / 2, $s / 2, $s)
    - 1 * $this->mIntegralImage->getIntegralValue($row - $s / 2, $column - $s / 2, $s / 2, $s);
  }

  private function getOrientation( /*SURFInterestPoint*/
    $input) {
    /*double*/
    $scale = $input->getScale();

    $s = (int) round($scale);
    $r = (int) round($input->getY());
    $c = (int) round($input->getX());

    /*List<Double > */
    $xHaarResponses = array();
    /*List<Double > */
    $yHaarResponses = array();
    /*List<Double > */
    $angles = array();

    //calculate haar responses for points within radius of 6*scale
    for ($i = -6; $i <= 6; ++$i) {
      for ($j = -6; $j <= 6; ++$j) {
        if ($i * $i + $j * $j < 36) {
          $gauss = GaussianConstants::$Gauss25[abs($i)][abs($j)];
					$xHaarResponse = $gauss * $this->haarX($r + $j * $s, $c + $i * $s, 4 * $s);
					$yHaarResponse = $gauss * $this->haarY($r + $j * $s, $c + $i * $s, 4 * $s);
					$xHaarResponses[] = $xHaarResponse;
					$yHaarResponses[] = $yHaarResponse;
					$angles[] = $this->getAngle($xHaarResponse, $yHaarResponse);
				}
      }
    }

    // calculate the dominant direction
    $max = 0;

    // loop slides pi/3 window around feature point
    for ($ang1 = 0; $ang1 < 2 * M_PI; $ang1 += 0.15) {
      $ang2 = (double) ($ang1 + M_PI / 3.0 > 2 * M_PI ? $ang1 - 5.0 * M_PI / 3.0 : $ang1 + M_PI / 3.0);
      $sumX = $sumY = 0;
      for ($k = 0; $k < count($angles); $k++) {
        $ang = (double) $angles[$k];

        if ($ang1 < $ang2 && $ang1 < $ang && $ang < $ang2) {
          $sumX += (double) $xHaarResponses[$k];
          $sumY += (double) $yHaarResponses[$k];
        }
        else {
          if ($ang2 < $ang1 && (($ang > 0 && $ang < $ang2) || ($ang > $ang1 && $ang < 2 * M_PI))) {
            $sumX += (double) $xHaarResponses[$k];
            $sumY += (double) $yHaarResponses[$k];
          }
        }
      }
      // if the vector produced from this window is longer than all
      // previous vectors then this forms the new dominant direction
      if ($curr = $sumX * $sumX + $sumY * $sumY > $max) {
        // store largest orientation
        $max = $curr;
        $orientation = (double) $this->getAngle($sumX, $sumY);
      }
    }
    $input->setOrientation($orientation);
  }

  private function getMDescriptor( /*SURFInterestPoint*/
    $point, /*boolean*/
    $upright) {
    $count = 0;
    $co = 1;
    $si = 0;
    $desc = array_fill(0, 64, 0);
    $len = 0.0;

    $cx = -0.5;
    $cy = 0.0; //Subregion centers for the 4x4 gaussian weighting

    $scale = $point->getScale();
    $x = round($point->getX());
    $y = round($point->getY());

    if (!$upright) {
      $co = cos($point->getOrientation());
      $si = sin($point->getOrientation());
    }

    $i = -8;

    //Calculate descriptor for this interest point
    //Area of size 24 s x 24 s
    //***********************************************

    while ($i < 12) {
      $j = -8;
      $i = $i - 4;

      $cx += 1.0;
      $cy = -0.5;

      while ($j < 12) {
        $dx = $dy = $mdx = $mdy = 0.0;
        $cy += 1.0;

        $j = $j - 4;

        $ix = $i + 5;
        $jx = $j + 5;

        $xs = round($x + (-$jx * $scale * $si + $ix * $scale * $co));
        $ys = round($y + ($jx * $scale * $co + $ix * $scale * $si));

        for ($k = $i; $k < $i + 9; ++$k) {
          for ($l = $j; $l < $j + 9; ++$l) {
            //Get coords of sample point on the rotated axis
            $sample_x = (int) round($x + (-1 * $l * $scale * $si + $k * $scale * $co));
            $sample_y = (int) round($y + ($l * $scale * $co + $k * $scale * $si));

            //Get the gaussian weighted x and y responses
            $gauss_s1 = $this->gaussian($xs - $sample_x, $ys - $sample_y, 2.5 * $scale);

            $rx = $this->haarX($sample_y, $sample_x, (int) (2 * round($scale)));
            $ry = $this->haarY($sample_y, $sample_x, (int) (2 * round($scale)));

            //Get the gaussian weighted x and y responses on rotated axis
            $rrx = $gauss_s1 * (-$rx * $si + $ry * $co);
            $rry = $gauss_s1 * ($rx * $co + $ry * $si);

            $dx += $rrx;
            $dy += $rry;

            $mdx += abs($rrx);
            $mdy += abs($rry);
          }
        }

        //Add the values to the descriptor vector
        $gauss_s2 = $this->gaussian($cx - 2.0, $cy - 2.0, 1.5);

        //Casting from a double to a double, might be a terrible idea
        //but doubles are expensive
        $desc[$count++] = (double) ($dx * $gauss_s2);
        $desc[$count++] = (double) ($dy * $gauss_s2);

        $desc[$count++] = (double) ($mdx * $gauss_s2);
        $desc[$count++] = (double) ($mdy * $gauss_s2);

        //Accumulate length for vector normalisation
        $len += ($dx * $dx + $dy * $dy + $mdx * $mdx + $mdy * $mdy) * ($gauss_s2 * $gauss_s2);

        $j += 9;
      }
      $i += 9;
    }

    $len = sqrt($len);

    for ($i = 0; $i < 64; $i++) {
      $desc[$i] /= $len;
    }

    $point->setDescriptor($desc);
  }

  private function getAngle( /*double*/
    $xHaarResponse, /*double*/
    $yHaarResponse) {
    if ($xHaarResponse >= 0 && $yHaarResponse >= 0) {
      return atan($yHaarResponse / $xHaarResponse);
    }

    if ($xHaarResponse < 0 && $yHaarResponse >= 0) {
      return M_PI - atan(-$yHaarResponse / $xHaarResponse);
    }

    if ($xHaarResponse < 0 && $yHaarResponse < 0) {
      return M_PI + atan($yHaarResponse / $xHaarResponse);
    }

    if ($xHaarResponse >= 0 && $yHaarResponse < 0) {
      return 2 * M_PI - atan(-$yHaarResponse / $xHaarResponse);
    }

    return 0;
  }

  // Map<SURFInterestPoint,SURFInterestPoint>
  public function getMatchingPoints( /*Surf*/
    $descriptor, /*boolean*/
    $upright) {

    /*Map<SURFInterestPoint,SURFInterestPoint>*/
    $matchingPoints = array();
    /*List<SURFInterestPoint>*/
    $points = $upright ? $descriptor->getUprightInterestPoints() : $descriptor->getFreeOrientedInterestPoints();

    foreach ($points as $a){
      $smallestDistance = INF;
      $nextSmallestDistance = INF;
      /*SURFInterestPoint*/
      $possibleMatch = NULL;

      $candidate_points = $upright ? $descriptor->getUprightInterestPoints() : $descriptor->getFreeOrientedInterestPoints();
      foreach ($candidate_points as $b ){
        $distance = $a->getDistance($b);
        if ($distance < $smallestDistance) {
          $nextSmallestDistance = $smallestDistance;
          $smallestDistance = $distance;
          $possibleMatch = $b;
        }
        else {
          if ($distance < $nextSmallestDistance) {
            $nextSmallestDistance = $distance;
          }
        }
      }

		  // If match has a d1:d2 ratio < 0.65 ipoints are a match
			//if ( smallestDistance/nextSmallestDistance < 0.65d ){
			if ($smallestDistance / $nextSmallestDistance < 0.75) {
        //not storing change in position
        $matchingPoints[$a] = $possibleMatch;
      }
		}

		return $matchingPoints;
  }

  private function gaussian( /*double*/
    $x, /*double*/
    $y, /*double*/
    $sig) {
    return (1.0 / (2.0 * M_PI * $sig * $sig)) * exp(-($x * $x + $y * $y) / (2.0 * $sig * $sig));
  }

  /*public function saveToFile(Surf surf,String file){
  }

  public function readFromFile(String location){
  }

  private void writeObject(ObjectOutputStream out) throws IOException {
  }*/

}