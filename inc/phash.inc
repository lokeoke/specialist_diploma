<?php

/**
 * @file
 * Contains pHash implementation.
 */

function phash_bin2hex($data) {
  $result = '';
  $len = strlen($data);
  $s = '';
  $li = 0;
  for ($i = $len - 1; $i >= 0; $i--) {
    $li++;
    $s = $data[$i] . $s;
    if ($li >= 4) {
      $result = dechex(bindec($s)) . $result;
      $li = 0;
      $s = '';
    }
  }

  if ($s) {
    $result = dechex(bindec($s)) . $result;
  }

  return $result;
}

function phash_crop_array($array, $offset1, $offset2, $length1, $length2) {
  $result = array();
  foreach (array_slice($array, $offset1, $length1) as $item) {
    $result[] = array_slice($item, $offset2, $length2);
  }

  return $result;
}

function phash_get_pixel($image, $x, $y) {
  $rgb = imagecolorsforindex($image->resource, imagecolorat($image->resource, $x, $y));

  return ($rgb['red'] + $rgb['green'] + $rgb['blue']) / 3;
}

function phash_calculate_dct($in) {
  $results = array();
  $N = count($in);
  for ($k = 0; $k < $N; $k++) {
    $sum = 0;
    for ($n = 0; $n < $N; $n++) {
      $sum += $in[$n] * cos($k * pi() * ($n + 0.5) / ($N));
    }

    $sum *= sqrt(2 / $N);
    if ($k == 0) {
      $sum *= 1 / sqrt(2);
    }

    $results[$k] = $sum;
  }

  return $results;
}

function phash_dct($image) {
  $result = array();
  $rows = array();
  $row = array();

  $width = 32;
  $height = 32;

  for ($j = 0; $j < $height; $j++) {
    for ($i = 0; $i < $width; $i++) {
      $row[$i] = phash_get_pixel($image, $i, $j);
    }

    $rows[$j] = phash_calculate_dct($row);
  }

  for ($i = 0; $i < $width; $i++) {
    for ($j = 0; $j < $height; $j++) {
      $col[$j] = $rows[$j][$i];
    }

    $result[$i] = phash_calculate_dct($col);
  }

  return $result;
}

function phash_get_average_dct_value($array) {
  $result = 0;
  $c = 0;
  foreach ($array as $y => $iy) {
    foreach ($iy as $x => $ix) {
      if ($x == 0 and $y == 0) {
        continue;
      }

      $c++;
      $result += $ix;
    }
  }

  return $result / $c;
}

function phash_dct_imagehash($image) {
  if (image_resize($image, 32, 32)) {
    if (image_desaturate($image)) {
      $dct = phash_crop_array(phash_dct($image), 0, 0, 8, 8);
      $median = phash_get_average_dct_value($dct);

      $result = "";
      for ($y = 0; $y < 8; $y++) {
        for ($x = 0; $x < 8; $x++) {
          $result .= ($dct[$y][$x] > $median) ? "1" : "0";
        }
      }

      $hex = phash_bin2hex($result);
      $len = strlen($hex);
      if ($len < 16) {
        $hex = str_repeat('0', 16 - $len) . $hex;
      }

      dpm($hex);

      return $hex;
    }
  }
}