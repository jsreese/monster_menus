<?php
// $Id: mm_ui.inc 5041 2011-02-16 14:22:32Z vquinn $

/**
 * @file
 * User interface routines for monster_menus
 */

define('MM_UI_MAX_USERS_IN_GROUP', 20); // max number of users to display (there can be more in the DB)
define('MM_UI_MAX_REORDER_ITEMS', 100); // max number of nodes/subpages to reorder

/**
 * Allow the user to edit a node
 *
 * This code is lifted from node_page() in node.module and modified to use
 * callback arguments
 *
 * @param $node
 *   Node to delete
 * @return
 *   The HTML code for the results
 */
function mm_ui_node_edit($node) {
  if ($_POST['op'] == t('Delete')) {
    // Redirect, to make the tabs disappear.
    if ($_REQUEST['destination']) {
      $destination = drupal_get_destination();
      unset($_REQUEST['destination']);
    }
    mm_goto("node/$node->nid/delete", $destination);
    return;
  }

  return drupal_get_form($node->type .'_node_form', $node);
}

/**
 * Allow the user to act upon a list of revisions to a node
 *
 * @param $mmtid
 *   The MM tree ID associated with the node
 * @param $node
 *   The node being manipulated
 * @param $vid
 *   (Optional) revision being manipulated
 * @param $op
 *   (Optional) operation to perform, defaults to 'overview'
 * @param $compare
 *   (Optional) when $op = 'compare', the secondary revision to compare with
 * @return
 *   The HTML code for the results
 */
function mm_ui_node_revisions($mmtid, $node, $vid = NULL, $op = 'overview', $compare = NULL) {
  if ($vid && $op != 'compare') {
    $revision = node_load($node->nid, $vid);
    if (!$revision || !$revision->nid) {
      drupal_not_found();
      return;
    }
  }

  switch ($op) {
    case 'overview':
      mm_content_add_breadcrumb('');
      if (module_exists('diff')) return diff_diffs_overview($node);
      module_load_include('inc', 'node', 'node.pages');
      return node_revision_overview($node);

    case 'view':
      mm_content_add_breadcrumb(t('Revision of %title from @date',
          array('%title' => $revision->title,
          '@date' => format_date($revision->revision_timestamp))));

      return mm_node_show($node, 'revisions');

    case 'revert':
      $return_val = drupal_get_form('node_revision_revert_confirm', $revision);
      break;

    case 'delete':
      $return_val = drupal_get_form('node_revision_delete_confirm', $revision);
      break;

    case 'compare':
      if (module_exists('diff')) {
        $return_val = diff_diffs_show($node, $vid, $compare);
      }
  }

  if ($return_val) {
    $new_title = drupal_get_title();
    mm_content_restore_breadcrumb();
    if ($op == 'compare') {
      mm_content_add_breadcrumb($new_title);
    }
    else {
      $return_val = "<div>$new_title</div>$return_val";
    }

    return $return_val;
  }

  drupal_not_found();
}

/**
 * Present a node submission form or a set of links to such forms.
 *
 * This code is lifted from node_add() in node.module and modified to use
 * callback arguments
 *
 * @param $mmtid
 *   MM Tree ID to add the node to
 * @param $type
 *   Type of node to create(optional)
 * @return
 *   The HTML code for the results
 */
function mm_ui_node_add($mmtid, $type = '') {
  global $user;

  if (!mm_content_user_can($mmtid, 'u')) {
    $tree = mm_content_get($mmtid);

    return t('You are not allowed to assign the page %cat to content.', array('%cat' => mm_content_expand_name($tree->name)));
  }
  $allowed_node_types = mm_content_resolve_cascaded_setting('allowed_node_types', $mmtid, $types_at, $types_parent);

  // If a node type has been specified, validate its existence.
  $types = node_get_types();
  $type = isset($type) ? str_replace('-', '_', $type) : NULL;
  if (isset($types[$type]) && node_access('create', $type) && (user_access('administer all menus') || array_search($type, $allowed_node_types) !== FALSE)) {
    $node = (object) array(
      'uid'      => $user->uid,
      'name'     => isset($user->name) ? $user->name : '',
      'type'     => $type,
      'language' => '',
    );
    mm_content_add_breadcrumb(t('Create @name', array('@name' => $types[$type]->name)));

    return drupal_get_form($type . '_node_form', $node);
  }
  else {
    // If no (valid) node type has been provided, display a node type overview.
    mm_content_add_breadcrumb(t('Add content'));
    $hidden_types = mm_get_node_info(MM_NODE_INFO_ADD_HIDDEN);
    $admin_only_item = array();
    foreach ($types as $type) {
      $type_url_str = $type->type;
      $title = t('Add a new @s.', array('@s' => $type->name));
      $direct_link = "mm/$mmtid/contents/add/$type_url_str";
      $out = '<dt>'. l(drupal_ucfirst($type->name), $direct_link, array('attributes' => array('title' => $title))) .'</dt>';
      $out .= '<dd>'. filter_xss_admin($type->description) .'</dd>';
      if (node_access('create', $type) && array_search($type_url_str, $hidden_types) === FALSE && array_search($type_url_str, $allowed_node_types) !== FALSE) {
        $item[$type->name] = $out;
        $sole_direct_link = $direct_link;
      }
      else if (user_access('administer all menus') && array_search($type->type, $allowed_node_types) === FALSE) {
        $admin_only_item[$type->name] = $out;
      }
    }
    if (isset($item)) {
      if (count($item) == 1 && !user_access('administer all menus')) {
        drupal_goto($sole_direct_link);
      }
      uksort($item, 'strnatcasecmp');
      uksort($admin_only_item, 'strnatcasecmp');
      $output = '';

      if ($mmtid && variable_get('mm_hide_empty_pages', FALSE)) {
        $count = db_result(db_query(
          'SELECT COUNT(*) FROM {mm_node2tree} t '.
            'INNER JOIN {node} n ON t.nid = n.nid '.
            'WHERE mmtid = %d AND (n.status = %d OR n.uid = %d)',
          $mmtid, 1, $user->uid));
        if (!$count) {
          $output .= '<div id="message"><div class="messages status">'.
            t('Until you have added some content to this page, it will not appear in the menus for anyone who does not also have the ability to add content.') . '</div></div>';
        }
      }

      $output .= t('Choose the type of content to create using this page:') . '<dl>' . implode('', $item) . '</dl>';
      if (user_access('administer all menus') && count($admin_only_item) > 0) {
        drupal_add_css(drupal_get_path('module', 'monster_menus') . "/monster_menus.css");
        $output .= '<br /><div class="mm-admin-types">' . t('The following content types will only be displayed to admin users:') . '<dl>' . implode('', $admin_only_item) . '</dl></div>';
      }
      return $output;
    }

    return t('You are not allowed to create content.');
  }
}

function mm_ui_comment() {
  $args = func_get_args();
  $op = array_shift($args);
  if ($op == 'edit' || $op == 'delete' || $op == 'reply') {
    $this_title = drupal_get_title();

    $return_val = call_user_func_array("comment_$op", $args);
    if ($return_val) {
      $new_title = drupal_get_title();
      if ($new_title != $this_title) $return_val = "<div>$new_title</div>$return_val";
      mm_content_restore_breadcrumb();
      return $return_val;
    }
  }

  drupal_not_found();
}

function mm_ui_comment_redirect($default) {
  if (mm_parse_args($mmtids, $oarg_list, $this_mmtid) == 'mm')
    return "mm/$this_mmtid";
  return $default;
}

/**
 * Implementation of hook_form_alter().
 *
 * Remove some unneeded node creation options, and add some of our own. This
 * only works if MM happens to load after the modules that create these form
 * elements. This is controlled by the 'weight' column of the 'system' table.
 */
