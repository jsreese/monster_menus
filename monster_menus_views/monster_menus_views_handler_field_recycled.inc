<?php
// $Id: monster_menus_views_handler_field_recycled.inc 2936 2009-06-02 20:13:15Z dan $

/**
 * @file
 * Views interface for Monster Menus
 */

class monster_menus_views_handler_field_recycled extends views_handler_field_node {
  function get_value_options() {debug_add_dump(1111);
    if (!isset($this->value_options)) {
      $this->value_title = t('foo bar');

      $workflows = workflow_get_all();
      if (count($workflows) > 1) {
        $states = array('' => t('No state'));
        foreach ($workflows as $wid => $wname) {
          $states[$wname] = workflow_get_states($wid);
        }
      }
      else {
        $states = workflow_get_states();
      }

      $this->value_options = $states;
    }
  }

  function query() {
    if (empty($this->value)) {
      return;
    }
    $this->ensure_my_table();
    $placeholder = !empty($this->definition['numeric']) ? '%d' : "'%s'";

    if (count($this->value) == 1) {
      $this->operator = ($this->operator == 'in') ? '= ' : '!= ';
      $in = !empty($this->definition['numeric']) ? '%d' : "'%s'";
    }
    else {
      $replace = array_fill(0, sizeof($this->value), $placeholder);
      $in = ' (' . implode(", ", $replace) . ')';
    }
    $this->query->add_where($this->options['group'], "$this->table_alias.$this->real_field " . $this->operator . $in, $this->value);
  }
}
