<?php
// $Id: mm_content.inc 5089 2011-03-01 15:19:22Z dan $

/**
 * @file
 * Functions related to the retrieval of data for monster menus
 */

// Constants used in mm_content_get()
define('MM_GET_ARCHIVE',                '_arch');  // return archive status
define('MM_GET_FLAGS',                  '_flags'); // return flags
define('MM_GET_PARENTS',                '_par');   // return parents

// Constants used in mm_content_get_tree() and mm_content_get_query()
define('MM_GET_TREE_ADD_SELECT',        '_sel');   // add to list of selected columns
define('MM_GET_TREE_BIAS_ANON',         '_bias');  // assume user 0 can't read any groups
define('MM_GET_TREE_DEPTH',             '_depth'); // tree recursion depth
define('MM_GET_TREE_FAKE_READ_BINS',    '_read');  // pretend user has read on all bins (used internally)
define('MM_GET_TREE_FILTER_BINS',       '_bins');  // return recycle bins
define('MM_GET_TREE_FILTER_DOTS',       '_dots');  // return all entries with names starting with '.'
define('MM_GET_TREE_FILTER_GROUPS',     '_grps');  // get groups
define('MM_GET_TREE_FILTER_NORMAL',     '_norm');  // get entries not group or in /users
define('MM_GET_TREE_FILTER_USERS',      '_usrs');  // get entries in /users
define('MM_GET_TREE_MMTID',             '_mmtid'); // tree ID to query
define('MM_GET_TREE_NODE',              '_node');  // node object to query permissions for
define('MM_GET_TREE_RETURN_BINS',       '_rbins'); // return list of parent recycle bins
define('MM_GET_TREE_RETURN_BLOCK',      '_rblk');  // return attributes from the mm_tree_block table
define('MM_GET_TREE_RETURN_FLAGS',      '_rflgs'); // return attributes from the mm_tree_flags table
define('MM_GET_TREE_RETURN_KID_COUNT',  '_rkids'); // return the number of children each entry has
define('MM_GET_TREE_RETURN_MTIME',      '_rmods'); // return the mtime and muid fields
define('MM_GET_TREE_RETURN_NODE_COUNT', '_rnode'); // include a "nodecount" field, containing the number of nodes using this entry
define('MM_GET_TREE_RETURN_PERMS',      '_rprms'); // return whether or not the user can perform an action
define('MM_GET_TREE_RETURN_TREE',       '_rtree'); // return attributes from the mm_tree table
define('MM_GET_TREE_SORT',              '_sort');  // sort results by weight, alpha, etc.; always TRUE when depth != 0
define('MM_GET_TREE_USER',              '_user');  // user object to test permissions against
define('MM_GET_TREE_WHERE',             '_where'); // add a WHERE clause to the outermost query

// Constants used only in mm_content_get_tree()
define('MM_GET_TREE_ADD_TO_CACHE',      '_cache'); // add results to the caches used by mm_content_get() and mm_content_get_parents()
define('MM_GET_TREE_BLOCK',             '_block'); // retrieve entries which appear in a particular block
define('MM_GET_TREE_FILTER_HIDDEN',     '_hide');  // get "hidden" entries
define('MM_GET_TREE_HERE',              '_here');  // list of tree IDs currently being viewed
define('MM_GET_TREE_ITERATOR',          '_iter');  // getTreeIterator (or subclass)
define('MM_GET_TREE_PRUNE_PARENTS',     '_ppar');  // prune parents depending upon max_parents
define('MM_GET_TREE_VIRTUAL',           '_virt');  // include virtual user list sub-entries

define('MM_GET_TREE_STATE_COLLAPSED',   (1<<0));
define('MM_GET_TREE_STATE_DENIED',      (1<<1));
define('MM_GET_TREE_STATE_EXPANDED',    (1<<2));
define('MM_GET_TREE_STATE_HERE',        (1<<3));
define('MM_GET_TREE_STATE_HIDDEN',      (1<<4));
define('MM_GET_TREE_STATE_LEAF',        (1<<5));
define('MM_GET_TREE_STATE_RECYCLE',     (1<<6));

// Constants used in mm_content_copy()
define('MM_COPY_ALIAS',                 'alia');
define('MM_COPY_COMMENTS',              'comm');
define('MM_COPY_CONTENTS',              'cont');
define('MM_COPY_ITERATE_ALTER',         'itra');
define('MM_COPY_NAME',                  'name');
define('MM_COPY_NODE_PRESAVE_ALTER',    'noda');
define('MM_COPY_OWNER',                 'ownr');
define('MM_COPY_READABLE',              'read');
define('MM_COPY_RECUR',                 'recr');
define('MM_COPY_TREE_PRESAVE_ALTER',    'trea');
define('MM_COPY_TREE_SKIP_DUPS',        'tdup');

// Constants present in mm_tree.name
define('MM_ENTRY_NAME_DEFAULT_USER',    '.Default');
define('MM_ENTRY_NAME_DISABLED_USER',   '.Disabled');
define('MM_ENTRY_NAME_GROUPS',          '.Groups');
define('MM_ENTRY_NAME_RECYCLE',         '.Recycle');
define('MM_ENTRY_NAME_USERS',           '.Users');
define('MM_ENTRY_NAME_VIRTUAL_GROUP',   '.Virtual');

// The maximum number of sub-items per item in the tree is
// MM_CONTENT_BTOA_BASE ^ MM_CONTENT_BTOA_CHARS. If you might have more than
// this many /Users (or any other level of the tree) someday, increase
// MM_CONTENT_BTOA_CHARS and run mm_content_update_sort(). A larger
// MM_CONTENT_BTOA_BASE cannot be used, unless you are using a case-sensitive
// collation on the mmtree.sort_idx database column.
//
// The maximum nesting level of the tree is the length of mm_tree.sort_idx /
// MM_CONTENT_BTOA_CHARS. While you can increase this by altering the schema,
// you may find that MySQL starts to complain about there being too many tables
// in the JOIN in custom_url_rewrite_inbound(). MM_CONTENT_MYSQL_MAX_JOINS is
// used in monster_menus.install to ensure that this limit isn't exceeded.
define('MM_CONTENT_BTOA_START',      ord('!'));
define('MM_CONTENT_BTOA_BASE',       64);
define('MM_CONTENT_BTOA_CHARS',      4);
define('MM_CONTENT_MYSQL_MAX_JOINS', 61);

// Constants related to the virtual group "dirty" field
define('MM_VGROUP_DIRTY_NOT',       0); // not dirty
define('MM_VGROUP_DIRTY_NEXT_CRON', 1); // update during next hook_cron()
define('MM_VGROUP_DIRTY_FAILED',    2); // previously failed sanity check
define('MM_VGROUP_DIRTY_REDO',      3); // failed, but OK to regenerate
// If the count of users in a virtual group decreases by more than this ratio,
// return an error message and stop the vgroup regeneration. Set the matching
// record's "dirty" field in mm_vgroup_query to MM_VGROUP_DIRTY_REDO to ignore
// this condition and regenerate the group during the next run.
define('MM_VGROUP_COUNT_SANITY',    0.20);

class mmRenderer {
  public function __construct() {
    // To avoid having to sort, this array must be in increasing order
    $this->state_to_css = array(     // Do not translate these with t()
      MM_GET_TREE_STATE_COLLAPSED => 'collapsed',
      MM_GET_TREE_STATE_DENIED =>    'denied',
      MM_GET_TREE_STATE_EXPANDED =>  'expanded',
      MM_GET_TREE_STATE_HERE =>      'here',
      MM_GET_TREE_STATE_HIDDEN =>    'hidden-entry',
      MM_GET_TREE_STATE_LEAF =>      'leaf',
      MM_GET_TREE_STATE_RECYCLE =>   'recycle-bin',
    );
    $this->state_cache = array();
    $keys = array_keys($this->state_to_css);
    $this->state_max = array_pop($keys);
  }

  public function state_class($leaf) {
    if (!isset($this->state_cache[$leaf->state])) {
      $state = $leaf->state;
      $state_css = array();
      $i = 0;
      $bit = 0;
      while ($state && $bit != $this->state_max) {
        $bit = 1 << $i++;
        if ($state & $bit && isset($this->state_to_css[$bit])) {
          $state_css[] = $this->state_to_css[$bit];
          $state ^= $bit;
        }
      }
      $this->state_cache[$leaf->state] = join(' ', $state_css);
    }
    return $this->state_cache[$leaf->state];
  }

  public function prefix_leaf($leaf, $item) {
    return '<li class="' . $this->state_class($leaf) . '">';
  }

  public function link_leaf($leaf, $item, $link_item) {
//    if( $leaf->nodecount === '0' ) $item['title'] .= t(' [no pages]');
    $atts = !empty($item['description']) ? array('title' => $item['description']) : array();

    if ($link_item['path'] != $_GET['q'] &&
        ($leaf->state & (MM_GET_TREE_STATE_EXPANDED|MM_GET_TREE_STATE_HERE)) == (MM_GET_TREE_STATE_EXPANDED|MM_GET_TREE_STATE_HERE))
      $atts['class'] = 'active';

    if ($leaf->state & MM_GET_TREE_STATE_HIDDEN) $item['title'] .= ' ' . t('(hidden)');

    if ($leaf->state & MM_GET_TREE_STATE_DENIED) $atts['rel'] = 'nofollow';

    return l($item['title'], $link_item['path'], array('attributes' => $atts, 'query' => isset($item['query']) ? $item['query'] : NULL));
  }

  public function suffix_leaf($leaf, $item) {
    return "</li>\n";
  }

  public function prefix_parent($leaf, $item) {
    return "\n<ul class=\"menu\">";
  }

  public function suffix_parent($leaf, $item) {
    return "</ul>\n";
  }

  public function is_visible($leaf) {
    if ($leaf->name == MM_ENTRY_NAME_RECYCLE) {
      return mm_content_user_can_recycle($leaf->mmtid, 'r');
    }

    if (isset($leaf->perms) && ($leaf->perms['w'] || $leaf->perms['a'] || $leaf->perms['u'])) {
      return TRUE;
    }

    return !variable_get('mm_hide_empty_pages', FALSE) || $leaf->nodecount !== '0';  // returns true if $leaf->nodecount is not set
  }
}

global $_mm_content_default_renderer;
$_mm_content_default_renderer = new mmRenderer();

/**
 * Render an entry and its children as HTML
 *
 * @param $tree
 *   Array of tree nodes, as generated by mm_get_tree
 * @param &$i
 *   Optional starting index
 * @param &$path
 *   Optional menu array, suitable to be passed to menu_set_location()
 * @param &$parents
 *   Optional array containing the full GET path of the current item. Used
 *   internally.
 * @param $depth
 *   Depth to display; set to -1 to show all levels
 * @param $renderer
 *   mmRenderer (or subclass) to use in rendering the HTML. If omitted,
 *   use $_mm_content_default_renderer, which produces a simple menu tree.
 * @return
 *   The HTML code
 */
function mm_content_render_tree($tree, &$i = 0, &$path = NULL, &$parents = array('mm'), $depth = -1, $renderer = NULL) {

  if ($i >= count($tree)) return;

  if (!isset($renderer)) {
    global $_mm_content_default_renderer;
    $renderer = $_mm_content_default_renderer;
  }

  $lev0 = $tree[$i]->level;
  while ($i < count($tree) && $tree[$i]->level == $lev0) {
    $leaf = $tree[$i];
    $name = mm_content_expand_name($leaf->name);

    if (!$renderer->is_visible($leaf) ||
        $name[0] == '.' && !(isset($leaf->perms) ?
        $leaf->perms['r'] : mm_content_user_can($leaf->mmtid, 'r'))) {
      while (++$i < count($tree) && $tree[$i]->level > $lev0) ; // skip kids
      continue;   // get next sibling
    }

    $item = array('title' => $name);
    if ($leaf->hover) $item['description'] = $leaf->hover;
    $link_item = array('path' => implode('/', $parents) . '/' . $leaf->mmtid);
    if (is_array($path) && $leaf->state & MM_GET_TREE_STATE_EXPANDED) {
      $path[] = array(
        'path' => $link_item['path'],
        'title' => $name,
        'type' => MENU_DYNAMIC_ITEM);
    }

    $output .= $renderer->prefix_leaf($leaf, $item) .
        $renderer->link_leaf($leaf, $item, $link_item);

    while (++$i < count($tree) && $tree[$i]->level > $lev0) {
      if ($tree[$i]->level == $lev0 + 1 && ($depth || $leaf->state & MM_GET_TREE_STATE_EXPANDED)) {
        $parents[] = $leaf->mmtid;
        $output .= $renderer->prefix_parent($tree[$i], $item) .
            mm_content_render_tree($tree, $i, $path, $parents, 0, $renderer) .
            $renderer->suffix_parent($tree[$i], $item);
        array_pop($parents);
        $i--;
      }
    }

    $output .= $renderer->suffix_leaf($leaf, $item);
  }
  return $output;
}

/**
 * Return a nicer version of entry names starting with '.'
 *
 * @param $name
 *   The entry name, obtained from mm_content_get_tree
 * @return
 *   The nicer version
 */
function mm_content_expand_name($name) {
  static $aliases;
  if (!isset($aliases)) {
    $aliases = array(
      MM_ENTRY_NAME_DEFAULT_USER       => t('[New account defaults]'),
      MM_ENTRY_NAME_DISABLED_USER      => t('[Disabled accounts]'),
      MM_ENTRY_NAME_GROUPS             => t('Permission groups'),
      MM_ENTRY_NAME_RECYCLE            => t('[Recycle bin]'),
      MM_ENTRY_NAME_USERS              => t('User list'),
      MM_ENTRY_NAME_VIRTUAL_GROUP      => t('[Pre-defined groups]'),
    );
    $aliases += mm_module_invoke_all('mm_item_name');
  }

  if (isset($aliases[$name])) return $aliases[$name];
  return $name;
}

/**
 * Class used with mm_content_get_tree to take an action as each node is found.
 * IMPORTANT: Do not depend on $item->state in iterate(). It is not correct.
 */
class getTreeIterator {
  private $parent_is_group, $parent_is_user;

  public function iterate($item) {
    // function must return 1 if no error, 0 if error, -1 if this node
    // and any of its children should be skipped
    return 1;
  }
}

/**
 * Traverse the tree
 *
 * @param $mmtid (1)
 *   Starting tree ID
 * @param $params
 *   An array containing parameters. The array is indexed using the constants
 *   below.
 *   - MM_GET_TREE_ADD_SELECT (none):
 *     A string or array of strings to add to the SELECT portion of the query
 *   - MM_GET_TREE_ADD_TO_CACHE (FALSE):
 *     Add results to the caches used by mm_content_get() and
 *     mm_content_get_parents()
 *   - MM_GET_TREE_BIAS_ANON (TRUE):
 *     If TRUE, assume user 0 can't read any groups (more secure)
 *   - MM_GET_TREE_BLOCK (0):
 *     Only retrieve entries that are part of one block. Defaults to all blocks.
 *   - MM_GET_TREE_DEPTH (-1):
 *     When 'mmtid' is used, a query to return all items in the tree below that
 *     point can be returned. This field specifies the depth of recursion:
 *     - 0:  just the item specified by $mmtid
 *     - -1: all levels
 *     - 1:  the item and its immediate children
 *     - N:  any other other number will return that many levels (can be slow)
 *   - MM_GET_TREE_FAKE_READ_BINS (FALSE):
 *     Pretend the user can read all recycle bins (used internally)
 *   - MM_GET_TREE_FILTER_BINS (TRUE):
 *     Get entries that are recycle bins
 *   - MM_GET_TREE_FILTER_DOTS (TRUE):
 *     Get all entries with names that start with '.'. If FALSE, only .Groups,
 *     .Users, and .Virtual are returned.
 *   - MM_GET_TREE_FILTER_GROUPS (TRUE):
 *     Get entries that are groups
 *   - MM_GET_TREE_FILTER_HIDDEN (FALSE):
 *     If TRUE, return entries with the "hidden" attribute set, even if the
 *     current user does not normally have permission to view them
 *   - MM_GET_TREE_FILTER_NORMAL (TRUE):
 *     Get entries that are neither groups nor in /users
 *   - MM_GET_TREE_FILTER_USERS (TRUE):
 *     Get entries in /users
 *   - MM_GET_TREE_HERE (none)
 *     An array of MM Tree IDs currently being viewed by the user. Parent
 *     entries will have their their state set to MM_GET_TREE_STATE_EXPANDED.
 *   - MM_GET_TREE_ITERATOR (none):
 *     getTreeIterator (or subclass) to call as each new item is found.
 *     When this option is used, memory is conserved by not returning anything.
 *   - MM_GET_TREE_PRUNE_PARENTS (FALSE):
 *     If TRUE, prune parents, depending upon max_parents in the block
 *   - MM_GET_TREE_RETURN_BINS (FALSE):
 *     A comma-separated list of the mmtids of any parent recycle bins
 *   - MM_GET_TREE_RETURN_BLOCK (FALSE):
 *     Attributes from the mm_tree_block table
 *   - MM_GET_TREE_RETURN_FLAGS (FALSE):
 *     Flags from the mm_tree_flags table
 *   - MM_GET_TREE_RETURN_KID_COUNT (FALSE):
 *     A count of the number of children each tree entry has
 *   - MM_GET_TREE_RETURN_MTIME (FALSE):
 *     The muid (user ID who made the last modification) and mtime (time) of the
 *     modification
 *   - MM_GET_TREE_RETURN_NODE_COUNT (FALSE):
 *     If TRUE, return a count of the number of nodes assigned to each item. If
 *     a string or array of strings, return a count of the number of nodes of
 *     that type.
 *   - MM_GET_TREE_RETURN_PERMS (none):
 *     If set, return whether or not the user can perform that action ('r', 'w',
 *     'a', 'u', 'IS_USER', 'IS_GROUP', 'IS_RECYCLE_BIN', 'IS_RECYCLED'). The
 *     requested permission can either be a single value or an array. If an
 *     empty array or TRUE is passed, all permissions are returned.
 *   - MM_GET_TREE_SORT (FALSE):
 *     If TRUE, sort the entries according to sort_idx; always TRUE when
 *     MM_GET_TREE_DEPTH != 0
 *   - MM_GET_TREE_USER (current user):
 *     User object to test permissions against
 *   - MM_GET_TREE_VIRTUAL (TRUE):
 *     Include virtual user list sub-entries
 *   - MM_GET_TREE_WHERE (none):
 *     Add a WHERE clause to the outermost query
 *   If none of ([...USERS], [...GROUPS], [...NORMAL]) is TRUE, all types are
 *   retrieved. MM_GET_TREE_RETURN_TREE is always TRUE.
 * @return
 *   Array of tree entries, unless $iter is used
 */
function mm_content_get_tree($mmtid = 1, $params = NULL) {
  global $user;

  $defaults = array(
    MM_GET_TREE_BLOCK =>         0,
    MM_GET_TREE_DEPTH =>         -1,
    MM_GET_TREE_FILTER_BINS =>   TRUE,
    MM_GET_TREE_FILTER_DOTS =>   TRUE,
    MM_GET_TREE_FILTER_HIDDEN => FALSE,
    MM_GET_TREE_HERE =>          NULL,
    MM_GET_TREE_ITERATOR =>      NULL,
    MM_GET_TREE_PRUNE_PARENTS => FALSE,
    MM_GET_TREE_USER =>          $user,
    MM_GET_TREE_VIRTUAL =>       TRUE,
    'found' =>        -1,
    'level' =>        0,
    'parent_level' => -1,
    'pprune' =>       -1,
    'q' =>            NULL,
  );

  if (!is_array($params)) $params = array();
  $params = array_merge($defaults, $params);

  if (!$params[MM_GET_TREE_FILTER_GROUPS] && !$params[MM_GET_TREE_FILTER_USERS] && !$params[MM_GET_TREE_FILTER_NORMAL]) {
    $params[MM_GET_TREE_FILTER_GROUPS] = $params[MM_GET_TREE_FILTER_USERS] = $params[MM_GET_TREE_FILTER_NORMAL] = TRUE;
  }

  if ($params[MM_GET_TREE_VIRTUAL] && !variable_get('mm_use_virtual_user_dir', TRUE)) {
    $params[MM_GET_TREE_VIRTUAL] = FALSE;
  }

  if ($params[MM_GET_TREE_SORT] || $params[MM_GET_TREE_DEPTH] != 0) mm_content_update_sort_queue();

  return _mm_content_get_tree($mmtid, $params);
}

function _mm_content_get_tree($mmtid, &$params) {
  $users_mmtid = mm_content_users_mmtid();

  if (!$params['q']) {
    $params['q'] = _mm_content_get_tree_query($mmtid, $params);
    if (is_array($params[MM_GET_TREE_HERE]))
      foreach ($params[MM_GET_TREE_HERE] as $i => $h)
        if ($h < 0) {
          unset($params[MM_GET_TREE_HERE][$i]);
          $have_virtual = TRUE;
          if ($params[MM_GET_TREE_DEPTH] > 0) $params[MM_GET_TREE_DEPTH]--;
          break;
        }
        else if ($params[MM_GET_TREE_VIRTUAL] && $h == $users_mmtid && $params[MM_GET_TREE_DEPTH]) {
          $have_virtual = TRUE;
        }
  }

  $rows = array();

  while ($r = $params['q']->next()) {
    if (!isset($params['q']->start_level)) $params['q']->start_level = strlen($r->sort_idx) / MM_CONTENT_BTOA_CHARS;
    $r->level = strlen($r->sort_idx) / MM_CONTENT_BTOA_CHARS - $params['q']->start_level + $params['q']->level_offset;

    if ($r->level <= $params['parent_level']) {
      $params['q']->back();
      break;
    }
    else if ($r->level > $params['parent_level']) {
      if (is_null($r->bid) || !isset($r->bid)) {
        $r->bid = 0;
        $r->max_depth = $r->max_parents = -1;
      }
      $add = _mm_content_get_tree_recurs($r, $params, $parent_is_group, $parent_is_user, $last);
    }

    if (is_array($rows) && is_array($add)) {
      $rows = array_merge($rows, $add);
      unset($add);   // save some memory
    }

    if ($last) break;
  }

  if ($params['pprune'] > 0 && $params['found']) $params['pprune']--;

  if (isset($params[MM_GET_TREE_ITERATOR])) return NULL;

  if (!$params['level'] && ($have_virtual || $mmtid == $users_mmtid) && $params[MM_GET_TREE_DEPTH]) {
    if ($params[MM_GET_TREE_VIRTUAL]) {
      $letters = mm_query_result("SELECT GROUP_CONCAT(DISTINCT UCASE(SUBSTR(name, %d, %d)) ORDER BY name SEPARATOR '') FROM {mm_tree} WHERE parent = %d", 1, 1, $users_mmtid);
      $letters = preg_replace('/[\W_]/', '', $letters, -1, $matches);
      if ($matches) $letters = "~$letters";
      $letters = str_split($letters);
      $parent = NULL;

      for ($i = 0; $i < count($rows); $i++) {
        if ($rows[$i]->mmtid == $users_mmtid) {
          $parent = $rows[$remainder = $i];
          $parent->state &= ~MM_GET_TREE_STATE_HERE;
          $last = 0;

          while (++$i < count($rows) && $rows[$i]->level > $parent->level) {
            if ($rows[$i]->level == $parent->level + 1) {
              $letr = drupal_strtoupper($rows[$i]->name[0]);
              $name = ctype_alpha($letr) ? $letr : t('(other)');
              if (!$last || $name != $rows[$last]->name) {
                $alias = ctype_alpha($letr) ? $letr : '~';
                while ($letters) {
                  $add = array_shift($letters);
                  $new = _mm_content_virtual_dir(-ord($add), $parent->mmtid, $parent->level + 1, $add == $alias ? MM_GET_TREE_STATE_EXPANDED|MM_GET_TREE_STATE_HERE : MM_GET_TREE_STATE_COLLAPSED);
                  $new->default_mode = $parent->default_mode;
                  array_splice($rows, $last = $i++, 0, array($new));  // insert virtual dir
                  $remainder++;
                  if ($add == $alias) break;
                }
              }

              $rows[$i]->parent = $rows[$last]->mmtid;
              if ($rows[$i]->state & MM_GET_TREE_STATE_EXPANDED) $rows[$last]->state = MM_GET_TREE_STATE_EXPANDED;
            }     // if
            $remainder++;

            $rows[$i]->level++;
          }       // while
          break;  // exit outer for loop
        }         // if
      }           // for
    }

    if (!user_access('administer all users')) {
      $hidden_names = variable_get('mm_hidden_user_names', array());
      $dels = array();
      foreach ($rows as $i => $r) {
        if ($r->alias == '~') {
          $other = $i;
        }
        else if ($r->parent == -126 || $r->parent == $users_mmtid) {  // -126 = -ord('~')
          if (in_array($r->name, $hidden_names)) {
            $dels[] = $i;
          }
          else {
            unset($other);
          }
        }
      }

      if (isset($other)) {
        // All 'other' rows are invisible to the user
        array_unshift($dels, $other);
      }

      foreach (array_reverse($dels) as $i)
        array_splice($rows, $i, 1);
    }

    if ($params[MM_GET_TREE_VIRTUAL]) {
      $i = $parent ? $remainder + 1 : count($rows);
      foreach ($letters as $add) {
        $new = _mm_content_virtual_dir(-ord($add), $users_mmtid, $parent ? $parent->level + 1 : 0, MM_GET_TREE_STATE_COLLAPSED);
        if ($parent) $new->default_mode = $parent->default_mode;
        array_splice($rows, $i++, 0, array($new));  // insert virtual dir
      }
    }
  }

// if( !$params['level'] ) debug_add_dump( $rows );
  return $rows;
}

/**
 * Helper function for _mm_content_get_tree()/mm_content_get()
 */
function _mm_content_split_flags($flags) {
  if (is_array($flags)) return $flags;
  preg_match_all('/(?:(.*?)\|1(.*?)(?:\|2|$))/', $flags, $matches);
  return $matches[0] ? array_combine($matches[1], $matches[2]) : array();
}

/**
 * Helper function for _mm_content_get_tree()
 */