function monster_menus_form_alter(&$form, $form_state, $form_id) {
  global $user;

  if (isset($form['type']) && isset($form['#node']) && $form['type']['#value'] . '_node_form' == $form_id) {
    $node = $form['#node'];

    $mm = mm_parse_args($mmtids, $oarg_list, $this_mmtid);
    if (isset($form_state['values'])) {  // user clicked Preview button
      $values = $form_state['values'];
      $everyone = $values['others_w'];
      $grouplist = $values['groups_w'];
      $userlist = $values['users_w'];
      $uid = intval($values['owner']);
      $mmlist = $values['mm_catlist'];
      $mmlist_restricted = $values['mm_catlist_restricted'];
      $mmlist_restricted_link = $values['mm_catlist_restricted_link'];
    }
    else {
      $mmlist = $mmlist_restricted = $mmlist_restricted_link = array();
    }

    if (isset($form['title'])) {
      $form['title']['#description'] .= ' ' . t('To prevent the title from displaying when viewing a page, surround it with square brackets. Example: [My Title]');
    }

    if (isset($form['nid']['#value'])) {   // existing node
      if (count($mmtids))
        $form['#redirect'] = "mm/$this_mmtid";

      if (mm_content_node_is_recycled($node)) {
        _mm_ui_form_array_merge($form, 'mm_categories', array(
          '#type' => 'fieldset',
          '#title' => t('Pages'),
          '#weight' => 20,
          '#collapsible' => TRUE,
          '#collapsed' => FALSE,
        ));

        _mm_ui_recycle_page_list($node, $names, $msg);
        if (!count($names))
          $msg = t('This content is not associated with a page. If restored, it will be visible only by its direct web address.');
        else if (count($names) == 1)
          $msg = t('If you restore this content, it will return to the page !title.',
              array('!title' => $names[0]));
        else
          $msg = t('If you restore this content, it will return to the following pages: !list', array('!list' => implode(', ', $names)));

        $form['mm_categories']['mm_catlist'] = array(
          '#type' => 'value',
          '#value' => array(),
        );
        $form['mm_categories']['mm_catlist_restricted'] = array(
          '#type' => 'value',
          '#value' => $node->recycle_from_mmtids
        );
        // Needed for mm_content_node_is_recycled()
        $form['mm_categories']['recycle_date'] = array(
          '#type' => 'value',
          '#value' => $node->recycle_date
        );
        $form['mm_categories']['mm_catlist_readonly'] = array(
          '#type' => 'item',
          '#value' => t('The list of pages cannot be changed because this content is in the recycle bin.'),
          '#description' => $msg,
        );
      }
      else {  // !recycled
        $nid = $node->nid;

        if ($nid && !isset($values)) {
          foreach (mm_content_get(mm_content_get_by_nid($nid)) as $r)
            if (mm_content_user_can($r->mmtid, 'u'))
              $mmlist[$r->mmtid] = mm_content_expand_name($r->name);
            else {
              $mmlist_restricted[] = $r->mmtid;
              $mmlist_restricted_link[] = l(mm_content_expand_name($r->name), mm_content_get_mmtid_url($r->mmtid));
            }
        }

        _mm_ui_form_array_merge($form, 'mm_categories', array(
          '#type' => 'fieldset',
          '#title' => t('Pages'),
          '#weight' => 20,
          '#collapsible' => TRUE,
          '#collapsed' => TRUE,
        ));
        $form['mm_categories']['mm_catlist_restricted'] = array(
          '#type' => 'value',
          '#value' => $mmlist_restricted,
        );
        $required = TRUE;
        if (count($mmlist_restricted)) {
          $required = FALSE;
          $form['mm_categories']['mm_catlist_warning'] = array(
            '#type' => 'item',
            '#value' => t('<font color="red">This content will also appear on the !plur !list. You do not have permission to change this fact.</font>', array('!plur' => (count($mmlist_restricted) == 1 ? t('page') : t('pages: ')), '!list' => implode(', ', $mmlist_restricted_link))),
          );
        }
        $form['mm_categories']['mm_catlist'] = array(
          '#type' => 'mm_catlist',
          '#required' => $required,
          '#description' => t('Choose additional pages where this content will appear.'),
          '#mm_list_popup_start' => implode('/', $mmtids),
          '#default_value' => $mmlist,
        );
      }   // if (not) recycled

      if (!isset($values)) {
        $everyone = FALSE;
        $grouplist = array();
        $result = db_query(
          'SELECT nw.gid, t.name FROM {mm_node_write} nw '.
            'LEFT JOIN {mm_tree} t ON nw.gid = t.mmtid '.
          'WHERE nw.gid >= %d AND nid = %d ORDER BY t.name',
          0, $node->nid);
        while ($r = db_fetch_object($result)) {
          if ($r->gid == 0) {
            $everyone = TRUE;
          }
          else {
            $members = mm_content_get_users_in_group($r->gid, '<br />', FALSE, 20, TRUE);
            if ($members == '') $members = t('(none)');
            $grouplist[$r->gid]['name'] = mm_content_expand_name($r->name);
            $grouplist[$r->gid]['members'] = $members;
          }
        }

        $userlist = array();
        $result = db_query('SELECT gid FROM {mm_node_write} WHERE gid < %d AND nid = %d', 0, $node->nid);
        if ($r = db_fetch_object($result)) {
          $users = mm_content_get_users_in_group($r->gid, NULL, TRUE, MM_UI_MAX_USERS_IN_GROUP);
          if (!is_null($users)) $userlist = $users;
        }

        if (count($userlist) || count($grouplist)) {
          $everyone = FALSE;
        }
        $uid = $node->uid;
      }

      _mm_ui_node_form_perms($form, $userlist, $grouplist, $everyone, node_get_types('name', $node), $uid);
    }
    else {    // new node
      if (count($mmtids) || count($mmlist)) {
        if (!isset($values)) {
          $tree = mm_content_get($this_mmtid);
          $mmlist = array($this_mmtid => mm_content_expand_name($tree->name));
          mm_content_get_default_node_perms($this_mmtid, $grouplist, $userlist, MM_UI_MAX_USERS_IN_GROUP);
          $uid = $user->uid;
          $everyone = array_search('u', explode(',', $tree->default_mode)) !== FALSE;
          $node->show_node_info = $tree->node_info;
          $node->comment = $tree->comment;
          if (variable_get('mm_finegrain_comment_readability', FALSE)) {
            $node->comments_readable = mm_content_resolve_cascaded_setting('comments_readable', $this_mmtid, $cascaded_at, $cascaded_parent);
          }
          else {
            $node->comments_readable = '';
          }
        }

        _mm_ui_form_array_merge($form, 'mm_categories', array(
          '#type' => 'fieldset',
          '#title' => t('Pages'),
          '#weight' => 20,
          '#collapsible' => TRUE,
          '#collapsed' => TRUE,
        ));
        $form['mm_categories']['mm_catlist'] = array(
          '#type' => 'mm_catlist',
          '#description' => t('Choose additional pages where this content will appear.'),
          '#mm_list_popup_start' => implode('/', $mmtids),
          '#default_value' => $mmlist,
        );
        $form['mm_categories']['mm_catlist_restricted'] = array(
          '#type' => 'value',
          '#value' => $mmlist_restricted,
        );

        _mm_ui_node_form_perms($form, $userlist, $grouplist, $everyone, node_get_types('name', $node), $uid, t('The default settings are taken from the settings of the page. '));
        // webform sets $form_state['redirect'] after submit, but there's no way
        // to tell that before this point, so unfortunately we have to hard code
        if ($node->type != 'webform') $form['#redirect'] = "mm/$this_mmtid";
      }
    }   // if existing/new node
    $form['mm_catlist_restricted_link'] = array(
      '#type' => 'value',
      '#value' => $mmlist_restricted_link
    );

    unset($form['menu']);
    unset($form['options']['sticky']);

    _mm_ui_form_array_merge($form, 'mm_appearance', array(
      '#type' => 'fieldset',
      '#title' => t('Appearance'),
      '#weight' => 23,
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    ));
    if ($form_id == 'redirect_node_form') {   // redirector
      unset($form['comment_settings']);
      unset($form['attachments']);
      $form['comment'] = array(// comments = 'Disabled'
        '#type' => 'value',
        '#value' => 0,
      );
      $form['options']['#access'] = FALSE;
      unset($form['buttons']['preview']);
    }
    else {
      if (isset($form['comment_settings']['comment']) && !$node->nid) {
        $form['comment_settings']['comment']['#default_value'] = $node->comment;
      }
      // provide alternate 'sticky' checkbox
      $form['mm_appearance']['sticky'] = array(
        '#type' => 'checkbox',
        '#title' => t('Sticky at top of page'),
        '#description' => t('Content that is "sticky" will stay at the top of the page, even after other content is added. But the content\'s owner must be listed under %who (see %perm, under the %set tab) in order for this setting to take effect.', array('%who' => t('Who can delete this page or change its settings'), '%perm' => t('Permissions'), '%set' => t('Settings'))),
        '#default_value' => $node->sticky,
      );
    }

    $toggle = theme_get_setting('toggle_node_info_' . $node->type);
    if (user_access('show/hide post information') && $toggle) {
      $form['mm_appearance']['show_node_info'] = array(
        '#type' => 'select',
        '#title' => t('Attribution style'),
        '#options' => _mm_ui_node_info_values(),
        '#default_value' => $node->show_node_info,
      );
    }
    else {
      if (!$toggle && !isset($form['nid']['#value'])) {
        // Always default to off for new nodes when disabled in all nodes of
        // this type. This way, if enabled later on for all nodes of this type,
        // the end result won't immediately change.
        $node->show_node_info = 0;
      }
      $form['mm_appearance']['show_node_info'] = array(
        '#type' => 'value',
        '#value' => $node->show_node_info,
      );
    }

    _mm_ui_form_array_merge($form, 'scheduling', array(
      '#type' => 'fieldset',
      '#title' => t('Scheduling'),
      '#weight' => 25,
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    ));
    $publish_on = _mm_ui_date_to_string($node->publish_on);
    $unpublish_on = _mm_ui_date_to_string($node->unpublish_on);
    if (module_exists('date_popup') && !$values['_via_drupal_execute']) {
      _mm_ui_form_array_merge($form['scheduling'], 'publish_on', array(
        '#type' => 'date_popup',
        '#title' => t('Publish on'),
        '#default_value' => $publish_on,
        '#description' => t('Leave blank to disable scheduled publishing.'),
        '#no_time_default' => '12:00AM', // used in _mm_ui_date_popup_validate
        '#element_validate' => array('_mm_ui_date_popup_validate'),
        '#date_format' => 'n/j/Y - g:iA',  // force here, because jQuery date widget can't handle spaces in time format
      ));

      _mm_ui_form_array_merge($form['scheduling'], 'unpublish_on', array(
        '#type' => 'date_popup',
        '#title' => t('Unpublish on'),
        '#default_value' => $unpublish_on,
        '#description' => t('Leave blank to disable scheduled unpublishing.'),
        '#no_time_default' => '11:59PM', // used in _mm_ui_date_popup_validate
        '#element_validate' => array('_mm_ui_date_popup_validate'),
        '#date_format' => 'n/j/Y - g:iA',  // force here, because jQuery date widget can't handle spaces in time format
      ));
    }
    else {
      $t_now = array('%time' => format_date(time(), 'custom', variable_get('date_format_short', 'n/j/Y g:iA')));
      _mm_ui_form_array_merge($form['scheduling'], 'publish_on', array(
        '#type' => 'textfield',
        '#title' => t('Publish on'),
        '#maxlength' => 25,
        '#default_value' => $publish_on,
        '#description' => t('Format: %time. Leave blank to disable scheduled publishing.', $t_now),
      ));

      _mm_ui_form_array_merge($form['scheduling'], 'unpublish_on', array(
        '#type' => 'textfield',
        '#title' => t('Unpublish on'),
        '#maxlength' => 25,
        '#default_value' => $unpublish_on,
        '#description' => t('Format: %time. Leave blank to disable scheduled unpublishing.', $t_now),
      ));
    }
    _mm_ui_form_array_merge($form['scheduling'], 'set_change_date', array(
      '#type' => 'checkbox',
      '#title' => t('Use <em>Publish on</em> date for attributions'),
      '#default_value' => $node->set_change_date,
      '#description' => t('Use the date above instead of the content\'s last modified date when showing <em>Submitted on [date]</em> or <em>Submitted by [user] on [date]</em>'),
    ));

    if (isset($form['author'])) {
      unset($form['author']['name']);
      $form['mm_appearance']['author'] = $form['author'];
      unset($form['author']);
      if ($form['mm_appearance']['author']['#access'] !== FALSE && isset($form['mm_appearance']['author']['date']) && module_exists('date_popup') && !$values['_via_drupal_execute']) {
        $form['mm_appearance']['author']['date']['#type'] = 'date_popup';
        $form['mm_appearance']['author']['date']['#description'] = '';
        unset($form['mm_appearance']['author']['date']['#maxlength']);
      }
    }

    if (isset($form['taxonomy'])) {
      $form['body_field']['#weight'] = -4;
    }

    // Admin users don't need the set author option, they should use "Who can edit/delete"
    if (isset($form['settings_perms']['owner-choose']) && isset($form['author']['name'])) {
      $form['author']['name'] = array(
        '#value' => t('<p>To change the author, set the %owner in %where, above.</p>',
          array(
            '%owner' => t('Owner'),
            '%where' => t('Who can edit or delete')
          ))
      );
    }

    if (isset($form['comment_settings'])) {
      $form['comment_settings']['#access'] = user_access('enable/disable comments') || user_access('administer comments') || variable_get('mm_finegrain_comment_readability', FALSE);
      if (variable_get('mm_finegrain_comment_readability', FALSE)) {
        $form['comment_settings']['comments_readable'] = array(
          '#type' => 'select',
          '#title' => t('Who can read comments'),
          '#default_value' => $node->comments_readable,
          '#options' => _mm_ui_comment_read_setting_values(t('(use default setting)')),
          '#weight' => -1,
        );
      }
      $form['comment_settings']['comment']['#access'] = user_access('enable/disable comments') || user_access('administer comments');
      $form['comment_settings']['comment']['#options'] = _mm_ui_comment_write_setting_values();
      if (variable_get('mm_finegrain_comment_readability', FALSE)) {
        $form['comment_settings']['comment']['#type'] = 'select';
        $form['comment_settings']['comment']['#title'] = t('Who can add comments');
      }
    }

    $form['buttons']['required-note'] = array('#type' => 'markup' , '#weight' => 51 , '#value' => t('<div class=\"requiredfields\"><span class="form-required">*</span> denotes required fields</div>'));
  }
  else if ($form_id == 'node_type_form' && isset($form['identity']['type'])) {
    // This can't be done in a monster_menus_form_node_type_form_alter() since
    // it gets called too early.
    if (isset($form['comment']['comment'])) {
      $form['comment']['comment']['#description'] = t('Users with the <em>administer comments</em> or <em>enable/disable comments</em> permission will be able to override this setting.');
    }
  }
}

