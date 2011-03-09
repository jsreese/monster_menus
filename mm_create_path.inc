<?php
// $Id: mm_create_path.inc 4551 2010-09-15 14:09:37Z dan $

/**
 * @file
 * Functions to create all needed parts of a path in the MM tree
 */

class MMCreatePathCat {
  public
    $alias = '',
    $comment = 0,
    $default_mode = 'r',
    $flags = '',
    $hidden = FALSE,
    $hover = '',
    $max_depth = -1,
    $max_parents = -1,
    $menu_start = -1,
    $mmtid,
    $name = '',
    $no_update,
    $node_info = 3,
    $perms,
    $previews = 0,
    $reset,
    $rss = 0,
    $theme = '',
    $type = 'cat',
    $uid,
    $weight = 0;
  public $cascaded = array(
    'allow_reorder' => -1,
    'allowed_themes' => NULL,
    'allowed_node_types' => NULL
  );

  public function __construct($arr) {
    foreach ($arr as $key => $val)
      if ($key == 'cascaded') {
        foreach ($val as $ckey => $cval)
          $this->cascaded[$ckey] = $cval;
      }
      else $this->$key = $val;
  }
}

class MMCreatePathGroup {
  public
    $alias = '',
    $comment = 0,
    $default_mode = 'u,r',
    $flags = '',
    $hidden = FALSE,
    $hover = '',
    $mmtid,
    $name = '',
    $no_update,
    $perms,
    $reset,
    $rss = 0,
    $theme = '',
    $type = 'group',
    $uid,
    $weight = 0;
  public
    $qfield,
    $members,
    $qfrom,
    $vgroup;

  public function __construct($arr) {
    foreach ($arr as $key => $val)
      $this->$key = $val;
  }
}

/**
 * Create an entire path of MM tree IDs, including any missing parents
 *
 * @param $items
 *   An array of MMCreatePathCat and/or MMCreatePathGroup objects, in path order
 *
 *   In the perms field, a group can be set to 'self' to include the outer group
 *   in the list or just the mmtid, instead of its full path.
 *
 *   The uid field can be either numeric or the username.
 *
 *   In groups, the members list can contain either numeric uids or usernames.
 *
 *   Normally, each visited entry is cached and therefore only updated once per
 *   run. Set the 'reset' flag to force an update, even if the mmtid was cached.
 *
 *   Set 'no_update' to an array of field names which should only be changed if
 *   the tree ID is new, and not if it already exists.
 * @param $clear_cache
 *   If TRUE, clear the cache of items that have already been touched and
 *   return immediately.
 * @return
 *   TRUE on success
 */
