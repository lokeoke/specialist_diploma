<?php

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

function phash_get_pixel($resource, $x, $y) {
  $rgb = imagecolorsforindex($resource, imagecolorat($resource, $x, $y));

  return ($rgb['red'] + $rgb['green'] + $rgb['blue']) / 3;
}

function phash_create_tmp($resource, $extension, $width, $height) {
  $res = imagecreatetruecolor($width, $height);

  if ($extension == 'gif') {
    // Grab transparent color index from image resource.
    $transparent = imagecolortransparent($resource);

    if ($transparent >= 0) {
      // The original must have a transparent color, allocate to the new image.
      $transparent_color = imagecolorsforindex($resource, $transparent);
      $transparent = imagecolorallocate($res, $transparent_color['red'], $transparent_color['green'], $transparent_color['blue']);

      // Flood with our new transparent color.
      imagefill($res, 0, 0, $transparent);
      imagecolortransparent($res, $transparent);
    }
  }
  elseif ($extension == 'png') {
    imagealphablending($res, FALSE);
    $transparency = imagecolorallocatealpha($res, 0, 0, 0, 127);
    imagefill($res, 0, 0, $transparency);
    imagealphablending($res, TRUE);
    imagesavealpha($res, TRUE);
  }
  else {
    imagefill($res, 0, 0, imagecolorallocate($res, 255, 255, 255));
  }

  return $res;
}

function phash_resize(&$resource, $extension, $src_width, $src_height, $width, $height) {
  $res = phash_create_tmp($resource, $extension, $width, $height);

  if (!imagecopyresampled($res, $resource, 0, 0, 0, 0, $width, $height, $src_width, $src_height)) {
    return FALSE;
  }

  // Update image object.
  $resource = $res;
  return TRUE;
}

function phash_desaturate($resource) {
  // PHP installations using non-bundled GD do not have imagefilter.
  if (!function_exists('imagefilter')) {
    watchdog('image', 'The image %file could not be desaturated because the imagefilter() function is not available in this PHP installation.', array('%file' => $resource->source));
    return FALSE;
  }

  return imagefilter($resource, IMG_FILTER_GRAYSCALE);
}

function phash_hamming_distance($string1, $string2) {
  $res = array_diff_assoc(str_split($string1), str_split($string2));
  return count($res);
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

function phash_dct($resource) {
  $result = array();
  $rows = array();
  $row = array();

  $width = 32;
  $height = 32;

  for ($j = 0; $j < $height; $j++) {
    for ($i = 0; $i < $width; $i++) {
      $row[$i] = phash_get_pixel($resource, $i, $j);
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

function phash_dct_imagehash($resource, $extension, $src_width, $src_height) {
  if (phash_resize($resource, $extension, $src_width, $src_height, 32, 32)) {
    if (phash_desaturate($resource)) {
      $dct = phash_crop_array(phash_dct($resource), 0, 0, 8, 8);
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

      return $hex;
    }
  }
}