function monster_menus_form_system_theme_settings_alter(&$form, $form_state) {
  $form['node_info']['#description'] = t('Because you are using Monster Menus, the setting for <em>Submitted by [username] on [date]</em> text is controlled independently, within each node. The setting below acts as a global override for all nodes of the given type. If not checked, the text is never displayed for those nodes.');
}

function monster_menus_form_user_admin_role_alter(&$form, $form_state) {
  _mm_ui_form_array_merge($form, 'mm_group', array(
    '#type' => 'fieldset',
    '#title' => t('Add group members'),
    '#description' => t('The members of the chosen Monster Menus group will be added to this role.'),
    '#collapsible' => TRUE, '#collapsed' => FALSE));
  $current = db_fetch_object(db_query('SELECT * FROM {mm_role2group} r2 LEFT JOIN {mm_tree} t ON r2.gid = t.mmtid WHERE r2.rid = %d', $form['rid']['#value']));
  if (!$current) $current = (object)array('negative' => 0);
  $form['mm_group']['mm_group'] = array(
    '#type' => 'mm_grouplist',
    '#mm_list_popup_start' => mm_content_groups_mmtid(),
    '#mm_list_max' => 1,
    '#default_value' => $current->gid ? array($current->gid => mm_content_expand_name($current->name)) : NULL
  );
  $form['mm_group']['mm_group_direction'] = array(
    '#type' => 'radios',
    '#options' => array(0 => t('Add all users <b>in</b> this group'),
        1 => t('Add all users <b>not in</b> this group')),
    '#default_value' => $current->negative
  );
  $form['#submit'][] = '_mm_ui_submit_role';
  $form['submit']['#weight'] = 9998;
  $form['delete']['#weight'] = 9999;
}

