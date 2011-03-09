<?php
// $Id: misc.inc 4973 2011-01-26 21:12:56Z dan $

/**
 * @file
 * Miscellaneous MM functions
 */

define('MM_NODE_INFO_NO_RENDER',  'no_render');  // don't render this node type
define('MM_NODE_INFO_ADD_HIDDEN', 'add_hidden'); // hide the Add node link for this type

define('MM_HOME_MMTID_DEFAULT', 7);
define('MM_MENU_BID', 1); // block ID (bid) containing the page section title

/**
 * Get a standard "Access Denied" message
 *
 * @return
 *   The message
 */
function mm_access_denied() {
  global $user;

  drupal_set_header('HTTP/1.0 403 Forbidden');

  $path403 = '';
  // First, try the original 403 location, saved by the httpauth module
  if (module_exists('httpauth')) {
    $path403 = drupal_get_normal_path(variable_get('httpauth_site_403', ''));
  }

  // Failing that, read the Drupal one
  if (empty($path403)) {
    $path403 = variable_get('site_403', '');
    if (empty($path403)) return t('<h2>Password Required</h2>');
  }

  if (!function_exists('menu_set_active_item')) {
    require_once './includes/bootstrap.inc';
    drupal_bootstrap(DRUPAL_BOOTSTRAP_FULL);
  }

  menu_set_active_item($path403);
  return menu_execute_active_handler($path403);
}

/**
 * Generate a node containing the standard "Access Denied" message
 *
 * @param $node
 *   The node object, whose teaser or body is modified to contain the message
 * @param $teaser
 *   TRUE if the node's teaser should contain the message, otherwise the body
 *   is set
 */
function mm_node_load_fail(&$node, $teaser=FALSE) {
  $node->content = array();
  if ($teaser) {
    $node->content['teaser']['#value'] = mm_access_denied();
    $node->content['body']['#value'] = '';
  }
  else {
    $node->content['teaser']['#value'] = '';
    $node->content['body']['#value'] = mm_access_denied();
  }
  $node->no_attribution = TRUE;
  $node->title = '';
}

/**
 * Retrieve a string of autocomplete suggestions for existing users
 */
function mm_autocomplete($want_username, $string = '', $misc = NULL) {
  $limit = 15;
  $min_string = 2;

  $matches = array();
  $too_short = array('' => t('Please type some more characters'));

  $string = trim($string);
  if (!empty($string)) {
    $hook = mm_module_implements('mm_autocomplete_alter');
    if ($hook) {
      $function = $hook[0] . '_mm_autocomplete_alter';
      $result = $function($string, $limit, $min_string, $misc);
      if (empty($result)) {
        $matches = $too_short;
      }
    }
    else if (drupal_strlen($string) >= $min_string) {
      // Consider Anonymous and Administrator first
      for ($i = 0; $i <= 1; $i++) {
        $name = mm_content_uid2name($i);
        if (($pos = stristr($name, $string)) !== FALSE) {
          $stmt = "(SELECT $i, '', '', '$name', '', '', '', 0, 0, 0) UNION ";
          if (!$pos) $startswith .= $stmt;
          else $contains .= $stmt;
        }
      }
      $status_limit = user_access('administer all users') ? '' : ' status = 1 AND';
      $contains = preg_replace('/ UNION $/', '', $contains);
      $result = db_query(
          $startswith .
          $contains .
          "(SELECT uid, '' AS pref_fml, '' AS pref_lfm, '' AS lastname, '' AS firstname, name, '' AS middlename ".
            "FROM {users} WHERE$status_limit uid > 0 AND name LIKE '%s%%' ".
            'ORDER BY name) UNION '.
          "(SELECT uid, '' AS pref_fml, '' AS pref_lfm, '' AS lastname, '' AS firstname, name, '' AS middlename ".
            "FROM {users} WHERE$status_limit uid > 0 AND name LIKE '%%_%s%%' ".
            'ORDER BY name) '.
          'LIMIT %d',
          $string, $string, $limit + 1);
    }
    else $matches = $too_short;

    while ($user = db_fetch_object($result))
      if (count($matches) == $limit) {
        $matches[''] = '...';
        break;
      }
      else {
        $name = check_plain(mm_content_uid2name($user->uid, 'lfmu', $user));
        if (!$want_username) $matches[$user->uid . '-' . $name] = $name;
        else if ($user->name) $matches[$user->name] = $name;
      }
  }

  drupal_set_header('Content-Type: text/plain');
  print drupal_to_js($matches);
  $GLOBALS['devel_shutdown'] = FALSE;
  exit();
}

/**
 * Redirect the user to a URL, while checking for possible recursion
 *
 * @param $url
 *   URL to redirect to
 * @param $query
 *   Optional query fragment
 * @param $hash
 *   Optional anchor to appear in the URL after '#'
 */
function mm_goto($url, $query = '', $hash = NULL) {
  $u = $url;
  custom_url_rewrite_outbound($u, $dummy, $url);
  if ($url == $_GET['q'] || $u == $_GET['q'] || mm_home_path() . "/$u" == $_GET['q']) {
    watchdog('mm', 'Recursive redirect: page=%page',
        array('%page' => $_GET['q']), WATCHDOG_ERROR, l(t('view'), $_GET['q']));

    menu_set_active_item('');
    drupal_set_title(t('Error'));
    print theme('page', t('This page tried to send you into an endless loop. Please contact the administrator, and let him or her know how you got here. Press your browser\'s Back button to return to the page you came from.'));
    return;
  }
  drupal_goto($url, $query, $hash);
}

/**
 * Return an <img> tag, or URL to an image, containing a user's email address in
 * a form not easily grabbed by spambots
 *
 * @param $string
 *   The text to encode
 * @param $alt
 *   The alternate text portion of the <img> tag
 * @param $url_only
 *   If TRUE, return the URL, rather than the <img> tag
 * @return
 *   Either the <img> tag or the URL
 */
function mm_text2image($string, $alt, $url_only = FALSE) {
  $md5 = md5($string);
  $_SESSION['mm_txtimg'][$md5] = $string;
  $url = "mm-txtimg/$md5/x.jpg";
  if ($url_only) return $url;
  return '<img ' . drupal_attributes(array('src' => url($url), 'alt' => $alt)) . '>';
}

/**
 * Return the actual image data containing a user's email address in a form not
 * easily grabbed by spambots. A given image may only be accessed once per
 * session.
 *
 * @param $md5
 *   If set, and there is a corresponding string set by mm_text2image(), output
 *   the image data
 */
function mm_text2image_img($md5 = NULL) {
  if (isset($md5) && isset($_SESSION['mm_txtimg'][$md5])) {
    $GLOBALS['devel_shutdown'] = TRUE;
    list($fonts, $images) = mm_dep_add_libs('php-captcha');
    $string = $_SESSION['mm_txtimg'][$md5];
    $cap = new AmhCaptcha($fonts);
    $cap->SetBackgroundImages($images);
    $cap->SetMinFontSize(10);
    $cap->SetMaxFontSize(10);
    $cap->Create($string);
    unset($_SESSION['mm_txtimg'][$md5]);
  }
  else drupal_not_found();
}

