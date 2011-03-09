<?php
// $Id: mm_browser.inc 5082 2011-02-28 14:29:19Z vquinn $

/**
 * @file
 * Tree browser for Monster Menus
 */

define('MM_BROWSER_MODE_GALLERY', 'gal');
define('MM_BROWSER_MODE_GROUP',   'grp');
define('MM_BROWSER_MODE_MEDIA',   'med');
define('MM_BROWSER_MODE_PAGE',    'pag');
define('MM_BROWSER_MODE_RSS',     'rss');
define('MM_BROWSER_MODE_USER',    'usr');
define('MM_BROWSER_MODE_NODE',    'nod');
define('MM_BROWSER_MODE_MENU',    'men');

function mm_browser_menu() {
  // Tree Browser Specific Items
  $items = array();
  $items['mm-browser'] = array(
    'title' => 'JSON Callback',
    'page callback' => 'mm_browser_json',
    'page arguments' => array(1),
    'access arguments' => array('use tree browser'),
    'file' => 'mm_browser.inc',
    'type' => MENU_CALLBACK);
  $items['mm-browser-load'] = array(
    'title' => 'JSON Callback',
    'page callback' => 'mm_browser_load',
    'access arguments' => array('use tree browser'),
    'file' => 'mm_browser.inc',
    'type' => MENU_CALLBACK);
  $items['mm-browser-getright'] = array(
    'title' => 'JSON Callback',
    'page callback' => 'mm_browser_getright',
    'page arguments' => array(FALSE),
    'access arguments' => array('access content'),
    'file' => 'mm_browser.inc',
    'type' => MENU_CALLBACK);
  $items['mm-browser-bookmark-exists'] = array(
    'title' => 'JSON Callback',
    'page callback' => 'mm_browser_bookmark_exists',
    'page arguments' => array(FALSE),
    'access arguments' => array('use tree browser'),
    'file' => 'mm_browser.inc',
    'type' => MENU_CALLBACK);
  $items['mm-browser-get-bookmarks'] = array(
    'title' => 'JSON Callback',
    'page callback' => 'mm_browser_get_bookmarks',
    'page arguments' => array(FALSE),
    'access callback' => 'user_is_logged_in',
    'file' => 'mm_browser.inc',
    'type' => MENU_CALLBACK);
  $items['mm-browser-get-lastviewed'] = array(
    'title' => 'JSON Callback',
    'page callback' => 'mm_browser_get_lastviewed',
    'access callback' => 'user_is_logged_in',
    'file' => 'mm_browser.inc',
    'type' => MENU_CALLBACK);
  $items['mm-browser-set-lastviewed'] = array(
    'title' => 'JSON Callback',
    'page callback' => 'mm_browser_set_lastviewed',
    'access callback' => 'user_is_logged_in',
    'file' => 'mm_browser.inc',
    'type' => MENU_CALLBACK);
  $items['mm-bookmarks/add'] = array(
    'title' => 'Add Bookmark',
    'page callback' => 'mm_browser_bookmark_add',
    'page arguments' => array(2),
    'access callback' => 'user_is_logged_in',
    'file' => 'mm_browser.inc',
    'type' => MENU_CALLBACK);
  $items['mm-bookmarks/manage'] = array(
    'title' => 'Add Bookmark',
    'page callback' => 'mm_browser_bookmark_manage',
    'page arguments' => array(FALSE),
    'access callback' => 'user_is_logged_in',
    'file' => 'mm_browser.inc',
    'type' => MENU_CALLBACK);
  $items['mm-bookmarks/delete'] = array(
    'title' => 'Delete Bookmark',
    'page callback' => 'mm_browser_bookmark_delete',
    'page arguments' => array(FALSE),
    'access callback' => 'user_is_logged_in',
    'file' => 'mm_browser.inc',
    'type' => MENU_CALLBACK);
  $items['mm-bookmarks/edit'] = array(
    'title' => 'Edit Bookmark',
    'page callback' => 'mm_browser_bookmark_edit',
    'page arguments' => array(FALSE),
    'access callback' => 'user_is_logged_in',
    'file' => 'mm_browser.inc',
    'type' => MENU_CALLBACK);
  $items['mm-bookmarks/sort'] = array(
    'title' => 'Sort Bookmarks',
    'page callback' => 'mm_browser_bookmark_sort',
    'page arguments' => array(FALSE),
    'access callback' => 'user_is_logged_in',
    'file' => 'mm_browser.inc',
    'type' => MENU_CALLBACK);
  return $items;
}

/**
 * Generate a tree browser
 *
 * @param $start
 *   Additional parameters to be passed to the underlying rendering code,
 *   concatenated with '-' as the separator. The current pattern is:
 *       startID-mode-instance-enabled-selectable
 *   -startID:    Tree ID to start the tree with
 *   -mode:       One of the MM_BROWSER_MODE_* constants
 *   -instance:   Numeric instance of the browser on the page
 *   -enabled:    If specified, only allow the user to click on elements for which
 *                they have this type of access. Must be one of 'r', 'w', 'a', 'u'.
 *   -selectable: If specified, only show a 'select' option for elements for which
 *                they have this type of access. Must be one of 'r', 'w', 'a', 'u'.
 * @param $path...
 *   Any remaining parameters are a path describing the pre-selected tree ID
 * @return
 *   The HTML code for the results
 */
