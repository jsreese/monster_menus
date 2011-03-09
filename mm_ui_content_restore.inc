<?php
// $Id: mm_ui_content_restore.inc 4216 2010-06-25 18:36:37Z dan $

/**
 * @file
 * User interface routines for restoring MM content from a recycle bin
 */

function mm_ui_content_restore_confirm(&$form_state, $item, $parent, $x) {
  $form['mmtid']     = array('#type' => 'value', '#value' => $item->mmtid);
  $form['mode']      = array('#type' => 'value', '#value' => 'move');
  $form['move_mode'] = array('#type' => 'value', '#value' => 'page');
  $form['dest']      = array('#type' => 'value', '#value' => array($parent => ''));
  $form['name']      = array('#type' => 'value', '#value' => $item->name);
  $form['alias']     = array('#type' => 'value', '#value' => $item->alias);
  $msg = t('Are you sure you want to restore this @thing as a @subthing of !name?', $x);
  $msg_short = t('Are you sure you want to restore this @thing?', $x);

  return confirm_form($form, $msg_short, mm_content_get_mmtid_url($item->mmtid), $msg,
      t('Restore'), t('Cancel'));
}

function mm_ui_content_restore_confirm_validate($form, &$form_state) {
  module_load_include('inc', 'monster_menus', 'mm_ui_content_copymove');
  return mm_ui_content_copymove_validate($form, $form_state, TRUE);
}

function mm_ui_content_restore_confirm_submit($form, &$form_state) {
  module_load_include('inc', 'monster_menus', 'mm_ui_content_copymove');
  $bin = mm_content_get_parent($form_state['values']['mmtid']);
  mm_ui_content_copymove_submit($form, $form_state, TRUE);
  $copymove_errors = form_get_errors();
  if (!isset($copymove_errors)) {
    db_query("DELETE FROM {mm_recycle} WHERE type = '%s' AND id = %d", 'cat', $form_state['values']['mmtid']);
    $err = mm_content_delete_bin($bin);
    if (is_string($err)) {
      drupal_set_message($err);
      unset($form_state['redirect']);
    }
  }
}
