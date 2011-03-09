<?php
// $Id: mm_admin.inc 5034 2011-02-15 19:56:50Z jreese $

/**
 * @file
 * Administrative functions
 */

/**
 * Called from monster_menus_menu().
 */
function mm_admin_menu() {
  $items = array();
  $base = array('file' => 'mm_admin.inc');

  $items['admin/settings/mm'] = $base + array(
    'title' => 'Monster Menus',
    'page callback' => 'drupal_get_form',
    'page arguments' => array('mm_admin_config'),
    'access callback' => '_mm_menu_access_any_admin');

  // Administer->Monster Menus
  $items['admin/mm'] = array(
    'title' => 'Monster Menus',
    'page callback' => 'system_admin_menu_block_page',
    'file' => 'system.admin.inc',
    'file path' => drupal_get_path('module', 'system'),
    'access callback' => '_mm_menu_access_any_admin');
  $items['admin/mm/settings'] = $base + array(
    'title' => 'Settings',
    'page callback' => 'drupal_get_form',
    'page arguments' => array('mm_admin_config'),
    'access callback' => '_mm_menu_access_any_admin');
  $items['admin/mm/dump'] = $base + array(
    'title' => 'Export tree',
    'page callback' => 'mm_admin_dump',
    'access callback' => '_mm_menu_access_all_admin');
  $items['admin/mm/vgroups'] = $base + array(
    'title' => 'Redo virtual groups',
    'page callback' => 'mm_regenerate_vgroup',
    'access callback' => 'user_access',
    'access arguments' => array('administer all groups'));
  $items['admin/mm/integrity'] = $base + array(
    'title' => 'Verify integrity of database relationships',
    'page callback' => 'drupal_get_form',
    'page arguments' => array('mm_admin_verify_integrity'),
    'access callback' => '_mm_menu_access_all_admin');
  $items['admin/mm/sort'] = $base + array(
    'title' => 'Verify sort index',
    'page callback' => 'mm_admin_validate_sort_index',
    'access callback' => '_mm_menu_access_all_admin');
  $items['admin/mm/reassign'] = $base + array(
    'title' => 'Reassign user content',
    'page callback' => 'drupal_get_form',
    'page arguments' => array('mm_admin_reassign_content'),
    'access callback' => '_mm_menu_access_all_admin');

  // Administer->Blocks
  $items['admin/build/block/mm'] = $base + array(
    'title' => 'MM blocks',
    'page callback' => 'drupal_get_form',
    'page arguments' => array('mm_admin_list_blocks'),
    'type' => MENU_LOCAL_TASK,
    'access callback' => '_mm_menu_access_all_admin');
  $items['admin/build/block/mm/list'] = $base + array(
    'title' => 'List',
    'weight' => -10,
    'type' => MENU_DEFAULT_LOCAL_TASK,
    'access callback' => '_mm_menu_access_all_admin');
  $items['admin/build/block/mm/edit'] = $base + array(
    'title' => 'Configure block',
    'page callback' => 'drupal_get_form',
    'page arguments' => array('mm_admin_edit_block'),
    'type' => MENU_CALLBACK,
    'access callback' => '_mm_menu_access_all_admin');
  $items['admin/build/block/mm/add'] = $base + array(
    'title' => 'Add block',
    'page callback' => 'drupal_get_form',
    'page arguments' => array('mm_admin_edit_block'),
    'type' => MENU_LOCAL_TASK,
    'access callback' => '_mm_menu_access_all_admin');
  $items['admin/build/block/mm/delete'] = $base + array(
    'title' => 'Delete block',
    'page callback' => 'drupal_get_form',
    'page arguments' => array('mm_admin_delete_block'),
    'type' => MENU_CALLBACK,
    'access callback' => '_mm_menu_access_all_admin');

  // Administer->Monster Menus->Browse
  $items['admin/mm/browse'] = $base + array(
    'title' => 'Browse tree',
    'type' => MENU_NORMAL_ITEM,
    'access callback' => '_mm_menu_access_any_admin');
  $items['admin/mm/browse/group'] = $base + array(
    'title' => 'Permission groups',
    'page callback' => 'mm_admin_browse_group',
    'weight' => 0,
    'access callback' => 'user_access',
    'access arguments' => array('administer all groups'));
  $items['admin/mm/browse/menu'] = $base + array(
    'title' => 'Top-level menus',
    'page callback' => 'mm_admin_browse_menu',
    'weight' => 1,
    'access callback' => 'user_access',
    'access arguments' => array('administer all menus'));
  $items['admin/mm/browse/user'] = $base + array(
    'title' => 'User menus',
    'page callback' => 'mm_admin_browse_user',
    'weight' => 2,
    'access callback' => 'user_access',
    'access arguments' => array('administer all users'));
  $items['admin/mm/browse/all'] = $base + array(
    'title' => 'Entire tree',
    'page callback' => 'mm_admin_browse_all',
    'weight' => 3,
    'access callback' => '_mm_menu_access_all_admin');

  // Administer->User Management->Users
  $items['admin/user/user/by-uid'] = $base + array(
    'title' => 'Find by name',
    'type' => MENU_LOCAL_TASK,
    'page callback' => 'drupal_get_form',
    'page arguments' => array('mm_admin_find_user'),
    'access arguments' => array('administer users'));

  return $items;
}