function mm_browser_load() {
  $path = func_get_args();
  $start = array_shift($path);
  $params = explode('-', $start);
  if ($path[0] != 1 && $path[0] != $params[0]) {
    array_unshift($path, $params[0]);
  }
  else if (!count($path)) {
    $path[] = 1;
  }

  print theme('mm_page_wrapper',
    t('Tree Browser'),
    mm_browser($path[count($path) - 1], $path[0], $params[1], $params[3], $params[4], $params[5], $params[6]),
    array(
      'id' => 'media-assist-load',
      'class' => 'media-assist'),
    FALSE
  );
  $GLOBALS['devel_shutdown'] = FALSE; // prevent the devel module from outputting
  exit;
}

function mm_browser($selected, $top_mmtid = 1, $mode = MM_BROWSER_MODE_PAGE, $enabled = 'r', $selectable = '', $title = '', $content_type = NULL) {
  global $user;

  if (empty($title)) {
    $titles = array(
      MM_BROWSER_MODE_GALLERY => t('Select a file or page of files'),
      MM_BROWSER_MODE_GROUP =>   t('Select a group'),
      MM_BROWSER_MODE_MEDIA =>   t('Select a file upload'),
      MM_BROWSER_MODE_RSS =>     t('Select a page to display'),
      MM_BROWSER_MODE_USER =>    t('Select a user'),
      MM_BROWSER_MODE_NODE =>    t('Select a piece of content'),
    );
    if (isset($titles[$mode])) $title = $titles[$mode];
    else $title = t('Select a page');
  }

  mm_dep_add_js_dep_path();
  if (!$top_mmtid) $top_mmtid = 1;
  drupal_add_js(array(
    'browserDots'       => $mode == MM_BROWSER_MODE_GROUP || user_access('administer all menus') || user_access('view all menus'),
    'browserEnabled'    => $enabled,
    'browserMode'       => $mode,
    'browserSelectable' => $selectable,
    'browserTop'        => $top_mmtid,
    'lastBrowserPath'   => _mm_browser_get_relative_path(_mm_browser_get_lastviewed($mode), $top_mmtid),
    'startBrowserPath'  => _mm_browser_get_relative_path($selected, $top_mmtid),
  ), 'setting');
  if ($content_type != NULL) {
    drupal_add_js(array('browserContentType' => $content_type), 'setting');
  }
/****  Use this code to jump to the last viewed location
  if ($last_mmtid = _mm_browser_get_lastviewed()) {
    drupal_add_js(array('lastBrowserPath' => mm_content_get_full_path($last_mmtid)), 'setting');
  }
****/
  mm_static('back_in_history', TRUE);
  drupal_add_js(drupal_get_path('module', 'monster_menus') . '/mm_browser.js');
  jquery_ui_add('ui.core');
  mm_dep_add_libs('jsTree');
  mm_dep_add_libs('fg.menu');
  mm_dep_add_libs('splitter');
  drupal_add_css(drupal_get_path('module', 'monster_menus') . '/mm_browser.css');

  $out = '<div id="mmtree-browse"><div id="mmtree-browse-nav">';
  $out .= mm_browser_getURLs($mode, $top_mmtid);
  $out .= '<h2 class="mmtree-assist-title">' . $title . '</h2></div>';
  $out .= '<div id="mmtree-browse-browser">';
  $out .= '<div id="mmtree-browse-tree" class="resizable-frame"></div>';
  $out .= '<div id="mmtree-browse-items"></div>';
  $out .= '</div></div>';
  return $out;
}

/*
 *  Create the list of URLs at the top of the tree browser.
 */
function mm_browser_getURLs($mode, $top_mmtid) {
  global $user, $base_url;

  $urls = '<ul class="dropdown">';
  $allowed_top = $mode == MM_BROWSER_MODE_GROUP ? mm_content_groups_mmtid() : 1;

  if ($top_mmtid != $allowed_top) {
    $urls .= '<li><a href="#" onclick="Drupal.mm_browser_goto_top(\'' . $allowed_top . '\');" class="fg-button fg-button-icon-right ui-widget ui-state-default ui-corner-all">' . t('View entire tree') . '</a></li>';
  }

  if ($mode != MM_BROWSER_MODE_GROUP && is_numeric($user->user_file_mmtid) && $user->user_file_mmtid > 0) {
    $urls .= '<li><a href="#" onclick="Drupal.mm_browser_reload_data(\'' . mm_content_get_full_path($user->user_file_mmtid) . '\');" class="fg-button fg-button-icon-right ui-widget ui-state-default ui-corner-all" id="my-uploaded-files-link">' . t('My uploaded files') . '</a></li>';
  }
  $urls .= '<li><span id="last-viewed"><a href="#" onclick="Drupal.mm_browser_last_viewed();" id="last-viewed-link" class="fg-button fg-button-icon-right ui-widget ui-state-default ui-corner-all">' . t('Last location') . '</a></span></li>';
  $urls .= join('', mm_module_invoke_all('mm_browser_navigation', $mode, $top_mmtid));

  // Display bookmarks.
  $urls .= '<li id="bookmarks-link"><a href="#" class="fg-button fg-button-icon-right ui-widget ui-state-default ui-corner-all mm-browser-button">' . t('Bookmarks') . ' <img src="' . $base_url . '/misc/menu-expanded.png" alt="" /></a><div class="mm-browser-button-list" class="hidden"><ul>';
  $urls .= '<li><a href="#" onclick="Drupal.mm_browser_manage_bookmarks();">' . t('Organize Bookmarks...') . '</a></li><li><hr /></li><span class="bm-list">';

  $result = _mm_browser_get_bookmarks($mode);
  while ($row = db_fetch_array($result)) {
    $ds_data = unserialize($row['data']);
    $urls .= '<li class="' . $ds_data['mmtid'] . '"><a href="#" onclick="Drupal.mm_browser_reload_data(\'' . _mm_browser_get_relative_path($ds_data['mmtid'], $top_mmtid) . "');\">" . check_plain($ds_data['title']) . '</a></li>';
  }
  $urls .= '</span>';
  $urls .= '</ul></div></li>';
  $urls .= '</ul>';
  return $urls;
}

