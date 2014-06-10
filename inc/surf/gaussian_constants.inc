<?php

class GaussianConstants {
  public static  $Gauss25 = array(
    array(0.02546481,   0.02350698,     0.01849125,     0.01239505,     0.00708017,     0.00344629,     0.00142946),
    array(0.02350698,   0.02169968,     0.01706957,     0.01144208,     0.00653582,     0.00318132,     0.00131956),
    array(0.01849125,   0.01706957,     0.01342740,     0.00900066,     0.00514126,     0.00250252,     0.00103800),
    array(0.01239505,   0.01144208,     0.00900066,     0.00603332,     0.00344629,     0.00167749,     0.00069579),
    array(0.00708017,   0.00653582,     0.00514126,     0.00344629,     0.00196855,     0.00095820,     0.00039744),
    array(0.00344629,   0.00318132,     0.00250252,     0.00167749,     0.00095820,     0.00046640,     0.00019346),
    array(0.00142946,   0.00131956,     0.00103800,     0.00069579,     0.00039744,     0.00019346,     0.00008024),
  );

  public static function getGaussianDistribution($sampleCount, $range, $sigma){
    $distribution = array_fill(0,$sampleCount, array_fill(0, $sampleCount, 0));
    $sigmaSquared = pow($sigma,2);
    $inverseTwoPiSigmaSquared = 1 / (2 * M_PI * $sigmaSquared);
    for ( $i = 0; $i < $sampleCount; $i++ ){
      for ( $j = 0; $j < $sampleCount; $j++ ){
        $x = ($range / ($sampleCount-1)) * $i;
        $y = ($range / ($sampleCount-1)) * $j;
        $power = pow($x,2)/(2*$sigmaSquared) + pow($y,2)/(2*$sigmaSquared);
        $distribution[$i][$j] = $inverseTwoPiSigmaSquared * exp(-1*$power);
      }
    }
    return $distribution;
  }

}