function _mm_content_get_tree_query($mmtid, $params) {
  $params[MM_GET_TREE_RETURN_TREE] = TRUE;
  if (isset($params[MM_GET_TREE_BLOCK]) && $params[MM_GET_TREE_BLOCK] != 0) $params[MM_GET_TREE_RETURN_BLOCK] = TRUE;

  if (!is_array($params[MM_GET_TREE_HERE])) $params[MM_GET_TREE_HERE] = array($mmtid);
  else if (!count($params[MM_GET_TREE_HERE])) $params[MM_GET_TREE_HERE][] = $mmtid;
  else if ($params[MM_GET_TREE_DEPTH] != 0) $params[MM_GET_TREE_DEPTH] = 1;

  $query = array();
  $max = count($params[MM_GET_TREE_HERE]) - 1;
  $users_mmtid = $params[MM_GET_TREE_VIRTUAL] ? mm_content_users_mmtid() : -1;
  if (isset($params[MM_GET_TREE_RETURN_PERMS])) {
    if (!isset($params[MM_GET_TREE_ITERATOR])) {
      $params[MM_GET_TREE_RETURN_BINS] = TRUE;
      $params[MM_GET_TREE_FAKE_READ_BINS] = TRUE;
    }
  }
  else {
    $params[MM_GET_TREE_RETURN_PERMS] = array('IS_GROUP', 'IS_USER');
  }

  for ($i = 0; $i <= $max; $i++) {
    $mmtid = $params[MM_GET_TREE_HERE][$i];
    if ($mmtid != $users_mmtid || $i == $max || $params[MM_GET_TREE_HERE][$i + 1] >= 0) {
      $params2 = $params;
      if ($mmtid < 0) {
        $ch = chr(-$mmtid);
        $re = $ch == '~' ? "t.name REGEXP '^[^[:alpha:]]'" : "UCASE(t.name) LIKE '$ch%'";
        $params2[MM_GET_TREE_INNER_FILTER] = " AND $re";
        $params2[MM_GET_TREE_DEPTH] = 1;
        $params2[MM_GET_TREE_MMTID] = $users_mmtid;
      }
      else {
        $params2[MM_GET_TREE_DEPTH] = $mmtid == $users_mmtid ? 0 : 1;
        $params2[MM_GET_TREE_MMTID] = $mmtid;
      }

      if ($i == $max) {
        if ($mmtid != $users_mmtid) $params2[MM_GET_TREE_DEPTH] = $params[MM_GET_TREE_DEPTH];
        $params2[MM_GET_TREE_BLOCK] = $params[MM_GET_TREE_BLOCK];
        $params2[MM_GET_TREE_SORT] = $params[MM_GET_TREE_DEPTH] != 0 || $params[MM_GET_TREE_SORT];
        $query[] = mm_content_get_query($params2);
      }
      else {
        $params2[MM_GET_TREE_BLOCK] = 0;
        $params2[MM_GET_TREE_SORT] = FALSE;
        $query[] = preg_replace('/ ORDER BY NULL$/', '', mm_content_get_query($params2));
      }
    }
  }

// debug_add_dump($mode, $params[MM_GET_TREE_HERE], db_prefix_tables(join(' UNION ', $query)));
  $params['q'] = mm_query(join(' UNION ', $query));
  $params['q']->level_offset = $params['level'];
  return $params['q'];
}

function _mm_content_get_tree_recurs($r, $params, $parent_is_group, $parent_is_user, &$last) {
  global $_mmtbt_cache, $_mmgp_cache, $_mmuc_cache;

  $rows = array();
  $last = TRUE;

  $xlate = array('w', 'a', 'u', 'r', 'IS_GROUP', 'IS_USER', 'ADMIN', 'IS_RECYCLE_BIN', 'IS_RECYCLED');
  foreach ($xlate as $field)
    if (isset($r->$field)) {
      if (!isset($r->perms)) $r->perms = array();
      $r->perms[$field] = $r->$field != 0;
      unset($r->$field);
    }

  if ($params[MM_GET_TREE_RETURN_PERMS] && !isset($params[MM_GET_TREE_ITERATOR])) {
    if ($r->perms['IS_RECYCLE_BIN']) {
      $r->perms['u'] = TRUE;
      $r->perms['r'] = mm_content_user_can_recycle($r->mmtid, 'r', $params[MM_GET_TREE_USER]);
    }
    else if (isset($r->recycle_bins)) {
      foreach (split(',', $r->recycle_bins) as $bin) {
        $r->perms['r'] = $r->perms['r'] && mm_content_user_can_recycle($bin, 'r', $params[MM_GET_TREE_USER]);
      }
    }
  }

  if ($params[MM_GET_TREE_ADD_TO_CACHE]) {
    if (!isset($_mmtbt_cache[$r->mmtid])) {
      $_mmtbt_cache[$r->mmtid] = $r;
    }

    if (!isset($_mmgp_cache[$r->mmtid])) {
      $_mmgp_cache[$r->mmtid] = $r->parent;
    }

    if (isset($r->perms))
      foreach ($r->perms as $field => $val)
        if (!isset($_mmuc_cache[$r->mmtid][$params[MM_GET_TREE_USER]->uid][$field]))
          $_mmuc_cache[$r->mmtid][$params[MM_GET_TREE_USER]->uid][$field] = $val;
  }

  if ($params[MM_GET_TREE_RETURN_FLAGS]) {
    $r->flags = _mm_content_split_flags($r->flags);
  }

  if (!isset($r->is_group)) $r->is_group =
    $parent_is_group || $r->name == MM_ENTRY_NAME_GROUPS ||
    isset($params[MM_GET_TREE_ITERATOR]) && $params[MM_GET_TREE_ITERATOR]->parent_is_group ||
    (isset($r->perms) ? $r->perms['IS_GROUP'] : mm_content_user_can($r->mmtid, 'IS_GROUP'));

  if (!isset($r->is_user)) $r->is_user =
    $parent_is_user  || $r->name == MM_ENTRY_NAME_USERS  ||
    isset($params[MM_GET_TREE_ITERATOR]) && $params[MM_GET_TREE_ITERATOR]->parent_is_user  ||
    (isset($r->perms) ? $r->perms['IS_USER']  : mm_content_user_can($r->mmtid, 'IS_USER'));

  $r->is_dot = $r->name[0] == '.';

  if ($r->is_group) unset($r->nodecount);

  $visible = ($params[MM_GET_TREE_FILTER_GROUPS] || !$r->is_group) &&
      ($params[MM_GET_TREE_FILTER_NORMAL] || $r->is_group || $r->is_user) &&
      ($params[MM_GET_TREE_FILTER_USERS] || !$r->is_user);

  if ($r->is_user && in_array($r->name, variable_get('mm_hidden_user_names', array()))) {
    $r->bid = -1;
  }

  if ($visible || $r->mmtid == 1) {
    if ($r->is_group || $r->name == MM_ENTRY_NAME_USERS || $r->mmtid == 1) {
      unset($r->nodecount);
    }

    $params2 = $params;
    if (is_array($params[MM_GET_TREE_HERE])) $params2[MM_GET_TREE_HERE] =& $params[MM_GET_TREE_HERE];
    $params2[MM_GET_TREE_DEPTH] = $params[MM_GET_TREE_DEPTH] < 0 ? -1 : $params[MM_GET_TREE_DEPTH] - 1;

    if (count($params[MM_GET_TREE_HERE]) && $r->mmtid == $params[MM_GET_TREE_HERE][0]) {
      $r->state = count($params[MM_GET_TREE_HERE]) >= 2 ? MM_GET_TREE_STATE_EXPANDED : MM_GET_TREE_STATE_EXPANDED|MM_GET_TREE_STATE_HERE;
      array_shift($params[MM_GET_TREE_HERE]);

      if ($params[MM_GET_TREE_BLOCK] && ($r->bid < 0 && $r->max_depth >= 0 || $r->bid > 0)) {
        $depth_new = $r->max_depth;
        if ($depth_new == -1) {
          $params2[MM_GET_TREE_DEPTH] = $params[MM_GET_TREE_DEPTH] = -1;
        }
        else {
          $params[MM_GET_TREE_DEPTH] = $depth_new;
          $params2[MM_GET_TREE_DEPTH] = $depth_new - 1;
        }
      }
      else if ($params[MM_GET_TREE_DEPTH] < 0 || $params[MM_GET_TREE_DEPTH] > 2) {
        $params[MM_GET_TREE_DEPTH] = count($params[MM_GET_TREE_HERE]) + 2;
        $params2[MM_GET_TREE_DEPTH] = $params[MM_GET_TREE_DEPTH] - 1;
      }

      $params['found'] = $r->mmtid;
      if ($params[MM_GET_TREE_PRUNE_PARENTS] && $r->max_parents != '' && $r->max_parents >= 0) {
        $params['pprune'] = $r->max_parents + 2;
      }
    }
    else {
      $r->state = $params[MM_GET_TREE_DEPTH] && $r->parent <= 0 ? MM_GET_TREE_STATE_EXPANDED : (isset($r->kids) && $r->kids > 0 ? MM_GET_TREE_STATE_COLLAPSED : MM_GET_TREE_STATE_LEAF);
      if (is_array($params[MM_GET_TREE_HERE])) {
        $params2[MM_GET_TREE_DEPTH] = 0;
        $params2['once'] = TRUE;
        foreach (array(MM_GET_TREE_PRUNE_PARENTS, MM_GET_TREE_RETURN_NODE_COUNT) as $mode)
          $params2[$mode] = FALSE;
      }
    }

    if ((!$params[MM_GET_TREE_BLOCK] || $r->bid == $params[MM_GET_TREE_BLOCK] || $r->bid <= 0) && (!$r->hidden || !$r->level || $params[MM_GET_TREE_FILTER_HIDDEN] || isset($params[MM_GET_TREE_ITERATOR]) || !isset($r->perms) || $r->perms['w'] || $r->perms['a'] || $r->perms['u'] || user_access('view all menus'))) {

      if (!isset($params[MM_GET_TREE_ITERATOR])) $parent = count($rows);

      if ($r->state) {
        if ($r->hidden) {
          $r->state |= MM_GET_TREE_STATE_HIDDEN;
        }
        else if ($r->name == MM_ENTRY_NAME_RECYCLE) {
          $r->state |= MM_GET_TREE_STATE_RECYCLE;
        }

        if (!$r->is_group && isset($r->perms['r']) && !$r->perms['r']) {
          $r->state |= MM_GET_TREE_STATE_DENIED;
          $skip_kids = TRUE;
        }

        if ($visible) {
          if (!isset($params[MM_GET_TREE_ITERATOR])) {
            $rows[] = $r;
            if ($params['once']) return $rows;
          }
          else if ($params['once']) return;
          else if (($iter_ok = $params[MM_GET_TREE_ITERATOR]->iterate($r)) < 0) {
            $last = FALSE;
            $skip_kids = TRUE;
          }
          else if (!$iter_ok) return;
        }
      }

      if (!isset($skip_kids) && $params[MM_GET_TREE_DEPTH]) {
        if (isset($params[MM_GET_TREE_ITERATOR])) {
          $ois_grp = $params[MM_GET_TREE_ITERATOR]->parent_is_group;
          $ois_user = $params[MM_GET_TREE_ITERATOR]->parent_is_user;
          $params[MM_GET_TREE_ITERATOR]->parent_is_group = $r->is_group;
          $params[MM_GET_TREE_ITERATOR]->parent_is_user = $r->is_user;
        }

        $params2['found'] = -1;
        if ($params2['once']) $params2['pprune'] = -1;
        $params2['level'] = $params['level'] + 1;
        $params2['parent_level'] = $r->level;
        $kids = _mm_content_get_tree($r->mmtid, $params2);

        if ($params2['pprune'] >= 0)
          if ($params2['pprune'] == 0) {
            $params['pprune'] = 0;
            return $kids;
          }
          else if ($params2['found']) $params['pprune'] = $params2['pprune'];

        if (isset($params[MM_GET_TREE_ITERATOR])) {
          $params[MM_GET_TREE_ITERATOR]->parent_is_group = $ois_grp;
          $params[MM_GET_TREE_ITERATOR]->parent_is_user = $ois_user;
        }
        else {
          if (count($rows) > $parent) {
            if ($rows[$parent]->is_group)
              foreach ($kids as $k) {
                $k->is_group = TRUE;
                unset($k->nodecount);
              }

            if ($rows[$parent]->is_user)
              foreach ($kids as $k) {
                $k->is_user = TRUE;
                $k->is_user_home = $k->level == $rows[$parent]->level + 1 &&
                    $rows[$parent]->name == MM_ENTRY_NAME_USERS;
              }

            if ($params['found'] != $r->mmtid) {
              $rows[$parent]->state &= ~(MM_GET_TREE_STATE_EXPANDED|MM_GET_TREE_STATE_COLLAPSED|MM_GET_TREE_STATE_LEAF);
              $rows[$parent]->state |= $params2['found'] >= 0 ? MM_GET_TREE_STATE_EXPANDED :
                  (count($kids) || isset($r->kids) && $r->kids > 0 ? MM_GET_TREE_STATE_COLLAPSED : MM_GET_TREE_STATE_LEAF);
            }
          }

          if (!$params['once'] && is_array($kids)) $rows = array_merge($rows, $kids);
        }

        if ($params2['found'] >= 0) $params['found'] = $params2['found'];
      }       // if( $params[MM_GET_TREE_DEPTH] )
      else $skip_kids = TRUE;
    }         // if( !$params[MM_GET_TREE_BLOCK] || ...
    else $skip_kids = TRUE;
  }           // if( $visible || $r->parent<=0 )
  else $skip_kids = TRUE;

  if (isset($skip_kids)) {
    while (($row = $params['q']->next()) && strlen($row->sort_idx) > strlen($r->sort_idx));
    if ($row) $params['q']->back();
  }

  $last = FALSE;
  return $rows;
}

/**
 * Get the cascaded (inherited by children) settings for an entry
 *
 * @param $mmtid
 *   ID of the entry to load settings for. If NULL, return a list of possible
 *   settings and their data representation. The structure of the returned array
 *   in this case is:
 *   - data_type:   'int' (integer) or 'string'
 *   - multiple:    TRUE if multiple values are accepted
 *   - user_access: user must have user_access() for this value in order to set
 *                  the setting
 *   - not_empty:   TRUE if only !empty() values should be stored
 * @return
 *   array containing the settings
 */
function mm_content_get_cascaded_settings($mmtid = NULL, $name = NULL) {
  static $cascaded_settings;

  if (is_null($mmtid)) {
    if (!isset($cascaded_settings)) {
      $cascaded_settings = array(
        'allow_reorder' => array('data_type' => 'int', 'user_access' => 'administer all menus'),
        'allowed_themes' => array('data_type' => 'string', 'multiple' => TRUE, 'user_access' => 'administer all menus'),
        'allowed_node_types' => array('data_type' => 'string', 'multiple' => TRUE, 'user_access' => 'administer all menus'),
        'comments_readable' => array('data_type' => 'string', 'not_empty' => TRUE),
        'nodes_per_page' => array('data_type' => 'int', 'not_empty' => TRUE),
      );
      // check for mm_cascaded_settings hooks
      $cascaded_settings = array_merge($cascaded_settings, module_invoke_all('mm_cascaded_settings'));
    }
    return $cascaded_settings;
  }

  $cascaded = array();
  $q = mm_query($mmtid, "SELECT * FROM {mm_cascaded_settings} WHERE mmtid = %d", $mmtid);
  while ($r = $q->next()) {
    if ($r->data_type == 'int') $r->data = (int)$r->data;

    if ($r->multiple) {
      if (!is_array($cascaded[$r->name])) $cascaded[$r->name] = array();
      if ($r->array_key != '') $cascaded[$r->name][$r->array_key] = $r->data;
      else $cascaded[$r->name][] = $r->data;
    }
    else $cascaded[$r->name] = $r->data;
  }

  return isset($name) ? $cascaded[$name] : $cascaded;
}

/**
 * Set the cascaded (inherited by children) settings for an entry
 *
 * @param $mmtid
 *   Tree ID of the entry to set settings for
 * @param $settings
 *   Array containing the settings
 * @param $delete
 *   If TRUE, delete the old settings first
 */
function mm_content_set_cascaded_settings($mmtid, $settings, $delete = TRUE) {
  static $cascaded_settings;
  if (!isset($cascaded_settings)) $cascaded_settings = mm_content_get_cascaded_settings();

  if ($delete) db_query('DELETE FROM {mm_cascaded_settings} WHERE mmtid = %d', $mmtid);

  foreach ($cascaded_settings as $name => $desc)
    if (isset($settings[$name]))
      if ($desc['multiple']) {
        foreach ($settings[$name] as $array_key => $data)
          _mm_content_insert_cascaded_setting($mmtid, $name, $desc, $array_key, $data);
      }
      else
        _mm_content_insert_cascaded_setting($mmtid, $name, $desc, '', $settings[$name]);
}

/**
 * Scan a tree entry and its parents upward, looking for the closest change in a
 * cascaded setting
 *
 * @param $name
 *   Setting to look for
 * @param $mmtid
 *   Tree ID of the entry (and its parents) to query
 * @param $at
 *   Tree ID where the closest change occurs
 * @param $parent
 *   Tree ID of the nearest parent after $at containing a change in state
 * @param $new_entry
 *   Set to TRUE if $mmtid is that of the (future) parent of a new child
 * @return
 *   An array or single value (depending on the data type) containing the state
 *   of the given settings at the level $at
 */
function mm_content_resolve_cascaded_setting($name, $mmtid, &$at, &$parent, $new_entry = FALSE) {
  $q = mm_query("SELECT s.* FROM (SELECT %d AS mmtid, %d AS depth UNION SELECT parent, depth FROM {mm_tree_parents} WHERE mmtid = %d ORDER BY depth DESC) t INNER JOIN {mm_cascaded_settings} s ON s.mmtid = t.mmtid WHERE s.name = '%s'", $mmtid, 10000, $mmtid, $name);

  $out = array();
  $r = $q->next();
  while ($r) {
    $this_mmtid = $r->mmtid;
    if (is_array($out) && !$out) {
      if ($r->multiple) {
        do {
          if ($r->data_type == 'int') $r->data = (int)$r->data;
          if ($r->array_key != '') $out[$r->array_key] = $r->data;
          else $out[] = $r->data;
          $r = $q->next();
        } while ($r && $r->multiple && $r->mmtid == $this_mmtid);
      }
      else {
        if ($r->data_type == 'int') $r->data = (int)$r->data;
        $out = $r->data;
      }
      $at = $this_mmtid;
    }
    else if ($r->multiple) {
      do {
        $r = $q->next();
      } while ($r && $r->multiple && $r->mmtid == $this_mmtid);
    }
    else {
      $r = $q->next();
    }

    if ($new_entry || $this_mmtid != $mmtid) {
      $parent = $this_mmtid;
      return $out;
    }
  }

  $parent = 0;
  if (!$out) {
    $cascaded_settings = mm_content_get_cascaded_settings();
    if (!$cascaded_settings[$name]['multiple']) return NULL;
  }
  return $out;
}

/**
 * Get the parent tree ID of an entry
 *
 * @param $mmtid
 *   Tree ID of the entry whose parent we are looking for
 * @return
 *   Tree ID of the parent
 */
function mm_content_get_parent($mmtid) {
  $t = mm_content_get($mmtid);
  if ($t) return $t->parent;
  return NULL;
}

/**
 * Get all parent tree IDs of a entry
 *
 * @param $mmtid
 *   Tree ID of the entry whose parent we are looking for
 * @param $slow
 *   If TRUE, don't rely on the 'parents' field of the mm_tree table, instead
 *   slowly traverse up the tree
 * @param $virtual
 *   If TRUE, include the negative IDs that are added to children of the
 *   /.Users entry by mm_content_get_tree().
 * @return
 *   Array of parent tree IDs, listed highest-first
 */
function mm_content_get_parents($mmtid, $slow = FALSE, $virtual = TRUE) {
  global $_mmgp_cache, $_mmtbt_cache;

  $list = array();
  $mmtid0 = $mmtid;

  if ($mmtid < 0) {
    return array(1, mm_content_users_mmtid());
  }

  if (!$slow) {
    if ($mmtid == 1) return $list;
    while ($mmtid > 1 && isset($_mmgp_cache[$mmtid])) {
      array_unshift($list, $mmtid = $_mmgp_cache[$mmtid]);
    }

    if ($mmtid > 1) {
      $r = mm_content_get($mmtid, MM_GET_PARENTS);
      if (empty($r)) return $list;

      $list = array_merge($r->parents, $list);
      $prev = $mmtid0;
      foreach (array_reverse($list) as $m) {
        $_mmgp_cache[$prev] = $m;
        $prev = $m;
      }
    }
  }
  else {
    $last = -1;
    do {
      $mmtid = mm_content_get_parent($mmtid);
      if ($mmtid) array_unshift($list, $mmtid);

      if ($mmtid == $last) break;    // shouldn't happen, but just in case

      $last = $mmtid;
    }
    while ($mmtid > 1);
  }

  $virtual = $virtual && variable_get('mm_use_virtual_user_dir', TRUE);
  if ($virtual && count($list) >= 2 && $list[1] == mm_content_users_mmtid()) {
    $m = count($list) >= 3 ? $list[2] : $mmtid0;
    if (isset($_mmtbt_cache[$m])) $tree = $_mmtbt_cache[$m];
    else $tree = mm_content_get($m);

    if ($tree) {
      $letr = drupal_strtoupper($tree->name[0]);
      $alias = ctype_alpha($letr) ? $letr : '~';
      array_splice($list, 2, 0, -ord($alias));
    }
  }

  return $list;
}

/**
 * Get all parent tree IDs of a entry, plus the ID itself
 *
 * @param $mmtid
 *   Tree ID of the entry whose parent we are looking for
 * @param $slow
 *   If TRUE, don't rely on the 'parents' field of the mm_tree table, instead
 *   slowly traverse up the tree
 * @param $virtual
 *   If TRUE, include the negative IDs that are added to children of the
 *   /.Users entry by mm_content_get_tree().
 * @return
 *   Array of parent tree IDs, listed highest-first, with $mmtid at the end
 */
function mm_content_get_parents_with_self($mmtid, $slow = FALSE, $virtual = TRUE) {
  $list = mm_content_get_parents($mmtid, $slow, $virtual);
  $list[] = $mmtid;
  return $list;
}

/**
 * Get the full tree path of a tree ID
 *
 * @param $mmtid
 *   Tree ID of the page whose path we are looking for
 * @return
 *   Full path in the format 1/7/234/847
 */
function mm_content_get_full_path($mmtid) {
  return join('/', mm_content_get_parents_with_self($mmtid));
}

/**
 * Get a page's name
 *
 * @param $mmtid
 *   Tree ID of the page whose name is being requested
 * @return
 *   The expanded name
 */
function mm_content_get_name($mmtid) {
  if (($tree = mm_content_get($mmtid))) {
    return mm_content_expand_name($tree->name);
  }
  return '';
}

/**
 * Get a list of tree entries, using their tree IDs
 *
 * @param $options
 *   Either a single tree ID, an array of tree IDs, or an associative array
 *   containing key => value pairs of attributes to query. When using an
 *   associative array, the value can be an array of values. The allowed keys
 *   are all of the columns in the mm_tree table, plus:
 *   - query: a sub-query which returns a list of mmtids to query against
 *   - flags: an array of key => value pairs which are ANDed together; a NULL
 *            value becomes IS NULL in the query
 * @param $return
 *   A single value, or an array of values, from the list of constants below:
 *   - MM_GET_ARCHIVE: return archive status (mm_archive)
 *   - MM_GET_FLAGS:   return flags (mm_tree_flags)
 *   - MM_GET_PARENTS: return parents (mm_tree_parents)
 * @param $limit
 *   Optional maximum number of results to return (0)
 * @param $sort
 *   If TRUE, sort the results by their position in the tree (FALSE)
 * @return
 *   If $options['mmtids'] is a single tree ID, return the one tree object.
 *   Otherwise, return an array of tree objects (order is random).
 */
function mm_content_get($options, $return = array(), $limit = 0, $sort = FALSE) {
  global $_mmtbt_cache;

  $single = FALSE;
  if (!is_array($options)) {
    $single = TRUE;
    $options = array('mmtid' => array($options));
  }
  else if (is_numeric(mm_ui_mmlist_key0($options))) {
    $options = array('mmtid' => $options);
  }

  if (!is_array($return)) {
    $return = array($return);
  }
  $return = array_flip($return);

  $out = $args = $wheres = $joins = array();
  $add_field = $group_by = '';
  $cache_mmtid = 0;

  // Use a cache in the simple case where the only keys are mmtids
  $update_cache = FALSE;
  if (isset($options['mmtid']) && count(array_keys($options)) == 1) {
    $update_cache = !isset($return[MM_GET_ARCHIVE]);

    if (!is_array($options['mmtid'])) $options['mmtid'] = array($options['mmtid']);
    if (count($options['mmtid']) == 1) $cache_mmtid = $options['mmtid'][0];

    foreach ($options['mmtid'] as $key => $mmtid) {
      if (!isset($return[MM_GET_ARCHIVE]) && isset($_mmtbt_cache[$mmtid]) && (!isset($return[MM_GET_FLAGS]) || isset($_mmtbt_cache[$mmtid]->flags)) && (!isset($return[MM_GET_PARENTS]) || isset($_mmtbt_cache[$mmtid]->parents))) {
        if (!$limit || count($out) < $limit) {
          $out[] = clone $_mmtbt_cache[$mmtid];
        }
        unset($options['mmtid'][$key]);
      }
      else if ($mmtid < 0) {
        if (!$limit || count($out) < $limit) {
          $out[] = _mm_content_virtual_dir($mmtid, mm_content_users_mmtid(), 0, 0);
        }
        unset($options['mmtid'][$key]);
      }
      else if (!is_numeric($mmtid) || !$mmtid) {
        unset($options['mmtid'][$key]);
      }
    }
    // Reset array keys after unset()
    $options['mmtid'] = array_merge($options['mmtid']);
  }

  if (isset($options['query']) && !isset($options['mmtid'])) {
    $wheres[] = 't.mmtid IN (' . $options['query'] . ')';
    $single = FALSE;
    unset($options['query']);
  }

  if (isset($options['flags']) && is_array($options['flags'])) {
    $n = 0;
    foreach ($options['flags'] as $flag => $data) {
      $joins[] = "LEFT JOIN {mm_tree_flags} f$n ON f$n.mmtid = t.mmtid";
      $wheres[] = "f$n.flag = '%s'";
      $args[] = $flag;
      if (is_null($data)) $wheres[] = "f$n.data IS NULL";
      else {
        $wheres[] = is_numeric($v2) ? "f$n.data = %d" : "f$n.data = '%s'";
        $args[] = $data;
      }
      $n++;
    }
    $single = FALSE;
    unset($options['flags']);
  }

  $string_fields = array('name', 'alias', 'theme', 'hover');
  foreach ($options as $k => $v) {
    if (!is_array($v) || $v) {
      if (is_array($v) && count($v) == 1) $v = $v[0];

      $not_string = array_search($k, $string_fields) === FALSE;
      $k = strtolower($k);
      if (strchr($k, '.') === FALSE) $k = "t.$k";

      if (is_array($v)) {
        $vals = array();
        foreach ($v as $v2) {
          $vals[] = is_numeric($v2) && $not_string ? '%d' : "'%s'";
          $args[] = $v2;
        }
        $wheres[] = "$k IN(" . join(', ', $vals) . ')';
      }
      else {
        if (is_numeric($v) && $not_string)
          $wheres[] = "$k = %d";
        else
          $wheres[] = "$k = '%s'";
        $args[] = $v;
      }
    }
  }

  if ($limit) {
    // Consider cached data already copied to $out
    $limit -= count($out);
    if ($limit <= 0) {
      return $single ? $out[0] : $out;
    }
  }

  if (isset($return[MM_GET_ARCHIVE])) {
    $joins[] = 'LEFT JOIN {mm_archive} a ON a.main_mmtid = t.mmtid OR a.archive_mmtid = t.mmtid';
    $add_field .= ', a.*';
  }

  if (isset($return[MM_GET_FLAGS])) {
    $joins[] = 'LEFT JOIN {mm_tree_flags} f ON f.mmtid = t.mmtid';
    $add_field .= ", GROUP_CONCAT(DISTINCT CONCAT_WS('|1', f.flag, f.data) SEPARATOR '|2') AS flags";
    $group_by = ' GROUP BY t.mmtid';
  }

  if (isset($return[MM_GET_PARENTS])) {
    $joins[] = 'LEFT JOIN {mm_tree_parents} p ON p.mmtid = t.mmtid';
    $add_field .= ", GROUP_CONCAT(DISTINCT p.parent ORDER BY p.depth) AS parents";
    $group_by = ' GROUP BY t.mmtid';
  }

  if (!empty($wheres)) {
    $space = count($joins) ? ' ' : '';
    $query = "SELECT t.*$add_field FROM {mm_tree} t$space" . join(' ', $joins) . ' WHERE ' . join(' AND ', $wheres) . $group_by;
    if ($sort) {
      $query .= ' ORDER BY t.sort_idx';
    }

    if ($limit) {
      $q = mm_query_range($cache_mmtid, $query, $args, 0, $limit);
    }
    else {
      $q = mm_query($cache_mmtid, $query, $args);
    }

    while ($r = $q->next()) {
      if (isset($return[MM_GET_FLAGS])) $r->flags = _mm_content_split_flags($r->flags);
      if (isset($return[MM_GET_PARENTS]) && !is_array($r->parents)) $r->parents = empty($r->parents) ? array() : explode(',', $r->parents);
      $out[] = $r;
      if ($update_cache) $_mmtbt_cache[$r->mmtid] = $r;
    }
  }

  return $single ? $out[0] : $out;
}