function mm_browser_json() {
  $term_ids = func_get_args();
  $num_ids = func_num_args();
  $mmtid = intval(substr($_GET['id'], 5));

  if ($mmtid) {
    $mm_children = mm_browser_get_tree($num_ids, $mmtid, 0, 'single');
  }
  else {
    $num_ids--;
    $mm_children = mm_browser_get_tree($num_ids, $term_ids, 1, 'multi');
  }
  drupal_set_header('Pragma: no-cache');
  drupal_json($mm_children);
  $GLOBALS['devel_shutdown'] = FALSE; // prevent the devel module from outputting
  exit();
}

function mm_browser_get_tree(&$num_ids, &$term_ids, $index, $level) {
  $mm_children = array();
  $mmtid = is_array($term_ids) ? $term_ids[$index] : $term_ids;
  if (!$mmtid) return $mm_children;

  $params = array(
    MM_GET_TREE_ADD_TO_CACHE =>     TRUE,
    MM_GET_TREE_FILTER_BINS =>      FALSE,
    MM_GET_TREE_FILTER_DOTS =>      $_GET['browserDots'] == 'true',
    MM_GET_TREE_FILTER_HIDDEN =>    TRUE,
    MM_GET_TREE_DEPTH =>            1,
    MM_GET_TREE_RETURN_KID_COUNT => TRUE,
    MM_GET_TREE_RETURN_PERMS =>     TRUE,
  );
  switch ($_GET['browserMode']) {
    case MM_BROWSER_MODE_GALLERY:
      $params[MM_GET_TREE_ADD_SELECT] = "(SELECT COUNT(DISTINCT n.nid) FROM {mm_node2tree} n INNER JOIN {media_files} mf ON mf.nid = n.nid WHERE n.mmtid = t.mmtid AND mf.filemime LIKE 'image%') AS nodecount";
      $params[MM_GET_TREE_FILTER_NORMAL] = $params[MM_GET_TREE_FILTER_USERS] = TRUE;
      break;

    case MM_BROWSER_MODE_NODE:
      $item_fields = content_fields($_GET['browserContentType']);
      $allowed_node_types = array();
      foreach ($item_fields['mm_list_nodetypes'] as $node_type) {
        if ($node_type !== 0) {
          $allowed_node_types[] = "'" . $node_type . "'";
        }
      }
      $wheres = (count($allowed_node_types) != 0) ? "AND nd.type IN (" . implode(', ', $allowed_node_types) . ") " : "";
      $wheres .= "AND nd.status = 1";
      $params[MM_GET_TREE_ADD_SELECT] = "(SELECT COUNT(DISTINCT n.nid) FROM {mm_node2tree} n INNER JOIN {node} nd ON nd.nid = n.nid WHERE n.mmtid = t.mmtid {$wheres}) AS nodecount ";
      $params[MM_GET_TREE_FILTER_NORMAL] = $params[MM_GET_TREE_FILTER_USERS] = TRUE;
      break;

    case MM_BROWSER_MODE_GROUP:
      $params[MM_GET_TREE_FILTER_GROUPS] = TRUE;
      break;

    case MM_BROWSER_MODE_MEDIA:
      $params[MM_GET_TREE_RETURN_NODE_COUNT] = 'media';
      $params[MM_GET_TREE_FILTER_NORMAL] = $params[MM_GET_TREE_FILTER_USERS] = TRUE;
      break;

    case MM_BROWSER_MODE_MENU:
      $params[MM_GET_TREE_RETURN_BLOCK] = TRUE;
      $params[MM_GET_TREE_BLOCK] = 1;
      break;

    default:
    case MM_BROWSER_MODE_PAGE:
    case MM_BROWSER_MODE_RSS:
      $params[MM_GET_TREE_FILTER_NORMAL] = $params[MM_GET_TREE_FILTER_USERS] = TRUE;
      break;

    case MM_BROWSER_MODE_USER:
      $params[MM_GET_TREE_FILTER_USERS] = TRUE;
      break;
  }

  $list = mm_content_get_tree($mmtid, $params);

  $root = array_shift($list);
  foreach ($list as $item) {
    $class = $children = array();
    $state = $item->state & MM_GET_TREE_STATE_LEAF ? 'leaf' : 'closed';
    $attributes = array('id' => 'mmbr-' . $item->mmtid);

    $name = mm_content_expand_name($item->name);
    $hidden = $item->state & MM_GET_TREE_STATE_HIDDEN;
    if ($hidden) {
      $name .= ' ' . t('(hidden)');
      $class['mmtree-hidden'] = 1;
    }
    $data = array('title' => $name);
    if ($item->nodecount > 0) {
      $data['title'] = t('!name <span class="mmtree-browse-filecount">!count</span>', array('!name' => $name, '!count' => format_plural($item->nodecount, '(1 item)', '(@count items)')));
    }

    if ($_GET['browserMode'] == MM_BROWSER_MODE_MENU) {
      $path = 'mm/' . $item->mmtid;
      $options = NULL;
      custom_url_rewrite_outbound($path, $options, NULL);
      $data['path'] = $path;
      $data['mmtid'] = $item->mmtid;
      $parent = db_result(db_query("SELECT parent from {mm_tree} WHERE mmtid = %d", $mmtid));
      $data['parent'] = $parent;
    }

    if ($state == 'leaf') {
      $class['leaf'] = 1;
    }

    $denied = $item->state & MM_GET_TREE_STATE_DENIED || !empty($_GET['browserEnabled']) && !$item->perms[$_GET['browserEnabled']];
    if ($denied) {
      $class['disabled'] = $class['leaf'] = 1;
      $state = 'leaf';
    }
    else if ($level != 'single' && $term_ids[$index + 1] == $item->mmtid) {
      $state = 'open';
      $children = mm_browser_get_tree($num_ids, $term_ids, $index + 1, $level);
    }
    
    $attributes['class'] = $data['attr']['class'] = join(' ', array_keys($class));

    $mm_children[] = array(
      'attr' =>     $attributes,
      'children' => $children,
      'data' =>     $data,
      'state' =>    $state,
    );
  }
  return $mm_children;
}