/**
 * Parse GET parameters in URL
 *
 * @param &$mmtids
 *   Array to receive the list of tree IDs
 * @param &$oarg_list
 *   Optional array to receive the parameters following the tree IDs, un-parsed
 * @param &$this_mmtid
 *   Optional variable to receive the last tree ID (that of the current page)
 * @param $url
 *   URL to parse; defaults to the current page URL
 * @return
 *   The first GET parameter, usually 'mm'
 */
function mm_parse_args(&$mmtids, &$oarg_list = NULL, &$this_mmtid = NULL, $url = NULL) {
  if (is_null($url)) $url = $_GET['q'];

  $this_mmtid = NULL;
  $mmtids = explode('/', $url);
  $oarg_list = array();
  if (($out = array_shift($mmtids)) == 'mm') {       // skip 'mm'
    for ($i = 0; $i < count($mmtids); $i++)
      if (!is_numeric($mmtids[$i])) {
        $oarg_list = array_splice($mmtids, $i);
        break;
      }

    if (count($mmtids)) {
      $this_mmtid = $mmtids[count($mmtids) - 1];
      $parents = mm_content_get_parents($this_mmtid);
      array_shift($parents);    // skip root node
      array_splice($mmtids, 0, -1, $parents);  // insert parents
    }
  }
  else {
    $oarg_list = $mmtids;
    $mmtids = array();
  }
  return $out;
}

/**
 * Get the internal URI of the homepage
 *
 * @return
 *   The URI
 */
function mm_home_path() {
  return 'mm/' . mm_home_mmtid();
}

/**
 * Get the MM Tree ID of the homepage
 *
 * @return
 *   The MM Tree ID
 */
function mm_home_mmtid() {
  return variable_get('mm_home_mmtid', MM_HOME_MMTID_DEFAULT);
}

/**
 * Show a page, based on the GET parameters in the URL
 *
 * @return
 *   HTML code for the page
 */
function mm_show_page() {
  $output = '';

  mm_parse_args($mmtids, $oarg_list, $this_mmtid);
  if (!count($mmtids)) $mmtids = array($this_mmtid = mm_home_mmtid());
  $perms = mm_content_user_can($this_mmtid);

  $output = _mm_render_pages($mmtids, $oarg_list, $err);
  $links = array();

  if ($perms['IS_RECYCLED']) {
    if ($perms['IS_RECYCLE_BIN']) {
      $recyc_msg = t('The contents below are in the recycle bin.');
    }
    else {
      $recyc_msg = t('This page is in the recycle bin.');

      if (mm_content_recycle_enabled()) {
        $when = db_result(db_query("SELECT recycle_date FROM {mm_recycle} WHERE type = 'cat' AND id = %d", $this_mmtid));
        $recyc_msg .= mm_content_get_recycle_autodel_time($when, NULL, $this_mmtid, t(' It'));
      }

      if ($perms['w'])
        if (count($mmtids) >= 2 && mm_content_user_can($mmtids[count($mmtids)-2], 'IS_RECYCLE_BIN')) {
          $recyc_msg .= t(' You can restore it using the %settings tab.', array('%settings' => t('Settings')));
        }
        else {
          foreach (array_reverse($mmtids) as $t)
            if (mm_content_user_can($t, 'IS_RECYCLE_BIN')) {
              if ($last_t && ($tree = mm_content_get($last_t)))
                $pg = l(mm_content_expand_name($tree->name), mm_content_get_mmtid_url($last_t));
              break;
            }
            else $last_t = $t;

          $recyc_msg .= t('<p>This page cannot be restored by itself. You must restore the topmost parent page in the recycle bin, !page.</p>', array('!page' => $pg));
        }
    }
  }

  if (!$err) {
    if ($perms['IS_RECYCLED']) drupal_set_message($recyc_msg);
  }
  else if ($err == 'no read') {
    $output .= $perms['IS_GROUP'] ? t('You do not have permission to see the members of this group.') : mm_access_denied();
  }
  else {    // $err=='no content'
    if (!$perms['IS_GROUP'] && !variable_get('mm_hide_empty_pages', FALSE)) {
      global $user;
      $list = array();
      $entry = mm_content_get($this_mmtid, array(MM_GET_FLAGS, MM_GET_PARENTS));
      if ($perms['IS_USER'] && isset($entry->flags['user_home'])) {
        if ($entry->flags['user_home'] == $user->uid) {
          $list[0] = variable_get('mm_default_homepage', '');
        }
        if (empty($list[0])) {
          $list[0] = t('<h2>Welcome</h2><p>This is a personal homepage that has not been modified yet.</p>');
        }
      }
      else if ($perms['IS_RECYCLE_BIN'])
        $list[0] = ' ';
      else
        $list[0] = t('<p>This page does not yet have any content.</p>');

      $entry->perms = $perms;
      mm_module_invoke_all_array('mm_empty_page_alter', array($entry, &$list));
      $nada = join('', $list);
    }
    else $nada = ' ';

    if ($perms['u'] && !$perms['IS_RECYCLED'] && !$perms['IS_GROUP']) {
      $link = "mm/$this_mmtid/contents/add";
      if (!$perms['IS_GROUP'] && !variable_get('mm_hide_empty_pages', FALSE)) {
        $links[] = array(
          'title' => t('Add content'),
          'href' => $link,
        );
      }
      else {
        mm_goto($link);
        return;
      }
    }
    else if ($perms['IS_GROUP']) {
      $users = mm_content_get_users_in_group($this_mmtid, NULL, FALSE, 100, TRUE);
      $node = (object)array();

      if (!count($users)) $node->title = t('There are no users in this group.');
      else if ($users[''] == '...') {
        $node->title = t('A partial list of users in this group:');
      }
      else {
        $node->title = t('All users in this group:');
      }

      $node->body = join('<br />', $users);
      $node->type = 'story';
      $output = theme('node', $node, FALSE, TRUE);
    }
    else if ($perms['w'] || $perms['a'])
      if (!$perms['IS_GROUP'] && !variable_get('mm_hide_empty_pages', FALSE)) {
        if (!$perms['IS_RECYCLED'])
          $nada .= t('<p>You do not have permission to add content, however you can use the %settings tab to make changes to the page itself.</p>', array('%settings' => t('Settings')));
      }
      else {
        mm_goto("mm/$this_mmtid/settings");
        return;
      }

    if ($perms['IS_RECYCLED'])
      if ($perms['IS_RECYCLE_BIN'])
        drupal_set_message(t('Select an item in the recycle bin from the menu.'));
      else
        drupal_set_message($recyc_msg);

    if ($output == '') {
      if (!isset($node)) $node = (object)array();
      $node->body = $nada;
      $node->links = $links;
      unset($links);
      $node->type = 'story';
      $output = theme('node', $node, FALSE, TRUE);
    }
  }   // $err=='no content'

  if ($links) {
    $output .= theme('links', $links);
  }

  return $output;
}

