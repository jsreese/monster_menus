<?php
// $Id: mm_api.inc 4922 2011-01-21 16:45:27Z dwilga $

/**
 * @file
 * These are the hooks that are invoked by Monster Menus.
 *
 * This file is here for documentation purposes only. It does not get included
 * in the running code.
 */

/**
 * @addtogroup mm_hooks
 * @{
 */

/**
 * Define settings which are inherited by lower levels of the MM tree. Data
 * passed to mm_content_insert_or_update() is saved using this definition.
 *
 * @see mm_content_get_cascaded_settings()
 * @see mm_share_widget_mm_cascaded_settings()
 *
 * @return
 *   An associative array containing the settings. Each key is a unique ID. Each
 *   value is an associative array with one or more of these elements, which
 *   describe the stored data:
 *   - data_type:   'int' (integer) or 'string'
 *   - multiple:    TRUE if multiple values are accepted
 *   - not_empty:   TRUE if only !empty() values should be stored (FALSE)
 *   - user_access: user must have user_access() for this value in order to set
 *                  the setting
 *   - use_keys:    TRUE if 'multiple' and data is in an associative array (TRUE)
 */
function hook_mm_cascaded_settings() {
  return array(
    'share_default' => array(
      'data_type' => 'int',
      'multiple' => TRUE,
      'use_keys' => TRUE,
    )
  );
}

/**
 * Determine whether the current user may perform the given operation on the
 * specified node. This differes from the build-in Drupal hook_node_access(), in
 * that it is always called, for every node, regardless of whether or not a
 * given module is the one that defines that node's content type.
 *
 * @param $op
 *   The operation to be performed on the node. Possible values are:
 *   - "view"
 *   - "update"
 *   - "delete"
 *   - "create"
 * @param $node
 *   The node object (or node array) on which the operation is to be performed,
 *   or node type (e.g. 'forum') for "create" operation.
 * @param $account
 *   Optional, a user object representing the user for whom the operation is to
 *   be performed. Determines access for a user other than the current user.
 * @return
 *   TRUE if the operation may be performed.
 */
function hook_mm_node_access($op, $node, $account) {
  if ($node->nid && _mm_workflow_access_node_has_workflow($node)) {
    if ($op == 'view') $mode = 'r';
    else if ($op == 'update') $mode = 'w';
    else if ($op == 'delete') $mode = 'd';
    else return;

    if (!isset($node->_workflow)) $node->_workflow = workflow_node_current_state($node);
    return _mm_workflow_access_get_user_perm($node, $mode, $account);
  }
}

/**
 * Alter nodes just prior to creation during mm_content_copy().
 *
 * @param $node
 *   The node object to be saved
 * @param $old_catlist
 *   The original value of the mm_catlist element of the node object in the
 *   source of the copy
 */
function hook_mm_copy_tree_node_alter(&$node, $old_catlist) {
  if ($node->type == 'mm_calendar') {
    // When copying a calendar node, update the list of subscribed pages to
    // point to the new node's location
    $old_catlist = array_keys($old_catlist);
    foreach ($node->field_mm_pages as $pos => $mm_page)
      if (array_search($mm_page['value'], $old_catlist) !== FALSE) {
        $new_cat = array_keys($node->mm_catlist);
        $node->field_mm_pages[$pos]['value'] = $new_cat[0];
      }
  }
}

/**
 * Alter tree entries just prior to creation during mm_content_copy().
 *
 * @param $new
 *   The MM Tree entry about to be created
 * @param $dest_mmtid
 *   The destination MM Tree entry
 */
function hook_mm_copy_tree_tree_alter(&$new, $dest_mmtid) {
  // This is a silly example, which appends "(copy)" to the item's name.
  $new->name .= ' ' . t('(copy)');
}

/**
 * Alter the processing of tree entries during mm_content_copy(). If the
 * function returns -1, the entry and any children will be skipped; if it
 * returns 1, just the current entry is skipped; if it returns 0, all further
 * processing is canceled; any other return value leads to no change. The
 * function can also alter the item passed to it.
 *
 * @param $item
 *   The source MM Tree entry
 * @param $options
 *   A temporary copy of the $options parameter passed to mm_content_copy().
 *   This can be altered to affect the behavior of the rest of the copy
 *   operation for just the current item.
 */