/**
 * Update a tree entry's list of parent nodes, or update the lists for all
 * entries in the tree.
 *
 * @param $mmtid
 *   ID of the entry to update, or NULL to update all entries
 * @param $parents
 *   Array of parent IDs, or NULL to recalculate from the tree
 * @param $is_new
 *   Set to TRUE if the entry doesn't already have parents, to avoid an extra
 *   DELETE
 * @param $force
 *   When $mmtid is NULL, set this parameter to TRUE in order to force all
 *   entries to be updated, not just those that currently have no 'parents' info
 */
function mm_content_update_parents($mmtid = NULL, $parents = NULL, $is_new = FALSE, $force = FALSE) {
  if (is_null($mmtid)) {
    $result = db_query($force ?
      'SELECT mmtid FROM {mm_tree}' :
      "SELECT t.mmtid FROM {mm_tree} t LEFT JOIN {mm_tree_parents} p ON p.mmtid = t.mmtid WHERE t.parent > 0 AND p.parent IS NULL");

    while ($r = db_fetch_object($result))
      mm_content_update_parents($r->mmtid, NULL);

    return;
  }

  if (is_null($parents)) $parents = mm_content_get_parents($mmtid, TRUE, FALSE);

  if (!count($parents)) {
    db_query("DELETE FROM {mm_tree_parents} WHERE mmtid = %d", $mmtid);
    return;
  }

  // Minimize the time mm_tree_parents is inaccurate by inserting all parents at once
  $values = $placeholders = array();
  $dels = array($mmtid);
  foreach ($parents as $depth => $parent) {
    $values[] = $mmtid;
    $values[] = $parent;
    $values[] = $depth;
    $placeholders[] = '(%d, %d, %d)';
    $dels[] = $parent;
  }

  if (!$is_new) {
    db_query("UPDATE {mm_tree} SET parent = %d WHERE mmtid = %d", $parents[count($parents) - 1], $mmtid);
    db_query("DELETE FROM {mm_tree_parents} WHERE mmtid = %d AND parent NOT IN(" . db_placeholders($placeholders) . ')', $dels);
  }
  db_query('REPLACE INTO {mm_tree_parents} (mmtid, parent, depth) VALUES' . join(', ', $placeholders), $values);
}

/**
 * Return the URL for a tree entry
 *
 * @param $mmtid
 *   ID of the entry
 * @return
 *   The URL, starting with 'mm/'
 */
function mm_content_get_mmtid_url($mmtid) {
  return "mm/$mmtid";
}

/**
 * Return a list of tree IDs to which a given node is assigned
 *
 * @param $nid
 *   Node ID to query
 * @return
 *   Array of tree IDs
 */
function mm_content_get_by_nid($nid) {
  $mmtids = array();
  $q = mm_query('SELECT mmtid FROM {mm_node2tree} WHERE nid = %d', $nid);
  while ($r = $q->next())
    $mmtids[] = $r->mmtid;
  return $mmtids;
}

/**
 * Figure out if a user can see or delete a recycle bin
 *
 * @param $mmtid
 *   ID of the bin being queried
 * @param $mode
 *   If set, return whether or not the user can perform that action
 *   ('r' (see), 'w' (delete)). Otherwise, return an array containing these
 *   elements with either TRUE or FALSE values. There is also a special mode,
 *   'EMPTY', which returns TRUE if the user has permission to empty the entire
 *   bin (i.e.: has write on everything in it.)
 * @param $usr
 *   User object of the user to test, or NULL to test the global $user
 * @return
 *   See above
 */
function mm_content_user_can_recycle($mmtid, $mode = '', $usr = NULL) {
  global $_mmucr_cache, $user;

  if (!$usr) $usr = $user;
  $uid = $usr->uid;

  if (!isset($_mmucr_cache[$mmtid][$uid]) || $mode == 'EMPTY' && !isset($_mmucr_cache[$mmtid][$uid]['EMPTY'])) {
    if (!class_exists('mmucrGetTreeIterator')) {
      class mmucrGetTreeIterator extends getTreeIterator {
        public $readable, $writable, $emptyable, $mode, $usr;

        public function __construct($mode, $usr) {
          $this->readable = FALSE;
          $this->writable = FALSE;
          $this->emptyable = TRUE;
          $this->mode = $mode;
          $this->usr = $usr;
        }

        public function iterate($item) {
          // function must return 1 if no error, 0 if error, -1 if this node
          // and any of its children should be skipped
          if ($item->name == MM_ENTRY_NAME_RECYCLE) { // the bin exists
            $this->writable = TRUE;                // no nodes: default to writable
            if ($item->perms['ADMIN'] || $item->perms['w']) $this->readable = TRUE;
            foreach (mm_content_get_nids_by_mmtid($item->mmtid) as $n) {
              $this->writable = FALSE;             // it's not empty, so not writable
              $node = node_load($n);
              if ($node->nid)
                if (node_access('delete', $node, $this->usr)) {
                  $this->readable = TRUE;          // the bin is readable
                  if ($this->mode != 'EMPTY') return 0;  // skip everything else
                }
                else if ($this->mode == 'EMPTY') {
                  $this->emptyable = FALSE;
                  return 0;
                }
            }
            return 1;
          }
          $this->writable = FALSE;                 // it's not empty, so not writable

          if ($item->perms['w']) {                 // if the user can write to at least one kid
            $this->readable = TRUE;                // the bin is readable
            if ($this->mode != 'EMPTY') return 0;  // skip everything else
          }
          else if ($this->mode == 'EMPTY') {       // looking for emptyable status
            $this->emptyable = FALSE;              // can't write, so not emptyable
            return 0;
          }

          return -1;                               // skip this node and kids; we only care about sibs
        } // iterate
      }   // class
    }     // !class_exists

    $iter = new mmucrGetTreeIterator($mode, $usr->uid == $user->uid ? NULL : $usr);
    if (mm_content_user_can(mm_content_get_parent($mmtid), 'r', $usr)) {
      $params = array(
        MM_GET_TREE_FAKE_READ_BINS => TRUE,
        MM_GET_TREE_USER           => $usr,
        MM_GET_TREE_RETURN_PERMS   => TRUE,
        MM_GET_TREE_DEPTH          => 1,
        MM_GET_TREE_ITERATOR       => $iter,
      );
      mm_content_get_tree($mmtid, $params);
    }
    else if ($mode == 'EMPTY') $iter->emptyable = FALSE;

    if ($mode == 'EMPTY') $_mmucr_cache[$mmtid][$uid]['EMPTY'] = $iter->emptyable;
    else {
      $_mmucr_cache[$mmtid][$uid]['r'] = $iter->readable;
      $_mmucr_cache[$mmtid][$uid]['w'] = $iter->writable;
    }
  }       // !isset($_mmucr_cache[$mmtid][$uid])

  if ($mode != '') return $_mmucr_cache[$mmtid][$uid][$mode];

  return $_mmucr_cache[$mmtid][$uid];
}

/**
 * Figure out if a given user can access a particular tree ID
 *
 * @param $mmtid
 *   ID of the term being queried
 * @param $mode
 *   If set, return whether or not the user can perform that action ('r', 'w',
 *   'a', 'u', 'IS_USER', 'IS_GROUP', 'IS_RECYCLE_BIN', 'IS_RECYCLED').
 *   Otherwise, return an array containing each of these permissions with either
 *   TRUE or FALSE values.
 * @param $usr
 *   User object to test against. Defaults to the current user.
 * @param $bias_anon
 *   If TRUE, assume user 0 can't read any groups (faster, more secure)
 * @return
 *   See above
 */
function mm_content_user_can($mmtid, $mode = '', $usr = NULL, $bias_anon = TRUE) {
  global $_mmuc_cache, $user;
  if (!$usr) $usr = $user;
  $uid = $usr->uid;

  $mmtid = intval($mmtid);
  if (!empty($mode) ? !isset($_mmuc_cache[$mmtid][$uid][$mode]) : !isset($_mmuc_cache[$mmtid][$uid])) {
    // set default values, in case mmtid does not exist
    $_mmuc_cache[$mmtid][$uid] = array(
      'w'              => FALSE,
      'a'              => FALSE,
      'u'              => FALSE,
      'r'              => FALSE,
      'IS_USER'        => FALSE,
      'IS_GROUP'       => FALSE,
      'IS_RECYCLE_BIN' => FALSE,
      'IS_RECYCLED'    => FALSE,
    );
    if ($mmtid < 0) {
      // speedup for virtual user directory (A-Z)
      $_mmuc_cache[$mmtid][$uid]['r'] = $_mmuc_cache[$mmtid][$uid]['IS_USER'] = TRUE;
    }
    else if ($mmtid) {
      $params = array(
        MM_GET_TREE_BIAS_ANON      => $bias_anon,
        MM_GET_TREE_FAKE_READ_BINS => TRUE,
        MM_GET_TREE_MMTID          => $mmtid,
        MM_GET_TREE_RETURN_BINS    => TRUE,
        MM_GET_TREE_RETURN_PERMS   => TRUE,
        MM_GET_TREE_USER           => $usr,
      );
      $q = mm_query(mm_content_get_query($params));
      if ($row = $q->next()) {
        $bins = array();
        foreach ((array)$row as $key => $val) {
          if ($key == 'recycle_bins') {
            if (!empty($val)) $bins = split(',', $val);
          }
          else {
            $_mmuc_cache[$mmtid][$uid][$key] = $val != 0;
          }
        }

        // it's too expensive to do this in the query
        if ($_mmuc_cache[$mmtid][$uid]['IS_RECYCLE_BIN']) {
          $_mmuc_cache[$mmtid][$uid]['u'] = TRUE;
          $_mmuc_cache[$mmtid][$uid]['r'] = mm_content_user_can_recycle($mmtid, 'r', $usr);
        }
        else {
          // re-calculate the 'r' flag for anything in a bin
          foreach ($bins as $bin) {
            $_mmuc_cache[$mmtid][$uid]['r'] = $_mmuc_cache[$mmtid][$uid]['r'] && mm_content_user_can_recycle($bin, 'r', $usr);
          }
        }
      }
    }
  }

  if (!empty($mode)) return $_mmuc_cache[$mmtid][$uid][$mode];

  return $_mmuc_cache[$mmtid][$uid];
}

/**
 * Get a database query to return a part of the tree, or to determine whether or
 * not a user has permission to access a particular node or part of the tree
 *
 * @param $params
 *   An array containing parameters. The array is indexed using the constants
 *   below. Either [MM_GET_TREE_NODE] or [MM_GET_TREE_MMTID] must be specified.
 *   - MM_GET_TREE_ADD_SELECT (none):
 *     A string or array of strings to add to the SELECT portion of the query
 *   - MM_GET_TREE_BIAS_ANON (TRUE):
 *     If TRUE, assume user 0 can't read any groups (more secure)
 *   - MM_GET_TREE_DEPTH (0):
 *     When 'mmtid' is used, a query to return all items in the tree below that
 *     point can be returned. This field specifies the depth of recursion:
 *     - 0:  just the item specified by MM_GET_TREE_MMTID
 *     - -1: all levels
 *     - 1:  the item and its immediate children
 *     - N:  any other other number will return that many levels (can be slow)
 *   - MM_GET_TREE_FAKE_READ_BINS (FALSE):
 *     Pretend the user can read all recycle bins (used internally)
 *   - MM_GET_TREE_FILTER_BINS (TRUE):
 *     Get entries that are recycle bins
 *   - MM_GET_TREE_FILTER_DOTS (TRUE):
 *     Get all entries with names that start with '.'. If FALSE, only .Groups,
 *     .Users, and .Virtual are returned.
 *   - MM_GET_TREE_FILTER_GROUPS (TRUE):
 *     Get entries that are groups (MM_GET_TREE_MMTID mode)
 *   - MM_GET_TREE_FILTER_NORMAL (TRUE):
 *     Get entries that are groups (MM_GET_TREE_MMTID mode)
 *   - MM_GET_TREE_FILTER_USERS (TRUE):
 *     Get entries in /user (MM_GET_TREE_MMTID mode)
 *   - MM_GET_TREE_INNER_FILTER:
 *     Used internally
 *   - MM_GET_TREE_MMTID:
 *     Tree ID to query
 *   - MM_GET_TREE_NODE:
 *     Node object to query permissions for
 *   - MM_GET_TREE_RETURN_BINS (FALSE):
 *     A comma-separated list of the mmtids of any parent recycle bins
 *   - MM_GET_TREE_RETURN_BLOCK (FALSE):
 *     Attributes from the mm_tree_block table (MM_GET_TREE_MMTID mode)
 *   - MM_GET_TREE_RETURN_FLAGS (FALSE):
 *     Flags from the mm_tree_flags table (MM_GET_TREE_MMTID mode)
 *   - MM_GET_TREE_RETURN_KID_COUNT (FALSE):
 *     A count of the number of children each tree entry has (MM_GET_TREE_MMTID
 *     mode)
 *   - MM_GET_TREE_RETURN_MTIME (FALSE):
 *     The muid (user ID who made the last modification) and mtime (time) of the
 *     modification
 *   - MM_GET_TREE_RETURN_NODE_COUNT (FALSE):
 *     If TRUE, return a count of the number of nodes assigned to each item. If
 *     a string or array of strings, return a count of the number of nodes of
 *     that type.
 *   - MM_GET_TREE_RETURN_PERMS (none):
 *     If set, return whether or not the user can perform that action ('r', 'w',
 *     'a', 'u', 'IS_USER', 'IS_GROUP', 'IS_RECYCLE_BIN', 'IS_RECYCLED'). Only
 *     ('r', 'w', 'a', 'u') are supported when [MM_GET_TREE_NODE] is used. The
 *     requested permission can either be a single value or an array. If an
 *     empty array or TRUE is passed, all permissions are returned.
 *   - MM_GET_TREE_RETURN_TREE (FALSE):
 *     Attributes from the mm_tree table (MM_GET_TREE_MMTID mode)
 *   - MM_GET_TREE_SORT (FALSE):
 *     If TRUE, sort the entries according to sort_idx; always TRUE when
 *     MM_GET_TREE_DEPTH != 0
 *   - MM_GET_TREE_USER (current user):
 *     User object to test permissions against
 *   - MM_GET_TREE_WHERE (none):
 *     Add a WHERE clause to the outermost query
 *   If none of ([...USERS], [...GROUPS], [...NORMAL]) is TRUE, all types are
 *   retrieved by the query.
 * @return
 *   The query string
 */
