<?php
// $Id: mm_menu.inc 4953 2011-01-24 21:55:03Z dan $

/**
 * @file
 * Menu-specific functions
 */

function _mm_menu_access_any_admin() {
  return
    user_access('administer all groups') ||
    user_access('administer all users') ||
    user_access('administer all menus');
}

function _mm_menu_access_all_admin() {
  return
    user_access('administer all groups') &&
    user_access('administer all users') &&
    user_access('administer all menus');
}

function _mm_menu_access_is_admin() {
  global $user;
  return $user->uid == 1;
}

function _mm_menu_access_node_delete($node) {
  return node_access('delete', $node) && mm_content_user_can_node($node, 'u');
}

function _mm_menu_access_node_restore($mmtid, $node) {
  return mm_content_node_is_recycled($node) && node_access('delete', $node) && mm_content_user_can($mmtid, 'IS_RECYCLE_BIN');
}

function _mm_menu_title_node_delete($node) {
  return mm_content_node_is_recycled($node) ? 'Delete permanently' : 'Delete';
}

function _mm_menu_access_node_revisions($node) {
  return node_access('update', $node) && node_access('view', $node) &&
      (user_access('view revisions') || user_access('administer nodes')) &&
      db_result(db_query('SELECT COUNT(vid) FROM {node_revisions} WHERE nid = %d',
      $node->nid)) > 1;
}

function _mm_menu_access_group_viewall($mmtid) {
  global $user;
  $perms = mm_content_user_can($mmtid);
  return $user->uid > 0 && $perms['r'] && $perms['IS_GROUP'];
}

function _mm_menu_access_add($mmtid) {
  $perms = mm_content_user_can($mmtid);
  if ($perms['u'] && !$perms['IS_GROUP'] && !$perms['IS_RECYCLED'] && !mm_content_is_archive($mmtid)) {
    // Make sure the user can create at least one type of content here
    $allowed_node_types = mm_content_resolve_cascaded_setting('allowed_node_types', $mmtid, $types_at, $types_parent);
    $hidden_types = mm_get_node_info(MM_NODE_INFO_ADD_HIDDEN);
    foreach (node_get_types() as $type)
      if (node_access('create', $type->type) && (user_access('administer all menus') || array_search($type->type, $hidden_types) === FALSE && array_search($type->type, $allowed_node_types) !== FALSE)) {
        return TRUE;
      }
  }
  return FALSE;
}

function _mm_menu_access_settings_tab($mmtid) {
  global $user;
  $perms = mm_content_user_can($mmtid);
  return $perms['w'] || ($perms['a'] || $perms['u'] && $mmtid != $user->user_portal_mmtid && (!$perms['IS_GROUP'] || $perms['r']) && !$perms['IS_RECYCLED']);
}

function _mm_menu_access_empty_bin($mmtid) {
  $perms = mm_content_user_can($mmtid);
  return $perms['IS_RECYCLE_BIN'] && mm_content_user_can_recycle($mmtid, 'EMPTY');
}

function _mm_menu_access_edit($mmtid) {
  $perms = mm_content_user_can($mmtid);
  return !$perms['IS_RECYCLE_BIN'] && $perms['w'];
}

function _mm_menu_access_copy($mmtid) {
  $perms = mm_content_user_can($mmtid);
  return !$perms['IS_RECYCLE_BIN'] && ($perms['w'] || $perms['a'] || $perms['u']) && !mm_content_is_archive($mmtid) && user_access('use tree browser');
}

function _mm_menu_access_restore($mmtid) {
  $parent = mm_content_get_parent($mmtid);
  $perms = mm_content_user_can($mmtid);
  return $perms['w'] && $perms['IS_RECYCLED'] && mm_content_user_can($parent, 'IS_RECYCLE_BIN');
}

function _mm_menu_title_settings_delete($mmtid) {
  $perms = mm_content_user_can($mmtid);
  return $perms['IS_RECYCLED'] || $perms['IS_GROUP'] ? 'Delete permanently' : 'Delete';
}

function _mm_menu_access_delete($mmtid) {
  $tree = mm_content_get_tree($mmtid, array(MM_GET_TREE_RETURN_PERMS => TRUE, MM_GET_TREE_RETURN_FLAGS => TRUE, MM_GET_TREE_DEPTH => 0));
  return $tree[0]->perms['w'] && (user_access('administer all menus') || !isset($tree[0]->flags['limit_delete'])) && !$tree[0]->perms['IS_RECYCLE_BIN'];
}

