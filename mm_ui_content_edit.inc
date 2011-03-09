<?php
// $Id: mm_ui_content_edit.inc 5017 2011-02-09 20:40:29Z dan $

/**
 * @file
 * User interface routines for copying/moving MM content
 */

function mm_ui_content_edit(&$form_state, $item, $mmtid, $is_group, $is_new, $is_search = FALSE) {
  global $user;

  $x = mm_ui_strings($is_group);
  $all_menus = user_access('administer all menus');
  $ilist = drupal_map_assoc(array(1, 2, 3, 4, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 75, 100));

  $form['path'] = array(
    '#type' => 'value',
    '#value' => $mmtid
  );

  $owner = isset($form_state['post']['owner']) ? $form_state['post']['owner'] : $item->uid;

  if (!$is_new && user_access('see create/modify times')) {
    // this code correctly handles legacy tree nodes without creation dates/users
    if (!empty($item->ctime)) {
      $x['@ctime'] = format_date($item->ctime, 'medium');
      $x['!cuser'] = mm_ui_uid2name($item->cuid, TRUE);
    }

    if (!empty($item->mtime)) {
      $x['@mtime'] = format_date($item->mtime, 'medium');
      $x['!muser'] = mm_ui_uid2name($item->muid, TRUE);
    }

    if (isset($x['@ctime'])) $msg = t('This @thing was created by !cuser on @ctime.', $x);

    if (isset($x['@mtime']) && $x['@mtime'] != $x['@ctime'])
      if ($msg) $msg .= ' ' . t('It was last modified by !muser on @mtime.', $x);
      else $msg = t('This @thing was last modified by !muser on @mtime.', $x);

    if ($msg) {
      $form['moddate'] = array(
        '#type' => 'markup',
        '#value' => $msg
      );
    }
  }

  if ($is_group) {
    $form['is_group'] = array(
      '#type' => 'value',
      '#value' => TRUE
    );
  }

  if ($is_new) {
    $form['is_new'] = array(
      '#type' => 'value',
      '#value' => TRUE
    );
  }
  else {
    $form['weight'] = array(
      '#type' => 'value',
      '#value' => $item->weight
    );
  }

  $flags_not_admin = _mm_ui_content_flags_not_admin($item, $all_menus);

  _mm_ui_form_array_merge($form, 'settings_perms', array(
    '#type' => 'fieldset',
    '#title' => t('Permissions'),
    '#collapsible' => TRUE, '#collapsed' => FALSE,
  ));

  if ($flags_not_admin['limit_write']) {
    $form['settings_perms']['message'] = array('#type' => 'item', '#value' => t('<p>You are not allowed to modify the first column of the permissions.</p>'));
  }

  if (_mm_menu_access_solver($mmtid)) {
    module_load_include('inc', 'monster_menus', 'mm_ui_solver');
    mm_static('solver_mm_list_callback', TRUE, $mmtid);
    $form['settings_perms']['solver_form'] = mm_ui_solver_form();
    $form['settings_perms']['solver_form']['#prefix'] = '<div style="display: none" id="solver-form">';
    $form['settings_perms']['solver_form']['#suffix'] = '<div id="mm-solver-table"></div></div>';
    $title = t('Solve permissions issues');
    $form['settings_perms']['#description'] = '<div id="mm-solver-link">' . l($title, "mm/$mmtid/settings/solver", array('fragment' => 'TB_inline?height=400&width=500&inlineId=solver-form', 'attributes' => array('class' => 'thickbox', 'title' => $title))) . '</div>' . $form['settings_perms']['#description'];
  }

  $types = array(
    'w' => isset($item->flags['limit_write']) ?
      array(
        t('Change @thing settings', $x),
        'If checked, !class can change this !thingpos settings.',
      ) :
      array(
        t('Delete/&#8203;change settings', $x),
        'If checked, !class can delete this @thing or change its settings.',
      ),
    'a' => array(
      t('Append @subthings', $x),
      'If checked, !class can append @subthings to this @thing.',
    ),
    'u' => array(
      t('Add content', $x),
      'If checked, !class can add content to this @thing.',
    ),
    'r' => $is_group ?
      array(
        t('See group members'),
        'If checked, !class can see the members of this group.',
      ) :
      array(
        t('Read', $x),
        'If checked, !class can read this @thing.',
      )
  );
  if ($is_group) unset($types['u']);

  $form['settings_perms']['table']['#theme'] = 'mm_ui_permissions';

  if (!isset($form_state['post']['group_r_everyone'])) {
    $default_modes = explode(',', $item->default_mode);
  }
  $x['!class'] = t('everyone');
  $checks = array();
  foreach (array_keys($types) as $type) {
    $checks[] = $type == 'w' && !$all_menus ? NULL : in_array($type, $default_modes);
    $checks[] = FALSE;
  }
  $form['settings_perms']['table']['everyone'] = array(
    '#type' => 'value',
    '#value' => array(
      'title' => t('Everyone'),
      'types' => $types,
      'headings' => TRUE,
    ),
  );
  $form['settings_perms']['table']['everyone'][] = _mm_ui_perms_table_row('group', 'everyone', t('All users'), '', NULL, $types, $x, $checks);

  $form['settings_perms']['table']['indiv'] = array(
    '#type' => 'value',
    '#value' => array(
      'title' => t('Individuals'),
      'types' => $types,
      'action' => mm_ui_add_user_subform('settings-perms-indiv-add', t('add'), t('User(s) to add to permissions:'), t('Add users to permissions'), 'Drupal.MMSettingsPermsAddUsers'),
    ),
  );
  list($name, $msg, $owner_readonly) = mm_ui_owner_desc($x, $owner, $is_search);
  $x['!class'] = t('this user');
  $form['settings_perms']['table']['indiv'][] = _mm_ui_perms_table_row(
    'user',
    'owner',
    t('<span class="settings-perms-owner-prefix">Owner: </span><span class="settings-perms-owner-name">!name</span>', array('!name' => $name)),
    $msg,
    $owner_readonly ? NULL : mm_ui_add_user_subform('settings-perms-indiv-owner', t('change'), t('Owner:'), t('Change the owner'), 'Drupal.MMSettingsPermsOwner', $owner, $name),
    $types,
    $x,
    array(TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE)
  );
  if (!$is_search) {
    $users = $groups = array();
    $checks = array(TRUE, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE); // defaults for new row when no users are listed
    // individual users
    if (isset($form_state['post']['all_values_user'])) {
      $form_state['post']['path'] = $mmtid;
      list($groups, $users, $default_modes) = _mm_ui_form_parse_perms($form_state['post'], FALSE);
    }
    else {
      $gids = array();
      $result = db_query(
        'SELECT a.gid FROM {mm_tree} t ' .
          'INNER JOIN {mm_tree_access} a ON a.mmtid = t.mmtid ' .
        'WHERE a.gid < 0 AND a.mmtid = %d', $item->mmtid);
      while ($r = db_fetch_object($result)) {
        $gids[] = $r->gid;
      }

      if ($gids) {
        $users_in_groups = mm_content_get_users_in_group($gids, NULL, FALSE, 0);
        if (!is_null($users_in_groups)) {
          foreach ($users_in_groups as $uid => $usr) {
            if (is_numeric($uid) && $uid >= 0) {
              $r = db_fetch_object(db_query('SELECT GROUP_CONCAT(a.mode) AS modes FROM {mm_group} g '.
                'INNER JOIN {mm_tree_access} a ON a.gid = g.gid '.
                'WHERE a.gid < 0 AND g.gid IN(' . join(', ', $gids) . ') AND g.uid = %d', $uid));
              if ($r) {
                $users[$uid]['modes'] = explode(',', $r->modes);
                $users[$uid]['name'] = $usr;
              }
            }
          }
        }
      }
    }

    $delete_link = '<a href="#" title="' . t('Remove this user') . '" onclick="return Drupal.MMSettingsPermsDelete(this)">' . t('delete') . '</a>';
    foreach ($users as $uid => $data) {
      $checks = array();
      foreach (array_keys($types) as $type) {
        $checks[] = in_array($type, $data['modes']);
        $checks[] = $type == 'w' && $flags_not_admin['limit_write'];
      }
      $name = array(
        array('#type' => 'item', '#value' => $data['name']),
      );
      $form['settings_perms']['table']['indiv'][] = _mm_ui_perms_table_row('user', $uid, $name, '', $checks[0] && $flags_not_admin['limit_write'] ? '' : $delete_link, $types, $x, $checks);
    }

    // Empty row to be used when adding new users
    if ($flags_not_admin['limit_write']) {
      $checks[0] = FALSE;
      $checks[1] = TRUE;
    }
    $form['settings_perms']['table']['indiv'][] = _mm_ui_perms_table_row('user', 'new', '', '', $delete_link, $types, $x, $checks);
    if (!$owner_readonly) {
      $form['settings_perms']['owner'] = array('#type' => 'hidden', '#default_value' => $owner);
    }

    $form['settings_perms']['all_values_user'] = array('#type' => 'hidden');
    if ($flags_not_admin['limit_write']) {
      // Tell the JS code that it needs to act differently
      $form['settings_perms']['limit_write_not_admin'] = array('#type' => 'hidden');
    }
  }

  $form['settings_perms']['table']['groups'] = array(
    '#type' => 'value',
    '#value' => array(
      'title' => t('Groups'),
      'types' => $types,
      'action' => '<a href="#" title="' . t('Add a group') . '" onclick="return Drupal.MMSettingsPermsAddGroup()">' . t('add') . '</a>',
    )
  );
  if (!$is_search) {
    // If there is a POST, $groups will have already been populated by _mm_ui_form_parse_perms()
    if (!isset($form_state['post']['all_values_group'])) {
      $result = db_query(
        'SELECT t2.mmtid, t2.name, GROUP_CONCAT(a.mode) AS modes FROM {mm_tree} t ' .
          'INNER JOIN {mm_tree_access} a ON a.mmtid = t.mmtid ' .
          'LEFT JOIN {mm_tree} t2 ON a.gid = t2.mmtid ' .
        'WHERE t2.mmtid >= 0 AND a.mmtid = %d ' .
        'GROUP BY a.gid ' .
        'ORDER BY t2.name', $item->mmtid);
      while ($r = db_fetch_object($result))
        if (!$is_new || mm_content_user_can($r->mmtid, 'u')) {
          $members = mm_content_get_users_in_group($r->mmtid, '<br />', FALSE, 20, TRUE);
          if ($members == '') $members = t('(none)');
          $groups[$r->mmtid]['name'] = mm_content_expand_name($r->name);
          $groups[$r->mmtid]['members'] = $members;
          $groups[$r->mmtid]['modes'] = explode(',', $r->modes);
        }
    }

    $delete_link = '<a href="#" title="' . t('Remove this group') . '" onclick="return Drupal.MMSettingsPermsDelete(this)">' . t('delete') . '</a>';
    $x['!class'] = t('the users in this group');
    $elem = array(
      array(
        '#type' => 'fieldset',
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        '#title' => 'temp title',
        array(
          '#type' => 'item',
          '#value' => 'temp members',
        ),
      )
    );
    foreach ($groups as $mmtid => $data) {
      $checks = array();
      foreach (array_keys($types) as $type) {
        $checks[] = in_array($type, $data['modes']);
        $checks[] = $type == 'w' && $flags_not_admin['limit_write'];
      }
      $elem[0]['#title'] = $data['name'];
      $elem[0][0]['#value'] = $data['members'];
      $form['settings_perms']['table']['groups'][] = _mm_ui_perms_table_row('group', $mmtid, $elem, '', $checks[0] && $flags_not_admin['limit_write'] ? '' : $delete_link, $types, $x, $checks);
    }

    // Empty row to be used when adding new groups
    if ($flags_not_admin['limit_write']) {
      $checks[0] = FALSE;
      $checks[1] = TRUE;
    }
    else if (!$limit_write) {
      $checks[0] = $checks[1] = FALSE;
    }
    $elem[0][0]['#id'] = 'mm-permissions-group-new';
    $form['settings_perms']['table']['groups'][] = _mm_ui_perms_table_row('group', 'new', $elem, '', $delete_link, $types, $x, $checks);

    $form['settings_perms']['all_values_group'] = array('#type' => 'hidden');
  }

  if (user_access('propagate page perms')) {
    $form['settings_perms']['propagate'] = array(
      '#type' => 'checkbox',
      '#title' => t('Copy these permissions to all @subthings of this @thing', $x),
      '#default_value' => FALSE,
      '#description' => t('If this option is checked, the permissions will be copied to all @subthings of this one that you have permission to change.', $x),
    );
    $node_prop_desc = ' ' . t('If the option above is also checked, permissions will be copied to the pieces of content on all @subthings.', $x);
  }

  $form['settings_perms']['node_propagate'] = array(
    '#type' => 'checkbox',
    '#title' => t('Copy these permissions to all nodes on this @thing', $x),
    '#access' => !$is_group && user_access('propagate node perms'),
    '#default_value' => FALSE,
    '#description' => t('If this option is checked, the permissions will be copied to all pieces of content on this @thing that you have permission to change.', $x) . $node_prop_desc,
  );

  $form['settings_perms']['hover'] = array(
    '#type' => 'value',
    '#value' => $is_new ? '' : $item->hover,
  );

  if (isset($item->flags['limit_name']) && !$all_menus) {
    $form['settings_general']['name'] = array(
      '#type' => 'value',
      '#value' => $item->name,
    );
  }
  else {
    $form['settings_general'] = array(
      '#type' => 'fieldset',
      '#title' => t('General settings'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
    );
    $form['settings_general']['name'] = array(
      '#type' => 'textfield',
      '#title' => $is_group ? t('Group name') : t('Page name'),
      '#default_value' => $item->name,
      '#required' => TRUE,
      '#size' => 40,
      '#maxlength' => 128,
      '#description' => $is_group ? '' : t('The name that appears in menus.'),
    );
  }

  if ($is_group) {
    $form['members'] = array(
      '#type' => 'fieldset',
      '#title' => t('Group members'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
    );

    if (!$is_new && user_access('administer permissions')) {
      $roles = array();
      if ($result = db_query(
        'SELECT r.rid, r.name FROM {mm_role2group} r2 INNER JOIN {role} r ON r.rid = r2.rid WHERE r2.gid = %d', $item->mmtid)) {
        while ($r = db_fetch_object($result)) {
          $roles[] = l($r->name, "admin/user/roles/edit/$r->rid") . ' (' . l(t('permissions'), "admin/user/permissions/$r->rid") . ')';
        }

        if ($roles) {
          $form['members']['warning'] = array(
            '#type' => 'item',
            '#value' => t('This group\'s members will be added to !plur !list.', array('!plur' => (count($roles) == 1 ? t('the role') : t('these roles: ')), '!list' => implode(', ', $roles)))
          );
        }
      }
    }

    if (mm_content_is_vgroup($item->mmtid)) {
      $form['members']['#description'] = t('Enter two portions of a SQL statement that returns the user IDs (uids) of the users in the group:<br />' .
          '<code>SELECT <font color="green"><u>ColumnName</u></font> AS uid <font color="green"><u>FROM TableName WHERE Condition</u></font></code>');
      if ($is_new) {
        $data = array(
          'qfield' => t('ColumnName'),
          'qfrom' => t('FROM TableName WHERE Condition'),
        );
      }
      else {
        $data = db_fetch_array(db_query('SELECT v.* FROM {mm_group} g INNER JOIN {mm_vgroup_query} v ON g.vgid = v.vgid WHERE g.gid = %d', $item->mmtid));
        if ($data) {
          $form['vgid'] = array(
            '#type' => 'value',
            '#value' => $data['vgid'],
          );

          $msgs = array(
            MM_VGROUP_DIRTY_NEXT_CRON => t('This group will be regenerated during the next cron run.'),
            MM_VGROUP_DIRTY_FAILED => t('This group has been marked as potentially corrupt, and must be examined before it will be regenerated.'),
            MM_VGROUP_DIRTY_REDO => t('This group was previously marked as potentially corrupt, but will be regenerated during the next cron run.'),
          );
          if (isset($msgs[$data['dirty']])) {
            drupal_set_message($msgs[$data['dirty']], $data['dirty'] == MM_VGROUP_DIRTY_FAILED ? 'error' : 'warning');
          }
        }
      }

      $form['members']['qfield'] = array(
        '#type' => 'textfield',
        '#title' => t('Column to select'),
        '#default_value' => $data['field'],
        '#size' => 40,
        '#maxlength' => 40,
        '#description' => t('The name of the database column (or a constant value) to SELECT; if blank, the group will not contain any users'),
      );
      $form['members']['qfrom'] = array(
        '#type' => 'textarea',
        '#title' => t('FROM clause'),
        '#default_value' => $data['qfrom'],
        '#rows' => 4,
        '#tinymce_disabled' => TRUE,
        '#description' => t('The FROM portion of the SELECT statement; can be blank'),
      );
    }
    else {    // normal group
      mm_static('ui_content_edit_js', TRUE, $item->mmtid, 'group', 'members');
      _mm_ui_userlist_setup(NULL, $form['members'], 'members', t('Members:'), FALSE, t('Choose the members of this group.'), '', TRUE);
      unset($form['members']['members']['#description']);
    }
  }
  else if (isset($item->flags['limit_alias']) && !$all_menus || $item->mmtid == mm_home_mmtid() && !$is_new) {      // !$is_group
    $form['settings_general']['alias'] = array(
      '#type' => 'value',
      '#value' => $item->alias,
    );
  }
  else {
    $form['settings_general']['alias'] = array(
      '#type' => 'textfield',
      '#title' => t('URL name'),
      '#default_value' => $item->alias,
      '#required' => !$item->is_user_home && !$all_menus,
      '#size' => 20,
      '#maxlength' => 128,
      '#description' => t('The name that will be used in the Web address of the page. '.
        'Make this a shortened version of the Page name, using only lowercase letters, '.
        'numerals, hyphens, periods, and underscores.'),
    );
  }

  if ($all_menus) {
    $form['flags'] = array(
      '#type' => 'fieldset',
      '#title' => t('Flags'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#description' => t('Attributes used in special queries; only administrators can edit this list'),
    );

    $predefined = array();
    // We need the module name, so don't use module_invoke_all()
    foreach (mm_module_implements('mm_tree_flags') as $module) {
      $function = $module . '_mm_tree_flags';
      $result = call_user_func_array($function, $args);
      if (isset($result) && is_array($result)) {
        $predefined[$module] = $result;
      }
    }
    ksort($predefined);

    foreach ($predefined as $module => $list) {
      if (count($predefined) > 1)
        $form['flags'][$module] = array(
          '#type' => 'fieldset',
          '#title' => $module,
          '#collapsible' => TRUE,
          '#collapsed' => TRUE,
        );
      ksort($list);
      $weight = 1;
      foreach ($list as $flag => $elem) {
        if (!isset($elem['#title'])) {
          $elem['#title'] = $flag;
        }
        $elem['#weight'] = $elem['#type'] == 'checkbox' ? $weight : $weight + 100;
        if ($elem['#type'] == 'textfield') {
          $elem['#maxlength'] = 255;
        }
        $elem['#default_value'] = $elem['#type'] == 'checkbox' ? isset($item->flags[$flag]) : (isset($item->flags[$flag]) ? $item->flags[$flag] : '');
        $elem['#prefix'] = "<div class='container-inline'>";
        $elem['#suffix'] = '&nbsp;&nbsp;&nbsp;' . theme('tooltip', t('help'), $elem['#title'], $elem['#description']) . '</div>';
        unset($elem['#description']);
        $form['flags'][$module]["flag_$flag"] = $elem;
        unset($item->flags[$flag]);
        $weight++;
      }
    }

    $free_flags = array();
    foreach ($item->flags as $flag => $data)
      if (!empty($data)) $free_flags[] = "$flag=$data";
      else $free_flags[] = $flag;

    $form['flags']['free_flags'] = array(
      '#type' => 'textarea',
      '#title' => t('Others'),
      '#default_value' => join("\n", $free_flags),
      '#tinymce_disabled' => TRUE,
      '#weight' => 200,
      '#rows' => max(count($free_flags) + 1, 2),
      '#description' => t('A free-form list of attributes, one per line. Can either be <code>name</code> or <code>name=value</code>.'),
    );
  }

  if (!$is_group) {
    $form['settings_appearance'] = array(
      '#type' => 'fieldset',
      '#title' => t('Appearance'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );

    $theme[''] = t('(use parent\'s theme)');
    $all_themes = array();
    $allowed_themes = mm_content_resolve_cascaded_setting('allowed_themes', $item->mmtid, $theme_at, $theme_parent, $is_new);
    $desc_add = '';
    foreach (list_themes() as $t)
      if ($t->status) {
        if (!count($allowed_themes) || array_search($t->name, $allowed_themes) !== FALSE)
          $theme[$t->name] = $t->name;
        else if ($all_menus) {
          $theme[$t->name] = $t->name . ' *';
          $desc_add = t(' Themes in the list ending with * are only available to administrators, based on the current settings.');
        }
        if ($all_menus) $all_themes[$t->name] = $t->name;
      }
    natcasesort($theme);

    if (count($theme) > 2 || $all_menus) {
      $form['settings_appearance']['theme'] = array(
        '#type' => 'fieldset',
        '#title' => t('Theme'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      );
    }

    if (count($theme) > 2) {
      $form['settings_appearance']['theme']['theme'] = array(
        '#type' => 'select',
        '#title' => t('Theme for this @thing and its children', $x),
        '#default_value' => $item->theme,
        '#options' => $theme,
        '#description' => t('The theme will be applied to this @thing and its @subthings, unless overridden at a lower level.', $x) . $desc_add,
      );
    }
    else {
      $form['settings_appearance']['theme']['theme'] = array(
        '#type' => 'value',
        '#value' => $item->theme,
      );
    }

    if ($all_menus) {
      $desc = t('This option is only available to administrators.');
      if (!$theme_parent) {
        $desc .= t(' No parents of this @thing have theme limits, so if you deselect all themes here, non-admin users will be able to choose any theme in the list.', $x);
      }
      else {
        $ptree = mm_content_get($theme_parent);
        $link = l(mm_content_expand_name($ptree->name), mm_content_get_mmtid_url($ptree->mmtid));
        $desc .= t(' To inherit the settings of the parent, !parent, deselect all themes here.',
            array('!parent' => $link));
      }
      natcasesort($all_themes);
      $form['settings_appearance']['theme']['allowed_themes'] = array(
        '#type' => 'select',
        '#title' => t('Allowed themes for this @thing and its children', $x),
        '#multiple' => TRUE,
        '#size' => 5,
        '#default_value' => !$is_new && $theme_at == $item->mmtid ? $allowed_themes : array(),
        '#options' => $all_themes,
        '#description' => $desc,
      );
    }

    $form['settings_appearance']['menu'] = array(
      '#type' => 'fieldset',
      '#title' => t('Menu and layout'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );

    $menu_vals = array(
      'bid' => -1,
      'max_depth' => -1,
      'max_parents' => -1,
      'allow_reorder' => -1,
    );
    if (!$is_new) {
      $result = db_query_range('SELECT bid, max_depth, max_parents FROM {mm_tree_block} WHERE mmtid = %d', $item->mmtid, 0, 1);
      if ($r = db_fetch_array($result)) $menu_vals = $r;
      $allow_reorder = mm_content_resolve_cascaded_setting('allow_reorder', $item->mmtid, $reorder_at, $reorder_parent);
      if ($reorder_at == $item->mmtid) {
        $menu_vals['allow_reorder'] = $allow_reorder;
      }
      else if (!isset($reorder_at) && ($item->mmtid == 1 || $item->mmtid == mm_home_mmtid())) {
        $menu_vals['allow_reorder'] = 0;
      }
      else {
        $menu_vals['allow_reorder'] = -1;
      }
    }

    if (!$flags_not_admin['limit_hidden']) {
      $form['settings_appearance']['menu']['hide_menu'] = array(
        '#type' => 'checkbox',
        '#title' => t('Don\'t show this @thing in the menu', $x),
        '#description' => t('If checked, this page will not be listed in the navigation menu but can still be accessed directly using its URL address.'),
        '#default_value' => $item->hidden,
      );
    }
    else {
      $form['settings_appearance']['menu']['hide_menu'] = array('#type' => 'value',
          '#value' => $item->hidden);
    }

    if ($flags_not_admin['limit_location']) {
      $form['settings_appearance']['menu']['menu_start'] = array('#type' => 'value',
          '#value' => $menu_vals['bid']);
    }
    else {
      $blocks[-1] = array(
        t('Standard page'),
        t('Creates a new page and the menu item that points to the page. The menu item will be indented and listed alphabetically under the parent menu item in the left navigation menu.'),
      );
      foreach (mm_content_get_blocks(TRUE) as $bid => $b)
        $blocks[$bid] = array($b['info'], $b['help']);

      $form['settings_appearance']['menu']['menu_start'] = array(
        '#type' => $is_search ? 'select' : 'mm_help_radios',
        '#title' => t('Location on screen'),
        '#default_value' => isset($blocks[$menu_vals['bid']]) ? $menu_vals['bid'] : mm_ui_mmlist_key0($blocks),
        '#attributes' => array('class' => 'settings-menu-start'),
        '#options' => $blocks,
      );
    }

    $form['settings_appearance']['menu']['max_depth'] = array(
      '#type' => 'select',
      '#title' => t('Max. number of child levels to display'),
      '#default_value' => $menu_vals['max_depth'],
      '#attributes' => array('class' => 'settings-max-depth'),
      '#options' => _mm_ui_levels()
    );
    if ($all_menus) {
      $form['settings_appearance']['menu']['max_parents'] = array(
        '#type' => 'select',
        '#title' => t('Max. number of parent levels to display'),
        '#default_value' => $menu_vals['max_parents'],
        '#attributes' => array('class' => 'settings-max-parents'),
        '#options' => _mm_ui_levels(),
        '#description' => t('As the user gets deeper down in the menu tree, higher-level entries will be removed. This keeps a deeply-nested menu from getting too indented. This setting is inherited by any @subthings.', $x)
      );
      $form['settings_appearance']['menu']['allow_reorder'] = array(
        '#type' => 'select',
        '#title' => t('Allow the menu and its children to be reordered'),
        '#default_value' => $menu_vals['allow_reorder'],
        '#options' => array(-1 => t('(inherit from parent page)'), 1 => t('Yes'), 0 => t('No')),
        '#description' => t('Administrators always have the ability to reorder menus.'),
      );
    }
    else {
      $form['settings_appearance']['menu']['max_parents'] = array(
        '#type' => 'value',
        '#value' => $menu_vals['max_parents'],
      );
    }

    if (user_access('show/hide post information')) {
      $form['settings_appearance']['new_nodes']['node_info'] = array(
        '#type' => 'select',
        '#title' => t('Default attribution style'),
        '#default_value' => $item->node_info,
        '#options' => _mm_ui_node_info_values(),
        '#description' => t('Unless disabled by an administrator, this value will be the default for all new content added to this page. You can change this behavior in the <em>Appearance</em> settings of the content.'),
      );
    }
    else if (!$is_search) {
      $form['settings_appearance']['new_nodes']['node_info'] = array(
        '#type' => 'value',
        '#value' => $item->node_info,
      );
    }

    if (!$is_group && module_exists('comment')) {
      if (variable_get('mm_finegrain_comment_readability', FALSE)) {
        $comments_readable = $is_new ? '' : mm_content_get_cascaded_settings($item->mmtid, 'comments_readable');
        $form['settings_appearance']['new_nodes']['comments_readable'] = array(
          '#type' => 'select',
          '#title' => t('Who can read comments by default'),
          '#default_value' => $comments_readable,
          '#options' => _mm_ui_comment_read_setting_values(t('(inherit from parent page)')),
          '#description' => t('This value will be the default for all new content added to this page. You can change this behavior in the <em>Comment settings</em> of the content.'),
        );
      }
      $form['settings_appearance']['new_nodes']['comment'] = array(
        '#type' => 'select',
        '#title' => t('Who can add comments by default'),
        '#access' => user_access('enable/disable comments') || user_access('administer comments'),
        '#default_value' => $item->comment,
        '#options' => _mm_ui_comment_write_setting_values(),
        '#description' => t('Unless disabled by an administrator, this value will be the default for all new content added to this page. You can change this behavior in the <em>Comment settings</em> of the content.'),
      );
    }

    if (isset($form['settings_appearance']['new_nodes']))
      $form['settings_appearance']['new_nodes'] = array_merge($form['settings_appearance']['new_nodes'], array(
      '#type' => 'fieldset',
      '#title' => t('Defaults for new content'),
      '#collapsible' => TRUE, '#collapsed' => TRUE)
    );

    $form['settings_appearance']['previews'] = array(
      '#type' => 'checkbox',
      '#title' => t('Show only the previews ("teasers") of all contents'),
      '#default_value' => $item->previews,
    );

    if (variable_get('mm_enable_rss', FALSE)) {
      $form['settings_appearance']['rss'] = array(
        '#type' => 'checkbox',
        '#title' => t('Enable the RSS feed for this page'),
        '#default_value' => $item->rss,
      );
    }
    else {
      $form['settings_appearance']['rss'] = array(
        '#type' => 'value',
        '#value' => $item->rss
      );
    }

    $nodes_per_page = $is_new ? '' : mm_content_get_cascaded_settings($item->mmtid, 'nodes_per_page');
    if (empty($nodes_per_page) && !$is_new && $item->mmtid == mm_home_mmtid()) $nodes_per_page = variable_get('default_nodes_main', 10);
    $form['settings_appearance']['nodes_per_page'] = array(
      '#type' => 'select',
      '#title' => t('Pieces of content to display at one time'),
      '#default_value' => $nodes_per_page,
      '#options' => array('' => t('(inherit from parent page)')) + $ilist,
      '#description' => t('If more than this number of pieces of content is present, pagination controls will be displayed.'),
    );

    if ($all_menus) {
      $form['settings_node_types'] = array(
        '#type' => 'fieldset',
        '#title' => t('Allowed node types for this @thing and its children', $x),
        '#collapsible' => TRUE, '#collapsed' => TRUE);

      $all_types = array();
      $allowed_node_types = mm_content_resolve_cascaded_setting('allowed_node_types', $item->mmtid, $types_at, $types_parent, $is_new);
      $types_inherit = $is_new || $types_at != $item->mmtid || count($allowed_node_types) == 0;
      foreach (node_get_types() as $t)
        $all_types[$t->type] = $t->name;
      natcasesort($all_types);

      $desc = t('This option is only available to administrators.');
      $suff = '';
      if (!$types_parent) {
        $cb_desc = t('<font color=red>No parents of this @thing have node types defined, so unless you select something in the list below, only administrators will be able to add content.</font>', $x);
        $form['settings_node_types']['#collapsed'] = FALSE;
      }
      else {
        $ptree = mm_content_get($types_parent);
        $link = l(mm_content_expand_name($ptree->name), mm_content_get_mmtid_url($ptree->mmtid));
        $cb_desc = t('If checked, the settings of the parent, !parent, will be inherited',
            array('!parent' => $link));
        $desc .= t(' If you deselect all node types here and do not check the Inherit option, only admin users will be able to add new content.');
      }
      $form['settings_node_types']['allowed_node_types_inherit'] = array(
        '#type' => 'checkbox',
        '#title' => t('Inherit from parent @thing', $x),
        '#default_value' => $types_inherit,
        '#description' => $cb_desc,
        '#attributes' => array('class' => 'settings-node-types'),
      );
      $form['settings_node_types']['allowed_node_types'] = array(
        '#type' => 'select',
        '#title' => t('Node types'),
        '#multiple' => TRUE,
        '#size' => 10,
        '#default_value' => $types_at == $item->mmtid ? $allowed_node_types : array(),
        '#options' => $all_types,
        '#description' => $desc,
      );
    }

    if (user_access('create archives')) {
      $form['settings_archive'] = array(
        '#type' => 'fieldset',
        '#title' => t('Archive'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      );
      $item->frequency = 'month';
      $item->main_nodes = 10;
      $item->archive_mmtid = array();
      if (!$is_new) {
        $tree = mm_content_get($mmtid, MM_GET_ARCHIVE);
        if (isset($tree->archive_mmtid)) {
          $item = (object)array_merge((array)$item, (array)$tree);
          $archive = mm_content_get($item->archive_mmtid);
          $item->archive_mmtid = array($item->archive_mmtid => mm_content_expand_name($archive->name));
        }
      }

      if (isset($tree->archive_mmtid) && $tree->archive_mmtid == $mmtid) {
        $atree = mm_content_get($tree->main_mmtid);
        $x['!link'] = l(mm_content_expand_name($atree->name), mm_content_get_mmtid_url($atree->mmtid) . '/settings');
        $form['settings_archive']['#description'] = t('This @thing is an archive for !link. To change the archival settings, visit that @thing.', $x);
      }
      else {
        $form['settings_archive']['#description'] = t('Contents past a certain age will be automatically moved to a secondary page, where they are organized by date.');
        $have_archive = count($item->archive_mmtid);
        $form['settings_archive']['archive'] = array(
          '#type' => 'checkbox',
          '#title' => t('Use an archive'),
          '#default_value' => $have_archive,
        );
        $form['settings_archive']['inner'] = array(
          '#prefix' => '<div class="settings-archive" style="display: none">',
          '#suffix' => '</div>',
        );
        $form['settings_archive']['inner']['frequency'] = array(
          '#type' => 'select',
          '#title' => t('Frequency'),
          '#options' => array(
            'day' => t('daily'), 'week' => t('weekly'), 'month' => t('monthly'),
            'year' => t('yearly')),
          '#default_value' => $item->frequency,
        );
        $form['settings_archive']['inner']['main_nodes'] = array(
          '#type' => 'select',
          '#title' => t('Pieces of content to show on the main page'),
          '#default_value' => $item->main_nodes,
          '#description' => t('At least this many posts will be shown, even if they have been archived.'),
          '#options' => $ilist,
        );
        $path_mmtids = mm_content_get_parents_with_self($item->mmtid);
        $form['settings_archive']['inner']['archive_mmtid'] = array(
          '#type' => 'mm_fake_required',
          '#mm_orig_type' => 'mm_catlist',
          '#mm_list_selectable' => 'w',
          '#mm_list_popup_start' => implode('/', $path_mmtids),
          '#mm_list_browser' => 'mm-browser-load',
          '#process' => array('_mm_ui_process_mmlist'),
          '#title' => t('Location of archive:'),
          '#description' => t('Choose the page to contain the archived contents. It must already exist.'),
          '#mm_list_min' => 1,
          '#mm_list_max' => 1,
          '#default_value' => $item->archive_mmtid,
        );
      }
    }   // user_access('create archives')
  }   // !$is_group

  $form['buttons']['submit'] = array(
    '#type' => 'submit',
    '#value' => $is_new ? t('Create @subthing', $x) : t('Save settings'),
  );
  if (!$is_search) mm_static('edit_cat', TRUE);

  if (!empty($_POST) && form_get_errors()) {
    drupal_set_message(t('The settings have not been saved because of the errors.'), 'error');
  }

  return $form;
}

/**
 * Validate the form for editing or creating a tree entry
 */
function mm_ui_content_edit_validate($form, &$form_state) {
  $form_vals =& $form_state['values'];
  $is_new = isset($form_vals['is_new']);

  $mmtid = $form_vals['path'];
  if (!is_numeric($mmtid)) {
    form_set_error('', t('Bad data'));
    return;
  }

  $x = mm_ui_strings($is_group = mm_content_is_group($mmtid));

  if (!mm_content_user_can($mmtid, $is_new ? 'a' : 'w')) {
    $message = $is_new ?
      'You do not have permission to add @subthings to this @thing' :
      'You do not have permission to modify this @thing';
    watchdog('mm', $message, $x, WATCHDOG_WARNING, $form_vals['path']);
    form_set_error('', t($message, $x));
    return;
  }

  $test_mmtid = $mmtid;
  if (!$is_new) $test_mmtid = mm_content_get_parent($mmtid);

  if (!_mm_ui_validate_entry($mmtid, $test_mmtid, $form_vals, $is_new, $is_new))
      return;

  if ($is_group)
    if (mm_content_is_vgroup($mmtid)) {
      if (trim($form_vals['qfield']) != '') {
        $q = 'SELECT ' . trim($form_vals['qfield']) . ' AS uid';
        if (trim($form_vals['qfrom']) != '') $q .= ' ' . trim($form_vals['qfrom']);
        if (!db_query($q)) {
          form_set_error('members', t('There was an error testing the query:<br />@query',
              array('@query' => $q)));
        }
      }
    }
    else if (isset($form_vals['members']))
        _mm_ui_verify_userlist($form_vals['members'], 'members');

  if (user_access('administer all menus'))
      _mm_ui_verify_userlist($form_vals['owner'], 'owner');

  list($form_vals['all_values_group'], $form_vals['all_values_user'], $form_vals['default_modes']) = _mm_ui_form_parse_perms($form_vals, TRUE);

  if ($form_vals['archive']) {
    $archive_mmtid = mm_ui_mmlist_key0($form_vals['archive_mmtid']);
    // We're using a "fake_required", so this test is necessary
    if (!isset($archive_mmtid)) {
      form_set_error('archive_mmtid', t('The archive location is required.'));
    }
    else if (!mm_content_user_can($archive_mmtid, 'w')) {
      form_set_error('archive_mmtid', t('You do not have permission to modify the @thing you chose for the archive.', $x));
    }
    else {
      $tree = mm_content_get($archive_mmtid, MM_GET_ARCHIVE);
      if (isset($tree->archive_mmtid))
        if ($tree->archive_mmtid == $archive_mmtid && ($is_new || $tree->main_mmtid != $mmtid)) {
          form_set_error('archive_mmtid', t('The @thing you chose for the archive is already the archive for another @thing.', $x));
        }
        else if ($tree->main_mmtid == $archive_mmtid) {
          form_set_error('archive_mmtid', t('The @thing you chose for the archive already contains an archive. Please choose a different @thing.', $x));
        }
      $content = mm_content_get_nids_by_mmtid($archive_mmtid, 1);
      if (count($content)) {
        form_set_error('archive_mmtid', t('The @thing you chose for the archive already has contents. Before it can become an archive, you must remove the contents.', $x));
      }
    }
  }

  if (isset($form_vals['menu_start']) && $form_vals['menu_start'] != -1) {
    $blocks = mm_content_get_blocks(TRUE);
    $parent = mm_content_get_parent($mmtid);
    $bid = $form_vals['menu_start'];
    if (is_null($parent) || $bid != -1 && !isset($blocks[$bid])) {
      form_set_error('menu_start', t('The <em>Location on screen</em> you chose is not valid.', $x));
    }
    else if ($blocks[$bid]['show_node_contents'] && $form['settings_appearance']['menu']['menu_start']['#default_value'] != $bid && !mm_content_user_can($parent, 'w')) {
      form_set_error('menu_start', t('The <em>Location on screen</em> you chose would change the appearance of this @thing\'s parent. You do not have edit/delete permission for the parent @thing.', $x));
    }
  }
}

/**
 * Process the form for editing or creating a tree entry
 */
function mm_ui_content_edit_submit($form, &$form_state) {
  global $user;

  $form_vals =& $form_state['values'];
  $mmtid = $form_vals['path'];
  $x = mm_ui_strings($is_group = mm_content_is_group($mmtid));
  $is_new = isset($form_vals['is_new']);
  $alias = $is_group || $mmtid == mm_home_mmtid() && !$is_new ? '' : trim($form_vals['alias']);
  $name = trim($form_vals['name']);

  if ($is_group) {
    // Currently, groups always have 'u' for everyone. Change this if you want
    // to only allow some users to apply a group to something's permissions.
    $form_vals['all_values_group']['u'] = $form_vals['all_values_user']['u'] = array();
    if (!in_array('u', $form_vals['default_modes'])) $form_vals['default_modes'][] = 'u';
  }
  $perms = array();
  foreach (array('w', 'a', 'u', 'r') as $m) {
    $perms[$m]['groups'] = $form_vals['all_values_group'][$m];
    $perms[$m]['users']  = $form_vals['all_values_user'][$m];
  }

  $flags = array();
  // Get the current cascaded settings.
  $cascaded = $is_new ? array() : mm_content_get_cascaded_settings($mmtid);
  $owner = $user->uid;
  if (user_access('administer all menus')) {
    $owner = $form_vals['owner'];

    $predefined = module_invoke_all('mm_tree_flags');
    foreach ($predefined as $flag => $elem) {
      $val = $form_vals["flag_$flag"];
      if (!empty($val))
        if ($elem['#type'] == 'checkbox') $flags[$flag] = '';
        else $flags[$flag] = trim($val);
    }

    foreach (explode("\n", $form_vals['free_flags']) as $f) {
      $f = trim($f);
      if (!empty($f)) {
        if (preg_match('/^\s*(.*?)\s*=\s*(.*?)\s*$/', $f, $data))
          $flags[$data[1]] = $data[2];
        else $flags[trim($f)] = '';
      }
    }

    $cascaded['allowed_node_types'] = array();
    if (!$form_vals['allowed_node_types_inherit']) {
      $cascaded['allowed_node_types'] = count($form_vals['allowed_node_types']) ? $form_vals['allowed_node_types'] : array('');  // intentionally empty
    }
  }
  else if (!$is_new) {
    if ($tree = mm_content_get($mmtid, MM_GET_FLAGS)) {  // read old flags for item
      _mm_ui_is_user_home($tree);
      $flags = $tree->flags;
    }
  }

  // Merge cascaded settings not already handled above
  foreach (mm_content_get_cascaded_settings() as $setting_name => $desc)
    if ($setting_name != 'allowed_node_types')
      if (!isset($desc['user_access']) || user_access($desc['user_access']))
        if (isset($form_vals[$setting_name]))
          $cascaded[$setting_name] = $form_vals[$setting_name];

  $md = $form_vals['max_depth'];
  $mp = $form_vals['max_parents'];
  if (isset($form_vals['menu_start'])) {
    $ms = (int)$form_vals['menu_start'];
    if (!$ms && ($md != -1 || $mp != -1)) $ms = -1;
  }
  else {
    $ms = '';
    $md = $mp = -1;
  }

  $params = array(
    'name'                 => $name,
    'alias'                => $alias,
    'default_mode'         => implode(',', $form_vals['default_modes']),
    'uid'                  => $owner,
    'weight'               => isset($form_vals['weight']) && !$form_vals['hide_menu'] ? $form_vals['weight'] : 0,
    'theme'                => $form_vals['theme'],
    'cascaded'             => $cascaded,
    'perms'                => $perms,
    'recurs_perms'         => $form_vals['propagate'],
    'propagate_node_perms' => $form_vals['node_propagate'],
    'flags'                => $flags,
    'hover'                => $form_vals['hover'],
    'menu_start'           => $ms,
    'max_depth'            => $md,
    'max_parents'          => $mp,
    'rss'                  => $form_vals['rss'],
    'node_info'            => $form_vals['node_info'],
    'previews'             => $form_vals['previews'],
    'hidden'               => $form_vals['hide_menu'],
    'comment'              => $form_vals['comment'],
  );
  if ($is_group) {
    if (isset($form_vals['members'])) {
      $params['members'] = array_keys($form_vals['members']);
    }
    $params['large_group'] = $form_vals['members-use-large-group'] == 'yes' ? TRUE : FALSE;
    $params['qfield'] = $qfield = trim($form_vals['qfield']);
    $params['qfrom'] = trim($form_vals['qfrom']);
  }

  mm_module_invoke_all_array('mm_content_edit_submit_alter', array($is_new, $mmtid, &$params));

  $mmtid = mm_content_insert_or_update($is_new, $mmtid, $params);
  if (!isset($mmtid)) return;

  if ($is_group) {
    if ($qfield != '' && mm_content_is_vgroup($mmtid)) {
      drupal_set_message(t('The members of this virtual group will be updated during the next cron run.'));
    }
  }
  else if (user_access('create archives')) {
    db_query('DELETE FROM {mm_archive} WHERE main_mmtid = %d', $mmtid);
    if ($form_vals['archive']) {
      $archive_mmtid = mm_ui_mmlist_key0($form_vals['archive_mmtid']);
      if ($archive_mmtid) {
        db_query("INSERT INTO {mm_archive} (main_mmtid, archive_mmtid, frequency, main_nodes) VALUES(%d, %d, '%s', %d)", $mmtid, $archive_mmtid, $form_vals['frequency'], $form_vals['main_nodes']);
        // Don't allow custom ordering
        db_query('DELETE FROM {mm_node_reorder} WHERE mmtid = %d', $mmtid);
      }
    }
  }

  if ($is_new) {
    drupal_set_message(t('The @subthing was successfully created.', $x));
    $form_state['redirect'] = "mm/$mmtid";
  }
  else {
    drupal_set_message(t('The settings for this @thing have been saved.', $x));
    mm_content_clear_caches($mmtid);
  }
}

function _mm_ui_content_flags_not_admin($item, $all_menus) {
  $mm_flags = monster_menus_mm_tree_flags();
  foreach ($mm_flags as $flag => $val) {
    $flags_not_admin[$flag] = isset($item->flags[$flag]) && !$all_menus;
  }
  return $flags_not_admin;
}
