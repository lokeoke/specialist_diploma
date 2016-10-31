<?php

class KMeans_Cluster {
  private $_x;
  private $_y;
  private $_data = array();
  private $_config;

  public function __construct($config) {
    $this->_config = $config;
  }

  public function getX() {
    return $this->_x;
  }

  public function setX($x) {
    $this->_x = $x;
    return $this;
  }

  public function getY() {
    return $this->_y;
  }

  public function setY($y) {
    $this->_y = $y;
    return $this;
  }

  public function getData() {
    return $this->_data;
  }

  public function setData($data) {
    $this->_data = $data;
    return $this;
  }

  public function addData($data) {
    $this->_data[] = $data;
    return $this;
  }

  public function removeData($data) {
    if (($key = array_search($data, $this->_data)) !== FALSE) {

      unset ($this->_data[$key]);
    }

    return $this;
  }

  public function updateCentroid() {
    if (!count($this->_data)) {

      return;
    }

    $xTotal = 0;
    $yTotal = 0;

    foreach ($this->_data as $point) {
      $xTotal += $point[$this->_config->getXKey()];
      $yTotal += $point[$this->_config->getYKey()];
    }

    $this->_x = $xTotal / count($this->_data);
    $this->_y = $yTotal / count($this->_data);

    return $this;
  }

  public function toArray() {
    return array(
      $this->_config->getXKey() => $this->_x,
      $this->_config->getYKey() => $this->_y,
      'data' => $this->_data
    );
  }

  public function getBounds() {
    $bounds = array(
      'minX' => INF,
      'minY' => INF,
      'maxX' => 0,
      'maxY' => 0
    );

    foreach ($this->_data as $point) {
      $x = $point[$this->_config->getXKey()];
      $y = $point[$this->_config->getYKey()];
      if ($x < $bounds['minX']) {
        $bounds['minX'] = $x;
      }
      if ($y < $bounds['minY']) {
        $bounds['minY'] = $y;
      }
      if ($x > $bounds['maxX']) {
        $bounds['maxX'] = $x;
      }
      if ($y > $bounds['maxY']) {
        $bounds['maxY'] = $y;
      }
    }

    return $bounds;
  }
}