function mm_create_path(&$items, $clear_cache = FALSE) {
  static $did_existing_perms = array();

  if ($clear_cache) {
    $did_existing_perms = array();
    return TRUE;
  }

  if (is_array($items) && is_array($items[0])) {
    foreach ($items as &$item)
      if (!mm_create_path($item)) return FALSE;

    return TRUE;
  }

  if (!count($items)) {
    watchdog('MM', 'Tried to create an empty item in mm_create_path()', array(), WATCHDOG_ERROR);
    return FALSE;
  }

  $last = $items[count($items) - 1];
  if (!isset($last->mmtid)) {
    $path = $parms = $exists = array();

    foreach ($items as $item) {
      if (isset($item->mmtid)) {
        // PHP doesn't have a way to keep string keys that look like integers
        // from being converted to ints, so flag the true ints with \001.
        $path["\001" . $item->mmtid] = $item->alias;
        $did_existing_perms[$item->mmtid] = TRUE;
      }
      else if (empty($item->name)) {
        $msg = 'This item has no name: ' . var_dump_ret($item);
        drupal_set_message($msg);
        watchdog('MM', $msg, array(), WATCHDOG_ERROR);
        return FALSE;
      }
      else $path[$item->name] = $item->alias;
      $parms[$item->name][$item->alias] = $item;
    }

    $mmtid = _mm_create_path($path, '_mm_create_path_create', $parms, $exists);
    if (!$mmtid) {
      $thing = $last->type == 'group' ? 'group' : 'entry';
      $msg = "Failed to create $thing described by " . var_dump_ret($items);
      drupal_set_message($msg);
      watchdog('MM', $msg, array(), WATCHDOG_ERROR);
      return FALSE;
    }

    if ($exists) {
      $i = 0;
      foreach ($exists as $k => $v) {
        if (!$v) break;
        $items[$i++]->mmtid = $k;
      }
    }
    $last->mmtid = $mmtid;

    foreach ($items as $item) {
      if ($exists[$item->mmtid] && (!$did_existing_perms[$item->mmtid] || $item->reset)) {
        $item->uid = _mm_create_path_get_uid($item->uid, 1);

        if (is_array($item->no_update) && ($tree = mm_content_get($item->mmtid))) {
          unset($block);
          unset($vquery);
          foreach ($item->no_update as $nu) {
            if ($nu == 'mmtid') continue;
            if ($nu == 'menu_start' || $nu == 'max_depth' || $nu == 'max_parents') {
              if (!$block) $block = db_fetch_object(db_query('SELECT bid AS menu_start, max_depth, max_parents FROM {mm_tree_block} WHERE mmtid=%d', $item->mmtid));

              if (isset($block->$nu)) $item->$nu = $block->$nu;
              else $item->$nu = -1;
            }
            else if ($nu == 'members') {
              $item->members = '';
            }
            else if ($nu == 'qfrom' || $nu == 'qfield') {
              if (!$vquery) {
                $vquery = db_fetch_object(db_query('SELECT v.* FROM {mm_group} g '.
                  'INNER JOIN {mm_vgroup_query} v ON g.vgid = v.vgid WHERE g.gid = %d',
                  $item->mmtid));
                if ($vquery) $vquery->qfield = $vquery->field;
              }
              if ($vquery && isset($vquery->$nu)) $item->$nu = $vquery->$nu;
            }
            else if (isset($tree->$nu)) $item->$nu = $tree->$nu;
          }
        }
        $parms = _mm_create_path_clear_parms(clone($item));
        $parms->recurs_perms = FALSE;
        if (($parms->perms = _mm_create_path_create_perms($item)) === FALSE || !mm_content_insert_or_update(FALSE, $item->mmtid, $parms)) {
          return FALSE;
        }

        $did_existing_perms[$item->mmtid] = TRUE;
      }
    }
  }

  return TRUE;
}

// ****************************************************************************
// * Private functions start here
// ****************************************************************************

/**
 * Helper function
 *
 * @param $path
 *   Array of name => alias pairs, or just names as the array keys, defining the
 *   path to create. If an array key starts with "\001", it is assumed to be
 *   the mmtid of an existing entry, which is then ignored.
 * @param $func
 *   Function to call, in order to create a given entry. The first parameter
 *   must be the mmtid of the new entry's parent. The second parameter is the
 *   new entry's name, and the third is its alias. The function must return the
 *   tree ID of the entry that was created, or 0 if there was an error.
 * @param $func_parms
 *   Optional parameters to pass to $func. This is passed by reference, so any
 *   changes made to it in $func are permanent.
 * @param $exists
 *   Optional parameter which refers to an array. On completion, this array
 *   contains elements, in order, for each segment of the path, with TRUE for
 *   any that already existed.
 * @return
 *   The tree ID of the newly-created (or previously existing) entry
 */
function _mm_create_path($path, $func, &$func_parms = NULL, &$exists = NULL) {
  static $cache;

  if (!count($path)) return 1;       // topmost mmtid is root node

  $elem = array_keys($path);         // name=>alias
  if ($elem[0] === 0) {              // no alias
    $no_alias = TRUE;
    $elem = array_values($path);
  }
  else if ($elem[count($elem)-1][0] === "\001") {
    // PHP doesn't have a way to keep string keys that look like integers
    // from being converted to ints, so this code expects mmtids to be preceded
    // with \001, instead. That way we can tell the difference between an mmtid
    // and an entry name containing only numbers.
    $mmtid = intval(substr($elem[count($elem)-1], 1));
    if (is_array($exists)) {
      array_pop($path);
      _mm_create_path($path, $func, $func_parms, $exists);
      $exists[$mmtid] = TRUE;
    }
    return $mmtid;
  }

  $longpath = implode('|:', $elem);
  if (!isset($cache[$longpath])) {
    if ($no_alias) {
      $current_name = array_pop($path);
      $current_alias = '';
    }
    else {
      $current_name = array_pop($elem);
      $current_alias = array_pop($path);
    }

    $parent = _mm_create_path($path, $func, $func_parms, $exists);
    if (!$parent) return $cache[$longpath] = 0;    // error

    if ($current_alias != '') {
      $tree = mm_content_get(array('parent' => $parent, 'alias' => $current_alias), array(), 1);
    }
    else {
      $tree = mm_content_get(array('parent' => $parent, 'name' => $current_name), array(), 1);
    }

    if (count($tree)) {
      if (is_array($exists)) $exists[$tree[0]->mmtid] = TRUE;
      $cache[$longpath] = $tree[0]->mmtid;
    }
    else {
      $cache[$longpath] = $func($parent, $current_name, $current_alias, $func_parms);
      if (is_array($exists) && $cache[$longpath]) $exists[$cache[$longpath]] = FALSE;
    }
  }
  else if (is_array($exists)) {
    array_pop($path);
    _mm_create_path($path, $func, $func_parms, $exists);
    $exists[$cache[$longpath]] = $cache[$longpath] != 0;
  }

  return $cache[$longpath];
}