function mm_content_get_query($params) {
  global $user;
  static $user_access;
  $defaults = array(
    MM_GET_TREE_BIAS_ANON      => TRUE,
    MM_GET_TREE_DEPTH          => 0,
    MM_GET_TREE_FAKE_READ_BINS => FALSE,
    MM_GET_TREE_FILTER_BINS    => TRUE,
    MM_GET_TREE_FILTER_DOTS    => TRUE,
    MM_GET_TREE_FILTER_GROUPS  => FALSE,
    MM_GET_TREE_FILTER_NORMAL  => FALSE,
    MM_GET_TREE_FILTER_USERS   => FALSE,
    MM_GET_TREE_USER           => $user,
  );

  $params = array_merge($defaults, $params);
  if (!$params[MM_GET_TREE_FILTER_GROUPS] && !$params[MM_GET_TREE_FILTER_USERS] && !$params[MM_GET_TREE_FILTER_NORMAL]) {
    $params[MM_GET_TREE_FILTER_GROUPS] = $params[MM_GET_TREE_FILTER_USERS] = $params[MM_GET_TREE_FILTER_NORMAL] = TRUE;
  }

  $is_node = isset($params[MM_GET_TREE_NODE]);

  if (isset($params[MM_GET_TREE_RETURN_PERMS]) && $params[MM_GET_TREE_RETURN_PERMS] === TRUE ||
      ($is_node ? !isset($params[MM_GET_TREE_RETURN_PERMS]) || !$params[MM_GET_TREE_RETURN_PERMS] :
      isset($params[MM_GET_TREE_RETURN_PERMS]) && empty($params[MM_GET_TREE_RETURN_PERMS]))) {
    $params[MM_GET_TREE_RETURN_PERMS] = array('w', 'a', 'u', 'r', 'IS_USER', 'IS_GROUP', 'IS_RECYCLE_BIN', 'IS_RECYCLED');
  }
  else if (!isset($params[MM_GET_TREE_RETURN_PERMS])) {
    $params[MM_GET_TREE_RETURN_PERMS] = array();
  }
  else if (!is_array($params[MM_GET_TREE_RETURN_PERMS])) {
    $params[MM_GET_TREE_RETURN_PERMS] = array($params[MM_GET_TREE_RETURN_PERMS]);
  }
  $perms = array_flip($params[MM_GET_TREE_RETURN_PERMS]);

  if (!$is_node && empty($params[MM_GET_TREE_MMTID])) {
    watchdog('mm', 'mm_content_get_query() called without a node ID or MM tree ID.',
        array(), WATCHDOG_ERROR);
    if (function_exists('debug') && variable_get('error_level', 0) == 1) debug();
    return;
  }

  // TODO: fix for recursive where mmtid<0
  if (!$is_node && $params[MM_GET_TREE_DEPTH] == 0 && $params[MM_GET_TREE_MMTID] < 0) {
    // virtual user directory (A-Z)
    return 'SELECT 0 AS w, 0 AS a, 0 AS u, 0 AS r, 1 AS IS_USER, 0 AS IS_GROUP, 0 AS IS_RECYCLE_BIN, 0 AS IS_RECYCLED';
  }

  $uid = $params[MM_GET_TREE_USER]->uid;

  $is_admin = $uid == 1;
  if (!$is_admin && !isset($user_access[$uid])) {
    foreach (array('administer all menus', 'administer all users', 'administer all groups', 'view all menus') as $access_mode)
      if ($uid || substr($access_mode, 0, 4) == 'view')
        $user_access[$uid][$access_mode] = user_access($access_mode, $params[MM_GET_TREE_USER]);
  }
  $is_admin |= $user_access[$uid]['administer all menus'];

  $outside_selects = array();
  if (isset($params[MM_GET_TREE_ADD_SELECT]))
    if (is_array($params[MM_GET_TREE_ADD_SELECT])) $outside_selects = $params[MM_GET_TREE_ADD_SELECT];
    else $outside_selects[] = $params[MM_GET_TREE_ADD_SELECT];

  if (isset($params[MM_GET_TREE_RETURN_NODE_COUNT]) && !empty($params[MM_GET_TREE_RETURN_NODE_COUNT])) {
    if (is_array($params[MM_GET_TREE_RETURN_NODE_COUNT])) $compare = " AND node.type IN ('" . join("', '", $params[MM_GET_TREE_RETURN_NODE_COUNT]) . ')';
    else if (is_string($params[MM_GET_TREE_RETURN_NODE_COUNT])) $compare = " AND node.type = '" . $params[MM_GET_TREE_RETURN_NODE_COUNT] . "'";
    else $compare = '';
    $outside_selects[] = "(SELECT COUNT(DISTINCT n.nid) FROM {mm_node2tree} n INNER JOIN {node} node ON node.nid = n.nid WHERE n.mmtid = t.mmtid$compare) AS nodecount";
  }

  $outside_group_by = $node_selects = array();
  $outside_where = $params[MM_GET_TREE_WHERE];
  $inside_joins = $outside_joins = $outside_order_by = $anon_group = '';
  $inside_selects = $inside_group_by = array('i.mmtid');
  $having = '';
  $filter_dots = "(SUBSTR(name, 1, 1) <> '.' OR name IN('" . MM_ENTRY_NAME_GROUPS . "', '" . MM_ENTRY_NAME_USERS . "', '" . MM_ENTRY_NAME_VIRTUAL_GROUP . "'))";

  $inside_selects[] = 'i.container';
  $inside_group_by[] = 'i.container';
  $outside_group_by[] = 'o.container';

  if (!$is_node && (!$params[MM_GET_TREE_FILTER_GROUPS] || !$params[MM_GET_TREE_FILTER_USERS] || !$params[MM_GET_TREE_FILTER_NORMAL] || !$params[MM_GET_TREE_FILTER_BINS] || !$params[MM_GET_TREE_FILTER_DOTS])) {
    $havings = array();
    if ($params[MM_GET_TREE_FILTER_GROUPS]) {
      $perms['IS_GROUP'] = 1;
      $havings[] = 'SUM(o.is_group) > 0';
    }

    if ($params[MM_GET_TREE_FILTER_NORMAL]) {
      $perms['IS_USER'] = $perms['IS_GROUP'] = 1;
      if ($params[MM_GET_TREE_FILTER_USERS] && !$params[MM_GET_TREE_FILTER_GROUPS]) {
        $havings[] = 'SUM(o.is_group) = 0';
      }
      else {
        $havings[] = 'SUM(o.is_user) = 0 AND SUM(o.is_group) = 0';
      }
    }
    else if ($params[MM_GET_TREE_FILTER_USERS]) {
      $perms['IS_USER'] = 1;
      $havings[] = 'SUM(o.is_user) > 0';
    }

    $having = join(' OR ', $havings);
    $condit = array();
    if (!$params[MM_GET_TREE_FILTER_DOTS]) {
      $condit[] = $filter_dots;
    }
    if (!$params[MM_GET_TREE_FILTER_BINS]) {
      $condit[] = 'SUM(o.is_recycled) = 0';
    }

    if (!$condit) {
      $having = ' HAVING ' . $having;
    }
    else if ($having) {
      $having = ' HAVING (' . $having . ') AND ' . join(' AND ', $condit);
    }
    else {
      $having = ' HAVING ' . join(' AND ', $condit);
    }

    if (!$outside_group_by) {
      $outside_group_by[] = 'o.mmtid';
    }
  }

  if ($perms) {
    if ($uid) {
      $inside_joins .=
        'LEFT JOIN {mm_tree_access} a ON a.mmtid = i.mmtid ' .
        'LEFT JOIN {mm_group} g ON a.gid = g.gid ' .
        'LEFT JOIN {mm_virtual_group} v ON g.vgid = v.vgid ';
    }
    else if ($params[MM_GET_TREE_BIAS_ANON]) {
      $anon_group = 'SUM(o.is_group) = 0 AND ';
    }

    if ($is_admin) {
      foreach (array('w', 'a', 'u', 'r') as $m) {
        if (isset($perms[$m])) {
          $outside_selects[$m] = "COUNT(*) > 0 AS $m";
        }
      }
      $outside_selects[] = "1 AS ADMIN";
    }
    else {
      $outside_admin = array();
      if ($user_access[$uid]['administer all groups']) {
        $outside_admin[] = 'SUM(o.is_group) > 0';
      }
      if ($user_access[$uid]['administer all users']) {
        $outside_admin[] = 'SUM(o.is_user) > 0';
      }
      $outside_admin = count($outside_admin) ? join(' OR ', $outside_admin) . ' OR ' : $anon_group;

      foreach (array('w', 'a', 'u') as $m) {
        if (isset($perms[$m])) {
          if ($uid) {
            $mode_cmp = $m == 'w' ? "= 'w'" : "IN ('w', '$m')";
            $not_anon = "a.mode $mode_cmp AND (v.uid = $uid OR g.vgid = 0 AND g.uid = $uid) OR i.uid = $uid OR ";
          }
          else $not_anon = '';  // ignore anon user when owner or in a group

          $outside_selects[$m] = "{$outside_admin}(SUM(o.container = o.mmtid AND o.can_$m) > 0" . ($user_access[$uid]['view all menus'] ? '' : ' AND COUNT(*) = SUM(o.can_r)') . ") AS $m";
          $mode_cmp = "LOCATE('w', i.default_mode) > 0";
          if ($m != 'w') $mode_cmp .= " OR LOCATE('$m', i.default_mode) > 0";
          $inside_selects[$m] = "SUM($not_anon$mode_cmp) > 0 AS can_$m";
          $node_selects[] = "SUM($m) > 0 AS $m";
        }
      }

      if (isset($perms['r'])) {
        if ($user_access[$uid]['view all menus']) {
          $outside_selects[] = "SUM(o.is_recycled) = 0 AND (SUM(o.is_group) = 0 AND COUNT(*) > 0 OR COUNT(*) = SUM(o.can_r)) AS r";
        }
        else if ($is_node) {
          $outside_selects[] = "{$outside_admin}COUNT(o.container = o.mmtid) = SUM(o.can_r) AS r";
        }
        else {
          $outside_selects[] = "{$outside_admin}IF(SUM(o.is_group), SUM(o.container = o.mmtid AND o.can_m) > 0, COUNT(o.container = o.mmtid) = SUM(o.can_r)) AS r";
        }
        $node_selects[] = "SUM(r) > 0 AS r";
      }

      $not_anon = $uid ? "(v.uid = $uid OR g.vgid = 0 AND g.uid = $uid) OR i.uid = $uid OR " : '';
      if ($params[MM_GET_TREE_FAKE_READ_BINS]) {
        $not_anon .= "i.name = '" . MM_ENTRY_NAME_RECYCLE . "' OR ";
      }
      $inside_selects[] = "SUM({$not_anon}i.default_mode <> '') > 0 AS can_r";

      if (!$is_node) {
        $not_anon = $uid ? "a.mode IN ('w', 'a', 'r') AND (v.uid = $uid OR g.vgid = 0 AND g.uid = $uid) OR i.uid = $uid OR " : '';
        $inside_selects[] = "SUM({$not_anon}LOCATE('w', i.default_mode) > 0 OR LOCATE('a', i.default_mode) > 0 OR LOCATE('r', i.default_mode) > 0) > 0 AS can_m";
      }
    }

    if (isset($perms['IS_USER']) || $user_access[$uid]['administer all users']) {
      $inside_selects[] = 'i.mmtid = ' . mm_content_users_mmtid() . ' AS is_user';
      if (isset($perms['IS_USER']) && !$is_node) {
        $outside_selects[] = 'SUM(o.is_user) > 0 AS IS_USER';
      }
    }

    if (isset($perms['IS_GROUP']) || !empty($anon_group) || $user_access[$uid]['administer all groups'] || $user_access[$uid]['view all menus']) {
      $inside_selects[] = 'i.mmtid = ' . mm_content_groups_mmtid() . ' AS is_group';
      if (isset($perms['IS_GROUP']) && !$is_node) {
        $outside_selects[] = empty($anon_group) ? 'SUM(o.is_group) > 0 AS IS_GROUP' : '0 AS IS_GROUP';
      }
    }

    if (isset($perms['IS_RECYCLE_BIN'])) {
      $inside_selects[] = "i.mmtid = i.container AND i.name = '" . MM_ENTRY_NAME_RECYCLE . "' AS is_recycle_bin";
      $outside_selects[] = "SUM(o.container = o.mmtid AND o.is_recycle_bin) > 0 AS IS_RECYCLE_BIN";
      $node_selects[] = 'SUM(IS_RECYCLE_BIN) AS IS_RECYCLE_BIN';
    }

    if (isset($perms['IS_RECYCLED']) || $params[MM_GET_TREE_RETURN_BINS] || $user_access[$uid]['view all menus'] || !$params[MM_GET_TREE_FILTER_BINS]) {
      $inside_selects[] = "SUM(i.name = '" . MM_ENTRY_NAME_RECYCLE . "') AS is_recycled";
      if (isset($perms['IS_RECYCLED']) || !$params[MM_GET_TREE_FILTER_BINS]) {
        $outside_selects[] = 'SUM(o.is_recycled) > 0 AS IS_RECYCLED';
        $node_selects[] = 'SUM(IS_RECYCLED) AS IS_RECYCLED';
      }

      if ($params[MM_GET_TREE_RETURN_BINS]) {
        $inside_selects[] = "IF(i.name = '" . MM_ENTRY_NAME_RECYCLE . "', i.mmtid, NULL) AS recycle_bins";
        $outside_selects[] = 'GROUP_CONCAT(o.recycle_bins) AS recycle_bins';
        $node_selects[] = 'GROUP_CONCAT(recycle_bins) AS recycle_bins';
      }
    }
  }   // if ($perms)

  if ($is_node) {
    $i =
      'SELECT t.mmtid, t.default_mode, t.uid, t.name, n2.mmtid AS container ' .
        'FROM {mm_tree_parents} p ' .
          'LEFT JOIN {mm_tree} t ON t.mmtid = p.parent ' .
          'INNER JOIN {mm_node2tree} n2 ON n2.mmtid = p.mmtid ' .
        'WHERE n2.nid = ' . $params[MM_GET_TREE_NODE]->nid . ' ' .
      'UNION SELECT t.mmtid, t.default_mode, t.uid, t.name, n2.mmtid AS container ' .
        'FROM {mm_tree} t '.
          'LEFT JOIN {mm_node2tree} n2 ON n2.mmtid = t.mmtid ' .
        'WHERE n2.nid = ' . $params[MM_GET_TREE_NODE]->nid;
  }
  else {
    if ($params[MM_GET_TREE_RETURN_TREE] || $params[MM_GET_TREE_DEPTH] || $params[MM_GET_TREE_RETURN_MTIME]) {
      if ($params[MM_GET_TREE_RETURN_TREE]) {
        $outside_selects[] = 't.*';
      }

      if ($params[MM_GET_TREE_DEPTH] && $params[MM_GET_TREE_SORT]) {
        $outside_order_by = ' ORDER BY sort_idx';
      }

      if ($perms || $params[MM_GET_TREE_DEPTH] || $params[MM_GET_TREE_RETURN_MTIME]) {
        $outside_joins .= " INNER JOIN {mm_tree} t ON t.mmtid = o.container";
      }
    }

    if ($params[MM_GET_TREE_RETURN_FLAGS]) {
      $outside_selects[] = "(SELECT GROUP_CONCAT(CONCAT_WS('|1', flag, data) SEPARATOR '|2') FROM {mm_tree_flags} WHERE mmtid = o.container) AS flags";
    }

    if ($params[MM_GET_TREE_RETURN_MTIME]) {
      $outside_selects[] = 'tr.muid, tr.mtime';
      $outside_joins .= ' LEFT JOIN {mm_tree_revisions} tr ON tr.vid = t.vid';
    }

    if ($params[MM_GET_TREE_RETURN_BLOCK]) {
      $outside_selects[] = 'b.bid, b.max_depth, b.max_parents';
      $outside_joins .= " LEFT JOIN {mm_tree_block} b ON b.mmtid = o.container";
    }

    if ($params[MM_GET_TREE_RETURN_KID_COUNT]) {
      $condit = array();
      if (!$params[MM_GET_TREE_FILTER_BINS]) {
        $condit[] = "name <> '" . MM_ENTRY_NAME_RECYCLE . "'";
      }
      if (!$params[MM_GET_TREE_FILTER_DOTS]) {
        $condit[] = $filter_dots;
      }

      if ($condit) {
        $outside_selects[] = "(SELECT COUNT(*) FROM {mm_tree} WHERE parent = o.container AND " . join(' AND ', $condit) . ") AS kids";
      }
      else {
        $outside_selects[] = "(SELECT COUNT(*) FROM {mm_tree} WHERE parent = o.container) AS kids";
      }
    }

    switch ($params[MM_GET_TREE_DEPTH]) {
      case -1:
        $i =
          // the item
          'SELECT mmtid AS container, mmtid, default_mode, uid, name ' .
            'FROM {mm_tree} ' .
            'WHERE mmtid = ' . $params[MM_GET_TREE_MMTID] . ' ' .
          // the item's children
          'UNION SELECT p.mmtid, t.mmtid, t.default_mode, t.uid, t.name ' .
            'FROM {mm_tree_parents} p ' .
              'INNER JOIN {mm_tree} t ON t.mmtid = p.mmtid ' .
            'WHERE p.parent = ' . $params[MM_GET_TREE_MMTID];
        if ($perms) {
          $i .= ' ' .
          // the item's parents
          'UNION SELECT p.mmtid, t.mmtid, t.default_mode, t.uid, t.name ' .
            'FROM {mm_tree_parents} p ' .
              'INNER JOIN {mm_tree} t ON t.mmtid = p.parent ' .
            'WHERE p.mmtid = ' . $params[MM_GET_TREE_MMTID] . ' ' .
          // its children's parents
          'UNION SELECT p0.mmtid, t.mmtid, t.default_mode, t.uid, t.name ' .
            'FROM {mm_tree_parents} p0 ' .
              'INNER JOIN {mm_tree_parents} p1 ON p1.mmtid = p0.mmtid ' .
              'INNER JOIN {mm_tree} t ON t.mmtid = p1.parent ' .
            'WHERE p0.parent = ' . $params[MM_GET_TREE_MMTID];
        }
        break;

      case 0:
        if (!$perms) {
          return
            'SELECT ' . join(', ', $outside_selects) . ' FROM {mm_tree} t' .
            $outside_joins .
            (empty($params[MM_GET_TREE_WHERE]) ? '' : ' WHERE ' . $params[MM_GET_TREE_WHERE]);
        }

        $i =
          // the item
          'SELECT mmtid AS container, mmtid, default_mode, uid, name ' .
            'FROM {mm_tree} '.
            'WHERE mmtid = ' . $params[MM_GET_TREE_MMTID] . ' ' .
          // its parents
          'UNION SELECT p.mmtid, t.mmtid, t.default_mode, t.uid, t.name ' .
            'FROM {mm_tree_parents} p ' .
              'INNER JOIN {mm_tree} t ON t.mmtid = p.parent ' .
            'WHERE p.mmtid = ' . $params[MM_GET_TREE_MMTID];
        break;

      case 1:
        $i =
          // the item
          'SELECT t.mmtid AS container, t.mmtid, t.default_mode, t.uid, t.name ' .
            'FROM {mm_tree} t ' .
            'WHERE t.mmtid = ' . $params[MM_GET_TREE_MMTID] . ' ' .
          // the item's immediate children
          'UNION SELECT t.mmtid AS container, t.mmtid, t.default_mode, t.uid, t.name ' .
            'FROM {mm_tree} t ' .
            'WHERE t.parent = ' . $params[MM_GET_TREE_MMTID] . $params[MM_GET_TREE_INNER_FILTER];
        if ($perms) {
          $i .= ' ' .
          // the item's parents
          'UNION SELECT p.mmtid, t.mmtid, t.default_mode, t.uid, t.name ' .
            'FROM {mm_tree_parents} p ' .
              'INNER JOIN {mm_tree} t ON t.mmtid = p.parent ' .
            'WHERE p.mmtid = ' . $params[MM_GET_TREE_MMTID] . ' ' .
          // its immediate children's parents
          'UNION SELECT t.mmtid, t2.mmtid, t2.default_mode, t2.uid, t2.name ' .
            'FROM {mm_tree} t ' .
            'INNER JOIN {mm_tree_parents} p ON t.mmtid = p.mmtid ' .
            'INNER JOIN {mm_tree} t2 ON t2.mmtid = p.parent ' .
            'WHERE t.parent = ' . $params[MM_GET_TREE_MMTID] . $params[MM_GET_TREE_INNER_FILTER];
        }
        break;

      default:
        $depth = $params[MM_GET_TREE_DEPTH] * MM_CONTENT_BTOA_CHARS;
        $i =
          'SELECT p.mmtid AS container, t.mmtid, t.default_mode, t.uid, t.name ' .
            'FROM (' .
              'SELECT p0.mmtid ' .
                'FROM {mm_tree_parents} p0 ' .
                'INNER JOIN {mm_tree} t ON t.mmtid = p0.mmtid ' .
              'WHERE p0.parent = ' . $params[MM_GET_TREE_MMTID] . ' ' .
                'AND LENGTH(t.sort_idx) - (' .
                  'SELECT LENGTH(sort_idx) ' .
                    'FROM {mm_tree} ' .
                    'WHERE mmtid = ' . $params[MM_GET_TREE_MMTID] .
                ") <= $depth " .
              'UNION SELECT ' . $params[MM_GET_TREE_MMTID] .
            ') AS x ' .
            'INNER JOIN {mm_tree_parents} p ON p.mmtid = x.mmtid ' .
            'INNER JOIN {mm_tree} t ON t.mmtid = p.parent OR t.mmtid = x.mmtid ' .
            'GROUP BY p.mmtid, t.mmtid ' .
            'ORDER BY NULL';
        break;
    }
  }

  // Note: Always use ORDER BY NULL instead of GROUP BY without ORDER BY, as
  // this is slightly faster.
  $query =
    'SELECT ' . join(', ', $outside_selects) . ' FROM (' .
      'SELECT ' . join(', ', $inside_selects) . ' ' .
      'FROM (' .
        $i .
      ') AS i ' .
      $inside_joins .
      'GROUP BY ' . join(', ', $inside_group_by) . ' ' .
      'ORDER BY NULL' .
    ') AS o' .
    $outside_joins .
    (empty($outside_where) ? '' : ' WHERE ' . $outside_where) .
    ($outside_group_by ? ' GROUP BY ' . join(', ', $outside_group_by) . $having : '') .
    ($outside_order_by ? $outside_order_by : ($outside_group_by ? ' ORDER BY NULL' : ''));

  if ($is_node && $node_selects && !$is_admin) {
    return 'SELECT ' . join(', ', $node_selects) . " FROM ($query) q";
  }

  return $query;
}

/**
 * Clear one element of the caches used by various functions, or completely
 * clear all caches.
 *
 * @param $mmtid
 *   If set, delete just the cached user_can data for this tree ID or array of
 *   tree IDs. Otherwise, clear the whole cache.
 */
function mm_content_clear_caches($mmtid = NULL) {
  global $_mmuc_cache, $_mmucr_cache, $_mmq_cache, $_mmtbt_cache, $_mmgp_cache, $_mmcucn_cache, $_mm_custom_url_rewrite_outbound_cache;

  if (isset($mmtid)) {
    if (is_array($mmtid)) {
      foreach ($mmtid as $t) {
        mm_content_clear_caches($t);
      }
    }
    else {
      unset($_mmuc_cache[$mmtid]);
      unset($_mmucr_cache[$mmtid]);
      unset($_mmtbt_cache[$mmtid]);
      unset($_mmgp_cache[$mmtid]);

      foreach (array_keys($_mmq_cache) as $c) {
        $t = $_mmq_cache[$c]->mmtid;
        if ($t <= 0 || $t == $mmtid) unset($_mmq_cache[$c]);
      }
    }
  }
  else {
    $_mmuc_cache = $_mmq_cache = $_mmtbt_cache = array();
  }
  // Always clear arrays not indexed by mmtid
  $_mmcucn_cache = $_mm_custom_url_rewrite_outbound_cache = array();
}

/**
 * Queue mm_tree entries that have changed, but only if the last database query
 * caused changes in the mm_tree table.
 *
 * @param $parent
 *   Conditionally queue this portion of the tree for future update; this should
 *   be the parent term ID of the entry or entries that may have changed. If the
 *   parent is not known, use NULL and specify a value in $child, instead.
 * @param $child
 *   Instead of using $parent, a $child can be specified. In this case, the
 *   parent is queried in an additional step, so this method should be avoided
 *   when possible.
 */
function mm_content_update_sort_queue_if_affected($parent, $child = NULL) {
  if (db_affected_rows()) {
    mm_content_update_sort_queue($parent, $child);
  }
}

/**
 * Queue mm_tree entries that have changed, or update the sort index for all
 * queued entries.
 *
 * If neither $parent nor $child is set, perform all queued updates. This
 * usually happens during monster_menus_exit().
 *
 * @param $parent
 *   If set, queue this portion of the tree for future update; this should be
 *   the parent term ID of the entry that has changed. If the parent is not
 *   known, use NULL and specify a value in $child, instead.
 * @param $child
 *   Instead of using $parent, a $child can be specified. In this case, the
 *   parent is queried in an additional step, so this method should be avoided
 *   when possible.
 * @param $all
 *   If TRUE, update all entries, not just the dirty ones; this is generally
 *   only done the very first time the sort index is generated.
 */
function mm_content_update_sort_queue($parent = NULL, $child = NULL, $all = FALSE) {
  static $queue = array();

  if (empty($parent) && !empty($child)) {
    $parent = mm_content_get_parent($child);
    if (empty($parent)) return;
  }

  if (!empty($parent)) {
    if (isset($queue[$parent])) $queue[$parent] |= $all;
    else $queue[$parent] = $all;
  }
  else if (count($queue)) {
    $uniq = array_unique($queue);
    if (count($uniq) == 1 && !$uniq[mm_ui_mmlist_key0($uniq)]) {
      mm_content_update_sort(array_keys($queue), FALSE);
    }
    else {
      foreach ($queue as $parent => $all)
        mm_content_update_sort($parent, $all);
    }
    $queue = array();
  }
}

/**
 * Update the mm_tree column containing the sort index.
 *
 * @param $mmtid
 *   Point from which to update entries downward in the tree. Can be an array
 *   of values when $all is FALSE.
 * @param $all
 *   If TRUE, update all entries, not just the dirty ones; this is generally
 *   only done the very first time the sort index is generated.
 */
function mm_content_update_sort($mmtid = 1, $all = TRUE) {
  if ($all) {
    if (!function_exists(__mm_content_update_sort_all)) {
      function __mm_content_update_sort_all($sort_idx, $mmtid) {
        $order = 0;
        $q = db_query("SELECT mmtid FROM {mm_tree} WHERE parent = %d ORDER BY IF(hidden, 1, IF(name = '%s', 2, 0)), weight, name", $mmtid, MM_ENTRY_NAME_RECYCLE);
        while ($r = db_fetch_object($q)) {
          $new_idx = $sort_idx . _mm_content_btoa($order++);
          _mm_content_test_sort_length($new_idx, $r->mmtid, TRUE);
          db_query("UPDATE {mm_tree} SET sort_idx = '%s', sort_idx_dirty = %d WHERE mmtid = %d", $new_idx, 0, $r->mmtid);
          __mm_content_update_sort_all($new_idx, $r->mmtid);
          if (!isset($_SERVER['SERVER_NAME'])) {
            // only from commandline
            print $r->mmtid . ' ' . $new_idx . "\n";
          }
        }
      }
    }

    if ($mmtid <= 1) {
      db_query("UPDATE {mm_tree} SET sort_idx = '%s', sort_idx_dirty = %d WHERE mmtid = %d", $sort_idx = '', 0, $mmtid = 1);
    }
    else {
      $sort_idx = db_result(db_query('SELECT sort_idx FROM {mm_tree} WHERE mmtid = %d', $mmtid));
    }
    __mm_content_update_sort_all($sort_idx, $mmtid);
  }
  else {
    for ($last = -1;;) {
      $in = is_array($mmtid) ? 'IN (' . db_placeholders($mmtid) . ')' : '= %d';
      $parent_q = db_query_range(
        'SELECT t.parent, t.sort_idx, t.mmtid ' .
          'FROM {mm_tree} t ' .
          'INNER JOIN {mm_tree_parents} p ON p.mmtid = t.mmtid ' .
        "WHERE t.sort_idx_dirty = 1 AND p.parent $in " .
        'GROUP BY NULL ' .
        'ORDER BY LENGTH(t.sort_idx) DESC',
        $mmtid, 0, 1
      );
      if ($parent = db_fetch_object($parent_q)) {
        // Prevent the outer for loop from running amok if there is a DB error
        if ($parent->mmtid == $last) break;
        $last = $parent->mmtid;
        $sort_idx = substr_replace($parent->sort_idx, '', -MM_CONTENT_BTOA_CHARS);
        $sort_len = strlen($sort_idx);
        $order = 0;
        $quick = array();
        $q = db_query("SELECT mmtid, sort_idx, sort_idx_dirty FROM {mm_tree} WHERE parent = %d ORDER BY IF(hidden, %d, IF(name = '%s', %d, %d)), weight, name", $parent->parent, 1, MM_ENTRY_NAME_RECYCLE, 2, 0);
        while ($r = db_fetch_object($q)) {
          $new_idx = _mm_content_btoa($order++);
          if ($new_idx != substr($r->sort_idx, -MM_CONTENT_BTOA_CHARS)) {
            db_query(
              'UPDATE {mm_tree} t ' .
              'INNER JOIN {mm_tree_parents} p ON p.mmtid = t.mmtid ' .
              'SET t.sort_idx = CONCAT(' .
                  "SUBSTRING(t.sort_idx, %d, %d), '%s', SUBSTRING(t.sort_idx, %d)" .
                "), t.sort_idx_dirty = %d " .
              'WHERE p.parent = %d OR t.mmtid = %d',
              1, $sort_len, $new_idx, $sort_len + MM_CONTENT_BTOA_CHARS + 1, 0, $r->mmtid, $r->mmtid
            );
          }
          else if ($r->sort_idx_dirty) {
            $quick[] = $r->mmtid;
          }
        }

        if ($quick) {
          $place = db_placeholders($quick);
          array_unshift($quick, 0);
          db_query("UPDATE {mm_tree} SET sort_idx_dirty = %d WHERE mmtid IN($place)", $quick);
        }
      }
      else break;
    }
  }
}

/**
 * Decode a 32-bit word represented by four ASCII characters, derived from the
 * number's base-64 equivalent. This is the inverse of _mm_content_btoa().
 *
 * @param $str
 *   The string containing the encoded integer
 * @return
 *   The resulting integer
 */
function _mm_content_atob($str) {
  $out = 0;
  while (!empty($str) || $str === '0') {
    $out = $out * MM_CONTENT_BTOA_BASE + ord($str[0]) - MM_CONTENT_BTOA_START;
    $str = substr($str, 1);
  }
  return $out;
}

/**
 * Encode a 32-bit word using four ASCII characters, derived from the number's
 * base-64 equivalent. This produces a sequence that is suitable for sorting in
 * SQL, without sacrificing too much space. A larger base cannot be used without
 * the possibility of causing case-insensitive sorting errors.
 *
 * @param $uword
 *   The unsigned word to encode
 * @return
 *   The word, encoded in a string
 */
function _mm_content_btoa($uword) {
  static $pows, $max;

  if (!isset($pows)) {
    $max = (int)pow(MM_CONTENT_BTOA_BASE, MM_CONTENT_BTOA_CHARS);
    $pows = array();
    for ($i = 1; $i < MM_CONTENT_BTOA_CHARS; $i++)
      array_unshift($pows, (int)pow(MM_CONTENT_BTOA_BASE, $i));
  }

  if ($uword < 0 || $uword >= $max) {
    die('Range error');
  }

  $out = '';
  foreach ($pows as $pow) {
    $out .= chr(MM_CONTENT_BTOA_START + $uword / $pow);
    // The % operator doesn't work correctly, here
    $uword = $uword - intval($uword / $pow) * $pow;
  }
  $out .= chr(MM_CONTENT_BTOA_START + $uword);
  return $out;
}

/**
 * Automatically empty all recycle bins of content that has been there for more
 * than a set time. Run as uid=1 during monster_menus_cron().
 */
function mm_content_empty_all_bins() {
  if (($empty = variable_get('mm_recycle_auto_empty', 0)) > 0) {
    $result = db_query('SELECT * FROM {mm_recycle} WHERE recycle_date < %d',
        time() - $empty);
    $bins = $nodes = array();
    while ($r = db_fetch_object($result)) {
      if ($r->type == 'node') {
        if (!$nodes[$r->id]) {
          watchdog('mm', "Automatically emptying nid=$r->id from recycle bin");
          node_delete($r->id);
          // It's remotely possible for a node to be left over in mm_recycle,
          // even though it has already been deleted. In this case MM's deletion
          // code isn't called during node_delete(), so call it (again) now.
          $node = (object)array('nid' => $r->id);
          monster_menus_nodeapi($node, 'delete', NULL, NULL);
          $nodes[$r->id] = TRUE;
        }
      }
      else if ($r->type == 'cat') {
        watchdog('mm', "Automatically deleting mmtid=$r->id from recycle bin");
        mm_content_delete($r->id, TRUE);
      }
      $bins[] = $r->bin_mmtid;
    }

    foreach ($bins as $bin)
      mm_content_delete_bin($bin);
  }
}

/**
 * Figure out if a given user can access a particular node. When a node belongs
 * to more than one entry, logically OR the permissions for all entries.
 *
 * @param $node
 *   The node object or node number being queried
 * @param $mode
 *   If set, return whether or not the user can perform that action
 *   ('r', 'w', 'a', 'u'). Otherwise, return an array containing each of these
 *   elements with either TRUE or FALSE values.
 * @param $account
 *   User object of user to test against. Defaults to the current user.
 * @return
 *   See above
 */
function mm_content_user_can_node($node, $mode = '', $account = NULL) {
  global $_mmcucn_cache, $user;
  static $recursive = FALSE;

  $perms = array(
    'w' => FALSE,
    'a' => FALSE,
    'u' => FALSE,
    'r' => FALSE
  );

  // There's a chance that the calls to mm_content_node_access() below can lead
  // to a recursive call to this function.
  if ($recursive) {
    if (empty($mode)) return $perms;
    return FALSE;
  }
  $recursive = TRUE;

  if (is_object($node)) $nid = $node->nid;
  else {
    $nid = $node;
    $node = (object)array('nid' => $nid);
  }

  $uid = isset($account) ? $account->uid : $user->uid;
  if (!isset($_mmcucn_cache[$nid][$uid])) {
    $params = array(
      MM_GET_TREE_FAKE_READ_BINS => TRUE,
      MM_GET_TREE_NODE           => $node,
      MM_GET_TREE_RETURN_BINS    => TRUE,
      MM_GET_TREE_RETURN_PERMS   => array('w', 'a', 'u', 'r', 'IS_RECYCLE_BIN'),
      MM_GET_TREE_USER           => isset($account) ? $account : $user,
    );
    $q = mm_query(mm_content_get_query($params));
    if ($row = $q->next()) {
      foreach ((array)$row as $key => $val) {
        if (strlen($key) == 1) {
          $perms[$key] = $val != 0;
        }
      }

      if ($perms['w']) {
        $perms['w'] = mm_content_node_access('update', $node, $account);
      }

      if (isset($row->recycle_bins)) {
        $bins = split(',', $row->recycle_bins);
        if ($row->IS_RECYCLE_BIN) {
          $perms['u'] = TRUE;
          $perms['r'] = mm_content_user_can_recycle(array_pop($bins), 'r', $account);
        }
        else if (isset($row->recycle_bins)) {
          foreach (split(',', $row->recycle_bins) as $bin) {
            $perms['r'] = $perms['r'] && mm_content_user_can_recycle($bin, 'r', $account);
          }
        }
      }
    }
    else {
      // not assigned to any pages
      $perms['r'] = $perms['u'] = TRUE;
    }

    // If the user does not have write permission, consider the appearance schedule
    if (!$perms['w'] && !mm_content_node_access('update', $node, $account)) {
      $scheduled = (!isset($node->publish_on) || $node->publish_on == 0 || $node->publish_on <= time()) && (!isset($node->unpublish_on) || $node->unpublish_on == 0 || time() < $node->unpublish_on);
      $perms['r'] &= $scheduled;
    }
    $_mmcucn_cache[$nid][$uid] = $perms;
  }

  $recursive = FALSE;
  if (empty($mode)) return $_mmcucn_cache[$nid][$uid];
  return $_mmcucn_cache[$nid][$uid][$mode];
}