function mm_show_group($this_mmtid) {
  drupal_set_header('Content-Type: text/html');
  print '<pre>' . mm_content_get_users_in_group($this_mmtid, "\n", FALSE, 0) . '</pre>';
  $GLOBALS['devel_shutdown'] = FALSE; // prevent the devel module from outputting
  exit();
}

/**
 * Return a list of node types for which a flag is TRUE in the
 * hook_mm_node_info()
 *
 * @param $option
 *   The flag to test. Should be one of the MM_NODE_INFO_* constants.
 * @return
 *   An array of node types
 */
function mm_get_node_info($option = NULL) {
  static $list;

  if (!is_array($list)) {
    $list = array();
    foreach (module_invoke_all('mm_node_info') as $node_type => $data)
      foreach ($data as $field => $value)
        if ($value === TRUE)
          $list[$field][] = $node_type;
  }
  if (empty($option)) return $list;
  return isset($list[$option]) ? $list[$option] : array();
}

/**
 * Set the theme based on the page being displayed
 */
function mm_set_custom_theme() {
  global $custom_theme;

  mm_parse_args($mmtids, $oarg_list);
  if (count($mmtids)) {
    $place = db_placeholders($mmtids);
    if ($theme = db_result(db_query_range('SELECT theme FROM {mm_tree} WHERE mmtid IN(' . $place . ") AND theme <> '' ORDER BY LENGTH(sort_idx) DESC", $mmtids, 0, 1))) {
      $custom_theme = $theme;
    }
  }
}

/**
 * When the requested URL is not found, evaluate it and try to come up with some
 * possible guesses as to what the user mis-typed, or to where the intended page
 * has moved.
 *
 * @return
 *   HTML code containing the possible guesses. If an empty string is returned,
 *   the calling code should generate a default "page not found" message.
 */
function mm_get_detailed_404() {
  global $user;

  $max_results = 10;
  $skipped = FALSE;

  $bad_path = explode('/', $_REQUEST['q']);
  if ($site_404 = variable_get('site_404', '')) {
    custom_url_rewrite_inbound($site_404, $site_404, NULL);
  }

  $new_path = '';
  custom_url_rewrite_inbound($new_path, implode('/', $bad_path), NULL);
  if ($new_path == $site_404) return '';

  // Set the custom_theme to that of the 404 page, instead of the original page
  mm_set_custom_theme();

  $home_mmtid = mm_home_mmtid();
  $new_path = explode('/', $new_path);
  if ($new_path[0] == 'mm') {
    $found_mmtid = $new_path[1];
    $new_path = array_slice($new_path, 2);
  }
  else {
    $found_mmtid = $home_mmtid;
  }

  $mtime = 0;
  $output_list = array();
  $prefix_single = t('<p>The page you are looking for appears to have moved to: ');
  while ($child = array_shift($new_path)) {
    $use_mtime = $mtime > 0 ? ' AND mtime <= %d' : '';
    if (db_result(db_query("SELECT COUNT(*) FROM {mm_tree_revisions} r LEFT JOIN {mm_tree} t ON t.mmtid = r.mmtid WHERE r.parent = %d AND r.alias = '%s' AND t.mmtid IS NULL", $found_mmtid, $child))) {
      $prefix_single = t('<p>The page you requested has been permanently deleted.');
      if (!count($output_list) && $found_mmtid != $home_mmtid) {
        $output_list[0] = $found_mmtid;
      }
      if (count($output_list)) {
        $prefix_single .= t(' You may be able to find what you were looking for here: ');
      }
      else {
        return $prefix_single;
      }
    }
    else if ($row = db_fetch_object(db_query("SELECT mmtid, mtime FROM {mm_tree_revisions} WHERE parent = %d AND alias = '%s' AND LEFT(name, 1) <> '%s'$use_mtime ORDER BY vid DESC", $found_mmtid, $child, '.', $mtime))) {
      if (!mm_content_user_can($row->mmtid, 'r')) {
        $output_list = array();
        $skipped = TRUE;
        break;
      }
      $output_list[0] = $found_mmtid = $row->mmtid;
      $mtime = $row->mtime;
    }
    else {
      $output_list = array();
      // Use the SQL SOUNDEX() function instead of the PHP soundex() version,
      // because they produce different values
      $soundex = _mm_soundex($child);
      $soundex_short = substr($soundex, 1, 3);
      $queries = array(
        // hook_menu() paths
        'hook' => array("SELECT * FROM {menu_router} WHERE SUBSTR(path, 1, 4) <> '%s' AND (SUBSTRING_INDEX(path, '/', 1) = '%s' OR SOUNDEX(SUBSTRING_INDEX(path, '/', 1)) = '%s') ORDER BY fit DESC", 'mm/%', $child, $soundex),
        // Direct alias match at the correct level of the tree
        'p+a+' => array("t.parent = %d AND t.alias = '%s'", $found_mmtid, $child),
        // Alias starts with the string, at the correct level
        'p+a*' => array("t.parent = %d AND t.alias LIKE '%s'", $found_mmtid, $child . '%'),
        // Alias sounds like the string, at the correct level
        'p+as' => array("t.parent = %d AND SOUNDEX(t.alias) = '%s'", $found_mmtid, $soundex),
        // Alias matches at any level
        'p-a+' => array("t.alias = '%s'", $child),
        // Alias sounds like the string, at any level
        'p-as' => array("t.parent <> %d AND SOUNDEX(t.alias) = '%s'", $found_mmtid, $soundex),
        // Alias sounds like the string, at any level, using minimal match
        'p<as' => array("t.parent <> %d AND SUBSTR(SOUNDEX(t.alias), 2, 3) = '%s'", $found_mmtid, $soundex_short),
      );
      $soundex_bad = FALSE;
      foreach ($queries as $index => $params) {
        if ($index == 'p+as') {
          // See if there is enough variance in the soundex value, by checking
          // the number of consonants and the frequency of resulting digits.
          $len_test = preg_replace('/[^bcdfghjklmnpqrstvwxyz]/i', '', $child);
          if (strlen($len_test) < 3 || strlen(count_chars($soundex, 3)) / strlen($soundex) <= 2/3) {
            $soundex_bad = TRUE;
            continue;
          }
        }
        else if ($index == 'p-a+') {
          if (count($output_list) == 1) {
            $found_mmtid = $output_list[0];
            break;
          }
        }
        else if ($index == 'p-as' || $index == 'p<as') {
          // This is the last-ditch effort, only if everything before has
          // failed, and the soundex value is sufficiently unique
          if (count($output_list) || $soundex_bad) break;
          $max_results = 5;
        }
        else if ($index == 'hook') {
          // Search in the hook_menu() list for a close match, using just the
          // first element of the path. Only do this when MM found no match at
          // all.
          if ($found_mmtid != $home_mmtid) continue;

          $test_path = $new_path;
          array_unshift($test_path, $child);
          $query = array_shift($params);
          $results = db_query($query, $params);
          while ($menu = db_fetch_object($results)) {
            $matched = TRUE;
            $match = array();
            foreach (explode('/', $menu->path) as $path_index => $elem) {
              if ($path_index >= count($test_path)) {
                $matched = FALSE;
                break;
              }

              if ($elem == '%' || strcasecmp($test_path[$path_index], $elem) == 0) {
                $match[] = $test_path[$path_index];
              }
              else if (_mm_soundex($test_path[$path_index]) == _mm_soundex($elem)) {
                $match[] = $elem;
              }
              else {
                $matched = FALSE;
                break;
              }
            }

            if ($matched) {
              $item = (array)$menu;
              _menu_translate($item, $match, TRUE);
              if ($item['access']) {
                $output_list[] = implode('/', $match);
              }
            }
          }

          if (count($output_list)) {
            // Stop looking.
            $new_path = array();
          }
          continue;   // Go to next query.
        }

        $where = array_shift($params);
        // There's no need to do matches against mm_tree, since its data is
        // duplicated in mm_tree_revisions in the most recent revision.
        // Don't match items or parents with a name starting with '.'.
        $query = "SELECT DISTINCT t.mmtid FROM {mm_tree_revisions} t WHERE $where AND LEFT(t.name, 1) <> '%s' AND (SELECT COUNT(*) FROM {mm_tree} t2 INNER JOIN {mm_tree_parents} p ON p.parent = t2.mmtid WHERE p.mmtid = t.mmtid AND LEFT(t2.name, 1) = '%s') = 0";
        $params[] = '.';
        $params[] = '.';

        // Skip mmtids already found
        if (count($output_list)) {
          $query .= ' AND t.mmtid NOT IN (' . db_placeholders($output_list) . ')';
          $params = array_merge($params, $output_list);
        }

        $results = db_query($query . ' ORDER BY t.vid DESC', $params);
        while (count($output_list) < $max_results && ($item = db_fetch_object($results)))
          if (mm_content_user_can($item->mmtid, 'r')) {
            $output_list[] = $item->mmtid;
          }
          else {
            $skipped = TRUE;
          }

        if (count($output_list) >= $max_results) break;
      }

      if (!$output_list) {
        if ($found_mmtid == $home_mmtid) return '';
        $output_list[0] = $found_mmtid;
        $prefix_single = t('<p>The page you are looking for was not found, but you might be able to find it here: ');
        break;
      }

      $prefix_single = t('<p>This page might be what you are looking for: ');
      $prefix_many = t('<p>One of these pages might be what you are looking for:<ul><li>');

      if (count($output_list) != 1) break;
    }
  }

  if (count($output_list) == 1 && is_numeric($output_list[0]) && mm_content_is_recycled($output_list[0])) {
    // We'll only get here if the page is readable by the user
    return t('<p>The page you requested has been marked for future deletion. Therefore, it is no longer accessible.</p>');
  }

  $options = array();
  foreach ($output_list as $index => $elem) {
    $path = $path0 = 'mm/' . $elem;
    custom_url_rewrite_outbound($path, $options, NULL);
    if ($path == $path0) {
      unset($output_list[$index]);
    }
    else {
      $output_list[$index] = l(base_path() . $path, $path, array('attributes' => array('rel' => 'nofollow')));
    }
  }

  $post = !$user->uid && $skipped ? t('<p>You might get more results if you log-in.</p>') : '';
  if (count($output_list) == 1) {
    return $prefix_single . $output_list[0] . '</p>' . $post;
  }

  if (count($output_list) > 0) {
    return $prefix_many . join('</li><li>', $output_list) . '</li></ul></p>' . $post;
  }

  return !$user->uid && $skipped ? t('This page may be able to provide you with suggestions as to where to find what you are looking for, but you need to log-in first.') : '';
}