function mm_browser_getright() {
  function _mm_browser_choice($item, $cat, $usr, $group) {
    if ($item->is_user) return $usr;
    if ($item->is_group) return $group;
    return $cat;
  }

  $mmtid = intval(substr($_GET['id'], 5));
  if (!$mmtid) {
    print theme('mm_page_wrapper', '', '');
    $GLOBALS['devel_shutdown'] = FALSE; // prevent the devel module from outputting
    exit();
  }

  $mode = $_GET['browserMode'];
  $is_gallery = $mode == MM_BROWSER_MODE_GALLERY;
  $is_node_viewer = $mode == MM_BROWSER_MODE_NODE;
  $name = mm_content_get_name($mmtid);
  $close = '<a href="#" onclick="' . ($mode == MM_BROWSER_MODE_MEDIA ? 'parent.window.close();' : 'parent.parent.tb_remove();') . '">' . t('Close window') . '</a>';
  drupal_add_css(drupal_get_path('module', 'monster_menus') . '/mm_browser.css');

  $bookmark = '';
  if (!mm_browser_is_bookmarked($mmtid, $_GET['browserMode'])) {
    $bookmark = '<a href="#" id="add-bookmark-link" title="' . t('Bookmark this location') . '" onclick="parent.Drupal.mm_browser_add_bookmark_tb(' . $mmtid . "); this.blur();\">" . t('Bookmark') . '</a>';
  }

  $content = '';
  $links = array();
  $item = mm_content_get($mmtid, MM_GET_FLAGS);
  $perms = $item->perms = mm_content_user_can($mmtid);

  if ($is_node_viewer) { // Node View
    // This retrieves the attributes of the content type
    $slider_fields = content_fields($_GET['browserContentType']);
    if (!$perms[$slider_fields['mm_list_selectable']]) {
      if ($mmtid > 0) {
        $out .= '<div id="mmtree-browse-thumbnails">' . t('<p>You do not have permission to use the content on this page.</p>');
        $item_fields = content_fields($_GET['browserContentType']);
        $options = array(
          'w' => t('delete it or change its settings'),
          'a' => t('append subpages to it'),
          'u' => t('add content to it'),
          'r' => t('read it'));
        $out .= t('<p>To use content from this page, you must be able to @do.</p>', array('@do' => $options[$item_fields['mm_list_selectable']])) . '</div>';
      }
      $header = '<div id="mmtree-browse-header"><h4 class="mmtree-assist-title">' . $name . '</h4>' . $close . '</div><br /><hr />';
      print theme('mm_page_wrapper', t('Tree Browser'), $header . $out);
      $GLOBALS['devel_shutdown'] = FALSE; // prevent the devel module from outputting
      exit();
    }
    $allowed_node_types = array();
    foreach ($slider_fields['mm_list_nodetypes'] as $node_type) {
      if ($node_type !== 0) {
        $allowed_node_types[] = "'" . $node_type . "'";
      }
    }
    $wheres = (count($allowed_node_types) != 0) ? "AND n.type IN (" . implode(', ', $allowed_node_types) . ") " : "";
    $wheres .= "AND n.status = 1";
    $query =
      'SELECT n.nid, n.title, n.type, n.changed, t.name FROM {node} n ' .
      'LEFT JOIN {node_type} t ON n.type = t.type ' .
      'WHERE nid IN (' .
      'SELECT m.nid FROM {mm_node2tree} m, {node} n ' .
      "WHERE m.mmtid = '%s' AND m.nid = n.nid) " .
      $wheres;
    $limit = variable_get('mm_nodelist_pager_limit', 20);
    $header = array(
      array('data' => t('Type'), 'field' => 'n.type'),
      array('data' => t('Title'), 'field' => 'n.title'),
      array('data' => t('Last Modified'), 'field' => 'n.changed', 'sort' => 'desc'),
                    );
    $tablesort = tablesort_sql($header);
    $result = pager_query($query . $tablesort, $limit, 0, NULL, $mmtid);

    $num_query =
      'SELECT COUNT(*) FROM {node} n ' .
      'LEFT JOIN {node_type} t ON n.type = t.type ' .
      'WHERE nid IN (' .
      'SELECT m.nid FROM {mm_node2tree} m, {node} n ' .
      "WHERE m.mmtid = %d AND m.nid = n.nid) " .
      $wheres;
    $num_results = db_result(db_query($num_query, $mmtid));
    if ($num_results == 0) {
      $content = t('There is no selectable content on this page.');
    }
    else {
      $rows = array();
      while ($node = db_fetch_object($result)) {
        $row = array();
        if ($node->type == 'media') {
          $fileinfo = db_query("SELECT DISTINCT uploadedfilename, filemime, filepath FROM {media_files} WHERE nid = %d", $node->nid);
          $file = db_fetch_object($fileinfo);
          $media = media_icon_display($node->nid, 'thumbnail', $file->filemime, $file->filepath, $file->uploadedfilename, $node->title, array('no_resize' => TRUE));
          $node->uploadedfilename = $file->uploadedfilename;
          $mime = $file->filemime;
          $kind = media_get_media_type($mime);
          $node->name = $node->name . ' - ' . $kind;
          $node->title = empty($node->title) ? check_plain($node->uploadedfilename) : $node->title;
          $onclick = "javascript:parent.Drupal.mm_browser_nodepicker_add({$mmtid}, '" . mm_ui_js_escape($node->title) . "', {$node->nid});";
          $link = '<div class="mmtree-browse-imagesingle ui-state-default ui-widget-content"><div class="mmtree-browse-caption">' . $node->title . '</div><div class="mmtree-browse-imagepreview" onclick="' . $onclick . '">' . $media . '</div></div>';
        }
        else {
          $node->subtype = '';
          $link = l($node->title, '#', array('attributes' => array('onclick' => "javascript:parent.Drupal.mm_browser_nodepicker_add({$mmtid}, '" . mm_ui_js_escape($node->title) . "', {$node->nid});"), 'external' => TRUE));
        }
        $row[] = check_plain($node->name);
        $row[] = $link;
        $row[] = format_date($node->changed, 'custom', 'M j, Y g:i A');
        $rows[] = $row;
      }
      $content = theme('table', $header, $rows);
      $content .= theme('pager', NULL, $limit, 0);
      if (!$content) {
        $content = t('<p>There are no nodes on this page.</p>');
      }

      // Select all link currently disabled. May be nice to enable at a later date,
      //  but I couldn't think of a way for it to not be confusing. For example, if
      //  the max allowed nodes set in the cck config mm_list node is 3, then we could theoretically
      //  allow the user to "Select all" on a page with less than 3 nodes. However, the way it is
      //  currently architected, there is no way to prevent the user from selecting a single
      //  node for each of the first 3 picks, then a select all nodes on a page with 3 nodes,
      //  thereby selecting 5 total nodes. Anyway, I couldn't think of a way to make this
      //  not confusing for the user so I disabled it, but am leaving it here in case anyone
      //  can think of a way to make it not confusing in the future.
      // It also has the problem that a user could select a page which, at the time of selection
      //  had less than 3 nodes, but was later populated to have > 3 thereby overloading.
      //  Not any graceful way to prevent this, so it sits disabled. -VJQ 2/25/2011
      // $links['all'] = "<a href=\"javascript:parent.Drupal.mm_browser_gallery_add($mmtid,'" . mm_ui_js_escape($name) . ' ' . t('(all)') . "');\" title=\"" . t('Select all current and future files on this page') . '">' . t('Select all') . '</a>';
      if ($bookmark) $links['bkmark'] = $bookmark;
      jquery_ui_add('ui.core');
    }
  } // End Node View
  else if ($is_gallery || $mode == MM_BROWSER_MODE_MEDIA) { // Gallery View
    if (!$perms['u']) {
      if ($mmtid > 0) {
        $out .= '<div id="mmtree-browse-thumbnails">' . t('You do not have permission to use the file uploads on this page.') . '</div>';
      }
      $header = '<div id="mmtree-browse-header"><h4 class="mmtree-assist-title">' . $name . '</h4>' . $close . '</div><br /><hr />';
      print theme('mm_page_wrapper', t('Tree Browser'), $header . $out);
      $GLOBALS['devel_shutdown'] = FALSE; // prevent the devel module from outputting
      exit();
    }

    $and = $is_gallery ?
      "f.filename = 'icon' AND f.filemime LIKE 'image/%'" :
      "((f.filename = 'original' AND f.filemime NOT LIKE 'image/%') " .
        "OR (f.filename = 'icon' AND f.filemime LIKE 'image/%') " .
        "OR (f.filename = 'original' AND f.filemime LIKE 'image/%' AND f.nid NOT IN (" .
          'SELECT nid FROM {media_files} fi ' .
          "WHERE fi.nid = f.nid AND fi.filemime LIKE 'image/%' AND fi.filename = 'icon')))";
    $query =
      'SELECT f.*, nn.title AS title FROM {media_files} f, {node} nn ' .
      'WHERE f.nid = nn.nid AND f.nid IN (' .
        'SELECT m.nid FROM {mm_node2tree} m, {node} n ' .
        "WHERE m.mmtid = %d AND m.nid = n.nid AND n.type='media'" .
      ") AND $and ORDER BY f.mfid DESC";
    $result = db_query($query, $mmtid);
    while ($file = db_fetch_object($result)) {
      $media = media_icon_display($file->nid, 'thumbnail', $file->filemime, $file->filepath, $file->uploadedfilename, $file->title, array());
      $link = $is_gallery ?
        "javascript:parent.Drupal.mm_browser_gallery_add($mmtid, '" . mm_ui_js_escape($file->uploadedfilename) . "', $file->nid);" :
        "javascript:parent.Drupal.mm_browser_change_parent_url('" . url("media_assist/properties/$file->nid") . "');";

      $title = empty($file->title) ? $file->uploadedfilename : $file->title;
      $content .= '<div class="mmtree-browse-imagesingle ui-state-default ui-widget-content"><div class="mmtree-browse-caption">' . $title . '</div><div class="mmtree-browse-imagepreview" onclick="' . $link . '">' . $media . '</div></div>';
    }

    if (!$content) {
      $content = t('<p>There are no file uploads on this page.</p>');
    }

    $content = '<div id="mmtree-browse-thumbnails">' . $content .'</div>';

    if ($is_gallery) {
      $links['all'] = "<a href=\"javascript:parent.Drupal.mm_browser_gallery_add($mmtid,'" . mm_ui_js_escape($name) . ' ' . t('(all)') . "');\" title=\"" . t('Select all current and future files on this page') . '">' . t('Select all') . '</a>';
      if ($bookmark) $links['bkmark'] = $bookmark;
    }
    else {
      if ($bookmark) $links['bkmark'] = $bookmark;
    }

    jquery_ui_add('ui.core');
  } // End Gallery View
  else { // Non-media modes
    if (($mode == MM_BROWSER_MODE_PAGE || $mode == MM_BROWSER_MODE_RSS || $mode == MM_BROWSER_MODE_GROUP) && !$item->is_virtual) {
      $user_can = TRUE;
      if (drupal_strlen($_GET['browserSelectable'])) {
        $user_can = FALSE;
        foreach (str_split($_GET['browserSelectable']) as $check) {
          if ($perms[$check]) {
            $user_can = TRUE;
            break;
          }
        }
      }

      if ($user_can) {
        $js_parms = 0;
        if ($mode == MM_BROWSER_MODE_GROUP) {
          $users = mm_content_get_users_in_group($mmtid, '<br />', FALSE, 20, TRUE);
          if ($users == '') $users = t('(none)');
          $js_parms = "'" . mm_ui_js_escape($users) . "'";
        }

        $links['select'] = "<a href=\"javascript:parent.Drupal.mm_browser_page_add($mmtid,$js_parms);\">" . t('Select') . '</a>';
      }

      if (!$item->is_virtual) {
        $content = '<table>';

        if (!$item->is_group) {
          $content .= '<tr><td align="right"><b>' . t('URL alias:') . '</b>&nbsp;</td><td>' . check_plain($item->alias) . '</td></tr>';
        }

        if (isset($item->nodecount)) {
          $content .= '<tr><td align="right"><b>' . t('Items on page:') . '</b>&nbsp;</td><td>' . $item->nodecount . '</td></tr>';
        }

        $content .= '<tr><td align="right"><b>' . t('Owner:') . '</b>&nbsp;</td><td>' . mm_content_uid2name($item->uid) . '</td></tr>';

        $can = array();
        foreach (array(
          'w' => _mm_browser_choice($item, t('delete/edit page'),
              t('delete/edit user'), t('delete/edit group')),
          'a' => _mm_browser_choice($item, t('add sub-pages'),
              $item->name == MM_ENTRY_NAME_USERS ? t('add users') : t('add sub-pages'),
              t('add sub-groups')),
          'u' => $item->is_group ? t('apply this group'): t('assign content to page'),
          'r' => $item->is_group ? t('see group\'s members') : t('read contents of page')) as $type => $desc) {

          if ($perms[$type]) $can[] = $desc;
        }

        $content .= '<tr valign="top"><td align="right"><b>' . t('You can:') . '</b>&nbsp;</td><td>' . implode('<br />', $can) . '</td></tr>';

        if ($users) {
          $content .= '<tr valign="top"><td align="right"><b>' . t('Users in group:') . '</b>&nbsp;</td><td>' . $users . '</td></tr>';
        }

        $content .= '</table>';
      }
    }

    if ($bookmark) $links['bkmark'] = $bookmark;
  }
  $links['close'] = $close;
  mm_module_invoke_all_array('mm_browser_links_alter', array($mode, &$links, $item));

  $header = '<div id="mmtree-browse-header"><h4 class="mmtree-assist-title">' . $name . '</h4>' . implode($links, ' | ') . '</div><br /><hr />';
  $output = theme('mm_page_wrapper', t('Tree Browser'), $header . $content);
  print str_replace('<body', '<body onclick="parent.Drupal.mm_browser_close_menus();"', $output);
  // Remove old jQuery that is part of Drupal
//  print preg_replace('{<script type="text/javascript" src="[^"]*?jquery[^"]*?"></script>}', '', $output);
  $GLOBALS['devel_shutdown'] = FALSE; // prevent the devel module from outputting
  exit();
}