function _mm_ui_submit_role($form, &$form_state) {
  db_query('DELETE FROM {mm_role2group} WHERE rid=%d', $form_state['values']['rid']);
  // If the Save button was chosen (instead of Delete)
  if ($form_state['values']['submit'] == $form_state['values']['op']) {
    $gid = mm_ui_mmlist_key0($form_state['values']['mm_group']);
    if ($gid)
      db_query('INSERT INTO {mm_role2group} (rid, gid, negative) VALUES (%d, %d, %d)', $form_state['values']['rid'], $gid, $form_state['values']['mm_group_direction']);
  }
}

/**
 * Implementation of hook_elements().
 *
 * This hook declares to Drupal what form elements are provided by the module.
 */
function monster_menus_elements() {
  $type['mm_catlist'] = $type['mm_grouplist'] = $type['mm_userlist'] = $type['mm_nodelist'] = array(
      '#input' => TRUE,
      '#default_value' => array(),
      '#process' => array('_mm_ui_process_mmlist'),
      '#mm_list_min' => 0,             // min number of rows
      '#mm_list_max' => 0,             // max number of rows
      '#mm_list_popup_start' => '',
      '#mm_list_popup_root' => 1,
      '#mm_list_enabled' => '',        // in category browser, attribute of cats user can open
      '#mm_list_selectable' => 'u',    // in category browser, attribute of cats user can choose
      '#mm_list_buttons_underneath' => FALSE,
      '#mm_list_readonly' => FALSE,    // let the user select rows, but not edit them
      '#mm_list_no_info' => FALSE,     // don't show an item's info when clicked
      '#mm_list_browser' => 'mm-browser-load', // path to the popup tree browser
      '#mm_list_other_name' => '',
      '#mm_list_other_callback' => 'null');
  $type['mm_repeatlist'] = array(
      '#input' => TRUE,
      '#default_value' => array(),
      '#mm_list_id' => '',             // REQUIRED: form ID of repeatable DIV (mm_repeatlist_div type)
      '#mm_list_inputs_per_row' => '', // REQUIRED: number of values per row in repeatable DIV and #default_value
      '#mm_list_min' => 1,             // min number of rows
      '#mm_list_max' => 0,             // max number of rows
      '#mm_list_add_button' => t('Add a Row'), // text label for the 'add' button
      '#mm_list_buttons_underneath' => TRUE,
      '#mm_list_readonly' => FALSE,    // display the data rows as text instead of form inputs
      '#mm_list_reorder' => FALSE);    // show the up/down arrows in each row
  $type['mm_repeatlist_div'] = array();
  $type['mm_help_radios'] = array(
      '#input' => TRUE,
      '#process' => array('mm_ui_expand_mm_help_radios'));
  $type['mm_help_radio'] = array(
      '#input' => TRUE);
  $type['mm_fake_required'] = array(
      '#input' => TRUE,
      '#mm_orig_type' => '');          // REQUIRED: '#type' of element to render
  return $type;
}

// split result generated by setHiddenElt in mm.js
function _mm_ui_process_mmlist($element, $edit, &$form_state = NULL, $form = NULL) {
  if (is_string($edit)) {
    $element['#value'] = array();
    if (preg_match_all('#(\d+(?:/\d+)?)\{([^}]*)\}#', $edit, $matches, PREG_SET_ORDER)) {
      foreach ($matches as $match) {
        $element['#value'][$match[1]] = $match[2];
      }
    }
  }
  return $element;
}

// Kept for backward compatibility. Usually no longer needed, now that
// _mm_ui_process_mmlist handles this automatically.
function _mm_ui_split_mmlist($str) {
  if (is_array($str)) {
    $matches = array();
    foreach ($str as $key => $val) {
      $matches[] = array(NULL, $key, $val);
    }
    return $matches;
  }
}

// Return the first array key of an array. Useful when retrieving the first key
// from an mm_list value array.
function mm_ui_mmlist_key0($arr) {
  if (!is_array($arr)) return;
  reset($arr);
  if (count($arr)) {
    return key($arr);
  }
}

/**
 * Parse the hidden data field generated by an mm_repeatlist form element into a
 * set of arrays.
 *
 * @param $str
 *   Form data to parse
 * @param $per_row
 *   The number of data elements per row (set)
 * @return
 *   An array of arrays. Each inner array represents one set of data entered by
 *   the user.
 */
function mm_ui_parse_repeatlist($str, $per_row) {
  if ($per_row <= 0 || !preg_match_all('/\{:(.*?):\}/', $str, $matches, PREG_PATTERN_ORDER)) return array();
  return array_chunk($matches[1], $per_row);
}

function mm_ui_validate_fake_required($elt, $value, $message = NULL) {
  if ($elt['#type'] == 'mm_fake_required' && empty($value) && $value !== '0') {
    form_error($elt, empty($message) ? t('!name field is required.', array('!name' => $elt['#title'])) : $message);
    return FALSE;
  }
  return TRUE;
}

function mm_ui_expand_mm_help_radios($element, $edit, &$form_state, $form) {
  if (count($element['#options']) > 0) {
    foreach ($element['#options'] as $key => $choice) {
      if (!isset($element[$key])) {
        $element[$key] = array('#type' => 'mm_help_radio', '#title' => $choice,
            '#return_value' => $key, '#default_value' => $element['#default_value'],
            '#attributes' => $element['#attributes'],
            '#parents' => $element['#parents'], '#spawned' => TRUE);
      }
    }
  }
  return $element;
}

/**
 * Get a list of possible variable substitutions
 *
 * @param $weight
 *   Weight of the resulting form element
 * @param $list
 *   Array of human-readable field names to which the help text applies
 * @param $xvars
 *   Optional array of system variables which are allowed
 * @return
 *   A form element containing the help text
 */
function mm_ui_vars_help($weight, $list, $xvars = NULL) {
  global $user;

  $uid = db_result(db_query_range('SELECT uid FROM {users} WHERE uid > 1', 0, 1));
  if (!$uid) $uid = $user->uid;

  $vars = array();
  foreach (user_load(array('uid' => $uid)) as $k => $v)
    if (is_scalar($v)) $vars[] = '${' . $k . '}';

  sort($vars);
  $vars = join("\n", $vars);

  foreach ($list as &$l)
    $l = theme('placeholder', $l);
  $last = array_pop($list);
  $list = join(', ', $list) . " or $last";

  $ret = array(
    '#type' => 'fieldset',
    '#title' => t('Variable substitution'),
    '#weight' => $weight,
    '#collapsible' => TRUE, '#collapsed' => TRUE,
    '#description' => t('The variables below can be inserted into the fields !list.<br />Variables which describe the current user:',
        array('!list' => $list)) . "<pre>$vars</pre>",
  );
  if (count($xvars)) {
    sort($xvars);
    $xvars = '${' . join("}\n\${", $xvars) . '}';
    $ret['#description'] .= t("System variables:<pre>$xvars</pre>");
  }
  return $ret;
}

function mm_ui_mmlist_labels() {
  return array(t('Move to top'), t('Move up'), t('Delete this row'),
      t('Move to bottom'), t('Move down'), t('Edit'));
}