/**
 * Redirect the user to an MM URL based on a Drupal node
 *
 * @param $nid
 *   Drupal node ID to redirect to
 * @param $add
 *   Path elements to add after the last MM tree ID. If used, must start with '/'.
 * @param $hash
 *   Optional anchor to appear in the URL after '#'
 */
function mm_redirect_to_node($nid, $add = NULL, $hash = NULL) {
  if ($mmtid = db_result(db_query_range('SELECT mmtid FROM {mm_node2tree} WHERE nid = %d', $nid, 0, 1))) {
    mm_redirect_to_mmtid($mmtid, $add, $hash);
  }
}

/**
 * Redirect the user to an MM URL based on an MM tree ID
 *
 * @param $mmtid
 *   Tree ID to redirect to
 * @param $add
 *   Path elements to add after the last MM tree ID. If used, must start with '/'.
 * @param $hash
 *   Optional anchor to appear in the URL after '#'
 */
function mm_redirect_to_mmtid($mmtid, $add = NULL, $hash = NULL) {
  mm_goto("mm/$mmtid$add", NULL, $hash);
}

/**
 * Add MM group-based roles to user objects being loaded
 */
function mm_set_user_roles(&$account) {
  global $user;

  $added_role = FALSE;
  $q = mm_query(
    'SELECT rid, name FROM (SELECT r2.rid, r.name, (COALESCE(SUM(v.uid = %d OR g.vgid = 0 AND g.uid = %d), 0) > 0) <> r2.negative AS ok FROM {mm_role2group} r2 INNER JOIN {role} r ON r.rid = r2.rid INNER JOIN {mm_group} g ON g.gid = r2.gid LEFT JOIN {mm_virtual_group} v ON g.vgid = v.vgid GROUP BY r2.rid) AS s WHERE s.ok', $account->uid, $account->uid);
  while ($r = $q->next()) {
    if (!isset($account->roles[$r->rid])) {
      $account->roles[$r->rid] = $r->name;
      $added_role = TRUE;
    }
  }

  if ($account->uid == $user->uid && $added_role) {
    $user->roles = $account->roles;
    if (function_exists('user_access')) {
      // Clear out the user_access cache
      user_access('', NULL, TRUE);
    }
  }
}

/**
 * Update the table containing all results of virtual group queries.
 */