// Bookmark Specific Functions
function mm_browser_bookmark_add() {
  global $base_url, $user;

  $args = func_get_args();
  $mmtid = $args[1];
  $name = mm_content_get_name($mmtid);

  if (mm_browser_is_bookmarked($mmtid, $_GET['browserMode'])) {
    $output = '<link rel="stylesheet" href="' . base_path() . drupal_get_path('module', 'monster_menus') . '/mm_browser_tb.css" />';
    $output .= '<script type="text/javascript" src="' . $base_url . '/misc/jquery.js"></script>';
    $output .= '<script type="text/javascript" src="' . $base_url . '/sites/all/modules/thickbox/thickbox.js"></script>';
    $output .= '<body id="tb-body"><p>' . t('This location is already bookmarked.') . '</p>';
    $output .= '<a href="#" onclick="parent.tb_remove();">' . t('Close window') . '</a></body></html>';
  }
  else if (!empty($_POST)) {
    $mm_bookmark_serialized = serialize(array('title' => $_POST['linktitle'], 'mmtid' => $_POST['linkmmtid']));
    $type = _mm_browser_get_bookmarks_type($_GET['browserMode']);
    db_lock_table('mm_tree_bookmarks');
    $max_weight = db_result(db_query("SELECT IFNULL(MAX(weight), -1) + 1 FROM {mm_tree_bookmarks} WHERE uid = %d AND type = '%s'", $user->uid, $type));
    db_query("INSERT INTO {mm_tree_bookmarks} (uid, weight, type, data) VALUES(%d, %d, '%s', '%s')", $user->uid, $max_weight, $type, $mm_bookmark_serialized);
    db_unlock_tables();
  }
  else {
    $output = mm_static('mm_browser_bookmark_add', FALSE, $name, $mmtid);
  }

  print($output);
  $GLOBALS['devel_shutdown'] = FALSE; // prevent the devel module from outputting
  exit();
}

