<?php
// $Id: monster_menus_views_handler_field_sid.inc 2936 2009-06-02 20:13:15Z dan $

/**
 * @file
 * Views interface for Monster Menus
 */

// Modified version of workflow_views_handler_field_sid, without the workflow
// name at the beginning of the output
class monster_menus_views_handler_field_sid extends views_handler_field {
  function render($values) {
    if (empty($values->{$this->field_alias})) {
      return t('N/A');
    }

    static $states;
    if (!isset($states)) {
      $result = db_query("SELECT sid, state FROM {workflow_states} WHERE status = 1 ORDER BY sid");
      while ($data = db_fetch_object($result)) {
        $states[$data->sid] = $data->state;
      }
    }
    $output = $states[$values->{$this->field_alias}];
    if (empty($output)) {
      $output = t('(unknown)');
    }
    return check_plain($output);
  }
}