/**
 * Escape certain sequences for output as part of Javascript code
 *
 * @param $string
 *   The code to be escaped
 * @return
 *   The escaped code
 */
function mm_ui_js_escape($string) {
  return str_replace(
    array("\r", "\n", '<', '>', '&', '{', '}', '"'),
    array('', '', '\x3c', '\x3e', '\x26', '&#123;', '&#125;', '&quot;'),
    addslashes($string));
}

/**
 * Return TRUE if content of the given type should show the author (attribution)
 *
 * @param $type
 *   Name of the content type to test ('article', 'story', etc.)
 * @return
 *   TRUE if the attribution should be shown
 */
function mm_ui_show_author($type) {
  $settings = variable_get('theme_settings', array());
  return $settings['toggle_node_info_' . $type];
}

/**
 * Implementation of hook_theme().
 */
function monster_menus_theme() {
  $themes = array(
    'mm_catlist' => array(
        'file' => 'mm_theme.inc',
        'arguments' => array('elt' => NULL)),
    'mm_grouplist' => array(
        'file' => 'mm_theme.inc',
        'arguments' => array('elt' => NULL)),
    'mm_userlist' => array(
        'file' => 'mm_theme.inc',
        'arguments' => array('elt' => NULL)),
    'mm_nodelist' => array(
        'file' => 'mm_theme.inc',
        'arguments' => array('elt' => NULL)),
    'mm_admin_list_blocks' => array(
        'file' => 'mm_theme.inc',
        'arguments' => array('form' => NULL)),
    'mm_admin_verify_integrity' => array(
        'file' => 'mm_theme.inc',
        'arguments' => array('form' => NULL)),
    'tooltip' => array(
        'file' => 'mm_theme.inc',
        'arguments' => array('text' => NULL, 'title' => NULL, 'tip' => NULL, 'html' => FALSE)),
    'mm_repeatlist_div' => array(
        'file' => 'mm_theme.inc',
        'arguments' => array('elt' => NULL)),
    'mm_fake_required' => array(
        'file' => 'mm_theme.inc',
        'arguments' => array('elt' => NULL)),
    'mm_repeatlist' => array(
        'file' => 'mm_theme.inc',
        'arguments' => array('elt' => NULL)),
    'mm_help_radio' => array(
        'file' => 'mm_theme.inc',
        'arguments' => array('element' => NULL)),
    'mm_help_radios' => array(
        'file' => 'mm_theme.inc',
        'arguments' => array('element' => NULL)),
    'mm_ui_mark_yesno' => array(
        'file' => 'mm_theme.inc',
        'arguments' => array('yes' => NULL)),
    'mm_ui_menu_reorder' => array(
        'file' => 'mm_theme.inc',
        'arguments' => array('element' => NULL)),
    'mm_ui_node_reorder' => array(
        'file' => 'mm_theme.inc',
        'arguments' => array('element' => NULL)),
    'mm_ui_permissions' => array(
        'file' => 'mm_theme.inc',
        'arguments' => array('element' => NULL)),
    'mm_ui_node_permissions' => array(
        'file' => 'mm_theme.inc',
        'arguments' => array('element' => NULL)),
    'mm_archive_header' => array(
        'file' => 'mm_theme.inc',
        'arguments' => array('frequency' => NULL, 'date' => NULL,
          'date_in_url' => NULL)),
    'mm_archive' => array(
        'file' => 'mm_theme.inc',
        'arguments' => array('list' => NULL, 'frequency' => NULL,
          'this_mmtid' => NULL, 'main_mmtid' => NULL, 'archive_mmtid' => NULL,
          'date' => NULL)),
    'mm_page_wrapper' => array(
        'file' => 'mm_theme.inc',
        'arguments' => array('title' => NULL, 'body' => NULL, 'attributes' => NULL, 'wrap_body_with_div' => NULL)),
    'mm_tabbed_ui' => array(
        'file' => 'mm_theme.inc',
        'arguments' => array('title' => NULL, 'outer_id' => NULL, 'tabs' => NULL, 'selected' => '', 'before_tabs' => '')),
  );
  foreach (mm_node_types() as $type) {
    $themes = array_merge($themes, $type['themes']);
  }
  return $themes;
}

/**
 * Return "(untitled)" when a node's title is empty
 *
 * @param $title
 *   The title
 * @return
 *   The original title, trimmed, or "(untitled)"
 */
function mm_ui_fix_node_title($title) {
  $title = trim($title);
  if (empty($title)) return t('(untitled)');
  return $title;
}

/**
 * Hide a node's title when it is surrounded by [brackets]
 *
 * @param $title
 *   The title
 * @return
 *   The (possibly hidden) title
 */
function mm_ui_hide_node_title($title) {
  return preg_replace('/^\[.*?\]$/', '', $title);
}

/**
 * Modify certain form elements to make them appear read-only to the user
 *
 * @param $form
 *   The form array
 * @param $field_suffix
 *   String to be inserted into the field names
 */
function mm_ui_perms_readonly(&$form, $field_suffix) {
  unset($form["users_$field_suffix-choose"]);
  $form["users_$field_suffix"]['#mm_list_readonly'] = TRUE;
  $form["users_$field_suffix"]['#mm_list_no_info'] = TRUE;
  $form["users_$field_suffix"]['#mm_list_buttons_underneath'] = TRUE;
  $form["groups_$field_suffix"]['#mm_list_readonly'] = TRUE;
  // Prevent the vertical bar from appearing when there are no groups
  if (!count($form["groups_$field_suffix"]['#default_value']))
    $form["groups_$field_suffix"]['#mm_list_buttons_underneath'] = TRUE;
}

/**
 * Get a list of times of day
 *
 * $start
 *   If set, the starting number of minutes past midnight
 * $end
 *   If set, the ending number of minutes past midnight
 * $inc
 *   If set, the increment, in minutes, between times; the smallest supported
 *   number is 30 minutes
 * @return
 *   An array of times, indexed by the number of minutes past midnight
 */
function mm_ui_hour_list($start = NULL, $end = NULL, $inc = NULL) {
  $out = array(0 => t('midnight'), 30 => t('12:30 AM'), 60 => t('1:00 AM'), 90 => t('1:30 AM'), 120 => t('2:00 AM'), 150 => t('2:30 AM'), 180 => t('3:00 AM'), 210 => t('3:30 AM'), 240 => t('4:00 AM'), 270 => t('4:30 AM'), 300 => t('5:00 AM'), 330 => t('5:30 AM'), 360 => t('6:00 AM'), 390 => t('6:30 AM'), 420 => t('7:00 AM'), 450 => t('7:30 AM'), 480 => t('8:00 AM'), 510 => t('8:30 AM'), 540 => t('9:00 AM'), 570 => t('9:30 AM'), 600 => t('10:00 AM'), 630 => t('10:30 AM'), 660 => t('11:00 AM'), 690 => t('11:30 AM'), 720 => t('noon'), 750 => t('12:30 PM'), 780 => t('1:00 PM'), 810 => t('1:30 PM'), 840 => t('2:00 PM'), 870 => t('2:30 PM'), 900 => t('3:00 PM'), 930 => t('3:30 PM'), 960 => t('4:00 PM'), 990 => t('4:30 PM'), 1020 => t('5:00 PM'), 1050 => t('5:30 PM'), 1080 => t('6:00 PM'), 1110 => t('6:30 PM'), 1140 => t('7:00 PM'), 1170 => t('7:30 PM'), 1200 => t('8:00 PM'), 1230 => t('8:30 PM'), 1260 => t('9:00 PM'), 1290 => t('9:30 PM'), 1320 => t('10:00 PM'), 1350 => t('10:30 PM'), 1380 => t('11:00 PM'), 1410 => '11:30 PM');

  foreach (array_keys($out) as $key) {
    if ($inc && ($key % $inc) != 0)
      unset($out[$key]);

    if (!is_null($start) && !is_null($end) && ($key < $start || $key >= $end))
      unset($out[$key]);
  }
  return $out;
}