// Bookmark Specific Functions
function mm_browser_bookmark_manage() {
  global $user;

  $num_rows = db_result(db_query("SELECT COUNT(data) FROM {mm_tree_bookmarks} WHERE uid = %d AND type = '%s'", $user->uid, _mm_browser_get_bookmarks_type($_GET['browserMode'])));

  if ($num_rows < 1) {
    $output = '<div id="message"><p>' . t('No bookmarks found.') . '</p><p><a href="#" onclick="window.parent.tb_remove(); return false;">' . t('Continue') . '</a></p></div>';
  }
  else {
    drupal_add_css(drupal_get_path('module', 'monster_menus') . '/mm_browser_tb.css');
    mm_static('mm_browser_bookmark_manage', TRUE);
    jquery_ui_add(array('ui.widget', 'ui.mouse', 'ui.sortable'));
    $output = '<div><form id="manage-bookmarks-form"><div id="manage-bookmarks-div"><ul id="sortable">';

    $result = _mm_browser_get_bookmarks($_GET['browserMode']);
    while ($row = db_fetch_array($result)) {
      $ds_data = unserialize($row['data']);
      $escaped_title = mm_ui_js_escape($ds_data['title']);
      $output .= '<li class="ui-state-default" id="li_' . $ds_data['mmtid'] . '" name="' . $ds_data['mmtid'] . '"><table class="manage-bookmarks-table"><tr id=' . $ds_data['mmtid'] . '><td class="tb-manage-name"><span class="ui-icon ui-icon-arrowthick-2-n-s"></span>' . check_plain($ds_data['title']) . '</td><td><a href="#" onclick="return window.parent.Drupal.mm_browser_delete_bookmark_confirm(' . $ds_data['mmtid'] . ', \'' . $escaped_title . '\', document)">' . t('Delete') . '</a></td><td><a href="#" onclick="return window.parent.Drupal.mm_browser_edit_bookmark_edit(' . $ds_data['mmtid'] . ', \'' . $escaped_title . '\', document)">' . t('Edit') . '</a></td></tr></table></li>';
    }
    $output .= '</ul></div></form>';
  }

  print(theme('mm_page_wrapper', '', $output, array('id' => 'tb-manage-body')));
  $GLOBALS['devel_shutdown'] = FALSE; // prevent the devel module from outputting
  exit();
}

