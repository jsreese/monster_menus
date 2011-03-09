<?php
// $Id: mm_ui_node_restore.inc 4216 2010-06-25 18:36:37Z dan $

/**
 * @file
 * User interface routines for restoring nodes from the recycle bin
 */

/**
 * Confirm that the user wants to restore a node from the recycle bin
 *
 * @param $mmtid
 *   MM tree ID of the page containing the node to restore
 * @param $node
 *   Node to restore
 * @param $return
 *   Optional tree ID of the entry to return the user to when done
 * @return
 *   The HTML code for the results
 */
function mm_ui_node_restore_confirm(&$form_state, $mmtid, $node) {
  if (!_mm_ui_recycle_page_list($node, $names, $msg, TRUE)) {
    $form['error'] = array('#value' => $msg);
    return $form;
  }
  $form['nid'] = array('#type' => 'value', '#value' => $node->nid);
  $form['return'] = array('#type' => 'value', '#value' => mm_content_get_parent($mmtid));

  return confirm_form($form,
      t('Are you sure you want to restore this content?'),
      $_GET['destination'] ? $_GET['destination'] : mm_content_get_mmtid_url($mmtid),
      $msg, t('Restore'), t('Cancel'));
}

/**
 * Execute node restoration
 */
function mm_ui_node_restore_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm'] && ($node = node_load($form_state['values']['nid'])) && $node->nid) {
    db_query("DELETE FROM {mm_recycle} WHERE type = 'node' AND id = %d", $node->nid);
    db_query('DELETE FROM {mm_node2tree} WHERE nid = %d', $node->nid);
    foreach ($node->recycle_from_mmtids as $mmtid) {
      if ($mmtid) db_query('INSERT INTO {mm_node2tree} (nid, mmtid) VALUES(%d, %d)', $node->nid, $mmtid);
      watchdog('mm', "Restored node=$node->nid to mmtid=$mmtid", array());
    }

    foreach ($node->recycle_bins as $bin) {
      $err = mm_content_delete_bin($bin);
      if (is_string($err)) {
        drupal_set_message($err);
        return;
      }
    }

    if ($form_state['values']['return']) {
      mm_redirect_to_mmtid($form_state['values']['return']);
    }
  }
}