function hook_mm_copy_tree_iterate_alter(&$item, &$options) {
}

/**
 * This hook is called by mm_content_delete() when MM is permanently deleting a
 * tree entry. It allows modules to clean up database entries associated with
 * the entry and, if specified, the nodes appearing there.
 *
 * @param $mmtids
 *   Array of tree IDs, describing the entries being deleted
 * @param $nids
 *   Array of node IDs, describing the nodes being deleted. This aspect can also
 *   be handled in hook_delete(), but can be faster if performed in this hook,
 *   because more than one node can be processed at once.
 */
function hook_mm_delete($mmtids, $nids) {
  db_query('DELETE FROM {mm_workflow_access} WHERE gid IN(' . db_placeholders($mmtids) . ')', $mmtids);
}

/**
 * Add one or more alias to human-readable name expansions to the list used by
 * mm_content_expand_name().
 *
 * @return
 *   An associative array with the alias in the key and the expanded name in the
 *   value.
 */
function hook_mm_item_name() {
  return array(
    '.calendar' => t('[Calendar default]'),
  );
}

/**
 * Add one or more MM tree names to the list that is hidden from non-admin
 * users.
 *
 * @return
 *   An array of names. Each name usually starts with '.'.
 */
function hook_mm_hidden_user_names() {
  return array('.Calendar');
}

/**
 * Alter MM's behavior according to the node type being rendered. The results
 * of this hook can be queried using mm_get_node_info().
 *
 * @return
 *   An associative array whose key is the name of a content type, and whose
 *   value is an associative array containing one or more flags. The currently
 *   defined flags are:
 *   - MM_NODE_INFO_NO_RENDER:   Don't render this node type
 *   - MM_NODE_INFO_ADD_HIDDEN:  Hide the Add node link for this type
 */
function hook_mm_node_info() {
  return array(
    // mm_event is a CCK content type, but we need to set flags to prevent
    // MM from rendering and allowing users to create this node type.
    'mm_event' => array(
      MM_NODE_INFO_NO_RENDER => TRUE,
      MM_NODE_INFO_ADD_HIDDEN => TRUE,
    )
  );
}

/**
 * Define one or more functions to be called whenever MM renders nodes at a
 * particular part of the tree. Output can be appended to, pre-pended to, or
 * completely replace the normal list of nodes.
 *
 * The hook should return an associative array where the key is the page's path,
 * and the value is an associative array containing one or more values that
 * define how the callback should be used.
 *
 * The format and meaning of the array is similar to Drupal's hook_menu(). The
 * path can be a direct match, such as "foo/bar/baz", or the "%" wildcard can be
 * included any number of times, as in "foo/%/%". Other types of wildcard, such
 * as "%node", are not currently supported. Path matching is performed on the
 * entire URL, regardless of whether or not some of the later path elements
 * actually exist in the MM tree.
 *
 * It is also important to note that, unlike hook_menu() callbacks, all matching
 * hook_mm_showpage_routing() callbacks are called--not just the "best" match.
 * For example, if you have the paths "foo/bar" and "foo/%", and the URL
 * "foo/bar" is requested, then both callbacks will be used. It may be necessary
 * to add an access callback to prevent the "foo/%" callback from being used.
 *
 * Information returned by each module's implementation of this hook is cached.
 * Whenever you have made a change to your code, you need to either rebuild the
 * menu tree or call _mm_showpage_router(TRUE) in order for the change to take
 * effect.
 *
 * These options are supported:
 * - 'block id'
 *   If specified, the callback will only be used if the output is part of a
 *   specific block within the page template.
 * - 'file'
 *   If set, the specified file will be loaded with require_once() prior to
 *   calling any callbacks. The file's path should be relative to the module
 *   containing this hook.
 * - 'partial path'
 *   If TRUE, the URL supplied by the user will match if it begins with the
 *   portion of the path in the array key; otherwise, an exact match is
 *   required. This option is FALSE, by default.
 * - 'access callback'
 *   The callback to use when determining whether or not to call the page
 *   callback. If this function exists and returns FALSE, the page callback is
 *   not called. If, instead of a function name, one of the booleans TRUE or
 *   FALSE is given as the callback, that value controls the calling of the page
 *   callback.
 *
 *   Note that, in contrast to hook_menu(), access callbacks in this hook do not
 *   inherit access from their parents: each level is handled separately. Also,
 *   attempting to access a page that is not allowed simply results in no output
 *   being appended to the page, rather than a "403" or "access denied" message.
 *
 *   If this callback is left at its default value, the equivalent of
 *   "mm_content_user_can('_mmtid_', 'r')" is performed.
 * - 'access arguments'
 *   Arguments passed to the access callback. This is an array representing the
 *   order and numeric position of the values to pass to the callback. Integer
 *   values are taken to mean "add the Nth path element"; the constant "_mmtid_"
 *   means "pass the MM tree ID of the lowest, existing page"; the constant
 *   "_block_id_" means "pass the block ID"; any other value is passed
 *   unmodified.
 * - 'page callback'
 *   The callback which produces the output to insert into the page. If the
 *   callback returns a non-empty string, that value is appended to the page,
 *   below any other nodes. It is also possible to return an array containing
 *   these keys:
 *   - 'output_pre'
 *     Content which is pre-pended to the normal node list
 *   - 'output_post'
 *     Content which is appended to the normal node list
 *   - 'no_nodes'
 *     If TRUE, completely suppress the normal node list
 * - 'page arguments'
 *   Arguments passed to the page callback. This is an array representing the
 *   order and numeric position of the values to pass to the callback. Integer
 *   values are taken to mean "add the Nth path element"; the constant "_mmtid_"
 *   means "pass the MM tree ID of the lowest, existing page"; any other value
 *   is passed unmodified.
 *
 * @return
 *   An associative array where the key is the page's path, and the value is an
 *   associative array containing one or more of the values defined above.
 */