/**
 * Get a list of long week day names
 *
 * @return
 *   An array of weekday names, starting with Sunday at element 0
 */
function mm_ui_day_list() {
  if (function_exists('date_week_days')) return date_week_days(TRUE);
  return array(t('Sunday'), t('Monday'), t('Tuesday'), t('Wednesday'), t('Thursday'), t('Friday'), t('Saturday'));
}

// ****************************************************************************
// * Private functions start here
// ****************************************************************************

function _mm_ui_levels() {
  $list[-1] = t('(show all levels)');
  $list[0] = t('(show only the currently selected level)');
  for ($i = 1; $i <= 10; $i++)
    $list[$i] = $i;

  return $list;
}

/**
 * Validation common to copying/moving and editing a tree entry
 */
function _mm_ui_validate_entry($mmtid, $test_mmtid, $form_values, $is_new, $no_dup) {
  $x = mm_ui_strings($is_group = mm_content_is_group($test_mmtid));

  $test_tree = mm_content_get_tree($test_mmtid, array(MM_GET_TREE_DEPTH => 1, MM_GET_TREE_FILTER_HIDDEN => TRUE));

  if (!$test_mmtid || count($test_tree) < ($is_new ? 1 : 2) || !$is_new && $test_tree[1]->parent != $test_tree[0]->mmtid) {
    form_set_error('', t('Unexpected tree structure'));
    return 0;
  }

  $alias = $is_group ? '' : trim($form_values['alias']);
  $name = trim($form_values['name']);

  if (!user_access('administer all menus') && $name[0] == '.') {
    form_set_error('name',
        t('@thing names starting with a dot (.) are reserved for administrators.', $x));
  }

  if (preg_match('/^[-\.\w]*$/', $alias) == 0) {
    form_set_error('alias',
        t('You have entered one or more invalid characters in the URL name.'));
  }
  else {
    $reserved = preg_grep('{^' . preg_quote($alias) . '$}i', variable_get('mm_reserved_alias', array()));
    if ($reserved) {
      form_set_error('alias',
          t('The URL name %alias is not allowed. Please try changing it slightly.',
            array('%alias' => $alias)));
    }
  }

  for ($i = 1; $i < count($test_tree); $i++)
    if ($no_dup || $test_tree[$i]->mmtid != $mmtid) {
      if (strcasecmp($test_tree[$i]->name, $name) == 0) {
        $x['%name'] = $test_tree[$i]->name;
        form_set_error('name',
            t('A @thing named %name already exists at this level of the tree.', $x));
      }

      if ($alias != '' && strcasecmp($test_tree[$i]->alias, $alias) == 0) {
        $x['%name'] = $test_tree[$i]->name;
        $x['%alias'] = $test_tree[$i]->alias;
        form_set_error('alias',
            t('The @thing named %name is already using the URL name %alias at this level of the tree.', $x));
      }
    }

  return 1;
}

function _mm_ui_verify_userlist($form_elem, $elt_name) {
  if (is_array($form_elem)) {
    foreach ($form_elem as $uid => $name)
      if (mm_content_uid2name($uid) === FALSE)
        form_set_error($elt_name, t('There is no user with uid=@uid and name=@name.', array('@uid' => $uid, '@name' => $name)));
  }
  else if (is_numeric($form_elem)) {
    $uid = intval($form_elem);
    if (mm_content_uid2name($uid) === FALSE)
      form_set_error($elt_name, t('There is no user with uid=@uid.', array('@uid' => $uid)));
  }
}

/**
 * Return various versions of strings to be used in UI messages
 *
 * @param $is_group
 *   If TRUE, return strings related to groups
 * @return
 *   An associative array of strings, suitable to be passed to t()
 */
function mm_ui_strings($is_group) {
  $x['@thing'] =     $is_group ? t('group')      : t('page');
  $x['@things'] =    $is_group ? t('groups')     : t('pages');
  $x['@thingpos'] =  $is_group ? t('group\'s')   : t('page\'s');
  $x['@subthing'] =  $is_group ? t('sub-group')  : t('sub-page');
  $x['@subthings'] = $is_group ? t('sub-groups') : t('sub-pages');
  foreach (array_keys($x) as $key)
    $x['!' . substr($key, 1)] = $x[$key];
  return $x;
}

function _mm_ui_userlist_setup($users, &$form, $form_id, $title, $single, $desc, $other_name = '', $large_group = FALSE) {
  if (!is_null($users)) {
    $form["$form_id-choose"] = array(
      '#type' => 'textfield',
      '#title' => $single ? t('Choose the owner') : t('Add a user'),
      '#autocomplete_path' => 'mm-auto',
      '#description' => t('Type part of the person\'s last name, first name, or username. Choose a name from the list. If there are too many matches, use the format %lastfirst.',
        array('%lastfirst' => t('last name, first name'))),
      '#size' => 30,
      '#maxlength' => 40,
    );
    $form[$form_id] = array(
      '#type' => 'mm_userlist',
      '#description' => $desc,
      '#title' => $title,
      '#default_value' => $users,
      '#required' => $single,
      '#mm_list_autocomplete_name' => "$form_id-choose",
      '#mm_list_min' => $single,
      '#mm_list_max' => $single,
      '#mm_list_other_name' => $other_name
    );
  }
  else {
    if ($large_group) {
      $form["$form_id-add"] = array(
        '#type' => 'markup',
        '#value' => mm_ui_add_user_subform('mmUserAddUsers', t('Add users to this group'), t('Add user(s) to the group:'), t('Add users to group'), 'Drupal.mmGroupAddUser'),
      );
      $headers = _mm_ui_userlist_get_headers();
      $output = '<table cellpadding="0" cellspacing="0" border="0" class="tablesorter" id="mm-user-datatable-' . str_replace('_', '-', $form_id) . '" width="100%">';
      $output .= '<thead><tr>';
      foreach ($headers as $value) {
        $class = empty($value) ? ' class="no-sort"' : '';
        $output .= '<th' . $class . '>' . $value . '</th>';
      }
      $output .= '</tr></thead>';
      $output .= '<tbody><tr><td colspan="' . count($headers) . '" class="dataTables_empty">' . t('Loading data from server') . '</td></tr></tbody>';
      $output .= '</table>';
      mm_dep_add_libs('dataTables');
      $form[$form_id . '-additional'] = array(
        '#type' => 'markup',
        '#value' => $output,
      );
      $form[$form_id . '-use-large-group'] = array(
        '#type' => 'hidden',
        '#value' => 'yes',
        '#process' => array('mm_ui_process_large_group'),
      );
    }
    else {
      $form[$form_id . '-additional'] = array('#value' => t('There are more members in this list than can be displayed. The list can only be edited by directly modifying the database table.'));
    }
  }
}

function mm_ui_add_user_subform($id, $link, $title, $button_text, $click_func_name, $owner_uid = NULL, $owner_name = NULL) {
  static $instance = 0;
  drupal_add_js("\$('#mm-add-users-$instance').click(function() {setTimeout(function() {\$('#edit-adduser-choose$instance').focus();}, 500);});", 'inline', 'footer');
  drupal_set_content('footer', "<div id=\"$id\" style=\"display: none\"><div class=\"mm-add-users\">" . drupal_get_form('mm_ui_large_group_add_users_form', $instance, $title, $button_text, $click_func_name, $owner_uid, $owner_name) . '</div></div>');
  $link = "<a id=\"mm-add-users-$instance\" href=\"#TB_inline?height=300&width=500&inlineId=$id\" class=\"thickbox\">$link</a>";
  $instance++;
  return $link;
}