/**
 * Create a new MM tree entry
 *
 * @param $parent
 *   Tree ID of the parent, under which to create a new child
 * @param $name
 *   Human-readable name of the new child
 * @param $alias
 *   URL alias of the new child
 * @param $parms
 *   Other parameters passed to mm_content_insert_or_update()
 * @return
 *   The tree ID of the newly-created (or previously existing) entry, or FALSE
 *   on error.
 */
function _mm_create_path_create($parent, $name, $alias, $parms) {
  // Note: $parms is already a reference, and PHP passes it to this function that way
  $item = &$parms[$name][$alias];
  if (isset($item->mmtid)) return $item->mmtid;

  $parms = _mm_create_path_clear_parms(clone($item));
  $parms->name = $name;
  $parms->alias = $alias;
  if (($parms->perms = _mm_create_path_create_perms($item)) === FALSE) {
    return FALSE;
  }
  $parms->uid = _mm_create_path_get_uid($parms->uid, 1);
  $parms->recurs_perms = FALSE;

  return $item->mmtid = mm_content_insert_or_update(TRUE, $parent, $parms);
}

/**
 * Remove unneeded parameters which could cause mm_content_insert_or_update() to
 * complain
 *
 * @param $parms
 *   Parameters object to modify
 * @return
 *   The modified parameters object
 */
function _mm_create_path_clear_parms($parms) {
  if ($parms->members !== '') {
    $new_members = array();
    if (is_array($parms->members)) {
      foreach ($parms->members as $m) {
        $uid = _mm_create_path_get_uid($m, FALSE);
        if ($uid !== FALSE) $new_members[] = $uid;
      }
    }
    $parms->members = $new_members;
  }
  unset($parms->mmtid);
  unset($parms->type);
  unset($parms->no_update);
  unset($parms->reset);
  unset($parms->vgroup);
  return $parms;
}

/**
 * Get the uid associated with a username
 *
 * @param $uid
 *   uid or username to resolve
 * @param $failure
 *   Message to return upon failure
 * @return
 *   The resolved uid, or $failure
 */
function _mm_create_path_get_uid($uid, $failure) {
  static $cache;

  if (!isset($uid)) return 1;
  if (is_numeric($uid)) return $uid;
  if (isset($cache[$uid])) {
    if ($cache[$uid] === 'fail') return $failure;
    return $cache[$uid];
  }

  $u = db_result(db_query("SELECT uid FROM {users} WHERE name = '%s'", $uid));
  if ($u == '') {
    drupal_set_message("Could not find user '$uid' in users table");
    $cache[$uid] = 'fail';
    return $failure;
  }

  return $cache[$uid] = $u;
}

/**
 * Create any groups referred to by another group or entry's permissions
 *
 * @param $item
 *   MMCreatePathCat or MMCreatePathGroup object describing the item for which
 *   groups are to be created
 * @return
 *   An updated list of permissions, with the resulting group IDs
 */
function _mm_create_path_create_perms(&$item) {
  $perms = array();
  foreach (array('w', 'a', 'u', 'r') as $m) {
    if (is_array($item->perms) && isset($item->perms[$m]['groups']))
      foreach ($item->perms[$m]['groups'] as &$g)
        if ($g === 'self') {
          $perms[$m]['groups'][] = 'self';
        }
        else if (is_numeric($g)) {
          $perms[$m]['groups'][] = $g;
        }
        else {
          $last = $g[count($g) - 1];
          if (!isset($last->mmtid))
            if (!mm_create_path($g)) return FALSE;

          $perms[$m]['groups'][] = $last->mmtid;
        }

    if (isset($item->perms[$m]['users'])) {
      foreach ($item->perms[$m]['users'] as &$u)
        if (($uid = _mm_create_path_get_uid($u, FALSE)) !== FALSE) $u = $uid;

      $perms[$m]['users'] = $item->perms[$m]['users'];
    }
  }

  return $perms;
}