function hook_mm_showpage_routing() {
  $items = array();
  $items['myamherst'] = array(
    'page callback' => 'amhp_myamherst_getcontent',
    'page arguments' => array('_mmtid_', 1),
    'partial path' => TRUE,
  );
  $items['alumni/classpages/%/classmates'] = array(
    'file' => 'amherstprofile_search.inc',
    'page callback' => 'amhp_search_classmates_reunion',
    'page arguments' => array(2),
    'access callback' => '_amhp_search_classmates_reunion_access',
    'access arguments' => array('_mmtid_', 2),
  );
  $items['people/students/%/classmates'] = array(
    'file' => 'amherstprofile_search.inc',
    'page callback' => 'amhp_search_classmates_grad_year',
    'page arguments' => array(2, TRUE, 3),
    'access callback' => '_amhp_search_classmates_people_students_access',
    'access arguments' => array('_mmtid_', 2),
  );

  return $items;
}

/**
 * Define one or more flags in the UI for a page's settings
 *
 * @return
 *   An associative array where the key is the flag's name, and each value is
 *   an associative array containing the Forms API element to describe the
 *   UI to set that flag. Currently, the only supported types ("#type") are
 *   checkbox and textfield.
 *
 *   Additionally, these special keys can be used:
 *   - #flag_inherit: By default, no flags are inherited by newly created
 *     sub-pages/groups. Set this key's value to TRUE to have the flag, when
 *     present, copied to new sub-pages.
 *   - #flag_copy: By default, flags are always copied by mm_content_copy(). To
 *     change this behavior, set this key's value to FALSE.
 */
function hook_mm_tree_flags() {
  return array(
    'limit_write' => array('#type' => 'checkbox', '#description' => t('MM: Prevents non-admin users from changing "Delete or change settings"')),
    'no_breadcrumb' => array('#type' => 'checkbox', '#description' => t('MM: Prevents the page breadcrumb from showing at this level')),
  );
}

/**
 * This hook is called within Monster Menus' implementation of
 * custom_url_rewrite_outbound().
 *
 * @param $mmtid
 *   Tree ID of the entry referred to by $path, or NULL if it does not refer to
 *   part of the MM tree
 * @param $path
 *   The alias of the $original_path as defined in the database. If there is no
 *   match in the database it will be the same as $original_path.
 * @param $options
 *   An array of link attributes such as query and fragment. See url().
 * @param $original_path
 *   The unaliased Drupal path that is being linked to.
 */