function mm_regenerate_vgroup() {
  // delete entries for groups that no longer exist
  db_query('DELETE FROM v USING {mm_virtual_group} v LEFT JOIN {mm_vgroup_query} q ON v.vgid=q.vgid WHERE q.vgid IS NULL');

  $vgids = $email_errors = array();
  // split the queries up into chunks of mm_vgroup_regen_chunk, so that the SQL
  // buffer size isn't exceeded
  $chunksize = variable_get('mm_vgroup_regen_chunk', 15);
  $chunks_per_run = variable_get('mm_vgroup_regen_chunks_per_run', 50);
  for ($chunk = 0; $chunk < $chunks_per_run; $chunk++) {
    $i = $chunksize * $chunk;
    $list = array();
    // dirty==MM_VGROUP_DIRTY_FAILED means there was a previous sanity error, so
    // ignore it now
    $result = db_query_range('SELECT * FROM {mm_vgroup_query} WHERE dirty IN(%d, %d)', MM_VGROUP_DIRTY_NEXT_CRON, MM_VGROUP_DIRTY_REDO, $i, $chunksize);
    $nrows = 0;
    $chunk_vgids = array();
    while ($r = db_fetch_object($result)) {
      $list[] = "(SELECT $r->vgid, $r->field $r->qfrom)";
      $vgids[] = $r->vgid;
      $chunk_vgids[] = $r->vgid;
      $nrows++;
    }

    if (!$nrows) break;

    if (!$created) {
      // db_query_temporary() isn't flexible enough, because we need an index
      @db_drop_table($create_errs, 'mm_virtual_group_temp');
      db_create_table($create_errs, 'mm_virtual_group_temp', drupal_get_schema('mm_virtual_group'));

      if (!$create_errs[1]['success']) {
        watchdog('mm', 'Could not create table mm_virtual_group_temp',
            array(), WATCHDOG_ERROR);
        return;
      }

      $created = TRUE;
    }

    db_query('INSERT INTO {mm_virtual_group_temp} (vgid, uid) ' . join(' UNION ', $list));
    // Delete uids that no longer exist in users table
    db_query('DELETE vg FROM {mm_virtual_group_temp} vg LEFT JOIN {users} u ON u.uid = vg.uid WHERE u.uid IS NULL');

    $in = join(',', $chunk_vgids);
    $result = db_query('SELECT vgid1 AS vgid, gid, orig_count, IFNULL(temp_count, 0) AS temp_count ' .
      'FROM (' .
        'SELECT * FROM (' .
          'SELECT vgid AS vgid1, COUNT(*) AS orig_count ' .
            'FROM {mm_virtual_group} ' .
            'WHERE vgid IN(' . $in . ') ' .
            'GROUP BY vgid) ' .
          'AS t1 ' .
        'LEFT JOIN (' .
          'SELECT vgid AS vgid2, COUNT(*) AS temp_count ' .
            'FROM {mm_virtual_group_temp} ' .
            'WHERE vgid IN(' . $in . ') ' .
            'GROUP BY vgid) ' .
          'AS t2 ' .
        'ON vgid2 = vgid1 ' .
        'WHERE IFNULL(temp_count, 0) < orig_count AND (orig_count - IFNULL(temp_count, 0)) / orig_count > ' . MM_VGROUP_COUNT_SANITY . ' AND (temp_count IS NULL OR temp_count >= 10)) ' .
      'AS insane ' .
      'INNER JOIN {mm_vgroup_query} vg ON vg.vgid = insane.vgid1 ' .
      'LEFT JOIN {mm_group} g ON g.vgid = insane.vgid1 ' .
      'WHERE vg.dirty <> %d', MM_VGROUP_DIRTY_REDO);
    while ($r = db_fetch_object($result)) {
      $tree = mm_content_get($r->gid);
      $msg = t('The size of the virtual group with vgid=!vgid, gid=!gid, name=!name went down by more than !pct%. It went from !orig to !new users. To ignore this condition and regenerate the virtual group anyway, set its "dirty" field to !redo.', array('!vgid' => $r->vgid, '!gid' => $r->gid, '!name' => $tree->name, '!pct' => MM_VGROUP_COUNT_SANITY*100, '!orig' => $r->orig_count, '!new' => $r->temp_count, '!redo' => MM_VGROUP_DIRTY_REDO));
      $email_errors[$r->vgid] = $msg;
      watchdog('mm', $msg, array(), WATCHDOG_ERROR);
      // Set dirty to MM_VGROUP_DIRTY_FAILED so that the same error is not
      // logged repeatedly
      db_query('UPDATE {mm_vgroup_query} SET dirty = %d WHERE vgid = %d', MM_VGROUP_DIRTY_FAILED, $r->vgid);
      // Don't copy data from temp to real table for this vgid
      $vgid_index = array_search($r->vgid, $vgids);
      if ($vgid_index !== FALSE) array_splice($vgids, $vgid_index, 1);
    }

    if ($nrows < $chunksize) break;
  }

  if (count($vgids)) {
    // the anonymous user can never be in any groups
    db_query('DELETE FROM {mm_virtual_group_temp} WHERE uid = 0');

    // update the preview column for mm_content_get_users_in_group()
    $result = db_query('SELECT vgid FROM {mm_virtual_group_temp} GROUP BY vgid');
    while ($r = db_fetch_object($result)) {
      db_query('SET @i=0');
      $query =
          'UPDATE {mm_virtual_group_temp} t '.
          'INNER JOIN (' .
            'SELECT v.uid, (@i:=@i+1) AS ind FROM {mm_virtual_group_temp} v '.
              'INNER JOIN {users} u ON v.vgid = %d AND u.uid = v.uid '.
            'ORDER BY u.name' .
          ') AS j '.
          'ON j.uid = t.uid AND t.vgid = %d SET preview = j.ind';
      mm_module_invoke_all_array('mm_regenerate_vgroup_preview_alter', array(&$query));
      db_query($query, $r->vgid, $r->vgid);
    }
    $in = join(',', $vgids);
    db_query('INSERT INTO {mm_virtual_group} '.
             '(SELECT t.vgid, t.uid, t.preview '.
             'FROM {mm_virtual_group_temp} t LEFT JOIN {mm_virtual_group} o '.
             'ON t.vgid = o.vgid AND t.uid = o.uid '.
             'WHERE o.uid IS NULL)') &&
      db_query('UPDATE {mm_virtual_group} v '.
               'INNER JOIN {mm_virtual_group_temp} n ON '.
               'v.vgid = n.vgid AND v.uid = n.uid '.
               'SET v.preview = n.preview') &&
      db_query('DELETE o FROM {mm_virtual_group} o '.
               'LEFT JOIN {mm_virtual_group_temp} t '.
               'ON t.vgid = o.vgid AND t.uid = o.uid '.
               "WHERE t.uid IS NULL AND o.vgid IN($in)") &&
      db_query('UPDATE {mm_vgroup_query} '.
               "SET dirty = %d WHERE vgid IN($in)", MM_VGROUP_DIRTY_NOT);
  }

  if ($email_errors) {
    $in = join(',', array_keys($email_errors));
    $sql = "\n\nTo ignore the errors and regenerate the data in all of the listed virtual groups, execute this SQL code:\n\nUPDATE mm_vgroup_query SET dirty=" . MM_VGROUP_DIRTY_REDO . " WHERE vgid IN($in)";
    drupal_mail_send(array('id' => 'mm_regenerate_vgroup', 'to' => variable_get('academics_sync_reports_to_email', ''),
      'subject' => t('Error in virtual group regeneration'), 'body' => join("\n\n", array_values($email_errors)) . $sql, 'headers' => array()));
  }

  if ($created) db_drop_table($create_errs, 'mm_virtual_group_temp');
  db_query('OPTIMIZE TABLE {mm_virtual_group}');

  return 'Virtual groups have been regenerated.';
}