/**
 * Delete an entry and all of its children
 *
 * @param $mmtid
 *   ID of the entry to delete
 * @param $nodes
 *   If TRUE, also delete any nodes using these IDs (FALSE)
 * @param $allow_non_empty_bin
 *   If TRUE, allow a non-empty recycle bin at the top level to be deleted (FALSE)
 * @return
 *   An error message, if an error occurs
 */
function mm_content_delete($mmtid, $nodes = TRUE, $allow_non_empty_bin = FALSE) {
  $tree = mm_content_get_tree($mmtid, array(MM_GET_TREE_RETURN_PERMS => TRUE));
  if (!count($tree)) {
    return t('Page not found');
  }

  if (!$allow_non_empty_bin && $tree[0]->name == MM_ENTRY_NAME_RECYCLE && count($tree) > 1) {
    return t('This is a recycle bin that is not empty.');
  }

  $mmtids = $nids = $bins = array();
  foreach ($tree as $t) {
    if (!$t->perms['w'] && $t->name != MM_ENTRY_NAME_RECYCLE) {
      $msg = $t->mmtid == $mmtid ?
        'You do not have permission to delete the page %name' :
        'You cannot delete this page because you do not have permission to delete the sub-page %name';
      return t($msg, array('%name' => $t->name));
    }
    $mmtids[] = $t->mmtid;
    if ($t->name == MM_ENTRY_NAME_RECYCLE) {
      $bins[] = $t->mmtid;
    }
  }

  $vgroup = mm_content_is_vgroup($mmtid);

  if ($nodes)
    foreach (mm_content_get_nids_by_mmtid($mmtids, 0, TRUE) as $n)
      if (($node = node_load($n)) && node_access('delete', $node)) {
        node_delete($n);
        $nids[] = $n;
      }

  if ($bins) {
    $in = db_placeholders($bins);
    db_query("DELETE FROM {mm_tree_revisions} WHERE mmtid IN($in)", $bins);
  }

  $in = db_placeholders($mmtids);
  $mmtids2 = array_merge($mmtids, $mmtids);
  db_query("DELETE FROM {mm_tree} WHERE mmtid IN($in)", $mmtids);
  db_query("DELETE FROM {mm_tree_flags} WHERE mmtid IN($in)", $mmtids);
  db_query("DELETE FROM {mm_tree_parents} WHERE mmtid IN($in) OR parent IN($in)", $mmtids2);
  db_query("DELETE FROM {mm_node2tree} WHERE mmtid IN($in)", $mmtids);
  db_query("DELETE FROM {mm_tree_block} WHERE mmtid IN($in)", $mmtids);
  db_query("DELETE FROM {mm_node_reorder} WHERE mmtid IN($in)", $mmtids);
  db_query("DELETE FROM {mm_archive} WHERE main_mmtid IN($in) OR archive_mmtid IN($in)", $mmtids2);
  db_query("DELETE FROM {mm_recycle} WHERE type = 'cat' AND id IN($in)", $mmtids);
  db_query("UPDATE {mm_recycle} SET from_mmtid = 0 WHERE type = 'node' AND from_mmtid IN($in)", $mmtids);

  // remove ad-hoc groups (gid<0) first
  db_query('DELETE a, g FROM {mm_tree_access} a, {mm_group} g '.
      "WHERE a.gid = g.gid AND a.mmtid IN($in) AND a.gid < 0", $mmtids);

  // remove virtual groups; mm_virtual_group is cleaned up automatically in cron
  db_query('DELETE v, g FROM {mm_vgroup_query} v, {mm_group} g '.
      "WHERE v.vgid = g.vgid AND g.gid IN($in)", $mmtids);

  // remove remaining groups
  db_query("DELETE FROM {mm_tree_access} WHERE mmtid IN($in) OR gid IN($in)", $mmtids2);
  db_query("DELETE FROM {mm_group} WHERE gid IN($in)", $mmtids);
  db_query("DELETE FROM {mm_node_write} WHERE gid IN($in)", $mmtids);
  db_query("DELETE FROM {mm_node_redir} WHERE mmtid IN($in)", $mmtids);
  db_query("DELETE FROM {mm_cascaded_settings} WHERE mmtid IN($in)", $mmtids);
  db_query("DELETE FROM {mm_role2group} WHERE gid IN($in)", $mmtids);

  if ($vgroup)
    foreach ($mmtids as $mmtid)
      mm_content_update_vgroup_view($mmtid);

  // check for mm_delete hooks
  module_invoke_all('mm_delete', $mmtids, $nids);

  mm_content_clear_caches($mmtids);            // clear caches for this entry and it children
  mm_content_clear_caches($tree[0]->parent);   // clear caches for parent

  watchdog('mm', 'Deleted %name (%alias) mmtids = %mmtids', array(
    '%name' => $tree[0]->name,
    '%alias' => $tree[0]->alias,
    '%mmtids' => join(',', $mmtids))
  );
}

/**
 * Set breadcrumbs based on the current MM path
 *
 * @param $mmtids
 *   Optional list of tree IDs. If not set, the list is retrieved based on the
 *   current URL.
 * @return
 *   The new page title
 */
function mm_content_fix_breadcrumbs($mmtids = NULL) {
  global $_mm_content_saved_breadcrumb;

  if (!isset($mmtids)) {
    mm_parse_args($mmtids, $oarg_list);
    if (count($mmtids) == 0) return;
  }
  else {
    mm_parse_args($dummy, $oarg_list);
  }

  if ($mmtids[0] == mm_home_mmtid()) array_shift($mmtids);

  $base = mm_content_get(mm_home_mmtid());
  $bread[] = l(mm_content_expand_name($base->name), '<front>');
  $path[] = mm_home_path();
  foreach ($mmtids as $mmtid) {
    if (!($tree = mm_content_get($mmtid, MM_GET_FLAGS))) {
      break;
    }

    if ($mmtid == $mmtids[count($mmtids) - 1] || !isset($tree->flags['no_breadcrumb'])) {
      $path[] = $mmtid;
      $bread[] = l($title = mm_content_expand_name($tree->name), implode('/', $path));
    }

    if (!mm_content_user_can($mmtid, 'r')) break;
  }

  if (count($oarg_list) != 2 || $oarg_list[0] != 'node') {
    if (isset($title)) {
      array_pop($bread);
      drupal_set_title($title);
    }
    else if (count($bread) == 1) {   // homepage
      drupal_set_title($title = '');
    }
  }

  drupal_set_breadcrumb($bread);
  $_mm_content_saved_breadcrumb = array($bread, $title);
  return $title;
}

/**
 * Add a new page title to the end of the already-set breadcrumb
 *
 * @param $crumb
 *   New page title to add
 */
function mm_content_add_breadcrumb($crumb) {
  mm_parse_args($mmtids, $oarg_list, $this_mmtid);

  $bread = drupal_get_breadcrumb();
  if ($mmtids) {
    $bread[] = l(drupal_get_title(), "mm/$this_mmtid");
    drupal_set_breadcrumb($bread);
  }
  drupal_set_title($crumb);

  global $_mm_content_saved_breadcrumb;
  $_mm_content_saved_breadcrumb = array($bread, $crumb);
}

/**
 * Restore (part of) the previously-set breadcrumb, usually because another part
 * of Drupal has changed it
 *
 * @param $bread
 *   (optional) Array of URLs for upper levels of the breadcrumb
 * @param $crumb
 *   (optional) New page title
 */
function mm_content_restore_breadcrumb($bread = NULL, $crumb = NULL) {
  global $_mm_content_saved_breadcrumb;

  if (!isset($bread)) $bread = $_mm_content_saved_breadcrumb[0];
  if (!isset($crumb)) $crumb = $_mm_content_saved_breadcrumb[1];
  if (isset($bread) && isset($crumb)) {
    drupal_set_breadcrumb($bread);
    drupal_set_title($crumb);
    $_mm_content_saved_breadcrumb = array($bread, $crumb);
  }
}

/**
 * Return the long username ('last, first middle.' or 'first middle last')
 * associated with a uid
 *
 * @param $uid
 *   ID of the user to query
 * @param $order
 *   Either 'lfm', 'fml', 'lfmu', or 'fmlu' to choose the order. Defaults to
 *   'lfmu' [last, first, middle, (username)].
 * @param $usr
 *   Optional object, from which the name, pref_lfm, pref_fml, firstname,
 *   middlename, and lastname fields are used to construct (and cache) the
 *   output
 * @param $hover
 *   Optionally return the mouse 'hover' text associated with this user
 * @return
 *   The user's long name or FALSE if not found
 */
function mm_content_uid2name($uid, $order = 'lfmu', $usr = NULL, &$hover = NULL) {
  static $cache;

  if (!isset($cache[$uid])) {
    if ($uid == 0) {
      $cache[$uid] = (object) array(
        'name' => variable_get('mm_username_anon', t('Anonymous user'))
      );
    }
    else if ($uid == 1) {
      $cache[$uid] = (object) array(
        'name' => variable_get('mm_username_admin', t('Administrator'))
      );
    }
    else {
      if (!$usr) $usr = user_load(array('uid' => $uid));

      if (!is_object($usr)) {
        $cache[$uid] = FALSE;
      }
      else {
        $disabled = isset($usr->status) && $usr->status == 0;

        mm_module_invoke_all_array('mm_uid2name_alter', array(&$usr, &$disabled));

        if ($disabled && !user_access('administer all users')) {
          $cache[$uid] = (object) array(
            'name' => variable_get('mm_username_disabled', t('Inactive user'))
          );
        }
        else {
          $middle = '';
          if (drupal_strlen($usr->middlename)) $middle = ' ' . $usr->middlename[0] . '.';
          $cache[$uid] = (object) array(
            'pref_fml' => $usr->pref_fml,
            'pref_lfm' => $usr->pref_lfm,
            'last'     => $usr->lastname,
            'first'    => $usr->firstname,
            'name'     => $usr->name,
            'middle'   => $middle,
            'hover'    => $usr->hover,
            'disabled' => $disabled ? ' ' . t('(inactive)') : '',
          );
        }
      }
    }
  }

  if (($u = $cache[$uid]) !== FALSE) {
    $hover = $u->hover;

    $uname_only = FALSE;
    $fml = drupal_substr($order, 0, 3) == 'fml';
    if ($fml ? !empty($u->pref_fml) : !empty($u->pref_lfm)) {
      $out = $fml ? $u->pref_fml : $u->pref_lfm;
    }
    else if ($u->last && $u->first) {
      $out = $fml ? "$u->first$u->middle $u->last" : "$u->last, $u->first$u->middle";
    }
    else if ($u->last) {
      $out = $u->last;
    }
    else if ($u->first) {
      $out = $u->first;
    }
    else {
      $out = $u->name;
      $uname_only = TRUE;
    }

    if ($order[3] == 'u' && $u->name != '' && !$uname_only) {
      $out .= " ($u->name)";
    }
    $out .= $u->disabled;

    return $out;
  }

  return FALSE;
}

/**
 * Return the uid associated with a username
 *
 * @param $username
 *   ID of the user to query
 * @return
 *   The user's ID or FALSE if not found
 */
function mm_content_name2uid($username) {
  static $mmc_u2uid_cache;

  if (!isset($mmc_uid2u_cache[$username])) {
    $usr = user_load(array('name' => $username));
    $mmc_u2uid_cache[$username] = is_object($usr) ? $usr->uid : FALSE;
  }
  return $mmc_u2uid_cache[$username];
}

/**
 * Determine if a tree entry is a group
 *
 * @param $mmtid
 *   The numeric ID of the potential group
 * @return
 *   TRUE if the group ID is that of an existing (possibly virtual) group, and
 *   not a different type of tree entry
 */
function mm_content_is_group($mmtid) {
  static $mmc_isgrp_cache;

  if (!isset($mmc_isgrp_cache[$mmtid])) {
    $list = mm_content_get_parents_with_self($mmtid, FALSE, TRUE);
    $mmc_isgrp_cache[$mmtid] = isset($list[1]) && $list[1] == mm_content_groups_mmtid();
  }
  return $mmc_isgrp_cache[$mmtid];
}

/**
 * Determine if a tree entry is a virtual group
 *
 * @param $mmtid
 *   The numeric ID of the group
 * @return
 *   TRUE if the group ID is that of an existing virtual group, and not a
 *   different type of tree entry
 */
function mm_content_is_vgroup($mmtid) {
  $list = mm_content_get_parents_with_self($mmtid);
  return count($list) >= 2 && ($tree = mm_content_get($list[2])) && $tree->name == MM_ENTRY_NAME_VIRTUAL_GROUP;
}

/**
 * Determine if a tree ID belongs to a normal entry, as opposed to a group
 *
 * @param $mmtid
 *   The tree ID to test
 * @param $user_is_normal
 *   If TRUE, consider anything in .Users to be a normal entry
 * @return
 *   TRUE if the ID refers to a normal entry
 */
function mm_content_is_normal($mmtid, $user_is_normal = TRUE) {
  static $cache;

  if (!isset($cache[$mmtid][$user_is_normal])) {
    $list = mm_content_get_parents_with_self($mmtid);
    if (count($list) == 1) {   // root
      $cache[$mmtid][$user_is_normal] = FALSE;
    }
    else {
      $cache[$mmtid][$user_is_normal] = $list[1] != mm_content_groups_mmtid() &&
          ($user_is_normal || $list[1] != mm_content_users_mmtid());
    }
  }
  return $cache[$mmtid][$user_is_normal];
}

/**
 * Determine if a tree ID refers to the main page or the archive page of an
 * archive
 *
 * @param $mmtid
 *   The tree ID to test
 * @param $test_main
 *   If TRUE, see if the ID refers to the main page, otherwise the archive page
 * @return
 *   TRUE if the ID refers to an archive page of the requested type
 */
function mm_content_is_archive($mmtid, $test_main = FALSE) {
  $tree = mm_content_get($mmtid, MM_GET_ARCHIVE);
  return is_object($tree) && $mmtid == ($test_main ? $tree->main_mmtid : $tree->archive_mmtid);
}

/**
 * Determine if a tree entry is a child of another entry; useful in preventing
 * bad moves.
 *
 * @param $child
 *   The tree ID of the child entry
 * @param $of
 *   The tree ID of the entry to test for a relationship
 * @return
 *   TRUE if $child is a child of $of
 */
function mm_content_is_child($child, $of) {
  return array_search($of, mm_content_get_parents($child));
}

/**
 * Determine if a tree entry is in a recycle bin.
 *
 * @param $mmtid
 *   The tree ID of the entry to test
 * @return
 *   TRUE if $mmtid is in a bin
 */
function mm_content_is_recycled($mmtid) {
  return mm_content_user_can($mmtid, 'IS_RECYCLED');
}

/**
 * Determine if a tree entry is a recycle bin.
 *
 * @param $mmtid
 *   The tree ID of the entry to test
 * @return
 *   TRUE if $mmtid is a recycle bin
 */
function mm_content_is_recycle_bin($mmtid) {
  return mm_content_user_can($mmtid, 'IS_RECYCLE_BIN');
}

/**
 * Get a list of all blocks
 *
 * @param $allowed
 *   If set, only return the blocks the current user can apply to a page
 * @return
 *   An array indexed on block ID, containing these elements:
 *   - info: the 'name' field
 *   - title: the 'name' field
 *   - title_is_cat: set to 1 if the entry's top node should be displayed as
 *                   the block title
 */
function mm_content_get_blocks($allowed = FALSE) {
  static $cache;

  if (!is_array($cache) && ($result = db_query('SELECT * FROM {mm_block}'))) {
    while ($r = db_fetch_object($result)) {
      $cache[$r->bid] = array(
        'info' => $r->name,
        'title' => $r->title,
        'title_is_cat' => $r->title_is_cat,
        'show_node_contents' => $r->show_node_contents,
        'help' => $r->help,
        'allow_rss' => $r->allow_rss,
        'admin_only' => $r->admin_only,
        'cache' => BLOCK_NO_CACHE
      );
    }
  }

  if (!$allowed || user_access('administer all menus')) {
    return $cache;
  }

  return array_filter($cache, create_function('$elem', 'return !$elem["admin_only"];'));
}

/**
 * Search up the path, looking for the bottom-most block with an entry in
 * mm_tree_block
 *
 * @param $mmtids
 *   List of tree IDs comprising the path to search
 * @param $block_id
 *   - On entry:  Set to the block ID of the block to match, or leave as 0 to
 *                search all blocks
 *   - On return: If 0 on entry, this variable is set to the ID of the block
 *                that was found
 * @param $multiple
 *   If TRUE, return all blocks in the list
 * @return
 *   Tree ID of the starting point, all blocks in the list (if $multiple), or 0
 *   on error
 */
function mm_content_get_blocks_at_mmtid($mmtids, &$block_id = 0, $multiple = FALSE) {
  if (!count($mmtids)) return 0;

  // Remove virtual directory
  foreach ($mmtids as $i => $mmtid)
    if ($mmtid < 0) {
      array_splice($mmtids, $i, 1);
      break;
    }

  $list = implode(',', $mmtids);
  $q =
    'SELECT t.mmtid, t.parent, b.bid, b.show_node_contents, tb.max_depth, tb.max_parents ' .
    'FROM {mm_tree_block} tb ' .
      'INNER JOIN {mm_tree} t ON tb.mmtid = t.mmtid ' .
      'INNER JOIN {mm_block} b ON b.bid = tb.bid ' .
      "WHERE (t.mmtid IN($list) OR t.parent IN($list) AND b.show_node_contents)";

  $direct = $by_parent = array();
  $result = $block_id > 0 ?
    db_query("$q AND b.bid = %d", $block_id) :
    db_query($q);
  while ($r = db_fetch_object($result)) {
    $rec = array(
      'mmtid' => $r->mmtid,
      'bid' => $r->bid,
      'max_depth' => $r->max_depth,
      'max_parents' => $r->max_parents
    );

    if (array_search($r->mmtid, $mmtids) === FALSE) {   // starting point of a sub-menu
      if ($r->show_node_contents) $by_parent[$r->parent][] = $rec;
    }
    else
      $direct[$r->mmtid] = $rec;
  }

  if (count($direct) || count($by_parent))
    foreach (array_reverse($mmtids) as $t)
      if ($direct[$t]) {
        if (!$block_id) $block_id = $direct[$t];
        if ($multiple) return array($direct[$t]);
        return $t;
      }
      else if (is_array($by_parent[$t])) {
        if (!$block_id) $block_id = $by_parent[$t][0]['bid'];
        if ($multiple) return $by_parent[$t];
        return $by_parent[$t][0]['mmtid'];
      }

  return 0;
}

/**
 * Return the tree ID of the /root/.Groups entry
 *
 * @return
 *   The tree ID
 */
function mm_content_groups_mmtid() {
  if (!($mmtid = variable_get('mm_content_groups_mmtid', 0))) {
    $tree = mm_content_get(array('parent' => 1, 'name' => MM_ENTRY_NAME_GROUPS));
    variable_set('mm_content_groups_mmtid', $mmtid = $tree[0]->mmtid);
  }

  return $mmtid;
}

/**
 * Return the tree ID of the /root/.Users entry
 *
 * @return
 *   The tree ID
 */
function mm_content_users_mmtid() {
  if (!($mmtid = variable_get('mm_content_users_mmtid', 0))) {
    $tree = mm_content_get(array('parent' => 1, 'name' => MM_ENTRY_NAME_USERS));
    variable_set('mm_content_users_mmtid', $mmtid = $tree[0]->mmtid);
  }

  return $mmtid;
}

/**
 * Return the alias of the /root/.Users entry
 *
 * @return
 *   The alias
 */
function mm_content_users_alias() {
  if (is_null($alias = variable_get('mm_content_users_alias', NULL))) {
    $tree = mm_content_get(array('parent' => 1, 'name' => MM_ENTRY_NAME_USERS));
    variable_set('mm_content_users_alias', $alias = $tree[0]->alias);
  }

  return $alias;
}

/**
 * Return a list of user IDs in an MM group, query whether or not a given uid
 * is in one or more groups, or return all groups to which a uid belongs. This
 * function differs from mm_content_get_uids_in_group(), in that permissions
 * are not considered and the user's name is not returned.
 *
 * @param $mmtids
 *   A single tree ID (gid) or an array of gids (optional)
 * @param $uids
 *   A single user ID, or an array of user IDs to query (optional)
 * @return
 *   If both $mmtids and $uids are set:
 *   - If $uids is a single value, return an array containing the groups
 *     matching $mmtids to which the user belongs
 *   - If $uids is an array, return an array where the key is the uid and the
 *     value is an array containing the groups matching $mmtids to which the
 *     user belongs
 *   If only $mmtids is set:
 *   - If $mmtids is a single value, return an array containing all uids that
 *     are members of the group
 *   - If $mmtids is an array, return an array where the key is the mmtid and
 *     the value is an array containing all uids that are members of the group
 *   If only $uids is set:
 *   - If $uids is a single value, return all mmtids (gids) to which the user
 *     belongs
 *   - If $uids is an array, return an array where the key is the uid and the
 *     value is an array containing all mmtids (gids) to which the user belongs
 * @param $normal
 *   If TRUE, consider only "normal" (not ad-hoc or virtual) groups
 * @param $virtual
 *   If TRUE, consider only virtual groups
 * @param $ad_hoc
 *   If TRUE, consider only ad-hoc groups
 */
function mm_content_get_uids_in_group($mmtids, $uids = NULL, $normal = TRUE, $virtual = TRUE, $ad_hoc = TRUE) {
  if (!$virtual && !$normal && !$ad_hoc) return array();

  if (!empty($mmtids)) {
    if (is_array($mmtids)) {
      $in_mmtids = 'IN (' . db_placeholders($mmtids) . ')';
    }
    else {
      $in_mmtids = '= %d';
      $mmtids = array($mmtids);
      $mmtids_single = TRUE;
    }
  }

  if (!empty($uids)) {
    if (is_array($uids)) {
      $in_uids = 'IN (' . db_placeholders($uids) . ')';
    }
    else {
      $in_uids = '= %d';
      $uids = array($uids);
      $uids_single = TRUE;
    }

    // uid=0 should never appear in any group
    $uids = array_diff($uids, array(0));
    if (!$uids) return array();
  }

  $out = array();
  if (!empty($mmtids) && !empty($uids)) {
    $where1 = "g.gid $in_mmtids AND v.uid $in_uids";
    $where2 = "gid $in_mmtids AND uid $in_uids";
    $params = array_merge($mmtids, $uids);
  }
  else if (!empty($mmtids)) {
    $where1 = "g.gid $in_mmtids";
    $where2 = "gid $in_mmtids AND uid > 0";
    $params = $mmtids;
  }
  else if (!empty($uids)) {
    $where1 = "v.uid $in_uids";
    $where2 = "uid $in_uids";
    $params = $uids;
  }
  else {
    return array();
  }

  $qs = array();
  if ($virtual) {
    $qs[] = 'SELECT g.gid, v.uid FROM {mm_group} g ' .
      'INNER JOIN {mm_virtual_group} v ON v.vgid = g.vgid ' .
      "WHERE $where1";
  }
  if ($normal || $ad_hoc) {
    if (!$normal) $where2 .= ' AND gid < 0';
    else if (!$ad_hoc) $where2 .= ' AND gid > 0';

    $qs[] = 'SELECT gid, uid FROM {mm_group} ' .
      "WHERE $where2";
  }
  if (count($qs)) $params = array_merge($params, $params);
  $query = db_query(join(' UNION ', $qs), $params);

  if (!empty($uids)) {
    while ($row = db_fetch_object($query)) {
      $out[$row->uid][] = $row->gid;
    }
    if ($uids_single) return $out[$uids[0]];
    return $out;
  }

  while ($row = db_fetch_object($query)) {
    $out[$row->gid][] = $row->uid;
  }
  if ($mmtids_single) return $out[$mmtids[0]];
  return $out;
}

/**
 * Return a list of users in an MM group, suitable for presentation in the UI.
 * This function calls hook_mm_get_users_in_group_alter(), which can be used to
 * prevent the disclosure of group membership to unauthorized viewers.
 *
 * @param $mmtid
 *   Tree ID (gid) of the group
 * @param $sep
 *   If not set, return an array. If set, join the list of users with this
 *   string and return the result.
 * @param $halt
 *   If TRUE, and there are more than $limit matches, return NULL
 * @param $limit
 *   If non-zero, limit the number of results. If the number of results would
 *   exceed the limit, either append '...' to the return (when a string), or
 *   add a '...' element to the returned array (when an array is requested).
 * @param $see_all
 *   If TRUE, and there are more than $limit matches, include a "see all users"
 *   link
 * @return
 *   The textual or array list, or possibly NULL if $halt is set
 */
function mm_content_get_users_in_group($mmtid, $sep = NULL, $halt = FALSE, $limit = 20, $see_all = FALSE) {
  $mmtids = is_array($mmtid) ? $mmtid : array($mmtid);
  foreach ($mmtids as $test_mmtid)
    if (!mm_content_user_can($test_mmtid, 'r')) {
      $msg = t('(not permitted to see list)');
      if (isset($sep)) return $msg;
      return array($msg);
    }

  $limit_str = $limit ? "AND v.preview <= $limit + 1 " : '';
  $lim = '';

  $mmtid_match = 'IN(' . join(', ', $mmtids) . ')';
  $qs = 'SELECT %s FROM ((SELECT %s FROM '.
          '(SELECT u.uid, u.name FROM '.
            "(SELECT * FROM {mm_group} WHERE gid $mmtid_match) AS g ".
          "INNER JOIN {mm_virtual_group} v ON v.vgid = g.vgid $limit_str".
          'INNER JOIN {users} u ON u.uid = v.uid) AS u) '.
        'UNION '.
        '(SELECT %s FROM '.
          '(SELECT u.uid, u.name FROM '.
            "(SELECT * FROM {mm_group} WHERE gid $mmtid_match AND uid > 0) AS g ".
          'INNER JOIN {users} u ON u.uid = g.uid) AS u)) x ORDER BY x.name';

  $query = sprintf($qs, '*', 'u.uid, u.name', 'u.uid, u.name');
  $countquery = sprintf($qs, 'SUM(c)', 'COUNT(DISTINCT u.uid) AS c', 'COUNT(DISTINCT u.uid) AS c', '');

  mm_module_invoke_all_array('mm_get_users_in_group_alter', array($mmtids, &$query, &$countquery));

  if ($limit > 0) {
    if ($halt) {
      $r = mm_query_result($countquery);
      if ($r == NULL || $r > $limit) return NULL;
    }
    else {
      $lim .= ' LIMIT ' . ($limit + 1);
    }
  }

  $query .= $lim;
  $q = mm_query($query);

  $users = array();
  while ($r = $q->next())
    $users[$r->uid] = mm_content_uid2name($r->uid, 'lfmu', $r);
  if (!$halt && $limit > 0 && count($users) == $limit + 1 && count($mmtids) == 1) {
    $overflow = TRUE;
    array_pop($users);
    if ($see_all) {
      $tree = mm_content_get($mmtids[0]);
      $see_link = l(
        t('See all users in this group'),
        mm_content_get_mmtid_url($mmtids[0]) . '/viewall',
        array(
          'attributes' => array(
            'class' => 'thickbox',
            'title' => t('All users in the group @name', array('@name' => mm_content_expand_name($tree->name))), // can't use %name here
          ),
          'query' => 'TB_iframe=true&height=400&width=400'
        )
      );
      $users = array_merge(array(-1 => $see_link), $users);
    }
  }

  if (!isset($sep)) {
    if ($overflow) $users[''] = '...';

    return $users;
  }

  return implode($sep, $users) . ($overflow ? '...' : '');
}