function mm_admin_config() {
  $form = array();

  if (user_access('administer all menus')) {
    $form['mm_vgroup_regen_chunk'] = array(
      '#type' => 'textfield',
      '#title' => t('Chunk size used to split virtual group regeneration queries'),
      '#size' => 10, '#default_value' => variable_get('mm_vgroup_regen_chunk', 15),
      '#description' => t('To gain speed when regenerating dirty virtual groups, the separate queries are concatenated into one big query with a UNION. If the maximum SQL query buffer length is being exceeded or the large queries are taking too much memory, this value should be reduced.'),
    );
    $form['mm_vgroup_regen_chunks_per_run'] = array(
      '#type' => 'textfield',
      '#title' => t('Number of concatenated virtual group queries per cron run'),
      '#size' => 10, '#default_value' => variable_get('mm_vgroup_regen_chunks_per_run', 50),
      '#description' => t('If cron is taking too long to complete, this value should be reduced. The total number of dirty virtual groups updated per cron run is this number times the chunk size.'),
    );
    $form['mm_recycle_auto_empty'] = array(
      '#type' => 'select',
      '#title' => t('Automatic recycle bin deletion interval'),
      '#description' => t('Automatically delete content in recycle bins that has been there longer than this amount of time'),
      '#default_value' => variable_get('mm_recycle_auto_empty', 0),
      '#options' => array(
          -1                       => t('(don\'t use recycle bins)'),
          0                        => t('(never auto delete)'),
          30*60                    => t('30 minutes'),
          60*60                    => t('1 hour'),
          2*60*60                  => t('2 hours'),
          6*60*60                  => t('6 hours'),
          12*60*60                 => t('12 hours'),
          24*60*60                 => t('1 day'),
          2*24*60*60               => t('2 days'),
          3*24*60*60               => t('3 days'),
          7*24*60*60               => t('1 week'),
          2*7*24*60*60             => t('2 weeks'),
          30*24*60*60              => t('30 days'),
          60*24*60*60              => t('60 days'),
          intval(365/4*24*60*60)   => t('3 months'),
          intval(365/2*24*60*60)   => t('6 months'),
          intval(365/4*3*24*60*60) => t('9 months'),
          365*24*60*60             => t('1 year'),
      )
    );

    $form['mm_page'] = array(
      '#type' => 'fieldset',
      '#title' => t('Page Display'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE
    );
    $form['mm_page']['mm_home_mmtid'] = array(
      '#type' => 'textfield',
      '#title' => t('Location of the "home" page'),
      '#description' => t('The numeric Tree ID of the home (top level) page. By changing this, you can have multiple sites which use completely different sections of the tree, yet share the same permissions structures.'),
      '#size' => 6,
      '#default_value' => mm_home_mmtid(),
    );
    $form['mm_page']['mm_hide_empty_pages'] = array(
      '#type' => 'checkbox',
      '#title' => t('Hide empty pages in menus'),
      '#default_value' => variable_get('mm_hide_empty_pages', FALSE),
    );
    $form['mm_page']['mm_enable_rss'] = array(
      '#type' => 'checkbox',
      '#title' => t('Allow content creators to control the availability of RSS feeds on a per-page basis'),
      '#default_value' => variable_get('mm_enable_rss', FALSE),
    );

    $form['mm_node'] = array(
      '#type' => 'fieldset',
      '#title' => t('Node Display'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE
    );
    $form['mm_node']['mm_node_alert_frequency'] = array(
      '#type' => 'select',
      '#title' => t('Frequency of system alerts'),
      '#default_value' => variable_get('mm_node_alert_frequency', 'once'),
      '#options' => array(
        'once' => t('Show only once'),
        'login' => t('Show at every login'),
        'constantly' => t('Show at every page load')
      ),
    );
    $form['mm_node']['comments'] = array(
      '#type' => 'fieldset',
      '#title' => t('Comments'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE
    );
    $form['mm_node']['comments']['mm_show_count_instead_of_comments'] = array(
      '#type' => 'checkbox',
      '#title' => t('Show comment count instead of full comments'),
      '#description' => t('This option takes effect when viewing pages. When checked, nodes having comments show a link with the number of comments. Clicking on the link displays the node by itself, with the comments. If unchecked, all comments are displayed under their nodes, on the same page.'),
      '#default_value' => variable_get('mm_show_count_instead_of_comments', FALSE),
    );
    $form['mm_node']['comments']['mm_finegrain_comment_readability'] = array(
      '#type' => 'checkbox',
      '#title' => t('Control comment readability at the node level'),
      '#description' => t('This option lets users say who can read comments posted to each node on an individual basis. A default value for new nodes can also be set at the page level.'),
      '#default_value' => variable_get('mm_finegrain_comment_readability', FALSE),
    );
    $labels = variable_get('mm_comments_readable_labels', array());
    $labels[] = array();
    if (count($labels) == 1) $labels[] = array();
    $form['mm_node']['comments']['mm_comments_readable_labels'] = array(
      '#type' => 'markup',
      '#prefix' => '<table><tr><th>' . t('Permission') . '</th><th>' . t('Description') . '</th></tr>',
      '#suffix' => '</table>',
      '#description' => 'foo',
      '#tree' => TRUE
    );
    $i = 0;
    foreach ($labels as $label) {
      $form['mm_node']['comments']['mm_comments_readable_labels'][$i]['perm'] = array(
        '#type' => 'textfield',
        '#prefix' => '<tr><td>',
        '#default_value' => $label['perm'],
        '#size' => 30,
        '#suffix' => '</td><td>',
      );
      $form['mm_node']['comments']['mm_comments_readable_labels'][$i]['desc'] = array(
        '#type' => 'textfield',
        '#suffix' => '</td></tr>',
        '#default_value' => $label['desc'],
        '#size' => 30,
      );
      $i++;
    }
    $form['mm_node']['comments']['desc'] = array(
      '#type' => 'item',
      '#description' => t('<p><em>Permission</em> is the label appearing on the !url page; these are effectively ANDed with the <em>access comments</em> permission. Example: <em>comments readable by everyone</em></p><p><em>Description</em> is what users see in the list of choices for setting readability at the page/node level, it answers the question, "Who can read comments?" Example: <em>everyone</em></p><p>To remove a row, clear either value. <strong>Changing data in the Permission column or removing rows may affect the readability of comments in existing nodes!</strong> Don\'t forget to update the permissions after making changes here.</p>', array('!url' => l(t('Permissions'), 'admin/user/permissions', array('fragment' => 'module-monster_menus'))))
    );

    $form['mm_userhome'] = array(
      '#type' => 'fieldset',
      '#title' => t('User Home Pages'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE
    );
    $form['mm_userhome']['mm_use_user_dir'] = array(
      '#type' => 'checkbox',
      '#title' => t('Use user home directories'),
      '#description' => t('When enabled, each newly-added user gets a personal home page, starting at !url. Note: If you disable and then re-enable this option, any users created during the time it was disabled will not have home pages.', array('!url' => l('users', 'users'))),
      '#default_value' => variable_get('mm_use_user_dir', TRUE),
    );
    $form['mm_userhome']['mm_use_virtual_user_dir'] = array(
      '#type' => 'checkbox',
      '#title' => t('Use virtual user directories'),
      '#description' => t('If you have many users, the entire user list at !url can get very long. This feature will split the users into smaller chunks, based on the letter of the alphabet with which their name begins.', array('!url' => l('users', 'users'))),
      '#default_value' => variable_get('mm_use_virtual_user_dir', TRUE),
    );
    $form['mm_userhome']['mm_default_homepage'] = array(
      '#type' => 'textarea',
      '#title' => t('Default personal homepage message'),
      '#description' => t('What users see when viewing their own, empty homepage. Provide some instructions telling them how to create content.'),
      '#default_value' => variable_get('mm_default_homepage', ''),
    );

    $form['mm_username'] = array(
      '#type' => 'fieldset',
      '#title' => t('User Names'),
      '#description' => t('These names are displayed in content attribution lines and group membership lists.'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE
    );
    $form['mm_username']['mm_username_anon'] = array(
      '#type' => 'textfield',
      '#title' => t('Long name of the Anonymous user'),
      '#default_value' => variable_get('mm_username_anon', t('Anonymous user')),
    );
    $form['mm_username']['mm_username_admin'] = array(
      '#type' => 'textfield',
      '#title' => t('Long name of the Administrator user'),
      '#default_value' => variable_get('mm_username_admin', t('Administrator')),
    );
    $form['mm_username']['mm_username_disabled'] = array(
      '#type' => 'textfield',
      '#title' => t('Long name for all disabled users'),
      '#default_value' => variable_get('mm_username_disabled', t('Inactive user')),
    );

    $form['mm_nodelist'] = array(
      '#type' => 'fieldset',
      '#title' => t('Node Chooser'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );
    $form['mm_nodelist']['mm_nodelist_pager_limit'] = array(
      '#type' => 'select',
      '#title' => t('Number of nodes to show per page in the node chooser pager.'),
      '#options' => array(
        10 => 10,
        20 => 20,
        50 => 50,
        100 => 100
      ),
      '#default_value' => variable_get('mm_nodelist_pager_limit', 20),
    );

    $form['#submit'][] = 'mm_admin_config_submit';
  }

  mm_module_invoke_all_array('mm_config_alter', array(&$form));

  if ($form) return system_settings_form($form);

  $form['msg'] = array('#value' => t('You are not allowed to change any of the settings.'));
  return $form;
}

function mm_admin_config_validate($form, &$form_state) {
  foreach ($form_state['values']['mm_comments_readable_labels'] as $index => $row)
    if (empty($row['perm']) || empty($row['desc']))
      unset($form_state['values']['mm_comments_readable_labels'][$index]);
}

function mm_admin_config_submit($form, &$form_state) {
  // If mm_finegrain_comment_readability has never been set before, create the
  // default settings using the current 'access comments' setting.
  if ($form_state['values']['mm_finegrain_comment_readability'] && !variable_get('mm_finegrain_comment_readability_ever_set', FALSE)) {
    variable_set('mm_finegrain_comment_readability_ever_set', TRUE);
    db_query("UPDATE {permission} SET perm = CONCAT(perm, ', %s') WHERE perm REGEXP '(^|, )access comments(,|\$)'", MM_COMMENT_READABILITY_DEFAULT);
    db_query("UPDATE {permission} SET perm = CONCAT(perm, ', access comments') WHERE perm NOT REGEXP '(^|, )access comments(,|\$)'");
    drupal_set_message(t('Because you enabled the <em>Control comment readability at the node level</em> setting for the first time, the <em>access comments</em> permission has been enabled for all roles. This is necessary in order for this feature to work.'));
    drupal_set_message(t('You should now go to !url to set the roles for each permission you just created.', array('!url' => l(t('Permissions'), 'admin/user/permissions', array('fragment' => 'module-monster_menus')))));
  }
}

/**
 * Verify database relationships
 */
function mm_admin_verify_integrity() {
  foreach (module_list() as $module)
    module_load_include('install', $module);

  $tests = module_invoke_all('mm_verify_integrity');
  if (!$tests) return t('There are no tests to perform.');

  $form = array();
  $i = 0;
  foreach ($tests as $heading => $list) {
    $fieldset = "fs$i";
    $form[$fieldset] = array(
      '#type' => 'fieldset',
      '#title' => $heading,
      '#collapsible' => TRUE,
    );
    $form[$fieldset]['header'] = array(
      '#type' => 'value',
      '#value' => array(
        array('data' => t('Test')),
        array('data' => t('Result')),
      ),
    );

    foreach ($list as $description => $test)
      $form[$fieldset][] = array(
        'test' => array('#value' => check_plain(db_prefix_tables($test))),
        'description' => array('#value' => check_plain($description)),
        'result' => array('#value' => db_result(db_query("SELECT COUNT(*) FROM $test"))),
      );

    $i++;
  }
  return $form;
}

function mm_admin_list_blocks() {
  $form['header'] = array(
    '#type' => 'value',
    '#value' => array(
      array('data' => t('Name'), 'field' => 'name'),
      array('data' => t('Title'), 'field' => 'title'),
      array('data' => t('Flags')),
      array('data' => t('Operations'), 'colspan' => '2'),
    ),
  );
  $result = pager_query('SELECT * FROM {mm_block}' . tablesort_sql($form['header']['#value']), 20, 0, NULL);

  $destination = drupal_get_destination();
  while ($block = db_fetch_object($result)) {
    $form['name'][$block->bid] = array('#value' => check_plain($block->name));
    $form['title'][$block->bid] = array('#value' => check_plain($block->title));
    $flags = array();
    $fields = array(
      'title_is_cat' => t('Use page as title'),
      'allow_rss' => t('RSS feed'),
      'show_node_contents' => t('Show node contents'),
      'admin_only' => t('Admin. only')
    );
    foreach ($fields as $field => $label) {
      if ($block->$field) $flags[] = $label;
    }
    $form['flags'][$block->bid] = array('#value' => join(', ', $flags));
    $form['operations'][$block->bid][] = array('#value' => l(t('edit'), 'admin/build/block/mm/edit/'. $block->bid, array('query' => $destination)));
    $form['operations'][$block->bid][] = array('#value' => l(t('delete'), 'admin/build/block/mm/delete/'. $block->bid, array('query' => $destination)));
  }
  $form['pager'] = array('#value' => theme('pager', NULL, 20, 0));

  return $form;
}

function mm_admin_edit_block(&$form_state, $bid = NULL) {
  $blocks = mm_content_get_blocks();
  if (isset($bid) && isset($blocks[$bid])) {
    $form['bid'] = array('#type' => 'value', '#value' => $bid);
    $block = $blocks[$bid];
  }
  else $block = array();

  $x = mm_ui_strings(FALSE);
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#default_value' => $block['info'],
    '#size' => 40, '#maxlength' => 128,
    '#required' => TRUE,
    '#description' => t('The name visible to users in the Appearance settings for a @thing', $x)
  );
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#default_value' => $block['title'],
    '#size' => 40, '#maxlength' => 256,
    '#description' => t('An optional title to be placed above the block')
  );
  $form['help'] = array(
    '#type' => 'textarea',
    '#title' => t('Help text'),
    '#default_value' => $block['help'],
    '#rows' => 4,
    '#description' => t('The text which appears in the tooltip, describing when to use this block')
  );
  $form['title_is_cat'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use @thing as title', $x),
    '#default_value' => $block['title_is_cat'],
    '#description' => t('If checked, the title of the block\'s @thing is used as the block title instead of the Title field, above.', $x)
  );
  $form['allow_rss'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow RSS feed'),
    '#default_value' => $block['allow_rss'],
    '#description' => t('If checked, automatically generate an additional RSS feed <link> tag for this block.')
  );
  $form['show_node_contents'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show node contents'),
    '#default_value' => $block['show_node_contents'],
    '#description' => t('If checked, show the contents of all nodes in this @thing instead of links.', $x)
  );
  $form['admin_only'] = array(
    '#type' => 'checkbox',
    '#title' => t('Admin. only'),
    '#default_value' => $block['admin_only'],
    '#description' => t('If checked, only users with the "administer all menus" permission can assign @things to this block.', $x)
  );
  $form['submit'] = array('#type' => 'submit',
      '#value' => isset($block) ? t('Save settings') : t('Add block'));
  return $form;
}

function mm_admin_edit_block_submit($form, &$form_state) {
  $form_vals =& $form_state['values'];
  if (isset($form_vals['bid'])) {
    db_query("UPDATE {mm_block} SET name = '%s', title = '%s', help = '%s', title_is_cat = %d, allow_rss = %d, show_node_contents = %d, admin_only = %d WHERE bid = %d",
        $form_vals['name'], $form_vals['title'], $form_vals['help'], $form_vals['title_is_cat'], $form_vals['allow_rss'], $form_vals['show_node_contents'], $form_vals['admin_only'], $form_vals['bid']);
    if (drupal_get_destination()) return;
  }
  else {
    db_query("INSERT INTO {mm_block} (name, title, help, title_is_cat, allow_rss, show_node_contents, admin_only) VALUES('%s', '%s', '%s', %d, %d, %d, %d)",
        $form_vals['name'], $form_vals['title'], $form_vals['help'], $form_vals['title_is_cat'], $form_vals['allow_rss'], $form_vals['show_node_contents'], $form_vals['admin_only']);
    drupal_set_message(t('The new block has been created.'));
  }
  $form_state['redirect'] = 'admin/build/block/mm';
}

function mm_admin_delete_block(&$form_state, $bid = NULL) {
  $blocks = mm_content_get_blocks();
  if (isset($bid) && isset($blocks[$bid])) {
    $form['bid'] = array('#type' => 'value', '#value' => $bid);
    $block = $blocks[$bid];

    $dest = drupal_get_destination();
    if (!$dest) $dest = 'admin/build/block/mm';
    else $dest = urldecode(str_replace('destination=', '', $dest));

    $msg = t('Are you sure you want to delete the block %name?', array('%name' => $block['info']));
    $count = db_result(db_query('SELECT COUNT(*) FROM {mm_tree_block} WHERE bid = %d', $bid));
    if ($count) {
      $msg .= format_plural($count, ' <font color=red>There is a @thing using this block.</font>', ' <font color=red>There are @count @things using this block.</font>', mm_ui_strings(FALSE));
    }

    module_load_include('inc', 'monster_menus', 'mm_ui_content_delete');
    _mm_ui_content_delete_confirm_form($form, $msg, t('Delete'), $dest);
    return $form;
  }
  else {
    drupal_set_message(t('Block not found'));
  }
}

function mm_admin_delete_block_submit($form, &$form_state) {
  db_query('DELETE FROM {mm_block} WHERE bid = %d', $form_state['values']['bid']);
  db_query('DELETE FROM {mm_tree_block} WHERE bid = %d', $form_state['values']['bid']);
  drupal_set_message(t('The block has been deleted.'));
  if (drupal_get_destination()) return;
  $form_state['redirect'] = 'admin/build/block/mm';
}

function mm_admin_browse_group() {
  return mm_admin_show_tree(MM_GET_TREE_FILTER_GROUPS);
}

function mm_admin_browse_menu() {
  return mm_admin_show_tree(MM_GET_TREE_FILTER_NORMAL);
}

function mm_admin_browse_user() {
  return mm_admin_show_tree(MM_GET_TREE_FILTER_USERS);
}

function mm_admin_browse_all() {
  return mm_admin_show_tree();
}

/**
 * Display the tree as an HTML table
 *
 * @param $which
 *   The type of information to display (a MM_GET_TREE* constant)
 * @return
 *   The HTML code for the results
 */
function mm_admin_show_tree($which = NULL) {
  module_load_include('inc', 'monster_menus', 'mm_browser');

  if (!class_exists('browserRendererAdmin')) {
    /**
     * This class extends browserRenderer. It is used when rendering
     * administer->monster_menus pages.
     */
    class browserRendererAdmin extends browserRenderer {
      protected function add_col($n, $leaf, $item, $link_item, $url) {
        switch ($n) {
          case 0:
            $perms = $leaf->perms;
            if ($perms['r']) $links[] = l(t('contents'), "$url/contents", array('attributes' =>
                array('title' => t('View this page\'s contents'))));

            if ($perms['w'] || $perms['a']) $links[] = l(t('settings'),
                "$url/settings", array('attributes' => array('title' => t('Edit this page'))));

            $links[] = _mm_browser_info_tooltip($leaf, $item['title']);

            $ln = implode(' | ', $links);
            $this->cols[0] .= '<div class="browser-action">' .
                ($ln ? $ln : '&nbsp;') . "</div>\n";
        }
      }
    }
  }

  $type = mm_parse_args($mmtids, $path);

  $prefix = array_splice($path, 0, 3);
  array_unshift($prefix, $type);
  if ($prefix[1] == 'mm' && $prefix[2] == 1) unset($prefix[2]);

  drupal_add_css(drupal_get_path('module', 'monster_menus') .'/monster_menus.css');

  if (!count($path)) {
    $path[] = 1;
  }
  $gt_params = array(
    MM_GET_TREE_ADD_TO_CACHE =>      TRUE,
    MM_GET_TREE_HERE =>              $path,
    MM_GET_TREE_RETURN_KID_COUNT =>  TRUE,
    MM_GET_TREE_RETURN_NODE_COUNT => TRUE,
    MM_GET_TREE_RETURN_PERMS =>      TRUE,
  );
  if (!is_null($which)) {
    $gt_params[$which] = TRUE;
  }

  $tree = mm_content_get_tree(1, $gt_params);

  $i = 0;
  $br = new browserRendererAdmin($prefix, 1, array('', '', '', 'r', ''), TRUE);
  if (function_exists('over_text_params')) over_text_params('wrap', 1);

  return '<table class=browser><tr valign=top><td><ul class=menu>' .
      mm_content_render_tree($tree, $i, $xpath, $prefix, -1, $br) .
      "</ul></td>\n<td align=right>" . implode("</td>\n<td>", $br->get_cols()) .
      '</td></tr></table>';
}

/**
 * Print a CSV dump of the entire MM tree
 */
function mm_admin_dump() {
  class dumpIter {
    protected $fp;

    public function __construct() {
      $this->fp = fopen('php://output', 'w');
      fputcsv($this->fp, array(
          'visname', 'mmtid', 'level', 'name', 'alias', 'default_mode', 'owner', 'theme',
          'alw_theme', 'alw_type', 'hidden', 'groups_w', 'users_w', 'groups_a',
          'users_a', 'groups_u', 'users_u', 'groups_r', 'users_r', 'flags', 'block'));
    }

    public function iterate($item) {
      $visname = str_repeat('>', $item->level) . ' ' . $item->name;

      $allowed_themes = array();
      $allowed_node_types = array();
      $result = db_query("SELECT * FROM {mm_cascaded_settings} WHERE mmtid = %d AND type IN('allowed_themes', 'allowed_node_types')", $item->mmtid);
      while ($r = db_fetch_object($result)) {
        if ($r->type == 'allowed_themes') $allowed_themes[] = $r->data;
        else if ($r->type == 'allowed_node_types') $allowed_node_types[] = $r->data;
      }

      $groups = array('r' => array(), 'w' => array(), 'a' => array(), 'u' => array());
      $users = array('r' => array(), 'w' => array(), 'a' => array(), 'u' => array());
      $result =
        db_query('SELECT t2.mmtid,t2.name,a.mode FROM {mm_tree} t '.
                 'INNER JOIN {mm_tree_access} a ON a.mmtid=t.mmtid '.
                 'LEFT JOIN {mm_tree} t2 ON a.gid=t2.mmtid '.
                 'WHERE t2.mmtid>=0 AND a.mmtid=%d ORDER BY t2.name', $item->mmtid);
      while ($r = db_fetch_object($result))
        $groups[$r->mode][$r->mmtid] = $r->name;

      $result =
        db_query('SELECT a.mode,a.gid FROM {mm_tree} t INNER JOIN {mm_tree_access} a '.
                 'ON a.mmtid=t.mmtid '.
                 'WHERE a.gid<0 AND a.mmtid=%d', $item->mmtid);
      while ($r = db_fetch_object($result)) {
        $u = mm_content_get_users_in_group($r->gid, NULL, TRUE, 5);
        if (!is_null($u)) $users[$r->mode] = $u;
      }

      fputcsv($this->fp, array($visname, $item->mmtid, $item->level, $item->name,
          $item->alias, $item->default_mode, $item->uid, $item->theme,
          join(', ', $allowed_themes), join(', ', $allowed_node_types), $item->hidden,
          $this->dump($groups['w']), $this->dump($users['w']),
          $this->dump($groups['a']), $this->dump($users['a']),
          $this->dump($groups['u']), $this->dump($users['u']),
          $this->dump($groups['r']), $this->dump($users['r']),
          join(',', array_keys($item->flags)), $item->bid));

      return 1;
    }

    protected function dump($arr) {
      $out = array();
      foreach ($arr as $id => $name)
        if ($id) $out[] = "$name [$id]";
        else $out[] = $name;
      return join(', ', $out);
    }
  }

  header('Content-type: text/plain');
  $params = array(
    MM_GET_TREE_ITERATOR =>      new dumpIter(),
    MM_GET_TREE_RETURN_BLOCK =>  TRUE,
  );
  mm_content_get_tree(1, $params);
  exit();
}

function mm_admin_find_user(&$form_state) {
  $form['#theme'] = 'node_form';
  _mm_ui_userlist_setup(array(0 => ''), $form, 'userlist', t("User's name:"), TRUE, '');
  $form['userlist-choose']['#title'] = '';
  $form['buttons']['submit'] = array('#type' => 'submit',
      '#value' => t('View/edit user'));
  return $form;
}

function mm_admin_find_user_submit($form, &$form_state) {
  $uid = mm_ui_mmlist_key0($form_state['values']['userlist']);

  if (isset($uid)) {
    $form_state['redirect'] = array("user/$uid/edit", 'destination=admin/user/user/by-uid');
  }
}

/**
 * Check all mm_tree.sort_idx entries and optionally fix any that seem to be
 * incorrect.
 *
 * @param $fix
 *   If TRUE, queue any wrong entries to be fixed during monster_menus_exit().
 * @param $output_mode
 *   Sets the method for outputting any error messages. Currently, these methods
 *   are supported:
 *   - watchdog:  The Drupal watchdog() function (suitable for cron)
 *   - message:   The drupal_set_message() function, which sends output to the
 *                current web page (do not use in a batch script). This is the
 *                default method, and is used by the menu entry.
 *   - print:     Print the messages to standard i/o
 */
function mm_admin_validate_sort_index($fix = FALSE, $output_mode = 'message') {
  function _mm_admin_validate_sort_index_output($output_mode, $message, $strings) {
    static $errors = 0;

    if (is_null($message)) {
      if ($output_mode == 'message') {
        if ($errors) {
          return l(t('Click here to fix all errors'), 'admin/mm/sort/1');
        }
        else {
          return t('No errors found.');
        }
      }
      return;
    }

    if ($errors++ == 50) {
      $message = 'Only the first !num messages are shown';
      $strings = array('!num' => 50);
    }
    else if ($errors > 50) return;

    switch ($output_mode) {
      case 'watchdog':
        watchdog('mm_content_validate_sort_index', $message, $strings, WATCHDOG_ERROR);
        break;

      case 'message':
        drupal_set_message(t($message, $strings), 'error');
        break;

      case 'print':
        print t($message, $strings) . "\n";
        break;
    }
  }

  $parents = array();
  $skip_bad = '';
  $q = db_query(mm_content_get_query(array(
    MM_GET_TREE_MMTID => 1,
    MM_GET_TREE_DEPTH => -1,
    MM_GET_TREE_RETURN_TREE => TRUE,
    MM_GET_TREE_SORT => TRUE,
  )) . ', t.mmtid');
  while ($r = db_fetch_object($q)) {
    if ($r->sort_idx_dirty) {
      $skip_bad = $r->sort_idx;
      _mm_admin_validate_sort_index_output($output_mode, 'mmtid=!m (parent=!par) is marked as dirty. Updating now.', array('!m' => $r->mmtid, '!par' => $r->parent));
      // An update was previously missed, so definitely do it now
      mm_content_update_sort_queue($r->parent);
    }

    if ($r->mmtid > 1) {
      $is_bad = FALSE;
      if ($r->parent == $prev->mmtid || isset($sibling) && $r->parent == $sibling->mmtid) {
        // Going deeper in tree
        $parents[] = $parent;
        $parent = $r->parent == $prev->mmtid ? $prev : $sibling;
        $sibling = NULL;
      }
      else {
        if (strncmp($skip_bad, $r->sort_idx, strlen($skip_bad))) $skip_bad = '';

        $old_sib = $sibling;
        $old_parents = $parents;
        $old_parent = $parent;
        // See if we are going back up the tree
        while (count($parents) && $parent->mmtid != $r->parent) {
          $sibling = $parent;
          $parent = array_pop($parents);
        }

        // The parent was never found
        if ($parent->mmtid != $r->parent) {
          // If the item's parent exists, this means the sort index is off
          if (mm_content_get($r->parent)) {
            // The index is bad, but don't complain here; do it later on
            $sibling = $old_sib;
            $parents = $old_parents;
            $parent = $old_parent;
            $is_bad = TRUE;
          }
          else {
            _mm_admin_validate_sort_index_output($output_mode, 'MM Tree entry !mmtid has missing parent !parent. Testing halted.', array('!mmtid' => $r->mmtid, '!parent' => $r->parent));
            return;
          }
        }
      }

      if (empty($skip_bad)) {
        // Check the length of this entry's index against that of the parent
        if (strlen($r->sort_idx) - strlen($parent->sort_idx) != MM_CONTENT_BTOA_CHARS) {
          $msg = strlen($r->sort_idx) > strlen($parent->sort_idx) ? 'Sort index at mmtid=!m (parent=!par) is too long' : 'Sort index at mmtid=!m (parent=!par) is too short';
          _mm_admin_validate_sort_index_output($output_mode, $msg, array('!m' => $r->mmtid, '!par' => $r->parent));
          $is_bad = TRUE;
        }
        else {
          if ($r->sort_idx == $prev->sort_idx) {
            // Entry has same index as its predecessor
            _mm_admin_validate_sort_index_output($output_mode, 'mmtid=!m1 (parent=!par1) and mmtid=!m2 (parent=!par2) have the same sort index', array('!m1' => $r->mmtid, '!par1' => $r->parent, '!m2' => $prev->mmtid, '!par2' => $prev->parent));
            $is_bad = TRUE;
          }
          else if (strncmp($r->sort_idx, $parent->sort_idx, strlen($parent->sort_idx))) {
            // The indices should match, up to the parent's length
            _mm_admin_validate_sort_index_output($output_mode, 'Sort indices of child and parent do not match at mmtid=!m (parent=!par)', array('!m' => $r->mmtid, '!par' => $r->parent));
            $is_bad = TRUE;
          }
          else if (isset($sibling)) {
            if (substr($r->sort_idx, -MM_CONTENT_BTOA_CHARS) == substr($sibling->sort_idx, -MM_CONTENT_BTOA_CHARS)) {
              // Entry has same index as its last sibling
              _mm_admin_validate_sort_index_output($output_mode, 'Siblings mmtid=!m1 and mmtid=!m2 have the same sort index (their parent=!par)', array('!m1' => $r->mmtid, '!m2' => $sibling->mmtid, '!par' => $r->parent));
              $is_bad = TRUE;
            }
            else {
              $msg = '';
              if ($sibling->name == MM_ENTRY_NAME_RECYCLE) {
                $msg = 'Entry mmtid=!m1 comes after mmtid=!m2, which is a recycle bin (their parent=!par)';
              }
              else if ($r->name != MM_ENTRY_NAME_RECYCLE && !$r->hidden) {
                if ($sibling->hidden) {
                  $msg = 'Entry mmtid=!m1 is not hidden, but it comes after mmtid=!m2 which is (their parent=!par)';
                }
                else if ($r->weight < $sibling->weight) {
                  $msg = 'Entry mmtid=!m1 has a weight that is lower than mmtid=!m2 (their parent=!par)';
                }
                else if ($r->weight == $sibling->weight && strcasecmp($r->name, $sibling->name) < 0) {
                  // The simple cases are tested above, but strcasecmp() is not
                  // the same as collation-based comparisons in the DB, so upon
                  // failure, ask the DB if the sort is correct
                  if (db_result(db_query('SELECT t1.name < t2.name FROM {mm_tree} t1 INNER JOIN {mm_tree} t2 ON t2.parent = t1.parent WHERE t1.mmtid = %d AND t2.mmtid = %d', $r->mmtid, $sibling->mmtid))) {
                    $msg = 'Entry mmtid=!m1 comes after mmtid=!m2, even though it has a name that is earlier alphabetically (their parent=!par)';
                  }
                }
              }

              if ($msg) {
                _mm_admin_validate_sort_index_output($output_mode, $msg, array('!m1' => $r->mmtid, '!m2' => $sibling->mmtid, '!par' => $r->parent));
                $is_bad = TRUE;
              }
            }
          }
        }
      }

      if ($is_bad) {
        if ($fix) mm_content_update_sort_queue($r->parent, NULL, TRUE);
        if (empty($skip_bad)) $skip_bad = $r->sort_idx;
      }
      $sibling = $r;
    }
    $prev = $r;
  }

  return _mm_admin_validate_sort_index_output($output_mode, NULL, NULL);
}

function mm_admin_reassign_content() {
  $form['header'] = array(
    '#type' => 'markup',
    '#value' => t('This process is irreversible. Only run this script if you are absolutely sure you know what you are doing.'),
  );
  $form['old_user'] = array(
    '#type' => 'textfield',
    '#title' => t('Old username'),
  );
  $form['new_user'] = array(
    '#type' => 'textfield',
    '#title' => t('New username'),
  );
  $form['migrate_user_content'] = array(
    '#type' => 'checkbox',
    '#title' => t('Migrate content in personal user space'),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Submit'),
  );

  return $form;
}

function mm_admin_reassign_content_validate($form, &$form_state) {
  $old = user_load(array('name' => $form_state['values']['old_user']));
  $new = user_load(array('name' => $form_state['values']['new_user']));
  if (empty($old)) form_set_error('old_user', t('Could not find a match for old user.'));
  if (empty($new)) form_set_error('new_user', t('Could not find a match for new user.'));
  if ($form_state['values']["migrate_user_content"] == 1) {
    if (empty($old->user_mmtid)) {
      form_set_error('migrate_user_content', t('Unable to find the current user\'s home directory.  No further processing was done.'));
    }
  }
}

function mm_admin_reassign_content_submit($form, &$form_state) {
  $old = user_load(array('name' => $form_state['values']['old_user']));
  $new = user_load(array('name' => $form_state['values']['new_user']));
  $exclude_where = '';
  $args = array($old->uid);
  if (empty($form_state['values']["migrate_user_content"])) {
    $mmtids = array();
    $result = db_query("SELECT DISTINCT mmtid FROM {mm_tree_parents} WHERE parent = %d", $old->user_mmtid);
    $mmtids[] = $old->user_mmtid;
    while ($item = db_fetch_object($result)) {
      $mmtids[] = $item->mmtid;
    }
    $exclude_where = " AND mmtid NOT IN (" . db_placeholders($mmtids) . ")";
  }
  $args += $mmtids;
  $result = db_query("SELECT mmtid FROM {mm_tree} WHERE uid = %d" . $exclude_where, $args);
  $i = 0;
  while ($item = db_fetch_object($result)) {
    mm_content_update_quick(array('uid' => $new->uid), array('mmtid' => $item->mmtid), NULL, TRUE);
    $i++;
  }
  drupal_set_message(t('Groups and pages that have switched users: @count', array('@count' => $i)));
  array_unshift($args, $new->uid);
  db_query("UPDATE {node} n, {mm_node2tree} m SET n.uid = '%d' WHERE n.uid = %d AND n.nid = m.nid" . $exclude_where, $args);
  drupal_set_message(t('Nodes that have switched users: @count', array('@count' => db_affected_rows())));
  db_query("DELETE FROM {mm_virtual_group} WHERE uid = %d", $old->uid);
  drupal_set_message(t('Number of groups the user has been removed from: @count', array('@count' => db_affected_rows())));
}