function mm_ui_large_group_add_users_form(&$form_state, $instance, $title, $button_text, $click_func_name, $owner_uid = NULL, $owner_name = NULL) {
  $form['selectuser'] = array(
    '#type' => 'fieldset',
  );
  $form['selectuser']["adduser-choose$instance"] = array(
    '#type' => 'textfield',
    '#title' => t('Add a user'),
    '#autocomplete_path' => 'mm-auto',
    '#description' => t('Type part of the person\'s last name, first name, or username. Choose a name from the list. If there are too many matches, use the format %lastfirst.',
      array('%lastfirst' => t('last name, first name'))),
    '#size' => 30,
    '#maxlength' => 40,
  );
  $single = is_null($owner_uid) ? 0 : 1;
  $users = array();
  if ($single) $users[$owner_uid] = $owner_name;
  $form['selectuser']["adduser$instance"] = array(
    '#type' => 'mm_userlist',
    '#title' => $title,
    '#required' => $single,
    '#default_value' => $users,
    '#mm_list_autocomplete_name' => "adduser-choose$instance",
    '#mm_list_min' => $single,
    '#mm_list_max' => $single,
  );
  $form['adduser$instance-submit'] = array(
    '#type' => 'markup',
    '#value' => "<input type=\"button\" name=\"adduser$instance-submit\" value=\"$button_text\" onclick=\"return $click_func_name(\$(this).siblings('fieldset').find('.mm-list-hidden'), 'members')\">",
  );
  return $form;
}

function mm_ui_large_group_add_users_form_submit($form, &$form_state) {
  mm_parse_args($term_ids, $oarg_list, $this_tid);
  $recs_added = 0;
  foreach ($form_state['values']["adduser"] as $key => $value) {
    if (!db_result(db_query("SELECT * FROM {mm_group_temp} WHERE gid = %d AND uid = %d AND sessionid = '%s'", $this_tid, $key, session_id()))) {
      db_query("INSERT INTO {mm_group_temp} SET gid = %d, uid = %d, sessionid = '%s'", $this_tid, $key, session_id());
      $recs_added++;
    }
  }
  drupal_set_message(t('Added %recs_added user(s) to this group.', array('%recs_added' => $recs_added)), 'status', $repeat = TRUE);
}

function mm_ui_process_large_group($element, $edit, &$form_state = NULL, $form = NULL) {
  mm_parse_args($term_ids, $oarg_list, $this_tid);
  if (empty($_POST)) { // This will right group data even if the group fails validation, but none of the information passed seems to offer an alternative
    // Copy group data to the temp table
    db_query("DELETE FROM {mm_group_temp} WHERE gid = %d AND sessionid = '%s'", $this_tid, session_id());
    db_query("INSERT INTO {mm_group_temp} (gid, uid, sessionid) SELECT gid, uid, '%s' FROM {mm_group} WHERE gid = %d", session_id(), $this_tid);
  }
  return $element;
}

/**
 * Get the headers for the datatable used in large group management
 */
function _mm_ui_userlist_get_headers() {
  $headers = mm_module_invoke_all_array('mm_large_group_header', array());
  if (empty($headers)) {
    return array(t('Username'), '');
  }
  return $headers;
}

function _mm_ui_is_user_home(&$item) {
  if ($item->parent == mm_content_users_mmtid()) {
    $item->is_user_home = TRUE;
    $item->flags['limit_alias'] = '';
    $item->flags['limit_delete'] = '';
    $item->flags['limit_hidden'] = '';
    $item->flags['limit_location'] = '';
    $item->flags['limit_move'] = '';
    $item->flags['limit_name'] = '';
    $item->flags['limit_write'] = '';
  }
}

function mm_ui_owner_desc($x, $item_uid, $is_search = FALSE) {
  global $user;

  if ($is_search || user_access('administer all menus')) {
    $msg = t('The owner always has full access.', $x);
    if ($form['#id'] == 'node-form') $msg .= ' ' . t('The owner is also publicly visible as the submitter.');
    return array(mm_ui_uid2name($item_uid), $msg, FALSE);
  }

  $msg = '';
  if ($user->uid != $item_uid) {
    $owner = user_load(array('uid' => $item_uid));
    if ($owner) {
      $x['!name'] = mm_ui_uid2name($owner->uid);
      $msg = t('This @thing is owned by !name, who always has full access to it.', $x);
    }
  }
  else {
    $msg = t('As the owner of this @thing, you always have full access to it.', $x);
  }

  return array(mm_ui_uid2name($item_uid), $msg, TRUE);
}

function mm_ui_uid2name($uid, $link = FALSE) {
  if ($link) {
    if ($usr = user_load($uid)) {
      return theme('username', $usr);
    }
  }
  else if (($owner = mm_content_uid2name($uid, 'fmlu')) !== FALSE) {
    return $owner;
  }
  return t('Unknown user #!uid', array('!uid' => $uid));
}

function _mm_ui_node_form_perms(&$form, $userlist, $grouplist, $everyone,
    $item_type, $item_uid, $new_msg = NULL) {
  require_once('mm_browser.inc');

  // don't allow non-admins to permit everyone for write
  $allow_ev = user_access('administer all menus');

  _mm_ui_form_array_merge($form, 'settings_perms', array(
    '#type' => 'fieldset',
    '#title' => t('Who can edit or delete this content'),
    '#weight' => 21,
    '#collapsible' => TRUE, '#collapsed' => TRUE,
  ));
  $form['settings_perms']['table']['#theme'] = 'mm_ui_node_permissions';
  $form['settings_perms']['table']['#weight'] = 21;

  if ($allow_ev) {
    $form['settings_perms']['table']['everyone'] = array(
      '#type' => 'value',
      '#value' => array(
        'title' => t('Everyone'),
        'headings' => TRUE,
      ),
    );
    $checkbox = array(
      'node-everyone' => array(
        '#type' => 'checkbox',
        '#title' => t('Everyone can edit or delete this content'),
        '#default_value' => $everyone,
      ),
      'owner' => array(
        '#type' => 'hidden',
        '#default_value' => $item_uid,
      ),
    );
    $form['settings_perms']['table']['everyone'][] = _mm_ui_perms_table_row('', 'others_w', $checkbox, '');
  }

  $form['settings_perms']['table']['indiv_tbl'] = array();
  if (!$is_search) {
    $form['settings_perms']['table']['indiv_tbl'][] = array(
      '#type' => 'markup',
      '#mm_delete_link' => '<a href="#" title="' . t('Remove this user') . '" onclick="return Drupal.MMSettingsPermsDelete(this)">' . t('delete') . '</a>',
      '#mm_users' => $userlist,
      '#mm_owner' => array(
        'type' => $item_type,
        'uid' => $item_uid,
      ),
    );

    $form['settings_perms']['all_values_user'] = array('#type' => 'hidden');
  }

  $form['settings_perms']['table']['groups_tbl'] = array(
    '#type' => 'value',
    '#value' => array(
      'title' => t('Groups'),
      'action' => '<a href="#" title="' . t('Add a group') . '" onclick="return Drupal.MMSettingsPermsAddGroup()">' . t('add') . '</a>',
    )
  );
  if (!$is_search) {
    $elem = array(
      array(
        '#type' => 'fieldset',
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        '#title' => 'temp title',
        array(
          '#type' => 'item',
          '#value' => '.',
        ),
      )
    );
    $form['settings_perms']['table']['groups_tbl'][] = array(
      '#type' => 'markup',
      '#mm_delete_link' => '<a href="#" title="' . t('Remove this group') . '" onclick="return Drupal.MMSettingsPermsDelete(this)">' . t('delete') . '</a>',
      '#mm_groups' => $grouplist,
      '#mm_groups_elem' => $elem,
    );

    $form['settings_perms']['all_values_group'] = array('#type' => 'hidden');
  }
  mm_static('settings_perms', TRUE);
}