// Check to ensure a bookmark does not already exist prior to adding it.
function mm_browser_is_bookmarked($mmtid, $mode) {
  $already_exists = FALSE;
  $result = _mm_browser_get_bookmarks($mode);
  while ($row = db_fetch_object($result)) {
    $ds_data = unserialize($row->data);
    if ($ds_data['mmtid'] == $mmtid) {
      $already_exists = TRUE;
      break;
    }
  }
  return $already_exists;
}

// Quick check to see if bookmark exists with JSON return.
function mm_browser_bookmark_exists() {
  drupal_json(array('exists' => mm_browser_is_bookmarked($_GET['id'], $_GET['browserMode'])));
  $GLOBALS['devel_shutdown'] = FALSE; // prevent the devel module from outputting
  exit();
}

// Delete the Posted bookmark
function mm_browser_bookmark_delete() {
  global $user;

  $mmtid = $_POST['mmtid'];

  $result = _mm_browser_get_bookmarks($_GET['browserMode']);
  while ($row = db_fetch_array($result)) {
    $ds_data = unserialize($row['data']);
    if ($ds_data['mmtid'] == $mmtid) {
      db_query("DELETE FROM {mm_tree_bookmarks} WHERE bmid = %d AND uid = %d", $row['bmid'], $user->uid);
    }
  }
  drupal_json(array('mmtid' => $mmtid));
  $GLOBALS['devel_shutdown'] = FALSE; // prevent the devel module from outputting
  exit();
}