/**
 * Called when a new user account has been created, this function creates a
 * user's home directory in the MM tree.
 *
 * @param $account
 *   The user object describing the account being added
 */
function mm_content_add_user($account) {
  global $user;

  if ($account->user_mmtid || !variable_get('mm_use_user_dir', TRUE)) return;

  $users_mmtid = mm_content_users_mmtid();
  $default_tree = mm_content_get(array('name' => MM_ENTRY_NAME_DEFAULT_USER, 'parent' => $users_mmtid));
  if (!count($default_tree)) {
    watchdog('user', 'Missing /' . MM_ENTRY_NAME_USERS . '/' . MM_ENTRY_NAME_DEFAULT_USER,
        array(), WATCHDOG_ERROR);
    return;
  }

  $fullname = mm_content_uid2name($account->uid, 'lfmu', $account, $hover);

  $dest_mmtid = $users_mmtid;
  foreach (mm_module_implements('mm_add_user_alter') as $module) {
    $function = $module . '_mm_add_user_alter';
    if ($function($account, $dest_mmtid, $fullname) === FALSE) return;
  }

  // Find the first empty slot with a name ending in either $name or
  // '$name (username)'
  for ($i = 0;;) {
    $test = !$i ? preg_replace('/ \\(.*?\\)$/', '', $fullname) : $fullname;
    $exists = mm_content_get(array('name' => $test, 'parent' => $users_mmtid));
    if (!count($exists)) {
      $name = $test;
      break;
    }

    $other = user_load(array('uid' => $exists[0]->uid));
    // If owner of homedir no longer exists or his username is the same as
    // the one on the new account, move the old homedir out of the way
    if ($other === FALSE || $other->name == $account->name) {
      if (($err = mm_content_move_to_disabled($exists[0]->mmtid)) !== FALSE) {
        watchdog('user', "Error moving existing account '$name' into " .
            MM_ENTRY_NAME_DISABLED_USER . ": $err", array(), WATCHDOG_ERROR);
        return;
      }
      else
        watchdog('user', "Moved existing account '$name' into " .
            MM_ENTRY_NAME_DISABLED_USER, array(), WATCHDOG_WARNING);

      $name = $test;
      break;
    }

    if ($i++) {
      watchdog('user', "Could not create '$test' homepage because it already exists",
          array(), WATCHDOG_ERROR);
      return;
    }
  }

  $copy_params = array(
    MM_COPY_ALIAS =>    $account->name,
    MM_COPY_CONTENTS => TRUE,
    MM_COPY_NAME =>     $name,
    MM_COPY_OWNER =>    $account->uid,
  );
  $new_mmtid = mm_content_copy($default_tree[0]->mmtid, $dest_mmtid, $copy_params);

  if (is_numeric($new_mmtid)) {
    $user->user_mmtid = $new_mmtid;
    mm_content_set_flags($new_mmtid, array('user_home' => $account->uid), FALSE);
    mm_content_update_quick(array('hover' => $hover), array('mmtid' => $new_mmtid));
    mm_module_invoke_all_array('mm_add_user_post', array(&$account, $new_mmtid, $dest_mmtid));
  }
  else
    watchdog('user', "Error copying default user home dir into '$name': ".
        $new_mmtid, array(), WATCHDOG_ERROR);

  mm_content_clear_caches($users_mmtid);
}

/**
 * Prepend the MM path of the current page. This lets you generate URLs that
 * preserve the MM menu state.
 *
 * @param $rel_url
 *   Relative URL
 * @return
 *   The new URL, suitable for passing to the l() function
 */
function mm_content_prepend_mm_path($rel_url) {
  mm_parse_args($mmtids, $oarg_list, $this_mmtid);
  if (is_null($this_mmtid)) $this_mmtid = mm_home_mmtid();

  return "mm/$this_mmtid/$rel_url";
}

/**
 * Copy an item (and, optionally, its children) within the MM tree.
 *
 * @param $src_mmtid
 *   Tree ID of the entry to start copying from
 * @param $dest_mmtid
 *   Tree ID of the entry to copy to
 * @param $options
 *   An array containing options. The array is indexed using the constants
 *   below.
 *   - MM_COPY_ALIAS (NULL):
 *     URL alias of the new item, or NULL to keep the original value
 *   - MM_COPY_COMMENTS (FALSE):
 *     If TRUE, and MM_COPY_CONTENTS is also TRUE, copy the comments associated
 *     with any contents
 *   - MM_COPY_CONTENTS (FALSE):
 *     If TRUE, copy the contents of the page(s)
 *   - MM_COPY_ITERATE_ALTER (none):
 *     If set, this function or array of functions is called before any
 *     processing is done on each entry in the tree. If the function returns -1,
 *     the entry and any children will be skipped; if it returns 1, just the
 *     current entry is skipped; if it returns 0, all further processing is
 *     canceled; any other return value leads to no change. The function can
 *     also alter the item passed to it.
 *   - MM_COPY_NAME (NULL):
 *     Name of the new, top-level item, or NULL to keep the original value
 *   - MM_COPY_NODE_PRESAVE_ALTER (none):
 *     If set, this function or array of functions is passed copied nodes just
 *     before creation, for possible alteration
 *   - MM_COPY_OWNER (no change):
 *     If set, change the owner of the copies to this UID
 *   - MM_COPY_READABLE (FALSE):
 *     If TRUE, only copy entries readable by the user
 *   - MM_COPY_RECUR (TRUE):
 *     If TRUE, copy recursively (include all children)
 *   - MM_COPY_TREE (TRUE):
 *     If TRUE, copy the page(s)
 *   - MM_COPY_TREE_PRESAVE_ALTER (none):
 *     If set, this function or array of functions is passed the new page's
 *     description just before creation, for possible alteration
 *   - MM_COPY_TREE_SKIP_DUPS (FALSE):
 *     If TRUE, a check is done to ensure that tree entries with the same
 *     aliases as existing entries are not created in the destination
 * @return
 *   If successful, the tree ID of the first, new entry; otherwise, a human-
 *   readable error message
 */
function mm_content_copy($src_mmtid, $dest_mmtid, $options) {
  if (!class_exists('mmctcGetTreeIterator')) {
    class mmctcGetTreeIterator extends getTreeIterator {
      public $first_mmtid, $dest_mmtid, $error, $cont_userlist, $cont_grouplist, $nodemap, $options, $predefined_flags;

      public function __construct($src_mmtid, $dest_mmtid, $options) {
        $defaults = array(
          MM_COPY_ALIAS =>              NULL,
          MM_COPY_COMMENTS =>           FALSE,
          MM_COPY_CONTENTS =>           FALSE,
          MM_COPY_ITERATE_ALTER =>      NULL,
          MM_COPY_NAME =>               NULL,
          MM_COPY_NODE_PRESAVE_ALTER => NULL,
          MM_COPY_OWNER =>              NULL,
          MM_COPY_READABLE =>           FALSE,
          MM_COPY_RECUR =>              TRUE,
          MM_COPY_TREE =>               TRUE,
          MM_COPY_TREE_PRESAVE_ALTER => NULL,
          MM_COPY_TREE_SKIP_DUPS =>     FALSE,
        );
        $this->options = array_merge($defaults, $options);
        $this->dest_mmtid = $dest_mmtid;
        $this->nodemap = array(mm_content_get_parent($src_mmtid) => $dest_mmtid);

        $hooks = array(
          MM_COPY_ITERATE_ALTER =>      'mm_copy_tree_iterate_alter',
          MM_COPY_TREE_PRESAVE_ALTER => 'mm_copy_tree_tree_alter',
          MM_COPY_NODE_PRESAVE_ALTER => 'mm_copy_tree_node_alter',
        );
        foreach ($hooks as $constant => $hook) {
          $this->options[$constant] = isset($this->options[$constant]) ? (is_array($this->options[$constant]) ? $this->options[$constant] : array($this->options[$constant])) : array();
          foreach (mm_module_implements($hook) as $module)
            $this->options[$constant][] = "{$module}_$hook";
        }

        if ($this->options[MM_COPY_CONTENTS] && !$this->options[MM_COPY_TREE]) {
          mm_content_get_default_node_perms($dest_mmtid, $this->cont_grouplist, $this->cont_userlist, 0);
        }
        $this->predefined_flags = module_invoke_all('mm_tree_flags');
      }

      public function iterate($item) {
        // function must return 1 if no error, 0 if error, -1 if this node
        // and any of its children should be skipped
        if (isset($this->error)) return 0;   // was set in a previous invocation

        if ($this->options[MM_COPY_READABLE] && !$item->perms['r']) return -1;  // skip this node and kids

        if ($item->name == MM_ENTRY_NAME_RECYCLE) return -1;  // recycle bin: skip this node and kids

        $options_temp = $this->options;
        if (is_array($options_temp[MM_COPY_ITERATE_ALTER]))
          foreach ($options_temp[MM_COPY_ITERATE_ALTER] as $alter)
            switch (call_user_func_array($alter, array(&$item, &$options_temp))) {
              case 1:   // skip this one
                return;
              case -1:  // skip this one and kids
                return -1;
              case 0:   // completely stop
                return 0;
            }

        if ($options_temp[MM_COPY_CONTENTS] && !$options_temp[MM_COPY_TREE]) {
          if (!isset($this->first_mmtid)) {
            $this->nodemap[$item->mmtid] = $this->dest_mmtid;
          }
          else {
            $exists = mm_content_get(array('parent' => $this->nodemap[$item->parent], 'alias' => $item->alias));
            if ($exists) {
              $this->nodemap[$item->mmtid] = $exists[0]->mmtid;
            }
            else {
              $this->error = t('There is no destination page with the URL alias %alias to copy the content to.', array('%alias' => $item->alias));
              return 0;
            }
          }
        }
        else {
          $dest_mmtid = $this->nodemap[$item->parent];
          $alias = !empty($options_temp[MM_COPY_ALIAS]) ? $options_temp[MM_COPY_ALIAS] : $item->alias;

          $exists = FALSE;
          if ($options_temp[MM_COPY_TREE_SKIP_DUPS]) {
            $tree = mm_content_get(array('parent' => $dest_mmtid, 'alias' => $alias));
            if ($tree) {
              $this->nodemap[$item->mmtid] = $tree[0]->mmtid;
              $exists = TRUE;
            }
          }

          if (!$exists) {
            $perms = array();
            $result = db_query(
              'SELECT t.mmtid, a.mode ' .
              'FROM {mm_tree} t '.
                'INNER JOIN {mm_tree_access} a ON t.mmtid = a.gid '.
              'WHERE a.gid >= 0 AND a.mmtid = %d',
              $item->mmtid
            );
            while ($r = db_fetch_object($result)) {
              $perms[$r->mode]['groups'][] = $r->mmtid;
            }

            $result = db_query(
              'SELECT a.mode, g.uid ' .
              'FROM {mm_tree_access} a ' .
                'INNER JOIN {mm_group} g ON g.gid = a.gid ' .
              'WHERE a.gid < 0 AND a.mmtid = %d',
              $item->mmtid
            );
            while ($r = db_fetch_object($result)) {
              $perms[$r->mode]['users'][] = $r->uid;
            }

            $result = db_query(
              'SELECT bid, max_depth, max_parents ' .
              'FROM {mm_tree_block} ' .
              'WHERE mmtid = %d',
              $item->mmtid
            );
            if ($item->is_group || !($block = db_fetch_array($result)))
              $block = array('bid' => 0, 'max_depth' => -1, 'max_parents' => -1);

            if (!isset($this->nodemap[$item->parent])) {
              $this->error = t('Unexpected tree structure');
              return 0;
            }

            $new = array(
              'name' => !empty($options_temp[MM_COPY_NAME]) ? $options_temp[MM_COPY_NAME]  : $item->name,
              'alias' => $alias,
              'default_mode' => $item->default_mode,
              'uid' => $options_temp[MM_COPY_OWNER],
              'cascaded' => mm_content_get_cascaded_settings($item->mmtid),
              'perms' => $perms,
              'menu_start' => $block['bid'],
              'max_depth' => $block['max_depth'],
              'max_parents' => $block['max_parents'],
            );
            foreach (array('theme', 'flags', 'rss', 'node_info', 'previews', 'hidden', 'comment') as $field)
              $new[$field] = $item->$field;
            if (isset($this->first_mmtid)) {
              $new['weight'] = $item->weight;
            }

            foreach ($this->predefined_flags as $flag => $elem) {
              if (isset($elem['#flag_copy']) && $elem['#flag_copy'] === FALSE)
                unset($new['flags'][$flag]);
            }

            if (is_array($options_temp[MM_COPY_TREE_PRESAVE_ALTER]))
              foreach ($options_temp[MM_COPY_TREE_PRESAVE_ALTER] as $alter)
                call_user_func_array($alter, array(&$new, $dest_mmtid));

            $this->nodemap[$item->mmtid] = mm_content_insert_or_update(1, $dest_mmtid, $new);
          }
        }   // $options_temp[MM_COPY_CONTENTS] && !$options_temp[MM_COPY_TREE]

        if (!isset($this->first_mmtid)) {
          $this->first_mmtid = $this->nodemap[$item->mmtid];
          $this->options[MM_COPY_NAME] = $this->options[MM_COPY_ALIAS] = NULL;
        }

        if ($options_temp[MM_COPY_CONTENTS]) {
          foreach (mm_content_get_nids_by_mmtid($item->mmtid) as $n) {
            $node = node_load($n);
            if ($node->nid) {
              $old_catlist = $node->mm_catlist;
              $node->mm_catlist = array($this->nodemap[$item->mmtid] => '');
              $node->mm_catlist_restricted = array();
              $old_nid = $node->nid;
              unset($node->nid);
              unset($node->created);
              unset($node->recycle_date);
              unset($node->recycle_bins);
              unset($node->recycle_from_mmtids);
              if ($options_temp[MM_COPY_CONTENTS] && !$options_temp[MM_COPY_TREE] && !$node->others_w) {
                $node->groups_w = $this->cont_grouplist;
                $node->users_w = $this->cont_userlist;
              }

              if (is_array($options_temp[MM_COPY_NODE_PRESAVE_ALTER]))
                foreach ($options_temp[MM_COPY_NODE_PRESAVE_ALTER] as $alter)
                  call_user_func_array($alter, array(&$node, $old_catlist));

              node_save($node);

              // Clear-out the node_load() cache, since some modules (like
              // signup) can call node_load() during their hook_save() at times
              // before mm_catlist has been written to mm_node2tree, thus
              // caching the wrong data.
              node_load(NULL, NULL, TRUE);

              if ($node->nid && $options_temp[MM_COPY_COMMENTS] && module_exists('comment')) {
                $comment_map = array();
                $result = db_query('SELECT * FROM {comments} WHERE nid = %d', $old_nid);
                while ($comment = db_fetch_object($result)) {
                  $old_cid = $comment->cid;
                  $comment->nid = $node->nid;
                  drupal_write_record('comments', $comment);
                  $comment_map[$old_cid] = $comment->cid;
                }

                foreach ($comment_map as $old_cid => $new_cid)
                  db_query('UPDATE {comments} SET pid = %d WHERE pid = %d AND nid = %d', $new_cid, $old_cid, $node->nid);

                $result = db_query('SELECT * FROM {node_comment_statistics} WHERE nid = %d', $old_nid);
                $comment_stat = db_fetch_object($result);
                $comment_stat->nid = $node->nid;
                drupal_write_record('node_comment_statistics', $comment_stat, array('nid'));
              }

              watchdog('content', '%type: During copy, copied node nid=!id1 (%name) to nid=!id2',
                  array('%type' => $node->type, '%name' => $node->title, '!id1' => $n, '!id2' => $node->nid));
            }
          }
        }

        if ($item->is_group) // copy group entries
          db_query('INSERT INTO {mm_group} (gid, uid) SELECT %d, uid FROM {mm_group} WHERE gid = %d', $this->nodemap[$item->mmtid], $item->mmtid);

        return 1;
      }

      public function output() {
        if (isset($this->error)) return $this->error;

        return $this->first_mmtid;
      }
    }   // class
  }     // if (!class_exists ...

  $iter = new mmctcGetTreeIterator($src_mmtid, $dest_mmtid, $options);
  if ($iter->options[MM_COPY_RECUR])
    if ($msg = _mm_content_test_copy_move($src_mmtid, $dest_mmtid)) {
      return $msg;
    }

  $params = array(
    MM_GET_TREE_DEPTH        => $iter->options[MM_COPY_RECUR] ? -1 : 0,
    MM_GET_TREE_ITERATOR     => $iter,
    MM_GET_TREE_RETURN_PERMS => $iter->options[MM_COPY_READABLE] ? TRUE : NULL,
    MM_GET_TREE_RETURN_FLAGS => TRUE,
  );
  mm_content_get_tree($src_mmtid, $params);

  mm_content_clear_caches($dest_mmtid);
  watchdog('mm', "Copied mmtid=$src_mmtid to parent=$dest_mmtid!recur, new name=%name (%alias)",
      array('!recur' => $copy_recur ? t(' recursively') : '', '%name' => $iter->options[MM_COPY_NAME], '%alias' => $iter->options[MM_COPY_ALIAS]));

  return $iter->output();
}

/**
 * Move an entry within the MM tree.
 *
 * @param $src_mmtid
 *   Tree ID of the entry to move
 * @param $dest_mmtid
 *   Tree ID of the destination entry (new parent)
 * @param $recycle_mode
 *   Set to either 'recycle' or 'restore' to indicate if we are manipulating the
 *   recycle bin.
 * @return
 *   FALSE if successful; otherwise, a human-readable error message
 */
function mm_content_move($src_mmtid, $dest_mmtid, $recycle_mode = '') {
  if (!class_exists('mmctmGetTreeIterator')) {
    class mmctmGetTreeIterator extends getTreeIterator {
      public $parents, $pindex, $error, $recycle_mode, $bin, $delete_bins, $time, $first, $src_sort_idx;

      public function __construct($parents, $recycle_mode, $bin, $src_sort_idx) {
        $this->parents = $parents;
        $this->pindex = count($parents);
        $this->recycle_mode = $recycle_mode;
        $this->time = time();
        $this->bin = $bin;
        $this->delete_bins = array();
        $this->src_sort_idx = $src_sort_idx;
        $this->first = TRUE;
      }

      public function iterate($item) {
        // function must return 1 if no error, 0 if error, -1 if this entry
        // and any of its children should be skipped
        if (isset($this->error)) return 0;   // was set in a previous invocation

        $this->parents = array_slice($this->parents, 0, $this->pindex + $item->level);
        if ($this->first) {
          $parent = $this->parents[count($this->parents) - 1];
          $parent_idx = _mm_content_get_next_sort($parent);
          mm_content_update_parents($item->mmtid, $this->parents);
          mm_content_write_revision($item->mmtid);

          // Temporarily move the sort_idx into the right part of the tree, using
          // an index which is larger than it needs to be, then re-sort later.
          $like = str_replace(array('\\', '_', '%'), array('\\\\', '\_', '\%'), $this->src_sort_idx) . '%';
          db_lock_table('mm_tree');
          db_query("UPDATE {mm_tree} SET sort_idx_dirty = %d, weight = %d WHERE mmtid = %d", 1, 0, $item->mmtid);
          db_query("UPDATE {mm_tree} SET sort_idx = CONCAT('%s', SUBSTRING(sort_idx, %d)) WHERE sort_idx LIKE '%s'", $parent_idx, strlen($this->src_sort_idx) + 1, $like);
          db_unlock_tables();
          mm_content_update_sort_queue($parent);
          $this->first = FALSE;
        }
        else {
          mm_content_update_parents($item->mmtid, $this->parents);
        }

        $this->parents[] = $item->mmtid;
        mm_content_clear_caches($item->mmtid);

        if ($this->recycle_mode == 'recycle' && !$item->perms['IS_RECYCLED'])
          foreach (mm_content_get_nids_by_mmtid($item->mmtid) as $nid) {
            $from_mmtids = mm_content_get_by_nid($nid);
            foreach ($from_mmtids as $mmtid) {
              if ($mmtid == $item->mmtid) $bin = $this->bin;
              else {
                $bin = _mm_content_make_recycle($mmtid);
                if (!is_numeric($bin)) {
                  $this->error = $bin;
                  return 0;
                }
                db_query('UPDATE {mm_node2tree} SET mmtid = %d WHERE nid = %d AND mmtid = %d', $bin, $nid, $mmtid);
                db_query('DELETE FROM {mm_node_reorder} WHERE nid = %d AND mmtid = %d', $nid, $mmtid);
              }
              db_query('INSERT IGNORE INTO {mm_recycle} '.
                  '(type, id, bin_mmtid, recycle_date, from_mmtid) '.
                  "VALUES('node', %d, %d, %d, %d)", $nid, $bin, $this->time, $mmtid);
              watchdog('mm', "Recycled node=$nid from mmtid=$mmtid");
            }
          }
        else if ($this->recycle_mode == 'restore') {
          $q = mm_query($this->bin, "SELECT r2.* FROM {mm_recycle} r1 INNER JOIN {mm_recycle} r2 ON r1.type = '%s' AND r2.type = '%s' AND r2.id = r1.id WHERE r1.bin_mmtid = %d", 'node', 'node', $this->bin);
          $dels = array();
          while ($r = $q->next()) {
            db_query('UPDATE {mm_node2tree} SET mmtid = %d WHERE mmtid = %d AND nid = %d',
                $r->from_mmtid, $r->bin_mmtid, $r->id);
            mm_content_clear_caches(array($r->from_mmtid, $r->bin_mmtid));
            $this->delete_bins[$r->bin_mmtid] = 1;
            $dels[] = $r->id;
          }

          if ($dels) {
            db_query("DELETE FROM {mm_recycle} WHERE type = 'node' AND id IN(" . db_placeholders($dels) . ')', $dels);
            watchdog('mm', "Restored nodes to mmtid=$item->mmtid");
          }
        }

        return 1;
      }
    }
  }

  if ($msg = _mm_content_test_copy_move($src_mmtid, $dest_mmtid)) {
    return $msg;
  }

  $src = mm_content_get($src_mmtid);
  $list = mm_content_get_parents_with_self($dest_mmtid, FALSE, FALSE); // don't include virtual parents
  $bin = $recycle_mode == 'recycle' ? $dest_mmtid : mm_content_get_parent($src_mmtid);
  $iter = new mmctmGetTreeIterator($list, $recycle_mode, $bin, $src->sort_idx);
  $params = array(
    MM_GET_TREE_RETURN_PERMS => $recycle_mode == 'recycle' ? TRUE : NULL,
    MM_GET_TREE_DEPTH        => -1,
    MM_GET_TREE_ITERATOR     => $iter,
  );
  mm_content_get_tree($src_mmtid, $params);

  mm_content_clear_caches(array($src_mmtid, $dest_mmtid));

  foreach (array_keys($iter->delete_bins) as $bin) {
    mm_content_delete_bin($bin);
  }

  switch ($recycle_mode) {
    case 'recycle':
      watchdog('mm', "Recycled mmtid=$src_mmtid to bin=$dest_mmtid");
      break;

    case 'restore':
      mm_content_clear_caches($iter->bin);
      watchdog('mm', "Restored mmtid=$src_mmtid to parent=$dest_mmtid");
      break;

    default:
      watchdog('mm', "Moved mmtid=$src_mmtid to new parent=$dest_mmtid");
  }

  if ($iter->error) {
    watchdog('mm', 'Move error: %error', array('%error' => $iter->error), WATCHDOG_ERROR);
    return $iter->error;
  }
  return FALSE;
}

/**
 * Move a user's home directory into /.Users/.Disabled within the MM tree.
 *
 * @param $mmtid
 *   Tree ID of the entry to move
 * @return
 *   FALSE if successful; otherwise, a human-readable error message
 */
function mm_content_move_to_disabled($mmtid) {
  $user_dir = mm_content_get(array('name' => MM_ENTRY_NAME_USERS, 'parent' => 1));
  if (!count($user_dir))
    return t('@dir not found', array('@dir' => MM_ENTRY_NAME_USERS));

  $disab_dir = mm_content_get(array('name' => MM_ENTRY_NAME_DISABLED_USER, 'parent' => $user_dir[0]->mmtid));
  if (!count($disab_dir))
    return t('@dir not found', array('@dir' => MM_ENTRY_NAME_DISABLED_USER));

  // This might create duplicate names in /.Users/.Disabled, but it's probably
  // best to leave them.
  return mm_content_move($mmtid, $disab_dir[0]->mmtid);
}

function _mm_content_get_next_sort($parent) {
  $max = db_result(db_query_range('SELECT sort_idx FROM {mm_tree} WHERE parent = %d ORDER BY sort_idx DESC', $parent, 0, 1));
  if (empty($max)) {
    return db_result(db_query('SELECT sort_idx FROM {mm_tree} WHERE mmtid = %d', $parent)) . _mm_content_btoa(0);
  }
  return substr($max, 0, -MM_CONTENT_BTOA_CHARS) . _mm_content_btoa(_mm_content_atob(substr($max, -MM_CONTENT_BTOA_CHARS)) + 1);
}