function _mm_ui_perms_table_row($elem_name, $elem_id, $name, $msg = '', $action = NULL, $types = array(), $x = NULL, $checks = array()) {
  $out = array('#mm_is_data_row' => is_numeric($elem_id));
  $out[] = is_array($name) ? $name : array(
    '#type' => 'item',
    '#value' => $name,
    '#description' => $msg,
    '#id' => "mm-permissions-$elem_name-$elem_id",
  );
  $chk = 0;
  foreach (array_keys($types) as $mode) {
    if (!is_bool($checks[$chk])) {
      $out[] = array();
    }
    else {
      $att = array('title' => t($types[$mode][1], $x));
      if ($checks[$chk + 1]) {
        $att['class'] = 'mm-permissions-disabled';
      }
      $out["$elem_name-$mode-$elem_id"] = array(
        '#type' => 'checkbox',
        '#default_value' => $checks[$chk],
        '#disabled' => $checks[$chk + 1],
        '#attributes' => $att,
      );
    }
    $chk += 2;
  }
  $out[] = array('#type' => 'item', '#value' => $action);
  return $out;
}

function _mm_ui_delete_node_groups($node, $everyone) {
  // ad-hoc and (maybe) "everyone"
  db_query('DELETE nw, g FROM {mm_node_write} nw '.
      'LEFT JOIN {mm_group} g ON nw.gid = g.gid '.
      'WHERE nw.nid = %d AND nw.gid '. ($everyone ? '<= 0' : '< 0'), $node->nid);

  // everything else
  db_query('DELETE FROM {mm_node_write} WHERE nid = %d'.
      ($everyone ? '' : ' AND gid <> 0'), $node->nid);
}

function _mm_ui_recycle_page_list($node, &$names, &$msg, $can = FALSE) {
  $names = array();
  foreach (mm_content_get($node->recycle_from_mmtids) as $tree) {
    $link = l(mm_content_expand_name($tree->name), mm_content_get_mmtid_url($tree->mmtid));
    if ($can) {
      $perms = mm_content_user_can($tree->mmtid);
      if (!$perms['u']) {
        $msg = t('You do not have permission to restore this content to the page it originally came from, !page.',
            array('!page' => $link));
        return FALSE;
      }
      if ($perms['IS_RECYCLED']) {
        $msg = t('This content cannot be restored because the page it originally came from, !page, is also in the recycle bin. You must restore that page first.',
            array('!page' => $link));
        return FALSE;
      }
    }
    $names[] = $link;
  }

  if (!count($names))
    $msg = t('This content is not associated with a page. It will be visible only by its direct web address.<p>Are you sure you want to restore it?</p>');
  else if (count($names) == 1)
    $msg = t('Are you sure you want to restore this content to the page !title?',
        array('!title' => $names[0]));
  else $msg = t('Are you sure you want to restore this content to the following pages: !list',
      array('!list' => implode(', ', $names)));

  return TRUE;
}

function _mm_ui_node_info_values() {
  return array(
    0 => t('(none)'),
    1 => t('Submitted by [username]'),
    2 => t('Submitted on [date]'),
    3 => t('Submitted by [username] on [date]'),
  );
}

function _mm_ui_date_popup_validate(&$element, &$form_state) {
  if (empty($element['#value']['date'])) {
    // if the date is blank, clear the time
    $element['#value']['time'] = '';
  }
  else if (empty($element['#value']['time']) && !empty($element['#no_time_default'])) {
    // if a date is chosen but the time is empty, set a sensible default
    $element['#value']['time'] = $element['#no_time_default'];
  }
}

function _mm_ui_date_to_string($value) {
  if (isset($value)) {
    if (is_numeric($value)) {
      if ($value == 0) return '';
      return format_date($value, 'custom', 'Y-m-d H:i:s');
    }
    return $value;
  }
  return '';
}

function _mm_ui_comment_write_setting_values() {
  return array(
    t('Disabled: No comments allowed'),
    t('Read-only: Existing comments can be read, no new ones can be added'),
    t('Read/Write: All logged-in users can add comments'));
}

function _mm_ui_comment_read_setting_values($blank) {
  $out = array('' => $blank);
  foreach (variable_get('mm_comments_readable_labels', array()) as $label)
    $out[$label['perm']] = $label['desc'];
  return $out;
}

// Set form arrays in such a way that content added beforehand in other modules'
// hook_form_alter() is preserved
function _mm_ui_form_array_merge(&$form, $element, $value) {
  $form[$element] = is_array($form[$element]) ? array_merge($value, $form[$element]) : $value;
}

function _mm_ui_form_parse_perms($form_vals, $validate) {
  $limit_write_not_admin = isset($form_vals['limit_write_not_admin']);

  $groups = array();
  preg_match_all('/(\w)(\d+)/', $form_vals['all_values_group'], $matches);
  $i = 0;
  foreach ($matches[1] as $short) {
    if (!$limit_write_not_admin || $short != 'w') {   // can't use 'w' if limit_write is set
      if ($group = mm_content_get(intval($matches[2][$i]))) {
        $name = mm_content_expand_name($group->name);
        if ($validate) {
          if (!mm_content_user_can($group->mmtid, 'u'))
            form_set_error('', t('You do not have permission to use the group %grp.', array('%grp' => $name)));
          $groups[$short][] = $group->mmtid;
        }
        else {
          $groups[$group->mmtid]['modes'][] = $short;
          $groups[$group->mmtid]['name'] = $name;
          if (!isset($groups[$group->mmtid]['members'])) {
            $groups[$group->mmtid]['members'] = mm_content_get_users_in_group($group->mmtid, '<br />', FALSE, 20, TRUE);
          }
        }
      }
    }
    $i++;
  }

  $users = array();
  preg_match_all('/(\w)(\d+)/', $form_vals['all_values_user'], $matches);
  $i = 0;
  foreach ($matches[1] as $short) {
    if (!$limit_write_not_admin || $short != 'w') {   // can't use 'w' if limit_write is set
      if ($usr = user_load(intval($matches[2][$i]))) {
        if ($validate) $users[$short][] = $usr->uid;
        else {
          $users[$usr->uid]['modes'][] = $short;
          $users[$usr->uid]['name'] = mm_content_uid2name($usr->uid);
        }
      }
    }
    $i++;
  }

  if ($limit_write_not_admin) {
    // Replace data protected by limit_write flag
    $result = db_query(
      'SELECT a.gid, t2.name FROM {mm_tree} t ' .
        'INNER JOIN {mm_tree_access} a ON a.mmtid = t.mmtid ' .
        'INNER JOIN {mm_tree} t2 ON t2.mmtid = a.gid ' .
      "WHERE a.mmtid = %d AND a.gid >= 0 AND a.mode = 'w'", $form_vals['path']);
    while ($r = db_fetch_object($result)) {
      $name = mm_content_expand_name($r->name);
      if ($validate) {
        $groups['w'][] = $r->gid;
      }
      else {
        $groups[$r->gid]['modes'][] = 'w';
        $groups[$r->gid]['name'] = $name;
        if (!isset($groups[$r->gid]['members'])) {
          $groups[$r->gid]['members'] = mm_content_get_users_in_group($r->gid, '<br />', FALSE, 20, TRUE);
        }
      }
    }

    $gids = array();
    $result = db_query(
      'SELECT a.gid FROM {mm_tree} t ' .
        'INNER JOIN {mm_tree_access} a ON a.mmtid = t.mmtid ' .
      "WHERE a.mmtid = %d AND a.gid < 0 AND a.mode = 'w'", $form_vals['path']);
    while ($r = db_fetch_object($result)) {
      $gids[] = $r->gid;
    }

    if ($gids) {
      $users_in_groups = mm_content_get_users_in_group($gids, NULL, FALSE, 0);
      if (!is_null($users_in_groups)) {
        foreach ($users_in_groups as $uid => $usr) {
          if (is_numeric($uid) && $uid >= 0) {
            if ($validate) $users['w'][] = $uid;
            else {
              $users[$uid]['modes'][] = 'w';
              $users[$uid]['name'] = $usr;
            }
          }
        }
      }
    }
  }

  $default_modes = array();
  foreach (array('w', 'a', 'u', 'r') as $mode)
    if (!empty($form_vals["group-$mode-everyone"]))
      $default_modes[] = $mode;

  return array($groups, $users, $default_modes);
}