/**
 * @defgroup mm_hooks Monster Menus Hooks
 * @{
 * Allow modules to interact with Monster Menus.
 */

/**
 * Get a list of all enabled modules and MM sub-modules that implement a hook.
 *
 * @param $hook
 *   The name of the hook to query
 * @return
 *   An array of module names
 */
function mm_module_implements($hook) {
  $list = module_implements($hook);
  foreach (monster_menus_node_info() as $type => $desc)
    if (function_exists($desc['module'] . '_' . $hook)) $list[] = $desc['module'];
  return $list;
}

/**
 * Invoke a hook in all enabled modules and MM sub-modules that implement it.
 *
 * @param $hook
 *   The name of the hook to invoke.
 * @param ...
 *   Arguments to pass to the hook.
 * @return
 *   An array of return values of the hook implementations. If modules return
 *   arrays from their implementations, those are merged into one array.
 */
function mm_module_invoke_all() {
  $args = func_get_args();
  $hook = array_shift($args);
  $return = array();
  foreach (mm_module_implements($hook) as $module) {
    $function = $module .'_'. $hook;
    $result = call_user_func_array($function, $args);
    if (isset($result) && is_array($result)) {
      $return = array_merge_recursive($return, $result);
    }
    else if (isset($result)) {
      $return[] = $result;
    }
  }

  return $return;
}


/**
 * Invoke a hook in all enabled modules and MM sub-modules that implement it.
 * Unlike mm_module_invoke_all(), any references are preserved.
 *
 * @param $hook
 *   The name of the hook to invoke.
 * @param $args
 *   Arguments to pass to the hook. Any references in the array are preserved.
 * @return
 *   An array of return values of the hook implementations. If modules return
 *   arrays from their implementations, those are merged into one array.
 */
function mm_module_invoke_all_array($hook, $args) {
  $return = array();
  foreach (mm_module_implements($hook) as $module) {
    $function = $module .'_'. $hook;
    $result = call_user_func_array($function, $args);
    if (isset($result) && is_array($result)) {
      $return = array_merge_recursive($return, $result);
    }
    else if (isset($result)) {
      $return[] = $result;
    }
  }

  return $return;
}

/**
 * @} End of "defgroup mm_hooks".
 */

// ****************************************************************************
// * Private functions start here
// ****************************************************************************

