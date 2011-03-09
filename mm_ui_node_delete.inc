<?php
// $Id: mm_ui_node_delete.inc 4216 2010-06-25 18:36:37Z dan $

/**
 * @file
 * User interface routines for deleting nodes
 */

/**
 * Confirm that the user wants to delete a node
 *
 * @param $mmtid
 *   MM tree ID containing the node. Can be NULL, if there is none.
 * @param $node
 *   Node to delete
 * @return
 *   The HTML code for the results
 */
function mm_ui_node_delete_confirm($form_state, $mmtid, $node) {
  $title = mm_ui_fix_node_title($node->title);
  if (!mm_content_user_can_node($node, 'u')) {
    drupal_access_denied();
  }
  else {
    $permanent = mm_content_node_is_recycled($node) || !mm_content_recycle_enabled();
    $form['nid'] = array(
      '#type' => 'value',
      '#value' => $node->nid,
    );
    $form['permanent'] = array(
      '#type' => 'value',
      '#value' => $permanent,
    );
    if (isset($mmtid)) {
      $form['return'] = array(
        '#type' => 'value',
        '#value' => $mmtid,
      );
    }

    if ($permanent) {
      $msg = $msg_short = t('Are you sure you want to permanently delete %title?', array('%title' => $title));
    }
    else {
      $mmtids = mm_content_get_by_nid($node->nid);
      $names = array();
      foreach (mm_content_get($mmtids) as $tree)
        $names[] = l(mm_content_expand_name($tree->name), mm_content_get_mmtid_url($tree->mmtid));

      if (count($names) != count($mmtids)) return;

      $msg_short = t('Are you sure you want to move %title to the recycle bin?',
          array('%title' => $title));
      if (!count($mmtids)) {
        $msg = $msg_short;
      }
      else if (count($mmtids) == 1) {
        $msg = t('Are you sure you want to move %title from page !page to the recycle bin?', array('!page' => $names[0], '%title' => $title));
      }
      else {
        $msg = t('Are you sure you want to move %title to the recycle bin? It currently appears on these pages: !list', array('%title' => $title, '!list' => implode(', ', $names)));
      }
    }

    if ($mmtid) {
      mm_content_add_breadcrumb($msg);
    }

    $cancel = $mmtid ? mm_content_get_mmtid_url($mmtid) : 'node/'. $node->nid;
    if (!empty($_REQUEST['destination'])) {
      extract(parse_url(urldecode($_REQUEST['destination'])));
      $cancel = array('path' => $path, 'query' => $query, 'fragment' => $fragment);
    }

    return confirm_form($form, $msg_short, $cancel, $msg, t('Delete'), t('Cancel'));
  }
}

/**
 * Execute node deletion
 */
function mm_ui_node_delete_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm'] && ($node = node_load($form_state['values']['nid'])) && $node->nid) {
    $return = $form_state['values']['return'];

    if ($form_state['values']['permanent']) {
      if ($return) $return = mm_content_get_parent($return);
      node_delete($node->nid);
      foreach ($node->recycle_bins as $bin) {
        $err = mm_content_delete_bin($bin);
        if (is_string($err)) break;
      }
    }
    else {
      $err = mm_content_move_to_bin(NULL, $node->nid);
    }

    if (is_string($err)) {
      drupal_set_message($err);
      return;
    }
    if ($return) mm_redirect_to_mmtid($return);
  }
  $form_state['redirect'] = '';
}
