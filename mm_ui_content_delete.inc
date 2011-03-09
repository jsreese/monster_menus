<?php
// $Id: mm_ui_content_delete.inc 4953 2011-01-24 21:55:03Z dan $

/**
 * @file
 * User interface routines for deleting MM content
 */

function mm_ui_content_delete($tree, $x, $is_group) {
  if (isset($tree[0]->flags['limit_delete'])) {
    drupal_access_denied();
    return;
  }

  $del_perm = !mm_content_recycle_enabled() ||
      $tree[0]->perms['IS_RECYCLED'] || $tree[0]->perms['IS_GROUP'];

  $mmtids = $form = array();
  $kids = 0;
  $x['%sub'] = '';
  foreach ($tree as $t) {
    if (!$t->perms['IS_RECYCLE_BIN']) {
      if ($t != $tree[0]) $kids++;
      if ($t->level == 1 && $x['%sub'] == '') $x['%sub'] = mm_content_expand_name($t->name);
    }

    if ($del_perm && !$t->perms['w'] && !$t->perms['IS_RECYCLE_BIN'] || isset($t->flags['limit_delete'])) {
      $x['%name'] = mm_content_expand_name($t->name);
      return t('You cannot delete this @thing because you do not have permission to delete the @subthing %name', $x);
    }

    $mmtids[] = $t->mmtid;
  }
  $x['@num'] = $kids;

  $nodes = $is_group ? array() : mm_content_get_nids_by_mmtid($mmtids, 0, TRUE);
  $form['mmtid'] = array('#type' => 'value', '#value' => $tree[0]->mmtid);

  if ($tree[0]->mmtid == mm_home_mmtid()) {
    return t('The @thing %name cannot be deleted.', $x);
  }

  if ($tree[0]->perms['IS_RECYCLE_BIN']) {
    mm_content_add_breadcrumb(t('Are you sure you want to empty this recycle bin?'));

    $n = array('@pages' => $kids, '@nodes' => count($nodes));
    if ($kids && $n) {
      $message = t('There are currently @pages page(s) and @nodes piece(s) of content in this recycle bin. ', $n);
    }
    else if ($kids) {
      $message = t('There are currently @pages page(s) in this recycle bin. ', $n);
    }
    else if ($n) {
      $message = t('There are currently @nodes piece(s) of content in this recycle bin. ', $n);
    }
    $message .= t('Are you sure you want to empty it?');
    $action = t('Empty Bin');
    $cancel_url = 'mm/' . $tree[0]->mmtid;
  }
  else {  // !IS_RECYCLE_BIN
    $no_nodes = t('Any page contents appear on other pages as well, and will therefore not be deleted.');
    if (!$kids) {
      $message = $del_perm ?
        t('The @thing %name will be permanently deleted.', $x) :
        t('The @thing %name will be moved to the recycle bin.', $x);
    }
    else if ($kids == 1) {
      $message = $del_perm ?
        t('The @thing %name and its @subthing %sub will be permanently deleted.', $x) :
        t('The @thing %name and its @subthing %sub will be moved to the recycle bin.', $x);
    }
    else {
      $message = $del_perm ?
        t('The @thing %name and its @num @subthings will be permanently deleted.', $x) :
        t('The @thing %name and its @num @subthings will be moved to the recycle bin.', $x);
    }

    if ($is_group) {
      $mmtid_str = db_placeholders($mmtids);
      $counts = db_fetch_object(db_query('SELECT (SELECT COUNT(DISTINCT mmtid) FROM {mm_tree_access} WHERE gid IN(' . $mmtid_str . ')) AS mmtids, (SELECT COUNT(DISTINCT nid) FROM {mm_node_write} WHERE gid IN(' . $mmtid_str . ')) AS nodes', array_merge($mmtids, $mmtids)));
      $x['@mmtids'] = $counts->mmtids;
      $x['@nodes'] = $counts->nodes;
      if ($counts->mmtids && $counts->nodes) $message .= t(' The permissions of @mmtids page(s) and @nodes piece(s) of content will be affected.', $x);
      else if ($counts->mmtids) $message .= t(' The permissions of @mmtids page(s) will be affected.', $x);
      else if ($counts->nodes) $message .= t(' The permissions @nodes piece(s) of content will be affected.', $x);
    }
    else if ($del_perm) {   // !$is_group
      if (!count($nodes)) $message .= '<p>'. $no_nodes .'</p>';
      else if (count(nodes) > 0 && !mm_content_recycle_enabled() && user_access('administer nodes')) {
        $x['@kidthings'] = $kids ? $x['@things'] : $x['@thing'];
        $x['@nodes'] = count($nodes);
        $form['delnodes'] = array(
          '#type' => 'checkbox',
          '#title' => t('Also permanently delete the @nodes piece(s) of content appearing on the @kidthings, which are not assigned elsewhere',
              $x)
        );
      }
      else {
        $can_del = 0;
        foreach ($nodes as $n)
          if (($node = node_load($n)) && node_access('delete', $node)) $can_del++;
        $message .= '<p>';
        if ($can_del == count($nodes))
          if (!mm_content_recycle_enabled()) $message .= t('All contents on the page(s) not present on other pages will be permanently deleted.');
          else $message .= t('All contents on the page(s) will be permanently deleted.');
        else if (!mm_content_recycle_enabled()) $message .= t('Any contents you have permission to delete and are not present on other pages will be permanently deleted.');
        else $message .= t('Any contents you have permission to delete will be permanently deleted.');
        $message .= '</p>';
      }
    }

    mm_content_add_breadcrumb($del_perm ?
      t('Are you sure you want to permanently delete %name?', $x) :
      t('Are you sure you want to move %name to the recycle bin?', $x));
    $action = t('Delete');
    $cancel_url = 'mm/' . $tree[0]->mmtid . '/settings';
  }   // !IS_RECYCLE_BIN

  $form['del_perm'] = array('#type' => 'value', '#value' => $del_perm);
  _mm_ui_content_delete_confirm_form($form, $message, $action, $cancel_url);
  return drupal_get_form('mm_ui_content_delete_confirm', $form);
}

