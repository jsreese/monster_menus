<?php
// $Id: monster_menus_views_handler_filter_recycled.inc 2936 2009-06-02 20:13:15Z dan $

/**
 * @file
 * Views interface for Monster Menus
 */

class monster_menus_views_handler_filter_recycled extends views_handler_filter_boolean_operator {
  function query() {
    $this->ensure_my_table();
    $this->query->add_where($this->options['group'], "$this->table_alias.id " . (empty($this->value) ? 'IS NULL' : 'IS NOT NULL'));
  }
}
