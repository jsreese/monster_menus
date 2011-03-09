<?php
// $Id: mm_ui_content.inc 5075 2011-02-25 22:10:05Z dan $

/**
 * @file
 * User interface routines for editing/deleting MM content
 */

/**
 * Perform an operation on a tree entry
 *
 * @param $mmtid
 *   MM tree ID of entry to modify
 * @param $op
 *   Operation to perform
 * @return
 *   The HTML code for the results
 */
function mm_ui_content($mmtid, $op = '') {
  global $user;

  $perms = mm_content_user_can($mmtid);
  if (empty($op)) {
    if ($perms['IS_RECYCLE_BIN']) $op = 'empty';
    else if ($perms['w']) $op = 'edit';
    else {
      drupal_set_message(t('You do not have permission to change the settings. Please choose another option from the menu.'));
      return ' ';
    }
    if ($op != 'edit') menu_set_active_item("mm/$mmtid/settings/$op");
  }

  $params = array(
    MM_GET_TREE_DEPTH        => $op == 'delete' || $op == 'empty' ? -1 : 0,
    MM_GET_TREE_RETURN_FLAGS => TRUE,
    MM_GET_TREE_RETURN_PERMS => TRUE,
    MM_GET_TREE_RETURN_MTIME => TRUE,
  );
  $tree = mm_content_get_tree($mmtid, $params);

  if (!count($tree)) {
    drupal_not_found();
    return;
  }

  if (!$tree[0]->perms['w'] && !$tree[0]->perms['a'] && !$tree[0]->perms['u']) {
    drupal_access_denied();
    return;
  }

  $x = mm_ui_strings($is_group = $tree[0]->is_group);
  $x['%name'] = mm_content_expand_name($tree[0]->name);

  switch ($op) {
    case 'edit':
      mm_content_add_breadcrumb(t('Settings for %name', $x));
      _mm_ui_is_user_home($tree[0]);
      return _mm_ui_content_edit_form($tree[0], $mmtid, $is_group);

    case 'copymove':
      mm_content_add_breadcrumb(t('Copy/move %name', $x));
      _mm_ui_is_user_home($tree[0]);
      module_load_include('inc', 'monster_menus', 'mm_ui_content_copymove');
      return drupal_get_form('mm_ui_content_copymove', $tree[0]);

    case 'restore':
      foreach (array_reverse(mm_content_get_parents($tree[0]->mmtid)) as $t)
        if ($t < 0) next;     // virtual user dir
        else if ($next_par) {
          $par = mm_content_get($t);
          $pperms = mm_content_user_can($t);
          break;
        }
        else if (mm_content_user_can($t, 'IS_RECYCLE_BIN')) $next_par = TRUE;

      $x['!name'] = $par ? l(mm_content_expand_name($par->name), mm_content_get_mmtid_url($par->mmtid)) : '';
      if (!$pperms['a']) {
        return t('You cannot restore this @thing because you do not have permission to add to the parent @thing, !name. You may be able to copy or move this @thing to another location, however.', $x);
      }
      module_load_include('inc', 'monster_menus', 'mm_ui_content_restore');
      return drupal_get_form('mm_ui_content_restore_confirm', $tree[0], $par->mmtid, $x);

    case 'empty':
      if (!mm_content_user_can_recycle($mmtid, 'EMPTY')) {
        drupal_access_denied();
        return;
      }
      // intentionally fall through to 'delete'

    case 'delete':
      module_load_include('inc', 'monster_menus', 'mm_ui_content_delete');
      return mm_ui_content_delete($tree, $x, $is_group);

    case 'sub':
      mm_content_add_breadcrumb(t('Create a new @subthing of %name', $x));
      $sub = clone $tree[0];
      $sub->name = $is_group ? t('(new group)') : t('(new page)');
      $sub->alias = '';
      $sub->uid = $user->uid;
      $sub->theme = '';
      $sub->hidden = FALSE;
      $sub->flags = array();
      foreach (module_invoke_all('mm_tree_flags') as $flag => $elem) {
        if (isset($elem['#flag_inherit']) && $elem['#flag_inherit'] === TRUE && isset($tree[0]->flags[$flag])) {
          $sub->flags[$flag] = $tree[0]->flags[$flag];
        }
      }

      return _mm_ui_content_edit_form($sub, $mmtid, $is_group, TRUE);

    case 'search':
      mm_content_add_breadcrumb(t('Search')); // eventually Search/Replace
      module_load_include('inc', 'monster_menus', 'mm_search_replace');
      return drupal_get_form('mm_search_form', $tree[0], $mmtid);

    case 'search result':
      mm_content_add_breadcrumb(t('Search Results')); // eventually Search/Replace
      module_load_include('inc', 'monster_menus', 'mm_search_replace');
      return mm_search_results();

    case 'search result csv':
      module_load_include('inc', 'monster_menus', 'mm_search_replace');
      return mm_search_results_csv();

    default:
      drupal_not_found();
      return;
  }
}

/**
 * Display the form for editing or creating a tree entry
 */
function _mm_ui_content_edit_form($item, $mmtid, $is_group = FALSE, $is_new = FALSE) {
  module_load_include('inc', 'monster_menus', 'mm_ui_content_edit');
  return drupal_get_form('mm_ui_content_edit', $item, $mmtid, $is_group, $is_new);
}