function hook_mm_url_rewrite_outbound($mmtid, &$path, &$options, $original_path) {
  // Add a query string for one specific case
  if (empty($options['query']) && !empty($_GET['mm_calendar'])) {
    $options['query'] = drupal_query_string_encode(array('destination' => "mm/$mmtid?mm_calendar=" . $_GET['mm_calendar']));
  }
}

/**
 * This hook is called within Monster Menus' implementation of
 * hook_menu_alter(), at a point before checks are done to ensure that no
 * parts of the tree contain aliases which collide with menu paths. This allows
 * the offending menu paths to be renamed.
 *
 * @param $callbacks
 *   Associative array of menu router definitions returned from hook_menu()
 */
function hook_mm_menu_alter(&$callbacks) {
}

/**
 * Define queries which test the relationships between keys in a module's
 * database tables. This hook is usually implemented in an .install file. It is
 * only called when a user visits admin/mm/integrity.
 *
 * @return
 *   An associative array where the key is the human-readable module name, and
 *   the value is an associative array. This inner array's keys are human-
 *   readable descriptions of the test being performed, and the values are a
 *   query segment, without "SELECT * FROM".
 */
function hook_mm_verify_integrity() {
  return array('MM Workflow Access' => array(
    t('mm_workflow_access.sid refers to missing workflow_states.sid') =>
      "{mm_workflow_access} x LEFT JOIN {workflow_states} s ON s.sid=x.sid WHERE s.sid IS NULL",
    t('mm_workflow_access.gid refers to missing mm_tree.mmtid') =>
      "{mm_workflow_access} x LEFT JOIN {mm_tree} t ON t.mmtid=x.gid WHERE x.gid>0 AND t.mmtid IS NULL",

    t('mm_workflow_author.nid refers to missing node.nid') =>
      "{mm_workflow_author} x LEFT JOIN {node} n ON n.nid=x.nid WHERE n.nid IS NULL",
    t('mm_workflow_author.uid refers to missing users.uid') =>
      "{mm_workflow_author} x LEFT JOIN {users} u ON u.uid=x.uid WHERE u.uid IS NULL AND x.uid>0",
  ));
}

/**
 * Alter the query used to determine the members of an MM group.
 *
 * @param $mmtids
 *   An array of MM tree IDs of the group(s) being queried
 * @param $query
 *   The regular query to be modified
 * @param $countquery
 *   The query which returns the row count
 */
function hook_mm_get_users_in_group_alter($mmtids, &$query, &$countquery) {
}

/**
 * This hook is called by MM during hook_user() when an action is performed on a
 * user account. If the return is TRUE, MM does not do any processing during its
 * hook_user(). The function is free to modify $edit and $account here, as it
 * would during its own hook_user().
 *
 * Important note: In order for this hook to be called when $op=="load", it must
 * implement one of the bootstrap hooks, listed in Drupal's bootstrap_hooks()
 * function.
 */
function hook_mm_user($op, &$edit, &$account, $category) {
}

/**
 * Alter the HTML code which appears when a page is empty
 *
 * @param $entry
 *   The MM tree entry of the page
 * @param $list
 *   An array of HTML code segments, which are concatenated to produce the
 *   final page. Upon entry, $list[0] contains the default message supplied
 *   by MM. This entry can be modified, or the list appended to.
 */
function hook_mm_empty_page_alter($entry, &$list) {
}

/**
 * Alter the data used in mm_content_add_user() to create new user home pages.
 *
 * @param $account
 *   User object to create a home page for
 * @param $dest_mmtid
 *   Tree ID of the parent entry, under which the new home page is created
 * @param $full_name
 *   The menu name of the user's home page
 */
function hook_mm_add_user_alter(&$account, &$dest_mmtid, &$full_name) {
}

/**
 * Called after mm_content_add_user() has successfully created a user's home
 * page.
 *
 * @param $account
 *   User object for whom the home page was created
 * @param $new_mmtid
 *   Tree ID of the user's new home page
 * @param $dest_mmtid
 *   Tree ID of the parent entry, under which the new home page was created
 */
function hook_mm_add_user_post(&$account, $new_mmtid, $dest_mmtid) {
}