function _mm_render_pages($mmtids, $oarg_list, &$err, $no_attribution = FALSE, $allow_rss = TRUE, $block_id = 0) {
  global $user, $_mm_page_subscribe_item, $_mm_mmtid_of_node;
  $err = '';
  $this_mmtid = $mmtids[count($mmtids) - 1];
  $no_read = 0; $ok = 0;

  // Check for mm_showpage callbacks, specified in hook_mm_showpage_routing()
  $showpage_no_nodes = FALSE;
  $router = _mm_showpage_router();
  if ($router) {
    $temp_path = "mm/$this_mmtid";
    custom_url_rewrite_outbound($temp_path, $temp_options, $temp_path);
    $temp_path = join('/', array_merge(array($temp_path), $oarg_list));
    $temp_args = split('/', $temp_path);

    foreach ($router as $key => $item) {
      if (preg_match($key, $temp_path) && (!isset($item['block id']) || $item['block id'] == $block_id) && _mm_showpage_callback($item, 'access', $temp_args, $this_mmtid, $block_id)) {
        $showpage_output = _mm_showpage_callback($item, 'page', $temp_args, $this_mmtid, $block_id);
        if (is_array($showpage_output)) {
          $output .= $showpage_output['output_pre'];
          $output_post .= $showpage_output['output_post'];
          $showpage_no_nodes |= $showpage_output['no_nodes'];
        }
        else if ($showpage_output) {
          $output .= $showpage_output;
          $showpage_no_nodes = FALSE;
        }
      }
    }
  }

  $showpage_show_nodes = !$showpage_no_nodes;
  $nodes_per_page = mm_content_resolve_cascaded_setting('nodes_per_page', $this_mmtid, $npp_at, $npp_parent);
  if (empty($nodes_per_page)) $nodes_per_page = variable_get('default_nodes_main', 10);

  $perms = mm_content_user_can($this_mmtid);
  if (!$perms['r']) {
    $no_read++;
  }
  else if ((count($mmtids) == 1 || count($mmtids) == 2 && variable_get('mm_use_virtual_user_dir', TRUE)) && $mmtids[0] == mm_content_users_mmtid()) {
    module_load_include('inc', 'monster_menus', 'mm_ui_user_list');
    $output .= mm_ui_user_list_form($mmtids);
    $ok++;
  }
  else if ($showpage_show_nodes) {
    // display a list of pages assigned to the current tree entry
    $item = mm_content_get($this_mmtid, MM_GET_ARCHIVE);
    $result = NULL;

    $omit_nodes = '';
    if (!$perms['IS_RECYCLED']) {
      $omit_node_types = mm_get_node_info(MM_NODE_INFO_NO_RENDER);
      if ($omit_node_types) $omit_nodes = " AND n.type NOT IN('" . join("', '", $omit_node_types) . "')";
    }

    if (isset($item->main_mmtid) || isset($item->archive_mmtid)) {
      // This is an archive page, or the main page for which there is an archive
      if ($this_mmtid > 0) {
        $mmtid = isset($item->main_mmtid) ? $item->main_mmtid : $this_mmtid;
        $q = mm_content_get_accessible_nodes_by_mmtid_query($mmtid, $count_sql);
        $result = db_query($q);
      }
    }
    else if (count($oarg_list) && !$output) {
      // if the remaining parameters in the URL can't be accounted for, it's a dead link
      $other_mmtids = array_diff($oarg_list, variable_get('mm_reserved_alias', array()));
      if (count($other_mmtids)) return drupal_not_found();
    }

    if (is_null($result)) {
      global $pager_total;
      $pager_elem = count($pager_total) + 1;
      $result = mm_content_get_accessible_nodes_by_mmtid($this_mmtid, $nodes_per_page, $pager_elem, '', '', $omit_nodes);
    }

    $rss_link = NULL;
    if (isset($item->archive_mmtid)) {
      $archive_tree = array();
      $archive_count = 0;
      $now = time();
      if ($oarg_list && preg_match('/([12]\d\d\d)-(0[1-9]|1[0-2])-([0123]\d)/', $oarg_list[0], $matches)) {
        $archive_date = array(
          'year' => intval($matches[1]),
          'mon' =>  intval($matches[2]),
          'mday' => intval($matches[3]));
      }
    }

    while ($n = db_fetch_object($result)) {
      if (!mm_content_user_can_node($n->nid, 'r')) {
        $no_read++;
      }
      else {
        $ok++;
        $_mm_mmtid_of_node[$n->nid] = $this_mmtid;
        if ($oarg_list[0] != 'feed') {
          // This is an archive page, or the main page for which there is an archive,
          // and the node is not stuck on the main page, and it's always visible to everyone
          if (isset($item->archive_mmtid) && ($item->archive_mmtid == $this_mmtid || !$n->stuck && $n->scheduled && $n->status == 1)) {
            // skip this node if viewing the main page of an archive and we've seen main_nodes # of non-sticky, always-appearing nodes
            if (++$archive_count > $item->main_nodes || $item->archive_mmtid == $this_mmtid && !$n->stuck && $n->scheduled && $n->status == 1) {
              $date = getdate($n->created);
              switch ($item->frequency) {
                case 'year':
                  $rounded = mktime(0, 0, 0, 1, 1, $date['year']);
                  $archive_tree[$date['year']] = $rounded;
                  $archive_show = $archive_date['year'] == $date['year'];
                  break;

                case 'month':
                  $rounded = mktime(0, 0, 0, $date['mon'], 1, $date['year']);
                  $archive_tree[$date['year']][$date['mon']] = $rounded;
                  $archive_show = $archive_date['year'] == $date['year'] && $archive_date['mon'] == $date['mon'];
                  break;

                case 'week':
                  $rounded = mktime(0, 0, 0, $date['mon'], $date['mday']-$date['wday'], $date['year']);
                  $date = getdate($rounded);
                  $archive_tree[$date['year']][$date['mon']][$date['mday']] = $rounded;
                  $archive_show = $archive_date['year'] == $date['year'] && $archive_date['mon'] == $date['mon'] && $archive_date['mday'] == $date['mday'];
                  break;

                case 'day':
                  $rounded = mktime(0, 0, 0, $date['mon'], $date['mday'], $date['year']);
                  $archive_tree[$date['year']][$date['mon']][$date['mday']] = $rounded;
                  $archive_show = $archive_date['year'] == $date['year'] && $archive_date['mon'] == $date['mon'] && $archive_date['mday'] == $date['mday'];
                  break;
              }
              if (!isset($archive_date)) {
                // not in URL, so default to most recent node's date, rounded down
                $archive_date = getdate($rounded);
                $archive_show = TRUE;
              }
              if ($archive_show) $archive_date_int = $rounded;
            }

            // skip this node if viewing an archive page and it's sticky
            if ($item->archive_mmtid == $this_mmtid && ($n->stuck || !$n->scheduled || !$archive_show)) continue;
            // skip this node if viewing the main page and we've already seen main_nodes # of nodes
            if ($item->main_mmtid == $this_mmtid && $archive_count > $item->main_nodes) continue;
          }

          $node = node_load($n->nid);
          if ($node->nid) {
            $message = $body = '';
            if (!$node->status) {
              $message = t('This piece of content is not yet published. It can only be seen by people who can edit it.');
            }
            else if (!$n->scheduled) {
              $message = t('This piece of content can only be seen by people who can edit it, due to its appearance schedule.');
            }

            if ($node->type == 'blog') {
              $body = "<a id=\"node-$node->nid\"></a>" . node_view($node, TRUE);
            }
            else {
              if ($no_attribution) $node->no_attribution = TRUE;
              if ($item->previews) {
                $body = node_view($node, TRUE);
              }
              else if (variable_get('mm_show_count_instead_of_comments', FALSE) && $node->comment && module_exists('comment')) {
                // Generate the node body without comments turned on, but then
                // turn it back on in monster_menus_link_alter() so that the
                // link showing the number of comments appears.
                $node->old_comment = $node->comment;
                $node->comment = 0;   // Generate body without comments
                $node->need_comment_alter = TRUE;
                $body = node_view($node, FALSE, TRUE);
                $node->comment = $node->old_comment;
              }
              else {
                $body = mm_node_show($node, 0);
              }
            }
            // $node->no_display is a custom field that can be set by a hook_view module, such as rss_page_view
            if ($node->no_display) {
              $archive_count--;
            }
            else {
              if ($message) {
                $body = '<div class="preview"><div id="message"><div class="messages status">'. $message . '</div></div>' . $body . '</div>';
              }
              $output .= "<a id=\"node-$node->nid\"></a>" . $body;
            }

            $rss_link = is_null($rss_link) && $node->rss_link ? $node->rss_link : FALSE;
          }
        }
      }
    }

    if ($ok && $oarg_list[0] == 'feed') {
      $tree = mm_content_get($this_mmtid);
      $result = db_query_range(
          db_rewrite_sql(mm_content_get_accessible_nodes_by_mmtid_query($this_mmtid, $count_sql)),
          0, variable_get('feed_default_items', 10));
      $channel['link'] = url("mm/$this_mmtid", array('absolute' => TRUE));
      $channel['title'] = variable_get('site_name', 'drupal') . ': ' . $tree->name;
      $channel['description'] = '';
      $nids = array();
      while ($row = db_fetch_object($result)) {
        if ($row->scheduled) {
          $nids[] = $row->nid;
        }
      }
      node_feed($nids, $channel);
      exit;
    }

    // not a feed
    if ($block_id == 0) $_mm_page_subscribe_item = $item;

    if ($ok) {
      if ($allow_rss && ($item->rss || !variable_get('mm_enable_rss', FALSE))) {
        drupal_add_link(array('rel' => 'alternate',
          'type' => 'application/rss+xml',
          'title' => t('RSS'),
          'href' => $rss_link ? $rss_link : url("mm/$this_mmtid/feed", array('absolute' => TRUE))));
      }

      if (isset($item->main_mmtid) || isset($item->archive_mmtid)) {
        $archive = theme('mm_archive', $archive_tree, $item->frequency, $this_mmtid, $item->main_mmtid, $item->archive_mmtid, $archive_date_int);
        $output = $archive['header'] . $output . $archive['links'];
      }
      else {
        $output .= theme('pager', NULL, $nodes_per_page, $pager_elem);
      }
    }
  }

  $output .= $output_post;
  if ($output != '' || $ok) return $output;

  $err = $no_read ? 'no read' : 'no content';
  return '';
}

function _mm_resolve_archive(&$mmtid) {
  $mmtid = intval($mmtid);
  if ($mmtid) {
    if (!mm_content_user_can($mmtid, 'r')) {
      mm_access_denied();
    }
    $tree = mm_content_get($mmtid, MM_GET_ARCHIVE);
    if (isset($tree->main_mmtid) && $tree->archive_mmtid == $mmtid) {
      $mmtid = $tree->main_mmtid;
      if (!mm_content_user_can($mmtid, 'r')) {
        mm_access_denied();
      }
    }
    return $mmtid;
  }
  return FALSE;
}