function mm_ui_content_delete_confirm(&$form_state, $form) {
  return $form;
}

/**
 * Validate tree entry deletion
 */
function mm_ui_content_delete_confirm_validate($form, &$form_state) {
  $mmtid = $form_state['values']['mmtid'];

  $tree = mm_content_get_tree($mmtid, array(MM_GET_TREE_RETURN_PERMS => TRUE, MM_GET_TREE_RETURN_FLAGS => TRUE));
  if (!count($tree)) {
    form_set_error('', t('Node not found.'));
  }

  if (isset($tree[0]->flags['limit_delete'])) {
    drupal_access_denied();
    return;
  }

  $x = mm_ui_strings($tree[0]->is_group);
  foreach ($tree as $t) {
    if ($form_state['values']['del_perm'] && !$t->perms['w'] && !$t->perms['IS_RECYCLE_BIN'] || isset($t->flags['limit_delete'])) {
      $x['%name'] = mm_content_expand_name($t->name);
      form_set_error('', t('You cannot delete this @thing because you do not have permission to delete the @subthing %name', $x));
    }
  }

  if ($mmtid == mm_home_mmtid()) {
    $x['%name'] = mm_content_expand_name($tree[0]->name);
    form_set_error('', t('The %name @thing cannot be deleted.', $x));
  }
}

/**
 * Process tree entry deletion
 */
function mm_ui_content_delete_confirm_submit($form, &$form_state) {
  if ($form_state['values']['confirm']) {
    if ($form_state['values']['mmtid'] == mm_home_mmtid()) {
      drupal_set_message(t('The Home page cannot be deleted.'));
    }
    else {
      $parent = mm_content_get_parent($form_state['values']['mmtid']);

      if ($form_state['values']['del_perm']) {
        $pars = mm_content_get_parents($form_state['values']['mmtid']);
        $err = mm_content_delete($form_state['values']['mmtid'],
            mm_content_recycle_enabled() || $form_state['values']['delnodes'], TRUE);
        if (!$err) {
          // Try to delete the bin, but only bother trying if it's this entry's immediate parent.
          if (count($pars) && mm_content_user_can($pars[count($pars) - 1], 'IS_RECYCLE_BIN')) {
            $err = mm_content_delete_bin($pars[count($pars) - 1]);
            $parent = count($pars) >= 2 ? $pars[count($pars) - 2] : mm_home_mmtid();
          }
        }
      }
      else {
        $err = mm_content_move_to_bin($form_state['values']['mmtid']);
      }

      if (is_string($err)) {
        drupal_set_message($err);
      }
      else {
        mm_redirect_to_mmtid($parent);
      }
    }
  }
  $form_state['redirect'] = '';
}

function _mm_ui_content_delete_confirm_form(&$form, $message, $ok_label, $cancel_url) {
  $form['#attributes'] = array('class' => 'confirmation');
  $form['description'] = array('#type' => 'item', '#value' => $message);
  $form['confirm'] = array('#type' => 'value', '#value' => TRUE);

  $form['actions'] = array(
    '#prefix' => '<div class="container-inline">',
    '#suffix' => '</div>');
  $form['actions']['submit'] = array('#type' => 'submit', '#value' => $ok_label);
  $form['actions']['cancel'] = array('#value' => l(t('Cancel'), $cancel_url));
}