// Edit the posted bookmark.
function mm_browser_bookmark_edit() {
  global $user;

  $mmtid = $_POST['mmtid'];
  $title = $_POST['title'];

  $result = _mm_browser_get_bookmarks($_GET['browserMode']);
  while ($row = db_fetch_array($result)) {
    $ds_data = unserialize($row['data']);
    if ($ds_data["mmtid"] == $mmtid) {
      $ds_data['title'] = $title;
      $ds_done = serialize($ds_data);
      db_query("UPDATE {mm_tree_bookmarks} SET data = '%s' WHERE bmid = %d AND uid = %d", $ds_done, $row['bmid'], $user->uid);
    }
  }
  drupal_json(array('title' => $title, 'mmtid' => $mmtid));
  $GLOBALS['devel_shutdown'] = FALSE; // prevent the devel module from outputting
  exit();
}

// Edit the posted bookmark.
function mm_browser_bookmark_sort() {
  global $user;

  $neworder = explode('|', $_POST['neworder']);
  $index = 0;

  $result = _mm_browser_get_bookmarks($_GET['browserMode']);
  while ($row = db_fetch_array($result)) {
    $ds_data = unserialize($row['data']);

    for ($i = 0; $i < count($neworder); $i++) {
      if ($ds_data['mmtid'] == $neworder[$i]) {
        db_query('UPDATE {mm_tree_bookmarks} SET weight = %d WHERE bmid = %d AND uid = %d', $i + 1, $row['bmid'], $user->uid);
      }
    }
  }
  drupal_json(array('neworder' => $neworder));
  $GLOBALS['devel_shutdown'] = FALSE; // prevent the devel module from outputting
  exit();
}

function mm_browser_get_lastviewed() {
  if ($last_mmtid = _mm_browser_get_lastviewed($_GET['browserMode'])) {
    drupal_json(array('path' => _mm_browser_get_relative_path($last_mmtid, $_GET['browserTop'])));
  }
  if (isset($_GET['id'])) mm_browser_set_lastviewed(intval(substr($_GET['id'], 5)));
  $GLOBALS['devel_shutdown'] = FALSE; // prevent the devel module from outputting
  exit();
}

function _mm_browser_get_relative_path($mmtid, $top) {
  $path = mm_content_get_full_path($mmtid);
  if (preg_match('{\b' . $top . '(/|$)(.*)}', $path, $matches)) return $matches[0];
  return $path;
}

function _mm_browser_get_lastviewed($mode) {
  global $user;

  return db_result(db_query("SELECT data FROM {mm_tree_bookmarks} WHERE uid = %d AND type = '%s'", $user->uid, _mm_browser_get_bookmarks_type($mode) . '_last'));
}

function mm_browser_set_lastviewed($mmtid) {
  global $user;

  if ($mmtid) {
    $type = _mm_browser_get_bookmarks_type($_GET['browserMode']) . '_last';
    db_query("UPDATE {mm_tree_bookmarks} SET data = '%s' WHERE uid = %d AND type = '%s'", $mmtid, $user->uid, $type);
    if (!db_affected_rows()) {
      db_query("INSERT INTO {mm_tree_bookmarks} (uid, weight, type, data) VALUES(%d, %d, '%s', '%s')", $user->uid, 0, $type, $mmtid);
    }
  }
}

function _mm_browser_get_bookmarks($mode) {
  global $user;

  return db_query("SELECT * FROM {mm_tree_bookmarks} WHERE uid = %d AND type = '%s' ORDER BY weight ASC", $user->uid, _mm_browser_get_bookmarks_type($mode));
}

function _mm_browser_get_bookmarks_type($mode) {
  return $mode == MM_BROWSER_MODE_GROUP ? 'browser_grp' : 'browser';
}

function mm_browser_get_bookmarks() {
  // Display bookmarks.
  $bookmarks = array();
  $result = _mm_browser_get_bookmarks($_GET['browserMode']);
  while ($row = db_fetch_array($result)) {
    $ds_data = unserialize($row['data']);
    $bookmarks[] = array($ds_data['mmtid'], $ds_data['title'], _mm_browser_get_relative_path($ds_data['mmtid'], $_GET['browserTop']));
  }
  drupal_json($bookmarks);
  $GLOBALS['devel_shutdown'] = FALSE; // prevent the devel module from outputting
  exit();
}