function _mm_menu_title_settings_sub($mmtid) {
  return t('Add @subthing', mm_ui_strings(mm_content_is_group($mmtid)));
}

function _mm_menu_access_sub($mmtid) {
  $perms = mm_content_user_can($mmtid);
  return $perms['a'] && !mm_content_is_node_content_block($mmtid) && $perms['a'] && !$perms['IS_RECYCLED'];
}

function _mm_menu_access_node_reorder($mmtid) {
  $perms = mm_content_user_can($mmtid);
  return $mmtid > 0 && !$perms['IS_RECYCLED'] && $perms['w'] && $mmtid != mm_content_users_mmtid() && !mm_content_is_group($mmtid) && !mm_content_is_archive($mmtid) && !mm_content_is_archive($mmtid, TRUE);
}

function _mm_menu_access_menu_reorder($mmtid) {
  $perms = mm_content_user_can($mmtid);
  if ($mmtid > 0 && !$perms['IS_RECYCLED'] && $perms['w'] && $mmtid != mm_content_users_mmtid() && !mm_content_is_group($mmtid) && !mm_content_is_node_content_block($mmtid) && !mm_content_is_archive($mmtid)) {
    return user_access('administer all menus') || mm_content_resolve_cascaded_setting('allow_reorder', $mmtid, $reorder_at, $reorder_parent);
  }
  return FALSE;
}

function _mm_menu_access_solver($mmtid) {
  return user_access('use permissions solver') && user_access('access user profiles') && mm_content_user_can($mmtid, 'u');
}

function _mm_menu_show_node($node, $mmtid = NULL) {
  mm_redirect_to_node($node->nid, "/$_GET[q]");
  return $mmtid ? mm_show_page() : mm_node_page_view($node);
}

function _mm_menu_active_item($path = NULL) {
  static $cache;
  global $_mm_page_args, $_mm_mmtid_of_node;

  module_load_include('module', 'node', 'node');  // needed for node_load() in mm_content_user_can_recycle() and below
  if (!isset($cache[$path])) {
    $cache[$path] = (object)array();
    $arg = arg(NULL, $path);
    if ($arg[0] == 'node') $nid = $arg[1];
    else if ($arg[0] == 'comment')
      if ($arg[1] == 'reply') $nid = $arg[2];
      else if ($arg[1] == 'delete' || $arg[1] == 'edit')
        if (intval($arg[2]) && function_exists('_comment_load') && ($row = _comment_load(intval($arg[2])))) $nid = $row->nid;

    if (isset($nid)) {
      $nid = intval($nid);
      if ($nid &&
        ($mmtid = mm_query_result("SELECT mmtid FROM {mm_node2tree} WHERE nid = %d", $nid)) &&
        !empty($mmtid)) {
          $found_nid = $nid;
          $cache[$path]->mmtid = $mmtid;
          if (!$path) $_mm_page_args = "mm/$mmtid";
      }
    }
    else if ($arg[0] == 'mm') {
      if ($arg[2] == 'node' && is_numeric($arg[3]) && ($found_nid = mm_query_result("SELECT nid FROM {node} WHERE nid = %d", $arg[3]))) {
        if ($found_nid == $arg[3]) {
          $resolved = TRUE;
          if (_mm_resolve_archive($arg[1]) && !$path) {
            $_mm_page_args = "mm/$arg[1]";
          }
          $_mm_mmtid_of_node[$found_nid] = $arg[1];
        }
        else {
          unset($found_nid);
        }
      }

      if (!isset($found_nid) && ($resolved || _mm_resolve_archive($arg[1]))) {
        $cache[$path]->mmtid = $arg[1];
        if (!$path) $_mm_page_args = "mm/$arg[1]";
        $found_nid = TRUE;
      }
    }
    $cache[$path]->nid = isset($found_nid) ? $found_nid : NULL;
  }
  return $cache[$path];
}

function mm_mmtid_load($mmtid) {
  static $set_breadcrumb;

  $mmtid = intval($mmtid);
  if ($mmtid) {
    $test = "mm/$mmtid";
    if (!$set_breadcrumb && ($test == $_GET['q'] || drupal_substr($_GET['q'], 0, drupal_strlen($test) + 1) == "$test/")) {
      $mmtids = mm_content_get_parents_with_self($mmtid);
      array_shift($mmtids);
      mm_content_fix_breadcrumbs($mmtids);
      $set_breadcrumb = TRUE;
    }
    return $mmtid;
  }
  return FALSE;
}