function _mm_content_test_sort_length($sort_idx, $msg, $critical = FALSE) {
  static $max, $did_error;
  if (empty($max)) {
    $schema = drupal_get_schema('mm_tree');
    $max = $schema['fields']['sort_idx']['length'];
  }

  if (strlen($sort_idx) > $max) {
    if (empty($did_error)) {
      if (is_numeric($msg)) {
        $vars = array('@mmtid' => $msg);
        $msg = 'The tree is nested too deeply, starting at mmtid=@mmtid. Presentation of sorted trees will suffer. To correct this problem, increase the length of mm_tree.sort_idx then run mm_content_update_sort().';
      }
      else {
        $vars = $msg[1];
        $msg = $msg[0];
      }

      watchdog('mm', $msg, $vars, $critical ? WATCHDOG_CRITICAL : WATCHDOG_WARNING);
      $did_error = TRUE;
    }
    return FALSE;
  }
  return TRUE;
}

function _mm_content_test_copy_move($src_mmtid, $dest_mmtid) {
  // Don't allow a copy/move to happen if the resulting sort_idx would be too long
  $max_sort_idx = mm_query_result('SELECT ' .
    'CONCAT(' .
      '(SELECT sort_idx FROM {mm_tree} WHERE mmtid = %d), ' .
      'SUBSTR(' .
        "REPEAT('x', " .
          '(SELECT MAX(LENGTH(t.sort_idx)) ' .
            'FROM {mm_tree_parents} p ' .
            'INNER JOIN {mm_tree} t ON t.mmtid = p.mmtid ' .
            'WHERE p.parent = %d OR t.mmtid = %d' .
          ')' .
        '), ' .
        '(SELECT LENGTH(sort_idx) ' .
          'FROM {mm_tree} WHERE mmtid = %d' .
        ') - %d' .
      ')' .
    ')',
    $dest_mmtid, $src_mmtid, $src_mmtid, $src_mmtid, MM_CONTENT_BTOA_CHARS - 1);
  $msg = array(
    'An attempt to copy or move mmtid=@src to mmtid=@dest failed, because it would result in a tree that is too deeply nested. To correct this problem, increase the length of mm_tree.sort_idx then run mm_content_update_sort().',
    array('@src' => $src_mmtid, '@dest' => $dest_mmtid),
  );
  if (!_mm_content_test_sort_length($max_sort_idx, $msg)) {
    return t('This operation cannot be performed because it would cause the tree to become too deeply nested. Please contact a system administrator.');
  }
  return FALSE;
}

/**
 * Add a new entry or replace an existing entry in the MM tree.
 *
 * @param $add
 *   TRUE if the entry is new
 * @param $mmtid
 *   Tree ID of the new entry's parent ($add=TRUE), or the ID of the entry to
 *   replace
 * @param $parameters
 *   Either an array or an object with one more more of the attributes listed
 *   below. Other modules can add settings using hook_mm_cascaded_settings().
 *   - 'alias':
 *     The entry's alias
 *   - 'cascaded':
 *     An array containing these possible elements, which describe settings that
 *     are cascaded downward in the tree to any children:
 *     - 'allowed_node_types':
 *       If not NULL, then set the entry's allowed node type list to this array
 *       of values. To indicate that the parent settings should be inherited,
 *       pass an empty array or NULL. To indicate that there should be no node
 *       types allowed, pass array('').
 *     - 'allow_reorder':
 *       Allow users with write access to reorder the menu of this page and its
 *       children (-1 = inherit)
 *     - 'allowed_themes':
 *       If not NULL, then set the entry's allowed theme list to this array of
 *       values
 *     - 'comments_readable':
 *       Default comment readability for new nodes added to this entry
 *     - 'share_default':  (in mm_share_widget.module)
 *       Default state for the Share link in new nodes added to this entry and
 *       its children (0 = off, 1 = on, -1 = inherit)
 *   - 'comment':
 *     Default comment mode for new content added to this entry (0)
 *   - 'default_mode':
 *     Default access mode for the entry, a comma-separated list of 'r','w','a',
 *     'u' ('')
 *   - 'flags':
 *     A single string or array of strings in flag => value format to be added
 *     to the entry (admins. only)
 *   - 'hidden':
 *     If set, the entry only appears in menus if the user can edit it or add
 *     content to it (FALSE)
 *   - 'hover':
 *     Text for title attribute, displayed when the mouse hovers over the link
 *     ('')
 *   - 'max_depth':
 *     If $menu_start is set, the maximum depth of the menu block (-1)
 *   - 'max_parents':
 *     If $menu_start is set, the maximum number of parent levels to display
 *     (-1)
 *   - 'members':
 *     For non-virtual groups, an array of the uids of the group's members; if
 *     an empty string (''), do not change any existing users
 *   - 'menu_start':
 *     If non-zero, a menu block using this block ID starts at this level ('')
 *   - 'name':
 *     The entry's name
 *   - 'node_info':
 *     The default value for showing 'Submitted by' lines on nodes added to this
 *     page (TRUE)
 *   - 'perms':
 *     An array of arrays ['r', 'w', 'a', 'u']['groups', 'users']. All are
 *     optional. For 'groups', provide an array of gids; for 'users' an array of
 *     uids.
 *   - 'previews':
 *     If set, show all nodes on the page as teasers (FALSE)
 *   - 'propagate_node_perms':
 *     If TRUE, and !$add, set the permissions on all nodes for which the user
 *     has write access to match the entry. If recurs_perms is also true,
 *     recursively set the permissions on the nodes on all children as well.
 *     (FALSE)
 *   - 'qfield':
 *     For a virtual group, the "column to select" portion of the query; ignored
 *     for non-virtual groups
 *   - 'qfrom':
 *     For a virtual group, the "FROM clause" portion of the query; ignored for
 *     non-virtual groups
 *   - 'recurs_perms':
 *     If TRUE, and !$add, recursively set the permissions on all children for
 *     which the user has write access to match the parent. (FALSE)
 *   - 'rss':
 *     If set, show an 'Add this page to my portal' button on the page (FALSE)
 *   - 'theme':
 *     Name of the entry's theme, if any (none)
 *   - 'uid':
 *     User ID of the entry's owner (1)
 *   - 'weight':
 *     Order of the entry among its siblings (0)
 * @return
 *   Tree ID of the entry that was added or replaced, or 0 on error
 */
function mm_content_insert_or_update($add, $mmtid, $parameters) {
  global $user;
  static $defaults = array(
    'alias' => '',
    'cascaded' => array(),
    'comment' => 0,
    'default_mode' => '',
    'flags' => '',
    'hidden' => FALSE,
    'hover' => '',
    'large_group' => FALSE,
    'max_depth' => -1,
    'max_parents' => -1,
    'members' => '',
    'menu_start' => -1,
    'name' => '',
    'node_info' => TRUE,
    'perms' => array(),
    'previews' => FALSE,
    'propagate_node_perms' => FALSE,
    'qfield' => '',
    'qfrom' => '',
    'recurs_perms' => FALSE,
    'rss' => FALSE,
    'theme' => '',
    'uid' => 1,
    'weight' => 0,
  );
  static $cascaded_settings;

  $parameters = (array)$parameters;
  foreach (array_keys($parameters) as $p) {
    if (!isset($defaults[$p])) {
      watchdog('mm', 'Unknown parameter %name to mm_content_insert_or_update', array('%name' => $p), WATCHDOG_CRITICAL);
      drupal_set_message(t('An error occurred.'));
      return 0;
    }

    if ($p == 'cascaded') {
      if (!isset($cascaded_settings)) $cascaded_settings = mm_content_get_cascaded_settings();
      foreach (array_keys($parameters[$p]) as $c)
        if (!isset($cascaded_settings[$c])) {
          watchdog('mm', 'Unknown cascaded setting %name in mm_content_insert_or_update', array('%name' => $c), WATCHDOG_CRITICAL);
          drupal_set_message(t('An error occurred.'));
          return 0;
        }
    }
  }
  $parameters = array_merge($defaults, $parameters);

  $is_group = mm_content_is_group($mmtid);

  if ($add) {
    $parent = $mmtid;
    $list = mm_content_get_parents_with_self($parent, FALSE, FALSE); // don't include virtual parents

    $parameters['parent'] = $parent;
    $parameters['ctime'] = time();
    $parameters['cuid'] = $user->uid;
    $parameters['sort_idx'] = _mm_content_get_next_sort($parent);
    $parameters['sort_idx_dirty'] = 1;

    $msg = array('An attempt to create a new child of mmtid=@mmtid failed, because it would result in a tree that is too deeply nested. To correct this problem, increase the length of mm_tree.sort_idx then run mm_content_update_sort().', array('@mmtid' => $parent));
    if (!_mm_content_test_sort_length($parameters['sort_idx'], $msg)) {
      drupal_set_message(t('This operation cannot be performed because it would cause the tree to become too deeply nested. Please contact a system administrator.'));
      return 0;
    }

    drupal_write_record('mm_tree', $parameters);
    $mmtid = $parameters['mmtid'];
    mm_content_update_parents($mmtid, $list, TRUE);
    mm_content_write_revision($mmtid);

    mm_content_set_flags($mmtid, $parameters['flags'], FALSE);
    mm_content_set_perms($mmtid, $parameters['perms'], $is_group, FALSE);
    if ($is_group) mm_content_set_group_members($mmtid, NULL, $parameters['qfield'], $parameters['qfrom'], $parameters['members']);
    mm_content_clear_caches($parent);
    mm_content_update_sort_queue($parent);
    watchdog('mm', "Added %name (%alias) mmtid=@mmtid",
        array('%name' => $parameters['name'], '%alias' => $parameters['alias'], '@mmtid' => $mmtid));
  }
  else {
    if ($parameters['recurs_perms']) {
      $list = array();
      foreach (mm_content_get_tree($mmtid, array(MM_GET_TREE_RETURN_PERMS => TRUE)) as $t) {
        if ($t->perms['w']) $list[] = $t->mmtid;
        if (!isset($old)) $old = $t;
      }
    }
    else {
      $list = array($mmtid);
      $old = mm_content_get($mmtid);
    }

    unset($parameters['ctime']);
    unset($parameters['cuid']);
    unset($parameters['parent']);
    $count = 0;
    foreach ($list as $t) {
      unset($parameters['sort_idx_dirty']);
      if ($count++) {    // recursive: item after the first
        db_query("UPDATE {mm_tree} SET default_mode = '%s' WHERE mmtid = %d", $parameters['default_mode'], $t);
        if (db_affected_rows()) mm_content_write_revision($t);
        if ($is_group) mm_content_set_group_members($t, NULL, '', '', array());
      }
      else {             // first item only
        $parameters['mmtid'] = $t;
        if ($parameters['name'] != $old->name || $parameters['weight'] != $old->weight || $parameters['hidden'] != $old->hidden) {
          $parameters['sort_idx_dirty'] = 1;
          mm_content_update_sort_queue($old->parent);
        }
        drupal_write_record('mm_tree', $parameters, 'mmtid');
        if (db_affected_rows()) mm_content_write_revision($t);

        if ($is_group) {
          mm_content_set_group_members($mmtid, NULL, $parameters['qfield'], $parameters['qfrom'], $parameters['members'], $parameters['large_group']);
        }
        else {
          db_query('DELETE FROM {mm_tree_block} WHERE mmtid = %d', $t);
        }
        db_query('DELETE FROM {mm_cascaded_settings} WHERE mmtid = %d', $t);
        mm_content_set_flags($mmtid, $parameters['flags']);
      }

      mm_content_set_perms($t, $parameters['perms'], $is_group, TRUE);

      // Copy the permissions onto nodes attached to the entry if requested.
      if ($parameters['propagate_node_perms']) {
        foreach (mm_content_get_nids_by_mmtid($t) as $nid) {
          $node = node_load($nid);
          if ($node && node_access('update', $node)) {
            $node->users_w = is_array($parameters['perms']['w']['users']) ? array_flip($parameters['perms']['w']['users']) : array();
            $node->groups_w = is_array($parameters['perms']['w']['groups']) ? array_flip($parameters['perms']['w']['groups']) : array();
            $node->others_w = strpos($parameters['default_mode'], 'w') !== FALSE;

            mm_content_set_node_perms($node);
          }
        }
      }
    }
    mm_content_clear_caches();  // clear everything, because there may be affected kids, even without recursion

    $did_perms = array();
    if ($parameters['recurs_perms']) $did_perms[] = t(' Permissions were copied to all children.');
    if ($parameters['propagate_node_perms']) $did_perms[] = t(' Permissions were copied to nodes.');
    watchdog('mm', "Updated %name (%alias) mmtid = $mmtid.@perms", array(
      '%name' => $parameters['name'],
      '%alias' => $parameters['alias'],
      '@perms' => join('', $did_perms)));
  }

  if (!$is_group) {
    mm_content_set_cascaded_settings($mmtid, $parameters['cascaded'], FALSE);

    if ($parameters['menu_start'] && $parameters['menu_start'] != -1 ||
        $parameters['max_depth'] >= 0 || $parameters['max_parents'] >= 0)
      db_query('INSERT INTO {mm_tree_block} (mmtid, bid, max_depth, max_parents) VALUES(%d, %d, %d, %d)', $mmtid, $parameters['menu_start'], $parameters['max_depth'], $parameters['max_parents']);
  }

  return $mmtid;
}

/**
 * Update an existing entry in the MM tree. Only attributes which are stored in
 * the mm_tree table are supported.
 *
 * @param $parameters
 *   Either an array or an object with one more more of the attributes listed
 *   below. Only those attributes which are listed can be updated.
 *   - 'alias':
 *     The entry's alias
 *   - 'comment':
 *     Default comment mode for new content added to this entry
 *   - 'default_mode':
 *     Default access mode for the entry, a comma-separated list of 'r','w','a',
 *     'u'
 *   - 'hidden':
 *     If set, the entry only appears in menus if the user can edit it or add
 *     content to it
 *   - 'hover':
 *     Text for title attribute, displayed when the mouse hovers over the link
 *   - 'name':
 *     The entry's name
 *   - 'node_info':
 *     The default value for showing 'Submitted by' lines on nodes added to this
 *     page
 *   - 'previews':
 *     If set, show all nodes on the page as teasers
 *   - 'rss':
 *     If set, show an 'Add this page to my portal' button on the page
 *   - 'theme':
 *     Name of the entry's theme, if any
 *   - 'uid':
 *     User ID of the entry's owner
 *   - 'weight':
 *     Order of the entry among its siblings
 * @param $where
 *   Defines what tree entries to update. If a single value is supplied, then it
 *   it treated as the tree ID (mmtid) of an entry.
 *
 *   An associative array can also be supplied; it must contain one or more
 *   attributes from the list above, or "mmtid", as the array key. The array
 *   value becomes part of the update query WHERE. At a minimum, "mmtid" must be
 *   specified.
 * @param $parent
 *   If the name, hidden, or weight attribute is being changed, the parent tree
 *   ID may need to be queued for later update. If it is neeeded and not
 *   specified in $parent, it will be retrieved using an additional query.
 * @param $revision
 *   If TRUE, update the mm_tree_revisions table
 */
function mm_content_update_quick($parameters, $where, $parent = NULL, $revision = TRUE) {
  $parameters = (array)$parameters;
  if (!is_array($where)) {
    $where = array('mmtid' => $where);
  }

  $special = array_intersect_key($parameters, array('name' => 1, 'hidden' => 1, 'weight' => 1));
  $not_special = array_diff_key($parameters, $special);
  if ($special) {
    if ($not_special) {
      mm_content_update_quick($special, $where, $parent, FALSE);
      $parameters = $not_special;
      unset($special);
    }
    else {
      $parameters['sort_idx_dirty'] = 1;
    }
  }

  $schema = drupal_get_schema('mm_tree');

  foreach (array_keys($parameters) as $p) {
    if (!isset($schema['fields'][$p])) {
      watchdog('mm', 'Unknown attribute %name in $parameters to mm_content_update_quick', array('%name' => $p), WATCHDOG_CRITICAL);
      drupal_set_message(t('An error occurred.'));
      return;
    }
  }
  foreach (array_keys($where) as $p) {
    if (!isset($schema['fields'][$p])) {
      watchdog('mm', 'Unknown attribute %name in $where to mm_content_update_quick', array('%name' => $p), WATCHDOG_CRITICAL);
      drupal_set_message(t('An error occurred.'));
      return;
    }
  }

  $fields = $values = $placeholders = $conditions = $special_conditions = $special_values = array();

  // Go through our schema, build SQL.
  foreach ($schema['fields'] as $field => $info) {
    // Special case -- skip serial types
    if ($info['type'] == 'serial') {
      continue;
    }

    // Build arrays for the fields, placeholders, and values in our query.
    if (isset($parameters[$field])) {
      $fields[] = $field;
      $placeholders[] = db_type_placeholder($info['type']);
      $values[] = $parameters[$field];
      if ($special && $field != 'sort_idx_dirty') {
        $special_conditions[] = "$field <> " . db_type_placeholder($schema['fields'][$field]['type']);
        $special_values[] = $parameters[$field];
      }
    }
  }

  // Build the SQL.
  $query = '';
  foreach ($fields as $id => $field) {
    if ($query) {
      $query .= ', ';
    }
    $query .= $field . ' = ' . $placeholders[$id];
  }

  if ($special_conditions) {
    $conditions[] = '(' . join(' OR ', $special_conditions) . ')';
    $values = array_merge($values, $special_values);
  }

  foreach ($where as $key => $val) {
    $conditions[] = "$key = " . db_type_placeholder($schema['fields'][$key]['type']);
    $values[] = $val;
  }

  $query = "UPDATE {mm_tree} SET $query WHERE " . implode(' AND ', $conditions);

  if (db_query($query, $values) && db_affected_rows()) {
    if ($revision) {
      if (!isset($where['mmtid'])) {
        $where['mmtid'] = db_last_insert_id('mm_tree', 'mmtid');
      }
      mm_content_write_revision($where['mmtid']);
    }

    if ($special && (!empty($parent) || isset($where['mmtid']))) {
      mm_content_update_sort_queue($parent, $where['mmtid']);
    }
  }
}

/**
 * Makes a copy of an mm_tree entry to the mm_tree_revisions table
 *
 * @param $mmtid
 *   The tree ID of the entry being copied
 */
function mm_content_write_revision($mmtid) {
  global $user;

  $schema_rev = drupal_get_schema('mm_tree_revisions');
  $fields = array();
  foreach ($schema_rev['fields'] as $field => $info) {
    if ($info['type'] != 'serial' && $field != 'muid' && $field != 'mtime') {
      $fields[] = $field;
    }
  }

  $fields = join(', ', $fields);
  if (db_query("INSERT INTO {mm_tree_revisions} ($fields, muid, mtime) SELECT $fields, %d, %d FROM {mm_tree} WHERE mmtid = %d", $user->uid, time(), $mmtid)) {
    $vid = db_last_insert_id('mm_tree_revisions', 'vid');
    if ($vid) {
      db_query("UPDATE {mm_tree} SET vid = %d WHERE mmtid = %d", $vid, $mmtid);
    }
  }
}

/**
 * Based on an entry's permissions, return appropriate default values for 'who
 * can edit or delete this content' on nodes added to it.
 *
 * @param $mmtid
 *   The tree ID of the entry nodes are being added to
 * @param $grouplist
 *   On return, contains an array of long group names, indexed by the mmtid of
 *   the group
 * @param $userlist
 *   On return, contains an array of long user names, indexed by uid
 * @param $max
 *   The maximum number of users to copy in ad-hoc groups
 */
function mm_content_get_default_node_perms($mmtid, &$grouplist, &$userlist, $max) {
  // Find groups
  $grouplist = array();
  $result =
    db_query('SELECT DISTINCT t2.mmtid, t2.name FROM {mm_tree} t '.
             'INNER JOIN {mm_tree_access} a ON a.mmtid = t.mmtid '.
             'LEFT JOIN {mm_tree} t2 ON a.gid = t2.mmtid '.
             "WHERE t2.mmtid >= 0 AND (a.mode = 'u' OR a.mode = 'w') ".
               'AND a.mmtid = %d ORDER BY t2.name', $mmtid);
  while ($r = db_fetch_object($result))
    $grouplist[$r->mmtid] = $r->name;

  // Find individual users
  $userlist = array();
  $result = db_query("SELECT DISTINCT gid FROM {mm_tree_access} WHERE gid < 0 AND (mode = 'u' OR mode = 'w') AND mmtid = %d", $mmtid);
  while ($r = db_fetch_object($result)) {
    $users = mm_content_get_users_in_group($r->gid, NULL, TRUE, $max);

    if (!is_null($users))
      $userlist += $users;
  }
}

/**
 * Set a group's membership list or virtual group attributes
 *
 * @param $mmtid
 *   MM tree ID of the group
 * @param $vgid
 *   Virtual group ID of the group, if known. If this value is NULL and the
 *   group is determined to be a virtual group, its vgid is retrieved
 * @param $qfield
 *   For a virtual group, the "column to select" portion of the query; ignored
 *   for non-virtual groups
 * @param $qfrom
 *   For a virtual group, the "FROM clause" portion of the query; ignored for
 *   non-virtual groups
 * @param $members
 *   For non-virtual groups, an array of the uids of the group's members; if
 *   an empty string (''), do not change any existing users
 * @param $large_group
 *   A flag in the case where large group management is used.  In this case,
 *   $members contains only adds so no deleting should occur prior.
 */
function mm_content_set_group_members($mmtid, $vgid, $qfield, $qfrom, $members, $large_group = FALSE) {
  $vgroup = mm_content_is_vgroup($mmtid);
  if ($vgroup && empty($vgid)) {
    $vgid = db_result(db_query('SELECT v.vgid FROM {mm_group} g '.
        'INNER JOIN {mm_vgroup_query} v ON g.vgid = v.vgid WHERE g.gid = %d', $mmtid));
  }
  if ($vgroup && $qfield != '' && !empty($vgid)) {
    db_query("UPDATE {mm_vgroup_query} SET field = '%s', qfrom = '%s', dirty = %d WHERE vgid = %d", $qfield, $qfrom, MM_VGROUP_DIRTY_NEXT_CRON, $vgid);
  }
  else if ($vgroup || is_array($members) || $large_group) {
    db_query('DELETE {mm_vgroup_query}, {mm_group} FROM {mm_vgroup_query}, {mm_group} '.
        'WHERE {mm_vgroup_query}.vgid = {mm_group}.vgid AND {mm_group}.gid = %d', $mmtid);
    db_query('DELETE FROM {mm_group} WHERE gid = %d', $mmtid);

    if ($large_group) { // Copy from the temp table and then remove temp records
      db_query("INSERT INTO {mm_group} (gid, uid, vgid) SELECT gid, uid, 0 FROM {mm_group_temp} WHERE gid = %d AND sessionid = '%s'", $mmtid, session_id());
      db_query("DELETE FROM {mm_group_temp} WHERE gid = %d AND sessionid = '%s'", $mmtid, session_id());
    }

    if ($vgroup) {
      if ($qfield != '') {
        db_query("INSERT INTO {mm_vgroup_query} (field, qfrom, dirty) VALUES('%s', '%s', %d)", $qfield, $qfrom, MM_VGROUP_DIRTY_NEXT_CRON);
        $vgid = db_last_insert_id('mm_vgroup_query', 'vgid');
        db_query('INSERT INTO {mm_group} (gid, uid, vgid) VALUES(%d, %d, %d)', $mmtid, 0, $vgid);
      }
    }
    else if (is_array($members))
      foreach ($members as $uid)
        db_query('INSERT INTO {mm_group} (gid, uid, vgid) VALUES(%d, %d, %d)', $mmtid, $uid, 0);
  }
}

/**
 * Given a node, return TRUE if the node is in a recycle bin.
 *
 * @param $node
 *   The node to test
 * @return
 *   TRUE if the node is in a recycle bin
 */
function mm_content_node_is_recycled($node) {
  return isset($node->recycle_date);
}

/**
 * Given a set of tree IDs, return a list of the node IDs assigned to them.
 *
 * @param $mmtids
 *   An array of tree IDs, or a single ID
 * @param $limit
 *   Optional limit to the number of results
 * @param $unique
 *   If set, return only those nodes that are assigned to entries from the list,
 *   and not to other entries.
 * @return
 *   An array of node IDs
 */
function mm_content_get_nids_by_mmtid($mmtids, $limit = '', $unique = FALSE) {
  if (is_array($mmtids)) {
    if (count($mmtids) == 1) {
      $in = '= %d';
    }
    else {
      $in = 'IN(' . db_placeholders($mmtids) . ')';
    }
  }
  else {
    $in = '= %d';
    $mmtids = array($mmtids);
  }

  if ($unique) {
    $query = 'SELECT t1.nid FROM {mm_node2tree} t1 '.
        "INNER JOIN {mm_node2tree} t2 ON t1.nid = t2.nid AND t2.mmtid $in ".
        "GROUP BY t1.nid HAVING SUM(IF(t1.mmtid $in, %d, %d)) = COUNT(*)";
    $mmtids = array_merge($mmtids, $mmtids, array(1, 0));
  }
  else {
    $query = "SELECT nid FROM {mm_node2tree} WHERE mmtid $in";
  }

  if ($limit) {
    $q = mm_query_range($query, $mmtids, 0, $limit);
  }
  else {
    $q = mm_query($query, $mmtids);
  }

  $out = array();
  while ($r = $q->next()) {
    $out[] = $r->nid;
  }

  return $out;
}

/**
 * Given a tree ID, return a SQL query handle for nodes that appear on it. Only
 * nodes the user has permission to view are returned, and they are sorted
 * according to all of the various possible criteria.
 *
 * @param $mmtid
 *   The tree ID of the entry
 * @param $per_page
 *   Nodes per page in the pager (optional)
 * @param $element
 *   Pager element number (optional)
 * @param $add_select
 *   Text to add to the SELECT part of the main query (optional)
 * @param $add_join
 *   Additional LEFT JOINs for the main query (optional)
 * @param $add_where
 *   Text to add to the WHERE part of the main query (optional)
 * @param $add_groupby
 *   Text to add to the GROUP BY part of the main query (optional)
 * @param $add_orderby
 *   Text to add to the ORDER BY part of the main query (optional)
 * @return
 *   An SQL query handle
 */
function mm_content_get_accessible_nodes_by_mmtid($mmtid, $per_page = 0, $element = 0, $add_select = '', $add_join = '', $add_where = '', $add_groupby = '', $add_orderby = '') {
  if ($mmtid < 0) return NULL;

  $q = mm_content_get_accessible_nodes_by_mmtid_query($mmtid, $count_sql, $add_select, $add_join, $add_where, $add_groupby, $add_orderby);
  if ($per_page > 0)
    return pager_query(db_rewrite_sql($q), $per_page, $element, $count_sql);

  return db_query($q);
}