function _mm_showpage_router($reset = FALSE) {
  static $router;
  if (!isset($router) || $reset) {
    if (!$reset && ($cache = cache_get('mm_showpage')) && isset($cache->data)) {
      $router = $cache->data;
    }
    else {
      $callbacks = array();
      foreach (mm_module_implements('mm_showpage_routing') as $module) {
        $router_items = call_user_func($module . '_mm_showpage_routing');
        if (isset($router_items) && is_array($router_items)) {
          foreach (array_keys($router_items) as $path) {
            if (!isset($router_items[$path]['module'])) {
              $router_items[$path]['module'] = $module;
            }
          }
          $callbacks = array_merge($callbacks, $router_items);
        }
      }

      $router = $sort = array();
      foreach ($callbacks as $path => $item) {
        list($fit, $number_parts) = _mm_showpage_router_fit($path, $item['partial path']);
        $ending = isset($item['partial path']) && $item['partial path'] ? '(?:$|/)}' : '$}';
        $path = '{^' . str_replace('%', '[^/]+', preg_quote($path)) . $ending;
        $sort[$path] = $fit;

        if (!isset($item['access callback']) && isset($item['access arguments'])) {
          // Default callback.
          $item['access callback'] = 'mm_content_user_can';
        }
        if (empty($item['page callback'])) {
          $item['access callback'] = FALSE;
        }
        else if (!isset($item['access callback'])) {
          $item['access callback'] = 'mm_content_user_can';
          $item['access arguments'] = array('_mmtid_', 'r');
        }
        $item += array(
          'access arguments' => array(),
          'access callback' => '',
          'page arguments' => array(),
          'page callback' => '',
          'file' => '',
        );
        $router[$path] = $item;
      }
      array_multisort($sort, SORT_NUMERIC, SORT_DESC, $router);

      cache_set('mm_showpage', $router);
    }
  }

  return $router;
}

function _mm_showpage_callback($item, $type, $args, $this_mmtid, $block_id) {
  $callback = $item["$type callback"];
  if (is_bool($callback)) {
    return $callback;
  }

  $callback = trim($callback);
  if (empty($callback)) {
    return FALSE;
  }

  $arguments = $item["$type arguments"];
  foreach ($arguments as $k => $v) {
    if (is_int($v)) {
      $arguments[$k] = isset($args[$v]) ? $args[$v] : '';
    }
    else if ($v === '_mmtid_') {
      $arguments[$k] = $this_mmtid;
    }
    else if ($v === '_block_id_') {
      $arguments[$k] = $block_id;
    }
  }

  if (!empty($item['file'])) {
    $file = drupal_get_path('module', $item['module']) . '/' . $item['file'];
    require_once $file;
  }

  return call_user_func_array($callback, $arguments);
}

function _mm_showpage_router_fit($path, $partial_path) {
  $fit = 0;
  $parts = explode('/', $path);
  $number_parts = count($parts);
  foreach ($parts as $k => $part) {
    if ($part != '%') {
      $fit |= 1 << ($number_parts - 1 - $k);
    }
  }

  if (!$fit) {
    // If there is no %, it fits maximally.
    $fit = (1 << $number_parts) - 1;
  }
  $fit = ($fit << 1) + ($partial_path ? 0 : 1);

  return array($fit, $number_parts);
}

function _mm_soundex($string) {
  static $cache;

  // The PHP soundex() function is incorrect, so use the SQL version, but cache
  // the results.
  $string = preg_replace('/[\x80-\xFF]/', '', $string);
  if (!isset($cache[$string])) {
    $cache[$string] = db_result(db_query("SELECT SOUNDEX('%s')", $string));
  }
  return $cache[$string];
}

/**
 * Return JSON response for user list request.
 */
function mm_large_group_get_users_json($mmtid, $element) {
  $clean_element = str_replace('-', '_', $element);
  $users_array = mm_module_invoke_all_array('mm_large_group_get_users', array('mmtid' => $mmtid, 'element' => $clean_element));
  if (empty($users_array)) {
    $sort_array = array('name');
    $where = " WHERE gid = %d AND sessionid = '%s'";
    $values[] = $mmtid;
    $values[] = session_id();
    $order = "";
    $sort_list = array();
    if ($_GET['sSearch'] != "" && !empty($_GET['sSearch'])) {
      $where .= " AND u.name LIKE '%%%s%%'";
      $values[] = $_GET['sSearch'];
      $values[] = $_GET['sSearch'];
      $values[] = $_GET['sSearch'];
    }
    if (isset($_GET['iSortCol_0'])) {
      if (is_numeric($_GET['iSortingCols'])) {
        for ($i = 0; $i < $_GET['iSortingCols']; $i++ ) {
          $direction = $_GET["iSortDir_$i"] == 'desc' ? 'DESC' : 'ASC';
          if (is_numeric($_GET["iSortCol_$i"]) && isset($sort_list)) {
            $sort_list[] = $sort_array[(int)$_GET["iSortCol_$i"]] . ' ' . $direction;
          }
        }
        if (count($sort_list) > 0) $order = ' ORDER BY ' . implode(', ', $sort_list);
      }
    }

    $total_rows = db_result(db_query("SELECT COUNT(*) FROM {user} u INNER JOIN {mm_group_temp} m ON u.uid = m.uid" . $where, $values));
    $query = "SELECT u.uid, u.name FROM {user} u INNER JOIN {mm_group_temp} m ON u.uid = m.uid" . $where . $order;
    if (isset($_GET['iDisplayStart'])) {
      $results = db_query_range($query, $values, $_GET['iDisplayStart'], $_GET['iDisplayLength']);
    }
    else {
      $results = db_query($query, $values);
    }

    $users = array(
      'sEcho' => intval($_GET['sEcho']),
      'iTotalRecords' => $total_rows,
      'iTotalDisplayRecords' => $total_rows,
    );
    while($item = db_fetch_object($results)) {
      $users['aaData'][] = array($item->lastname, $item->firstname, $item->username, '<a href="Javascript:Drupal.mmGroupRemoveUser(' . $item->uid . ',\'' . $clean_element . '\')">' . t('Delete') . '</a>');
    }
  }
  else {
    $users = $users_array;
  }

  drupal_set_header('Pragma: no-cache');
  drupal_json($users);
  exit();
}

/**
 * Deletes a user from an mm_group.
 *
 * @param $mmtid
 *    The mmtid of the group from which the user is being removed
 * @param $delete_user
 *    User object of the user to be deleted
 */
function mm_large_group_delete_users($mmtid, $delete_user) {
  return db_query("DELETE FROM {mm_group_temp} WHERE gid = %d AND uid = %d AND sessionid = '%s'", $mmtid, $delete_user->uid, session_id());
}

/**
 * Deletes a user from an mm_group.
 *
 * @param $mmtid
 *    The mmtid of the group from which the user is being removed
 * @param $delete_user
 *    User object of the user to be deleted
 */
function mm_large_group_add_users($mmtid, $delete_user) {
  $userlist = explode('}', $delete_user);
  foreach ($userlist as $value) {
    $uid = substr($value, 0, strpos($value, '{'));
    print $uid . "\n";
    if (!empty($uid)) {
      db_query("INSERT INTO {mm_group_temp} SET gid = %d, uid = %d, sessionid = '%s'", $mmtid, $uid, session_id());
    }
  }
}