/**
 * Alter the data returned by mm_content_uid2name().
 *
 * @param $usr
 *   User object; these fields can be modified, in order to change what is
 *   eventually returned by mm_content_uid2name():
 *   - pref_fml: The user's long name, in "first, middle, last" format
 *   - pref_lfm: The user's long name, in "last, first, middle" format
 *   - last:     The user's surname
 *   - first:    The user's first name
 *   - name:     The user's Drupal username
 *   - middle:   The user's middle name
 *   - hover:    Optional text to appear when the mouse is hovering over a link
 * @param $disabled
 *   If TRUE upon entry, the user is known to be inactive (status == 0); can
 *   be modified
 */
function hook_mm_uid2name_alter(&$usr, &$disabled) {
  if (!$disabled && $usr->username === 'disabled-user') {
    $disabled = TRUE;
  }
}

/**
 * Alter the query used by mm_regenerate_vgroup() to pre-calculate the top N
 * users in a virtual group. This allows modules to impose their own sorting on
 * the list.
 *
 * @param $query
 *   The query
 */
function hook_mm_regenerate_vgroup_preview_alter(&$query) {
}

/**
 * Generate a query for mm_autocomplete() to use when getting a list of users
 * that match an autocomplete request.
 *
 * @param $string
 *   The autocomplete string typed by the user
 * @param $limit
 *   The maximum number of results to return
 * @param $min_string
 *   The minimum number of characters the user needs to type
 * @param $misc
 *   A miscellaneous value that is passed to mm_autocomplete() in the $misc
 *   parameter; mm_autocomplete(), itself, ignores this value
 * @return
 *   A database query object, or NULL if the user needs to type more characters
 */
function hook_mm_autocomplete_alter($string, $limit, $min_string, $misc) {
}

/**
 * Alter the configuration page, found at admin/settings/mm.
 *
 * @param $form
 *   The Forms API array, which can be altered
 */
function hook_mm_config_alter(&$form) {
}

/**
 * Suppress the display of content within one or more blocks, during
 * monster_menus_block('view', ...)
 *
 * @param $this_mmtid
 *   The MM tree ID of the outer page containing the block
 * @param $delta
 *   The ID of the block being rendered
 * @return
 *   TRUE if the block should be rendered. If any implementation of this hook
 *   returns FALSE, the block is not rendered.
 */
function hook_mm_menus_block_shown($this_mmtid, $delta) {
}

/**
 * Add one or more elements to the navigation bar in the tree browser.
 *
 * @param $mode
 *   One of the MM_BROWSER_MODE_* constants
 * @param $top_mmtid
 *   The MM tree ID of the topmost, displayed page in the tree
 * @return
 *   A string containing a <li>...</li> element to encompass the button and any
 *   child menu. The button must have the CSS class mm-browser-button. Any child
 *   menu must itself be a <ul> surrounded by a hidden <div> with the
 *   mm-browser-button-list class.
 */
function hook_mm_browser_navigation($mode, $top_mmtid) {
}

/**
 * Alter the links appearing in the right hand pane of the tree browser.
 *
 * @param $mode
 *   One of the MM_BROWSER_MODE_* constants
 * @param $links
 *   Array of named links. The order dictates what order the rendered links will
 *   appear in the final HTML.
 * @param $item
 *   The MM tree record for the item being displayed
 */
function hook_mm_browser_links_alter(&$links, $item) {
}

/**
 * Provide a list of column headers for group management
 *
 * @return
 *   Returns the headers for the mm_group editing datatable.
 */
function hook_mm_large_group_header() {
}

/**
 * Returns users that appear on the group editing pages
 *
 * @param $mmtid
 *   The group id for the group that is being edited
 * @param $element
 *   Used to identify the datatable for redrawing
 * @return
 *   User data that can be parsed by Datatables. The number of rows should match
 *   the headers defined by hook_mm_large_group_header().
 */
function hook_mm_large_group_get_users($mmtid, $element) {
}

/**
 * Alter the MM tree parameters resulting from editing a page's settings
 *
 * @param $add
 *   TRUE if the entry is new
 * @param $mmtid
 *   Tree ID of the new entry's parent ($add=TRUE), or the ID of the entry to
 *   replace
 * @param $parameters
 *   Please refer to mm_content_insert_or_update()
 */
function hook_mm_content_edit_submit_alter($add, $mmtid, &$parameters) {
}
/**
 * @} End of "addtogroup mm_hooks".
 */