/**
 * Given a tree ID, return a SQL query for nodes that appear on it. Only nodes
 * that are owned by the user or are currently published are returned, and they
 * are sorted according to all of the various possible criteria.
 *
 * @param $mmtid
 *   The tree ID (or array of tree IDs) of the entry
 * @param $count_sql
 *   The SQL query for querying the count of matches
 * @param $add_select
 *   Text to add to the SELECT part of the main query (optional)
 * @param $add_join
 *   Additional LEFT JOINs for the main query (optional)
 * @param $add_where
 *   Text to add to the WHERE part of the main query (optional)
 * @param $add_groupby
 *   Text to add to the GROUP BY part of the main query (optional)
 * @param $add_orderby
 *   Text to add to the ORDER BY part of the main query (optional)
 * @return
 *   The SQL query
 */
function mm_content_get_accessible_nodes_by_mmtid_query($mmtid, &$count_sql, $add_select = '', $add_join = '', $add_where = '', $add_groupby = '', $add_orderby = '') {
  global $user;

  if (is_array($mmtid)) $mmtid = 'IN(' . join(',', $mmtid) . ')';
  else $mmtid = "= $mmtid";
  // This includes the hack "n.changed AS created" to allow node_feed() to show
  // the changed date instead of the post date. It is called in _mm_render_pages().
  // Woe unto he who attempts to grok this.
  $now = time();
  $scheduled = "IFNULL((s.publish_on = 0 OR s.publish_on <= $now) AND (s.unpublish_on = 0 OR $now < s.unpublish_on), 1)";
  $inner =
    'FROM {mm_node2tree} t '.
      "INNER JOIN {mm_tree} tr ON tr.mmtid $mmtid ".
      "INNER JOIN {node} n ON t.nid = n.nid AND t.mmtid $mmtid ".
      'LEFT JOIN {mm_node_schedule} s ON s.nid = n.nid ';
  // skip some tests for users with 'administer nodes' permission
  if (!user_access('administer nodes')) {
    $node_writable = $user->uid > 0 ? " OR (n.uid = $user->uid OR gw.uid = $user->uid OR vw.uid = $user->uid)" : '';
    $inner .=
      'LEFT JOIN {mm_node_write} nw ON nw.nid = n.nid '.
      'LEFT JOIN {mm_group} gw ON gw.gid = nw.gid '.
      'LEFT JOIN {mm_virtual_group} vw ON vw.vgid = gw.vgid '.
      "WHERE (n.status = 1 AND $scheduled$node_writable)$add_where ";
  }
  else if ($add_where) {
    // remove AND, OR, etc.
    $add_where = preg_replace('/^\s*(\S+)/', '', $add_where);
    $inner .= "WHERE$add_where ";
  }
  $count_sql =
    'SELECT COUNT(DISTINCT n.nid) '.
      $inner;

  return
    'SELECT n.nid, '.
      'IF(n.set_change_date = 1 AND n.publish_on > 0, n.publish_on, n.changed) '.
        'AS created, '.                                               // creation date
      'n.sticky AND (n.uid = 1 OR n.owns_it OR '.
        'COUNT(v.uid = n.uid OR g.vgid = 0 AND g.uid = n.uid)) AS stuck, '. // node is sticky
      "n.scheduled, n.status$add_select ".
      'FROM '.
        '(SELECT n.*, t.mmtid, s.set_change_date, s.publish_on, '.
          "$scheduled AS scheduled, ".
          '(n.uid = tr.uid) AS owns_it '.                            // entry is owned by node's owner
          $inner .
        'GROUP BY n.nid) AS n '.
      "LEFT JOIN {mm_node_reorder} r ON r.mmtid = n.mmtid AND r.nid = n.nid$add_join ".
      "LEFT JOIN {mm_tree_access} a ON a.mode = 'w' AND a.mmtid = n.mmtid ".
      'LEFT JOIN {mm_group} g ON a.gid = g.gid '.
      'LEFT JOIN {mm_virtual_group} v ON g.vgid = v.vgid '.
        'AND (v.uid = n.uid OR g.vgid = 0 AND g.uid = n.uid) '.
      "GROUP BY n.nid$add_groupby ".
      "ORDER BY stuck DESC, r.weight ASC, created DESC$add_orderby";
}

/**
 * Return whether or not the user has some type of access on a given Drupal
 * node. This takes into account both the permissions on the node itself (for
 * writing) and the permissions of the tree entry it's in (for everything else.)
 *
 * @param $op
 *   The operation to be performed on the node. Possible values are:
 *   - "view"
 *   - "update"
 *   - "delete"
 * @param $node
 *   The node object on which the operation is to be performed.
 * @param $account
 *   The user object on which the operation is to be performed. (optional)
 * @return
 *   TRUE if the operation may be performed.
 */
function mm_content_node_access($op, $node, $account = NULL) {
  global $user;
  static $recursive = FALSE;

  switch ($op) {
    case 'view':
      foreach (mm_module_implements('mm_node_access') as $module) {
        $out = call_user_func_array($module .'_mm_node_access', array($op, $node, $account));
        if (isset($out)) return $out;
      }

      return mm_content_user_can_node($node, 'r', $account);

    case 'update':
    case 'delete':
      if (!$account) $account = $user;

      if (empty($account->uid)) return FALSE;  // failed login

      if ($account->uid == 1) return TRUE;   // admin user

      if (user_access('administer nodes', $account)) return TRUE;

      foreach (mm_module_implements('mm_node_access') as $module) {
        $out = call_user_func_array($module . '_mm_node_access', array($op, $node, $account));
        if (isset($out)) return $out;
      }

      // being called recursively from node_access() call, below
      if ($recursive) return;

      if ($node->uid == $account->uid) {
        // regular owner
        $owns = TRUE;
      }
      else {
        // everyone, or this user is in a matching group
        if ($account->uid > 0) {
          $node_writable = '(nw.gid = %d OR g.uid = %d OR v.uid = %d)';
          $args = array(0, $account->uid, $account->uid);
        }
        else {
          $node_writable = '(nw.gid = %d OR g.uid = %d AND g.vgid = %d)';
          $args = array(0, 0, 0);
        }

        $owns = mm_query_result(
            'SELECT COUNT(DISTINCT nw.gid) FROM {mm_node_write} nw '.
            'LEFT JOIN {mm_group} g ON nw.gid = g.gid '.
            'LEFT JOIN {mm_virtual_group} v ON v.vgid = g.vgid '.
            "WHERE nid = $node->nid AND $node_writable", $args) > 0;
      }

      if ($owns) {
        $recursive = TRUE;
        if (!isset($node->type)) $node = node_load($node->nid);
        $saved_owner = $node->uid;
        $node->uid = $account->uid;
        $ret = node_access($op, $node, user_load($node->uid));
        $node->uid = $saved_owner;
        $recursive = FALSE;
        return $ret;
      }
  }
  return FALSE;
}

/**
 * Flag a virtual group as needing its data regenerated. The actual update of
 * mm_virtual_group happens during monster_menus_cron().
 *
 * @param $mmtid
 *   Tree/Group ID (not virtual group ID!) of the virtual group to be updated.
 *   Omit or set to NULL to mark all groups for update.
 */
function mm_content_update_vgroup_view($mmtid = NULL) {
  if (!isset($mmtid))
    db_query('UPDATE {mm_vgroup_query} SET dirty = %d WHERE dirty = %d', MM_VGROUP_DIRTY_NEXT_CRON, MM_VGROUP_DIRTY_NOT);
  else
    db_query('UPDATE {mm_vgroup_query} q INNER JOIN {mm_group} g ON g.vgid = q.vgid SET q.dirty = %d WHERE g.gid = %d AND dirty = %d', MM_VGROUP_DIRTY_NEXT_CRON, $mmtid, MM_VGROUP_DIRTY_NOT);
}

/**
 * Figure out if a given node is assigned to one or more hidden tree entries
 *
 * @param $nid
 *   The node object or node number being queried
 * @param $all_pages
 *   If TRUE, return TRUE when all pages on which the node appears are hidden.
 *   Otherwise, return TRUE if any page is hidden.
 * @return
 *   TRUE if the node is assigned to one or more hidden tree entries
 */
function mm_content_is_hidden_node($nid, $all_pages = TRUE) {
  if (is_object($nid)) $nid = $nid->nid;

  if ($all_pages) {
    return mm_query_result("SELECT COUNT(*) = SUM(hidden) FROM (SELECT SUM(t.hidden) > %d AS hidden FROM {mm_node2tree} n LEFT JOIN {mm_tree_parents} p ON p.mmtid = n.mmtid LEFT JOIN {mm_tree} t ON t.mmtid = p.parent OR t.mmtid = n.mmtid WHERE nid = %d GROUP BY p.mmtid) AS x", 0, $nid) > 0;
  }

  return mm_query_result("SELECT COUNT(*) FROM {mm_node2tree} n LEFT JOIN {mm_tree_parents} p ON p.mmtid = n.mmtid LEFT JOIN {mm_tree} t ON t.mmtid = p.parent OR t.mmtid = n.mmtid WHERE nid = %d AND t.hidden = %d", $nid, 1) > 0;
}

/**
 * Delete a recycling bin tree node, if it's empty
 *
 * @param $bin
 *   Tree ID of the bin to possibly delete
 * @return
 *   An error message, if an error occurs; otherwise, TRUE if the bin was deleted.
 */
function mm_content_delete_bin($bin) {
  $nodes = mm_content_get_nids_by_mmtid($bin, 1);
  if (count($nodes) == 0) {    // no nodes in the bin itself
    $tree = mm_content_get_tree($bin, array(MM_GET_TREE_DEPTH => 1));
    if (count($tree) == 1 && $tree[0]->name == MM_ENTRY_NAME_RECYCLE) {  // just the empty bin, and it's definitely a bin
      $err = mm_content_delete($bin, FALSE);
      if ($err) return $err;
      return TRUE;
    }
  }
  return FALSE;
}

/**
 * Move content to the recycle bin, creating the bin if needed
 *
 * @param $mmtids
 *   Array, or a single tree ID to move
 * @param $nids
 *   Array, or a single node ID to move
 */
function mm_content_move_to_bin($mmtids = NULL, $nids = NULL) {
  $time = time();   // preset, just in case the clock changes during code below

  if (isset($mmtids)) {    // recycle one or more entries
    if (!is_array($mmtids)) $mmtids = array($mmtids);

    foreach ($mmtids as $mmtid) {
      $bin_parent = mm_content_get_parent($mmtid);
      if (!$bin_parent) return;

      $bin = _mm_content_make_recycle($bin_parent);
      if (!is_numeric($bin)) return $bin;

      if (!($tree = mm_content_get($mmtid))) {
        mm_content_delete_bin($bin);
        return 'error';
      }

      $n = 0;
      $name = $tree->name;
      $alias = $tree->alias;
      while (db_result(db_query("SELECT COUNT(*) FROM {mm_tree} WHERE (name = '%s' OR alias <> '' AND alias = '%s') AND parent = %d", $name, $alias, $bin))) {
        $n++;
        $name = $tree->name . " ($n)";
        $alias = empty($tree->alias) ? '' : $tree->alias . "-$n";
      }
      if ($n)
        mm_content_update_quick(array('name' => $name, 'alias' => $alias), array('mmtid' => $mmtid), $tree->parent);

      $err = mm_content_move($mmtid, $bin, 'recycle');
      if ($err) {
        mm_content_delete_bin($bin);
        return $err;
      }

      db_query("INSERT INTO {mm_recycle} (type, id, bin_mmtid, recycle_date) VALUES('%s', %d, %d, %d)", 'cat', $mmtid, $bin, $time);
    }
  }
  else if (isset($nids)) {    // recycle one or more nodes
    if (!is_array($nids)) $nids = array($nids);

    foreach ($nids as $nid) {
      $from_mmtids = mm_content_get_by_nid($nid);
      if (!count($from_mmtids)) $from_mmtids = array(0);

      foreach ($from_mmtids as $mmtid) {
        $bin = _mm_content_make_recycle($mmtid);
        if (!is_numeric($bin)) return $bin;

        db_query('DELETE FROM {mm_node2tree} WHERE nid = %d AND mmtid = %d', $nid, $mmtid);
        db_query('DELETE FROM {mm_node_reorder} WHERE nid = %d AND mmtid = %d', $nid, $mmtid);
        db_query('INSERT IGNORE INTO {mm_recycle} '.
            '(type, id, bin_mmtid, recycle_date, from_mmtid) '.
            "VALUES('%s', %d, %d, %d, %d)", 'node', $nid, $bin, $time, $mmtid);
        db_query('INSERT INTO {mm_node2tree} (nid, mmtid) VALUES(%d, %d)', $nid, $bin);
        watchdog('mm', "Recycled node=$nid from mmtid=$mmtid to bin=$bin", array());
      }
    }
  }
}

function mm_content_recycle_enabled() {
  return variable_get('mm_recycle_auto_empty', 0) >= 0;
}

function mm_content_is_node_content_block($mmtid) {
  return mm_query_result($mmtid,
      'SELECT COUNT(*) FROM {mm_tree_block} tb INNER JOIN {mm_block} b '.
      'ON tb.bid = b.bid AND b.show_node_contents = %d WHERE tb.mmtid = %d', 1, $mmtid);
}

/**
 * Helper for hook_file_download() implementations. Check MM access to a file
 * download, based upon its path.
 *
 * @param $filepath
 *   Path to file being downloaded
 * @param $node_types
 *   Name or array of names of node types to test access for
 * @return
 *   -1 if access is denied, an array of headers if access is granted. If this
 *   is not a download of a matching node type, return nothing.
 */
function mm_content_check_file_download($filepath, $node_types) {
  $filepath = file_create_path($filepath);
  $result = db_query("SELECT f.*, u.nid FROM {files} f INNER JOIN {upload} u ON f.fid = u.fid WHERE filepath = '%s'", $filepath);
  if ($file = db_fetch_object($result)) {
    if (!is_array($node_types)) $node_types = array($node_types);
    if (($node = node_load($file->nid)) && in_array($node->type, $node_types)) {
      if (mm_content_user_can_node($node, 'r')) {
        return array(
          'Content-Type: ' . $file->filemime,
          'Content-Length: ' . $file->filesize,
          'Pragma: cache',
          'Cache-Control: private',
        );
      }
      else {
        return -1;
      }
    }
  }
}

/**
 * Implementation of hook_views_pre_render()
 *
 * Prevent users from seeing results in views they should not be able to. This
 * is imperfect because it removes data without considering pagination, so
 * result sets have an uneven number of results per page. It also does not
 * consider appearance schedule, so nodes may appear when they should not.
 * This code should be considered mostly as a failsafe, to prevent unwanted data
 * disclosure based on permission.
 */
function monster_menus_views_pre_render($view) {
  $cache = array();
  foreach ($view->result as $index => $result) {
    if (!empty($result->nid)) {
      if (!isset($cache[$result->nid])) $cache[$result->nid] = mm_content_user_can_node($result->nid, 'r');
      if (!$cache[$result->nid]) unset($view->result[$index]);
    }
  }
}

/**
 * Implementation of hook_mm_tree_flags()
 */
function monster_menus_mm_tree_flags() {
  return array(
    'limit_alias' => array('#type' => 'checkbox', '#description' => t('MM: Prevents non-admin users from changing the item\'s alias')),
    'limit_move' => array('#type' => 'checkbox', '#description' => t('MM: Prevents non-admin users from moving the item')),
    'limit_delete' => array('#type' => 'checkbox', '#description' => t('MM: Prevents non-admin users from deleting the item')),
    'limit_hidden' => array('#type' => 'checkbox', '#description' => t('MM: Prevents non-admin users from changing "Don\'t show this page in the menu"')),
    'limit_location' => array('#type' => 'checkbox', '#description' => t('MM: Prevents non-admin users from changing the item\'s location on screen')),
    'limit_name' => array('#type' => 'checkbox', '#description' => t('MM: Prevents non-admin users from changing the item\'s name')),
    'limit_write' => array('#type' => 'checkbox', '#description' => t('MM: Prevents non-admin users from changing "Delete or change settings"')),
    'no_breadcrumb' => array('#type' => 'checkbox', '#description' => t('MM: Prevents the page breadcrumb from showing at this level'), '#flag_inherit' => TRUE),
    'user_home' => array('#type' => 'textfield', '#description' => t('MM: If a user homepage, contains the uid'), '#flag_copy' => FALSE),
  );
}

/**
 * Insert or update an entry's flags.
 *
 * @param $mmtid
 *   The MM tree ID of the entry to change
 * @param $flags
 *   A single string or array of strings in flag => value format to be added to
 *   the entry
 * @param $clear_old
 *   If TRUE, remove any existing flags. Otherwise, add the new flags to the
 *   existing set. If the calling function has only just created the entry, use
 *   FALSE to avoid the overhead of trying to delete flags that aren't even set.
 */
function mm_content_set_flags($mmtid, $flags, $clear_old = TRUE) {
  if ($clear_old) {
    db_query('DELETE FROM {mm_tree_flags} WHERE mmtid = %d', $mmtid);
  }

  if (!empty($flags)) {
    if (!is_array($flags)) $flags = array($flags => '');

    foreach ($flags as $flag => $data)
      db_query("INSERT INTO {mm_tree_flags} (mmtid, flag, data) VALUES(%d, '%s', '%s')", $mmtid, $flag, $data);
  }
}

/**
 * Set the permissions of an item in the MM tree
 *
 * @param $mmtid
 *   The MM tree ID of the entry to change
 * @param $perms
 *   An array of arrays ['r', 'w', 'a', 'u']['groups', 'users']. All are
 *   optional. For 'groups', provide an array of gids; for 'users' an array of
 *   uids.
 * @param $is_group
 *   TRUE if the item is a group
 * @param $clear_old
 *   If TRUE, remove any existing permissions. If the calling function has only
 *   just created the entry, use FALSE to avoid the overhead of trying to delete
 *   permissions that aren't even set.
 */
function mm_content_set_perms($mmtid, $perms, $is_group = FALSE, $clear_old = TRUE) {
  if ($clear_old) {
    // remove ad-hoc groups (gid<0) first
    db_query('DELETE {mm_tree_access}, {mm_group} FROM {mm_tree_access}, {mm_group} '.
        'WHERE {mm_tree_access}.gid = {mm_group}.gid AND {mm_tree_access}.mmtid = %d '.
        'AND {mm_tree_access}.gid < 0', $mmtid);

    // remove the rest
    db_query('DELETE FROM {mm_tree_access} WHERE mmtid = %d', $mmtid);
  }

  foreach (array('w', 'a', 'u', 'r') as $m) {
    if ($is_group && $m == 'u') continue;

    if (isset($perms[$m]) && isset($perms[$m]['groups']))
      foreach ($perms[$m]['groups'] as $gid) {
        if ($gid === 'self') $gid = $mmtid;
        db_query("INSERT INTO {mm_tree_access} (mmtid, gid, mode) VALUES(%d, %d, '%s')", $mmtid, $gid, $m);
      }

    $gid = '';
    if (isset($perms[$m]) && isset($perms[$m]['users']))
      foreach ($perms[$m]['users'] as $uid)
        _mm_content_ad_hoc_group($gid, $uid);

    if ($gid !== '') {
      db_query("INSERT INTO {mm_tree_access} (mmtid, gid, mode) VALUES(%d, %d, '%s')",
          $mmtid, $gid, $m);
    }
  }
}

/**
 * Calculate the likely deletion time of an item in the recycle bin, based on
 * various settings.
 *
 * @param $when
 *   The time when the item was placed into the recycle bin
 * @param $what
 *   Translated string to become part of the result
 * @return
 *   A text string describing when the content will be removed
 */
function mm_content_get_recycle_autodel_time($when, $nid, $mmtids, $what) {
  $run_last = variable_get('mm_cron_run_last', 0);
  $run_since = variable_get('mm_cron_run_since', 0);
  $run_count = variable_get('mm_cron_run_count', 0);
  $interval = variable_get('mm_recycle_auto_empty', 0);
  if (!$run_last || !$run_since || !$run_count || $interval <= 0) return '';

  if (!$when) {
    if (!is_array($mmtids))
      if (!$mmtids) $mmtids = array();
      else $mmtids = array($mmtids);
    if ($nid) $mmtids = array_merge($mmtids, mm_content_get_by_nid($nid));

    $allparents = array();
    foreach ($mmtids as $t)
      if (mm_content_user_can($t, 'IS_RECYCLED')) {
        $allparents = array_merge($allparents, mm_content_get_parents_with_self($t));
      }

    if (!count($allparents)) return '';
    $when = db_result(db_query(
        "SELECT MIN(recycle_date) FROM {mm_recycle} WHERE type = 'cat' AND id IN(" .
        db_placeholders($allparents) . ') AND recycle_date > 0', $allparents));
  }
  if (!$when) return '';

  $avg_run = ($run_last - $run_since) / $run_count;
  $fudge = $avg_run / 100;
  $next_run = $run_last + $avg_run;
  if ($when + $interval - $fudge <= $next_run) $which_run = $next_run;
  else $which_run = intval(($when + $interval - $run_since + $avg_run - 1) / $avg_run) * $avg_run + $run_since;

//   debug_add_dump( "run_last=$run_last", "run_since=$run_since", "run_count=$run_count",
//       "interval=$interval", "avg_run=$avg_run", "next_run=$next_run",
//       "time=".time(), "when=$when", "which_run=$which_run" );
  if (($which_run = $which_run-time()) <= 0) $when = t('very soon');
  else $when = t('in !when', array('!when' => format_interval($which_run)));

  return t('!what will be automatically deleted !when.', array('!what' => $what, '!when' => $when));
}

/**
 * Based on values stored in a node object, set the correct permissions. This
 * function is usually called in hook_nodeapi() during the insert and update
 * phases.
 *
 * @param $node
 *   A node object
 */
function mm_content_set_node_perms($node) {
  if ($node->mm_skip_perms) return;
  $everyone = $node->mm_others_w_force || user_access('administer all menus');
  if ($node->others_w && $everyone) {
    _mm_ui_delete_node_groups($node, TRUE);

    db_query('INSERT INTO {mm_node_write} (nid, gid) VALUES(%d, %d)', $node->nid, 0);
  }
  else {
    _mm_ui_delete_node_groups($node, $everyone);

    if (is_array($node->groups_w)) {
      foreach ($node->groups_w as $gid => $name) {
        if ($gid) {
          db_query('INSERT INTO {mm_node_write} (nid, gid) VALUES(%d, %d)', $node->nid, $gid);
        }
      }
    }

    if (is_array($node->users_w)) {
      $adhoc_gid = '';
      foreach ($node->users_w as $uid => $name) {
        if (!empty($uid)) {
          _mm_content_ad_hoc_group($adhoc_gid, $uid);
        }
      }

      if ($adhoc_gid != '') db_query('INSERT INTO {mm_node_write} (nid, gid) VALUES(%d, %d)', $node->nid, $adhoc_gid);
    }
  }
}

// ****************************************************************************
// * Private functions start here
// ****************************************************************************

/**
 * Create a new ad-hoc group, and add users to it
 *
 * @param $&gid
 *   ID of the group to add to. Before this function is called for the first
 *   time, set the passed parameter to ''. This function creates the group
 *   entry, adds the first user to it, and sets $gid to the ID of the new group.
 *   Later iterations reuse this ID.
 * @param $uid
 *   User ID to store in the group
 */
function _mm_content_ad_hoc_group(&$gid, $uid) {
  if ($gid === '') {   // first time, so get the next gid to use
    db_query('LOCK TABLES {mm_group} WRITE');
    $gid = db_result(db_query('SELECT LEAST(-1, MIN(gid)-1) FROM {mm_group}'));
    if (!$gid) $gid = -1;
    db_query('INSERT INTO {mm_group} (gid, uid) VALUES(%d, %d)', $gid, $uid);
    db_query('UNLOCK TABLES');
  }
  else {
    db_query('INSERT INTO {mm_group} (gid, uid) VALUES(%d, %d)', $gid, $uid);
  }
}

function _mm_content_virtual_dir($mmtid, $par, $level, $state) {
  $ch = chr(-$mmtid);
  if (!ctype_alpha($ch)) {
    $name = function_exists('t') ? t('(other)') : '(other)';
    $alias = '~';
  }
  else $name = $alias = $ch;

  $perms = mm_content_user_can($par);
  $perms['w'] = $perms['u'] = FALSE;
  return (object)array(
    'name' => $name,
    'alias' => $alias,
    'mmtid' => $mmtid,
    'parent' => $par,
    'uid' => 1,
    'default_mode' => 'r',
    'bid' => '',
    'max_depth' => -1,
    'max_parents' => -1,
    'perms' => $perms,
    'level' => $level,
    'is_group' => FALSE,
    'is_user' => TRUE,
    'is_dot' => FALSE,
    'is_virtual' => TRUE,
    'state' => $state
  );
}

/**
 * Create a new recycling bin, or return the tree ID of the existing one
 *
 * @param $mmtid
 *   The tree ID in which to create the recycle bin. If 0, use the user's home
 *   directory is used or, if possible, the top level of the tree.
 * @return
 *   Either the tree ID of the recycling bin or an error string. Use
 *   is_numeric() to evaluate.
 */
function _mm_content_make_recycle($mmtid = 0) {
  global $user;

  if (!$mmtid)
    if ($user->user_mmtid && mm_content_user_can($user->user_mmtid, 'a'))
      $mmtid = $user->user_mmtid;
    else if (mm_content_user_can(mm_home_mmtid(), 'a'))
      $mmtid = mm_home_mmtid();
    else
      return t('Could not create a recycle bin');

  $found = mm_content_get(array('parent' => $mmtid, 'name' => MM_ENTRY_NAME_RECYCLE));
  if ($found) return $found[0]->mmtid;

  return mm_content_insert_or_update(TRUE, $mmtid, array(
    'name' => MM_ENTRY_NAME_RECYCLE,
    'alias' => t('-recycle'),
    'uid' => $user->uid
  ));
}

function _mm_content_comments_readable($node) {
  if (variable_get('mm_finegrain_comment_readability', FALSE)) {
    $perm = empty($node->comments_readable) ? MM_COMMENT_READABILITY_DEFAULT : $node->comments_readable;
    return user_access($perm);
  }
  return user_access('access comments');
}

function _mm_content_insert_cascaded_setting($mmtid, $name, $desc, $array_key, $data) {
  if ($desc['data_type'] == 'int') {
    if (($data = intval($data)) == -1) return;
  }
  else if ($desc['not_empty'] && empty($data)) return;

  if (!$desc['use_keys']) $array_key = '';

  db_query("INSERT INTO {mm_cascaded_settings} (mmtid, name, data_type, multiple, array_key, data) VALUES (%d, '%s', '%s', %d, '%s', '%s')", $mmtid, $name, $desc['data_type'], $desc['multiple'], $array_key, $data);
}
