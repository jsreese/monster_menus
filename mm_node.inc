<?php
// $Id: mm_node.inc 4777 2010-11-12 19:20:34Z dan $

/**
 * @file
 * Custom node types for Monster Menus
 */

function mm_node_types() {
  $t = create_function('$s', function_exists('t') ? 'return t($s);' : 'return $s;');
  return array(
    'redirect' => array(
      'name' => $t('Redirector'),
      'module' => 'mm_node_redir',
      'perms' => array('create redirectors', 'edit own redirectors'),
      'menus' => array(),
      'themes' => array(),
      'description' => $t('Sends the user directly to another page you specify, either within the CMS or anywhere else on the Web')
    ),
    'gallery' => array(
      'name' => $t('Gallery'),
      'module' => 'mm_node_gallery',
      'perms' => array('create galleries', 'edit own galleries'),
      'menus' => array(),
      'themes' => array(),
      'description' => $t('Allows you to create galleries from the files you have previously uploaded')
    ),
    'subpglist' => array(
      'name' => $t('Subpage list'),
      'module' => 'mm_node_subpglist',
      'perms' => array('create subpage lists', 'edit own subpage lists'),
      'menus' => array(),
      'themes' => array(),
      'description' => $t('Allows you to create a list of subpages, usually in a multi-column table')
    ),
    'alert' => array(
      'name' => $t('System alert'),
      'module' => 'mm_node_alert',
      'perms' => array('create alerts', 'edit own alerts'),
      'menus' => array(),
      'themes' => array(),
      'description' => $t('Alerts are displayed in a popup window based on an appearance schedule')
    ),
  );
}

/**
 * Generate a page displaying a single node, along with its comments. This
 * modified version of node_show() is used by MM to allow comments to only be
 * visible to certain roles, when specified by the node's editor. In order for
 * this to work, you must enable the "access comments" permission for all roles.
 *
 * @param $node
 *   The node object ot display
 * @param $cid
 *   The ID of any comment to render
 * @param $message
 *   If TRUE, set the page's title to the revision status
 * @return
 *   The HTML of the resulting page
 */
function mm_node_show($node, $cid, $message = FALSE) {
  if ($message) {
    drupal_set_title(t('Revision of %title from %date', array('%title' => $node->title, '%date' => format_date($node->revision_timestamp))));
  }
  $output = node_view($node, FALSE, TRUE);

  if (function_exists('comment_render') && $node->comment && _mm_content_comments_readable($node)) {
    $output .= comment_render($node, $cid);
  }

  // Update the history table, stating that this user viewed this node.
  node_tag_new($node->nid);

  return $output;
}

/**
 * Menu callback; view a single node (taken from node_page_view()).
 */
function mm_node_page_view($node, $cid = NULL) {
  drupal_set_title(check_plain($node->title));
  return mm_node_show($node, $cid);
}

/******************** Redirector functions start here *********************/

/**
 * Implementation of hook_access().
 */
function mm_node_redir_access($op, $node, $account) {
  if ($op == 'create') {
    // Only users with permission to do so may create this node type.
    return user_access('create redirectors', $account);
  }

  // Users who create a node may edit or delete it later, assuming they have the
  // necessary permissions.
  if ($op == 'update' || $op == 'delete') {
    if (user_access('edit own redirectors', $account) && ($account->uid == $node->uid)) {
      return TRUE;
    }
  }
}

/**
 * Implementation of hook_form().
 */
function mm_node_redir_form(&$node) {
  drupal_set_title($node->nid ? t('Edit a redirector') : t('Create a redirector'));

  $form['title'] = array(
    '#type' => 'hidden',
    '#value' => t('Redirector'));

  $form['where'] = array(
    '#type' => 'fieldset',
    '#title' => t('Location to redirect to'),
    '#collapsible' => FALSE);
  $form['where']['redir_url'] = array(
    '#type' => 'textfield',
    '#maxlength' => 500,
    '#title' => t('Destination URL'),
    '#default_value' => $node->redir_url,
    '#attributes' => array('class' => 'node-redir-url'),
  );

  $mmlist = array();
  if ($node->nid) {
    if ($node->redir_category &&
        ($tree = mm_content_get_tree($node->redir_category, array(MM_GET_TREE_RETURN_PERMS => TRUE, MM_GET_TREE_DEPTH => 0))) &&
        $tree[0]->perms['r']) {

      $mmlist = array($node->redir_category => mm_content_expand_name($tree[0]->name));
      $pop_mmtid = $node->redir_category;
    }
    else if ($r = db_fetch_object(db_query('SELECT t.mmtid,t.name FROM {mm_node2tree} n INNER JOIN {mm_tree} t ON n.mmtid = t.mmtid WHERE nid = %d', $node->nid))) {
      $pop_mmtid = $r->mmtid;
    }

    $parents = mm_content_get_parents($pop_mmtid);
    array_shift($parents);  // skip root
    $pop_start = implode('/', $parents) . "/$pop_mmtid";
  }
  else {
    mm_parse_args($mmtids);
    $pop_start = implode('/', $mmtids);
  }

  $form['where']['redir_category'] = array(
    '#title' => t('Destination page (internal redirect):'),
    '#type' => 'mm_catlist',
    '#default_value' => $mmlist,
    '#mm_list_popup_start' => $pop_start,
    '#mm_list_max' => 1,
    '#mm_list_selectable' => 'r',
    '#mm_list_other_name' => 'redir_url',
    '#description' => t('Enter the absolute URL or choose the CMS page you want the user to go to when this page is accessed. When entering a URL be sure to use the format <code>http://...</code>')
  );
  mm_static('node_redir', TRUE);

  return $form;
}

/**
 * Implementation of hook_validate().
 */
function mm_node_redir_validate(&$node) {
  if (!empty($node->redir_category)) {
    $mmtid = mm_ui_mmlist_key0($node->redir_category);

    if (!isset($mmtid) || !mm_content_user_can($mmtid, 'r')) {
      form_set_error('redir_category',
          t('You are not allowed to redirect to page %cat.',
          array('%cat' => $node->redir_category[$mmtid])));
    }

    $node->redir_category = $mmtid;
  }
  else if (!empty($node->redir_url)) {
    $redir_url = trim($node->redir_url);

    if (!valid_url($redir_url, TRUE))
      form_set_error('redir_url', t('The destination you entered does not seem to be a valid URL.'));
  }
  else {
    form_set_error('redir_url', t('You must either enter a URL or choose a page to redirect to.'));
  }
}

/**
 * Implementation of hook_insert().
 */
function mm_node_redir_insert($node) {
  db_query("INSERT INTO {mm_node_redir} (nid, vid, url, mmtid) VALUES (%d, %d, '%s', %d)",
      $node->nid, $node->vid, $node->redir_url, $node->redir_category);
}

/**
 * Implementation of hook_update().
 */
function mm_node_redir_update($node) {
  // if this is a new node or we're adding a new revision,
  if ($node->revision) {
    mm_node_redir_insert($node);
  }
  else {
    db_query("UPDATE {mm_node_redir} SET mmtid=%d, url='%s' WHERE vid=%d",
        $node->redir_category, $node->redir_url, $node->vid);
  }
}

/**
 * Implementation of hook_nodeapi(), actually called by monster_menus_nodeapi()
 *
 * When a node revision is deleted, we need to remove the corresponding record
 * from our table.
 */
function mm_node_redir_nodeapi(&$node, $op, $teaser, $page) {
  switch ($op) {
    case 'presave':
      if (!empty($node->redir_category)) {
        $node->redir_category = mm_ui_mmlist_key0($node->redir_category);
      }
      else if (!empty($node->redir_url)) {
        $node->redir_url = trim($node->redir_url);
      }
      break;

    case 'delete revision':
      // Notice that we're matching a single revision based on the node's vid.
      db_query('DELETE FROM {mm_node_redir} WHERE vid = %d', $node->vid);
      break;
  }
}

/**
 * Implementation of hook_delete().
 */
function mm_node_redir_delete($node) {
  // Notice that we're matching all revision, by using the node's nid.
  db_query('DELETE FROM {mm_node_redir} WHERE nid = %d', $node->nid);
}

/**
 * Implementation of hook_load().
 */
function mm_node_redir_load($node) {
  $additions = db_fetch_object(db_query(
    'SELECT url AS redir_url, mmtid AS redir_category FROM {mm_node_redir} WHERE vid = %d',
    $node->vid));
  return $additions;
}

/**
 * Implementation of hook_view().
 */
function mm_node_redir_view($node, $teaser = FALSE, $page = FALSE) {
  $node = node_prepare($node, $teaser);
  if (node_access('update', $node)) {
    if (!empty($node->redir_category)) {
      $dest = 'mm/' . $node->redir_category;
      $abs = FALSE;
    }
    else if (!empty($node->redir_url)) {
      $dest = $node->redir_url;
      $abs = TRUE;
    }

    $output = t('This page would normally redirect the user to !dest. You are seeing this page because you are allowed to change the redirection.',
      array('!dest' => l(t('this location'), $dest, array('absolute' => $abs))));
    if ($teaser || !$page) {
      $node->content['teaser']['#value'] = $output;
      $node->content['body']['#value'] = '';
    }
    else {
      $node->content['teaser']['#value'] = '';
      $node->content['body']['#value'] = $output;
    }
  }
  else {
    if (!empty($node->redir_category)) {
      mm_redirect_to_mmtid($node->redir_category);
    }
    else if (!empty($node->redir_url)) {
      drupal_goto($node->redir_url);
    }
  }
  return $node;
}

/******************** Gallery functions start here *********************/

/**
 * Implementation of hook_access().
 */
function mm_node_gallery_access($op, $node, $account) {
  if (!module_exists('thickbox') || !module_exists('media')) return FALSE;

  if ($op == 'create') {
    // Only users with permission to do so may create this node type.
    return user_access('create galleries', $account);
  }

  // Users who create a node may edit or delete it later, assuming they have the
  // necessary permissions.
  if ($op == 'update' || $op == 'delete') {
    if (user_access('edit own galleries', $account) && ($account->uid == $node->uid)) {
      return TRUE;
    }
  }
}

/**
 * Implementation of hook_form().
 */
function mm_node_gallery_form(&$node, &$form_state) {
  drupal_set_title($node->nid ? t('Edit a gallery') : t('Create a gallery'));
  $defaults = _mm_node_gallery_settings();

  $type = node_get_types('type', $node);
  $form['title'] = array('#type' => 'textfield', '#title' => check_plain($type->title_label), '#required' => TRUE, '#default_value' => $node->title, '#weight' => -5);
  $form['body_filter']['body'] = array('#type' => 'textarea', '#title' => check_plain($type->body_label), '#default_value' => $node->body, '#rows' => 5);
  $form['body_filter']['filter'] = filter_form($node->format);

  mm_parse_args($mmtids);
  $pop_start = implode('/', $mmtids);
  $mmlist = array();
  $use_node = $node->nid || isset($form_state['values']);
  if ($use_node) {
    _mm_node_gallery_convert_values($node);
    foreach ($node->img_mmtids as $row)
      if ($row['nid']) {
        if (array_search($row['mmtid'], $mmtids = mm_content_get_by_nid($row['nid'])) !== FALSE)
            $tree = mm_content_get($row['mmtid']);
        else
            $tree = mm_content_get($mmtids[0]);

        $filename = db_result(db_query('SELECT uploadedfilename FROM {media_files} WHERE nid = %d', $row['nid']));
        if ($filename)
          if ($tree) $mmlist[$tree->mmtid . '/' . $row['nid']] = $filename;
          else $mmlist['0/' . $row['nid']] = $filename;
      }
      else {
        $tree = mm_content_get($row['mmtid']);
        if ($tree) $mmlist[$tree->mmtid] = mm_content_expand_name($tree->name) . ' ' . t('(all)');
      }
  }

  $form['where'] = array(
    '#type' => 'fieldset',
    '#title' => t('What to show in the gallery'),
    '#collapsible' => FALSE
  );
  $form['where']['img_mmtids'] = array(
    '#title' => t('File uploads:'),
    '#type' => 'mm_catlist',
    '#required' => TRUE,
    '#default_value' => $mmlist,
    '#mm_list_popup_start' => $pop_start,
    '#mm_list_selectable' => 'r',
    '#mm_list_browser' => 'media_assist/load_gallery',
    '#description' => t('Add one or more images. If you use %sa, every image on that page will appear in the gallery, including any that are added to the page in the future.', array('%sa' => t('Select All')))
  );

  $mode = $use_node ? $node->mode : $defaults->mode;
  $derivatives = _mm_node_gallery_derivatives();
  $derivatives_none = array_merge(array('' => t('(none)')), $derivatives);
  $thumb_deriv = $use_node ? $node->thumb_deriv : $defaults->thumb_deriv;
  $final_deriv = $use_node ? $node->final_deriv : $defaults->final_deriv;
  $per_page = _mm_node_gallery_per_page();

  $form['appearance'] = array(
    '#type' => 'fieldset',
    '#title' => t('Appearance'),
    '#attributes' => array('class' => 'node-gallery-appearance'),
    '#collapsible' => FALSE
  );
  $form['appearance']['mode'] = array(
    '#type' => 'radios',
    '#default_value' => $mode,
    '#attributes' => array('class' => 'node-gallery-mode'),
    '#options' => _mm_node_gallery_mode_types()
  );
  $form['appearance']['grid_opts'] = array(
    '#type' => 'fieldset',
    '#title' => t('Options'),
    '#attributes' => array('style' => 'display: none', 'id' => 'grid_opts'),
    '#collapsible' => FALSE
  );
  $form['appearance']['grid_opts']['grid_cols'] = array(
    '#type' => 'select',
    '#title' => t('Columns of thumbnails per page'),
    '#options' => $per_page->cols,
    '#default_value' => $use_node ? $node->grid_cols : $defaults->grid_cols,
  );
  $form['appearance']['grid_opts']['grid_rows'] = array(
    '#type' => 'select',
    '#title' => t('Rows of thumbnails per page'),
    '#options' => $per_page->rows,
    '#default_value' => $use_node ? $node->grid_rows : $defaults->grid_rows,
  );
  $form['appearance']['grid_opts']['grid_thumb_deriv'] = array(
    '#type' => 'select',
    '#title' => t('Thumbnail size'),
    '#options' => $derivatives,
    '#default_value' => $thumb_deriv,
    '#description' => t('This version of the image is displayed initially.')
  );
  $form['appearance']['grid_opts']['grid_final_deriv'] = array(
    '#type' => 'select',
    '#title' => t('Larger size to show after a click'),
    '#options' => $derivatives_none,
    '#default_value' => $final_deriv,
    '#description' => t('When the user clicks on the thumbnail, this larger version opens. If the image is too large to fit in the browser window, it will be automatically scaled-down to fit.')
  );

  $form['appearance']['flow_opts'] = array(
    '#type' => 'fieldset',
    '#title' => t('Options'),
    '#attributes' => array('style' => 'display: none', 'id' => 'flow_opts'),
    '#collapsible' => FALSE
  );
  $form['appearance']['flow_opts']['flow_per_page'] = array(
    '#type' => 'select',
    '#title' => t('Thumbnails per page'),
    '#options' => $per_page->flow,
    '#default_value' => $use_node ? $node->flow_per_page : $defaults->flow_per_page,
  );
  $form['appearance']['flow_opts']['flow_thumb_deriv'] = $form['appearance']['grid_opts']['grid_thumb_deriv'];
  $form['appearance']['flow_opts']['flow_final_deriv'] = $form['appearance']['grid_opts']['grid_final_deriv'];

  $form['appearance']['link_opts'] = array(
    '#type' => 'fieldset',
    '#title' => t('Options'),
    '#attributes' => array('style' => 'display: none', 'id' => 'link_opts'),
    '#collapsible' => FALSE
  );
  $form['appearance']['link_opts']['link_text'] = array(
    '#type' => 'textfield',
    '#title' => '<span class="form-required" title="'. t('This field is required.') .'">*</span> ' . t('Text of link'),
    '#default_value' => t('View the Image Gallery'),
    '#size' => 40,
    '#maxlength' => 255,
    '#description' => t('The user will click on this text to open the gallery popup.')
  );
  $form['appearance']['link_opts']['link_final_deriv'] = array(
    '#type' => 'select',
    '#title' => t('Size of image to show in the popup'),
    '#options' => $derivatives,
    '#default_value' => $final_deriv,
    '#description' => t('If the image is too large to fit in the browser window, it will be automatically scaled-down to fit.')
  );
  $form['appearance']['single_opts'] = array(
    '#type' => 'fieldset',
    '#title' => t('Options'),
    '#attributes' => array('style' => 'display: none', 'id' => 'single_opts'),
    '#collapsible' => FALSE
  );
  $form['appearance']['single_opts']['single_start'] = array(
    '#type' => 'radios',
    '#title' => t('Initial image to display'),
    '#options' => $per_page->start,
    '#default_value' => $use_node ? $node->single_start : $defaults->single_start,
  );
  $form['appearance']['single_opts']['single_thumb_deriv'] = $form['appearance']['grid_opts']['grid_thumb_deriv'];
  $form['appearance']['single_opts']['single_final_deriv'] = $form['appearance']['grid_opts']['grid_final_deriv'];
  $form['appearance']['1rand_opts'] = array(
    '#type' => 'fieldset',
    '#title' => t('Options'),
    '#attributes' => array('style' => 'display: none', 'id' => '1rand_opts'),
    '#collapsible' => FALSE
  );
  $form['appearance']['1rand_opts']['1rand_thumb_deriv'] = array(
    '#type' => 'select',
    '#title' => t('Image size'),
    '#options' => $derivatives,
    '#default_value' => $thumb_deriv,
    '#description' => t('The version of the image that is displayed.')
  );

  $form['appearance']['show_title'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show image titles'),
    '#default_value' => $use_node ? $node->show_title : $defaults->show_title,
  );
  $form['appearance']['show_caption'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show image captions'),
    '#default_value' => $use_node ? $node->show_caption : $defaults->show_caption,
  );
  $form['appearance']['show_comment'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show a %com link under each image', array('%com' => t('comments'))),
    '#default_value' => $use_node ? $node->show_comment : $defaults->show_comment,
  );
  mm_static('node_gallery', TRUE);

  return $form;
}

function mm_node_gallery_validate(&$node) {
  if ($node->mode == 'link' && empty($node->link_text)) {
    form_set_error('link_text', t('Missing required field'));
    return;
  }
}

function _mm_node_gallery_derivatives() {
  if (!module_exists('media')) {
    return array();
  }
  foreach (_media_image_get_sizes() as $size) {
    $derivatives[$size['label']] = $size['label'] . ' (' . $size['width'] . 'x' . $size['height'] . ')';
  }
  $derivatives['original'] = t('original size');
  return $derivatives;
}

function _mm_node_gallery_per_page() {
  return (object)array(
    'cols' => drupal_map_assoc(range(1, 20)),
    'rows' => drupal_map_assoc(range(1, 50)),
    'flow' => array(0 => t('(no limit)'), 1 => 1, 2 => 2, 3 => 3, 4 => 4, 5 => 5, 6 => 6, 7 => 7, 8 => 8, 9 => 9, 10 => 10, 15 => 15, 20 => 20, 25 => 25, 30 => 30, 40 => 40, 50 => 50, 75 => 75, 100 => 100, 150 => 150, 200 => 200),
    'start' => array('first' => t('First (most recently changed)'),
                     'last' => t('Last (oldest)'),
                     'random' => t('Random')),
  );
}

function _mm_node_gallery_settings() {
  return (object)array(
    'mode' => variable_get('mm_node_gallery_mode', 'flow'),
    'thumb_deriv' => variable_get('mm_node_gallery_thumb_deriv', 'thumbnail'),
    'final_deriv' => variable_get('mm_node_gallery_final_deriv', 'standard'),
    'grid_cols' => variable_get('mm_node_gallery_grid_cols', 5),
    'grid_rows' => variable_get('mm_node_gallery_grid_rows', 5),
    'flow_per_page' => variable_get('mm_node_gallery_flow_per_page', 20),
    'single_start' => variable_get('mm_node_gallery_single_start', 'first'),
    'show_title' => variable_get('mm_node_gallery_show_title', TRUE),
    'show_caption' => variable_get('mm_node_gallery_show_caption', TRUE),
    'show_comment' => variable_get('mm_node_gallery_show_comment', TRUE),
  );
}

function _mm_node_gallery_mode_types() {
  return array(
    'grid' => t('Show a grid of thumbnails'),
    'link' => t('Show a link that opens a popup window'),
    'single' => t('Show one image that opens a popup window when clicked'),
    '1rand' => t('Show a single, random image that is not clickable'),
    'flow' => t('Show the thumbnails one after another'),
  );
}

/**
 * Implementation of hook_mm_config_alter().
 *
 * This hook is called by MM when the settings page is being drawn.
 */
function mm_node_gallery_mm_config_alter(&$form) {
  if (user_access('administer content types')) {
    $defaults = _mm_node_gallery_settings();
    $derivatives = _mm_node_gallery_derivatives();
    $per_page = _mm_node_gallery_per_page();
    $form['gallery'] = array(
      '#type' => 'fieldset',
      '#title' => t('Gallery Defaults'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE
    );
    $form['gallery']['mm_node_gallery_mode'] = array(
      '#type' => 'radios',
      '#title' => t('Mode'),
      '#default_value' => $defaults->mode,
      '#options' => _mm_node_gallery_mode_types()
    );
    if ($derivatives) {
      $form['gallery']['mm_node_gallery_thumb_deriv'] = array(
        '#type' => 'select',
        '#title' => t('Thumbnail size'),
        '#options' => $derivatives,
        '#default_value' => $defaults->thumb_deriv,
        '#description' => t('This version of the image is displayed initially in "grid", "one after another", and "one at a time" modes. In "one random" mode, it is the only version displayed.')
      );
      $form['gallery']['mm_node_gallery_final_deriv'] = array(
        '#type' => 'select',
        '#title' => t('Larger size'),
        '#options' => $derivatives,
        '#default_value' => $defaults->final_deriv,
        '#description' => t('When the user clicks on the smaller version, this larger version opens (in "grid" and "one after another" modes.) In "link" mode, this size is always displayed. If the image is too large to fit in the browser window, it will be automatically scaled-down to fit.')
      );
    }
    else {
      $form['gallery']['mm_node_gallery_thumb_deriv'] = array(
        '#type' => 'value',
        '#value' => $defaults->thumb_deriv,
      );
      $form['gallery']['mm_node_gallery_final_deriv'] = array(
        '#type' => 'value',
        '#value' => $defaults->final_deriv,
      );
    }
    $form['gallery']['mm_node_gallery_show_title'] = array(
      '#type' => 'checkbox',
      '#title' => t('Show image titles'),
      '#default_value' => $defaults->show_title,
    );
    $form['gallery']['mm_node_gallery_show_caption'] = array(
      '#type' => 'checkbox',
      '#title' => t('Show image captions'),
      '#default_value' => $defaults->show_caption,
    );
    $form['gallery']['mm_node_gallery_show_comment'] = array(
      '#type' => 'checkbox',
      '#title' => t('Show a comments link under each image'),
      '#default_value' => $defaults->show_comment,
    );
    $form['gallery']['grid_opts'] = array(
      '#type' => 'fieldset',
      '#title' => t('"grid" mode'),
      '#collapsible' => FALSE
    );
    $form['gallery']['grid_opts']['mm_node_gallery_grid_cols'] = array(
      '#type' => 'select',
      '#title' => t('Columns of thumbnails per page'),
      '#options' => $per_page->cols,
      '#default_value' => $defaults->grid_cols,
    );
    $form['gallery']['grid_opts']['mm_node_gallery_grid_rows'] = array(
      '#type' => 'select',
      '#title' => t('Rows of thumbnails per page'),
      '#options' => $per_page->rows,
      '#default_value' => $defaults->grid_rows,
    );
    $form['gallery']['flow_opts'] = array(
      '#type' => 'fieldset',
      '#title' => t('"one after another" mode'),
      '#collapsible' => FALSE
    );
    $form['gallery']['flow_opts']['mm_node_gallery_flow_per_page'] = array(
      '#type' => 'select',
      '#title' => t('Thumbnails per page'),
      '#options' => $per_page->flow,
      '#default_value' => $defaults->flow_per_page,
    );
    $form['gallery']['single_opts'] = array(
      '#type' => 'fieldset',
      '#title' => t('"one at a time" mode'),
      '#collapsible' => FALSE
    );
    $form['gallery']['single_opts']['mm_node_gallery_single_start'] = array(
      '#type' => 'radios',
      '#title' => t('Initial image to display'),
      '#options' => $per_page->start,
      '#default_value' => $defaults->single_start,
    );
  }
}

/**
 * Implementation of hook_insert().
 */
function mm_node_gallery_insert($node) {
  foreach ($node->img_mmtids as $mmtid)
    db_query('INSERT INTO {mm_node_gallery_imgs} (nid, vid, img_mmtid, img_nid) VALUES (%d, %d, %d, %d)',
        $node->nid, $node->vid, $mmtid['mmtid'], $mmtid['nid']);
  db_query("INSERT INTO {mm_node_gallery} (nid, vid, mode, link_text, grid_cols, grid_rows, thumb_deriv, final_deriv, flow_per_page, single_start, show_title, show_comment, show_caption) VALUES (%d, %d, '%s', '%s', %d, %d, '%s', '%s', %d, '%s', %d, %d, %d)",
      $node->nid, $node->vid, $node->mode, $node->link_text, $node->grid_cols, $node->grid_rows, $node->thumb_deriv, $node->final_deriv, $node->flow_per_page, $node->single_start, $node->show_title, $node->show_comment, $node->show_caption);
}

/**
 * Implementation of hook_update().
 */
function mm_node_gallery_update($node) {
  // if this is not a new node or we're adding a new revision
  if (!$node->revision) {
    db_query('DELETE FROM {mm_node_gallery_imgs} WHERE vid = %d', $node->vid);
    db_query('DELETE FROM {mm_node_gallery} WHERE vid = %d', $node->vid);
  }
  mm_node_gallery_insert($node);
}

/**
 * Implementation of hook_nodeapi(), actually called by monster_menus_nodeapi()
 */
function mm_node_gallery_nodeapi(&$node, $op, $teaser, $page) {
  switch ($op) {
    case 'delete revision':
      // When a node revision is deleted, we need to remove the corresponding
      // record from our table. Notice that we're matching a single revision
      // based on the node's vid.
      db_query('DELETE FROM {mm_node_gallery_imgs} WHERE vid = %d', $node->vid);
      db_query('DELETE FROM {mm_node_gallery} WHERE vid = %d', $node->vid);
      break;

    case 'presave':
      _mm_node_gallery_convert_values($node);
      break;
  }
}


// Convert the img_mmtids field submitted by the form into an array, and add
// two other fields describing which derivatives to use
function _mm_node_gallery_convert_values(&$node) {
  $out = array();
  foreach ($node->img_mmtids as $img_mmtid => $name) {
    if ($img_mmtid == 'mmtid') return;   // already in correct format
    list($mmtid, $nid) = explode('/', $img_mmtid);
    $out[] = array('mmtid' => $mmtid, 'nid' => $nid);
  }
  $node->img_mmtids = $out;

  $which = $node->mode . '_final_deriv';
  $node->final_deriv = isset($node->$which) ? $node->$which : '';
  if ($mode != 'link') {
    $which = $node->mode . '_thumb_deriv';
    $node->thumb_deriv = isset($node->$which) ? $node->$which : '';
  }
}

/**
 * Implementation of hook_delete().
 */
function mm_node_gallery_delete($node) {
  // Notice that we're matching all revision, by using the node's nid.
  db_query('DELETE FROM {mm_node_gallery_imgs} WHERE nid = %d OR img_nid = %d', $node->nid, $node->nid);
  db_query('DELETE FROM {mm_node_gallery} WHERE nid = %d', $node->nid);
}

/**
 * Implementation of hook_mm_delete().
 */
function mm_node_gallery_mm_delete($mmtids) {
  db_query('DELETE FROM {mm_node_gallery_imgs} WHERE img_mmtid IN(' . db_placeholders($mmtids) . ') AND img_nid = 0', $mmtids);
  db_query('UPDATE {mm_node_gallery_imgs} SET img_mmtid = 0 WHERE img_mmtid IN(' . db_placeholders($mmtids) . ')', $mmtids);
}

/**
 * Implementation of hook_load().
 */
function mm_node_gallery_load($node) {
  $additions = db_fetch_object(db_query('SELECT * FROM {mm_node_gallery} WHERE vid = %d', $node->vid));
  $q = db_query('SELECT img_mmtid, img_nid FROM {mm_node_gallery_imgs} WHERE vid = %d ORDER BY id', $node->vid);
  $additions->img_mmtids = array();
  while ($row = db_fetch_object($q)) {
    $additions->img_mmtids[] = array('mmtid' => $row->img_mmtid, 'nid' => $row->img_nid);
  }
  return $additions;
}

/**
 * Implementation of hook_view().
 */
function mm_node_gallery_view($node, $teaser = FALSE, $page = FALSE) {
  global $user;

  $node = node_prepare($node, $teaser);
  if (module_exists('thickbox') && module_exists('media') && count($node->img_mmtids)) {
    $mmtids = $nodes = array();
    _mm_node_gallery_convert_values($node);
    foreach ($node->img_mmtids as $row)
      if ($row['nid']) {
        if (mm_content_user_can_node($row['nid'], 'r')) $nodes[] = $row['nid'];
      }
      else if (mm_content_user_can($row['mmtid'], 'r')) $mmtids[] = $row['mmtid'];

    if (!count($mmtids) && !count($nodes)) {
      $node->content['gallery'] = array(
        '#value' => t('Either this gallery is empty, or you do not have permission to read any of its contents.'),
        '#weight' => 1,
      );
      return $node;
    }

    // This is loosely based on mm_content_get_accessible_nodes_by_mmtid_query(),
    // but has to be even more complicated to account for nodes.
    // **** ONLY IMAGES ARE SUPPORTED (FOR NOW) ****
    $sql_part = array();
    for ($i = 0; $i < count($mmtids); $i++)
      if ($mmtids[$i] < 0) {
        array_splice($mmtids, $i, 1);
        break;
      }

    if (count($mmtids)) {
      $list = implode(',', $mmtids);
      $sql_part[] =
        '(SELECT n.nid, n.changed, n.type, '.
          'n.sticky AND (n.uid = 1 OR n.uid = tr.uid OR '.
            'COUNT(v.uid = n.uid OR g.vgid = 0 AND g.uid = n.uid)) AS stuck, r.weight '.
          'FROM {mm_node2tree} t '.
          "INNER JOIN {node} n ON t.nid = n.nid AND t.mmtid IN($list) AND n.status = 1 ".
          'INNER JOIN {mm_tree} tr ON tr.mmtid = t.mmtid '.
          'LEFT JOIN {mm_node_reorder} r ON r.mmtid = t.mmtid AND r.nid = n.nid '.
          "LEFT JOIN {mm_tree_access} a ON a.mode = 'w' AND a.mmtid = t.mmtid ".
          'LEFT JOIN {mm_group} g ON a.gid = g.gid '.
          'LEFT JOIN {mm_virtual_group} v ON g.vgid = v.vgid '.
            'AND (v.uid = n.uid OR g.vgid = 0 AND g.uid = n.uid) '.
          'GROUP BY n.nid)';
    }

    if (count($nodes)) {
      $list = implode(',', $nodes);
      $sql_part[] =
        '(SELECT nid, changed, type, sticky AS stuck, 0 AS weight FROM {node} '.
          "WHERE nid IN($list) AND status = 1)";
    }

    $now = time();
    $inner_sql =
      '(SELECT n.nid, n.changed AS created, n.stuck, n.weight ' .
      'FROM (' . join(' UNION ', $sql_part) . ') AS n ' .
      "INNER JOIN {media_files} mf ON mf.nid = n.nid AND n.type = 'media' " .
        "AND INSTR(mf.filemime, 'image') > 0 " .
      'LEFT JOIN {mm_node_schedule} s ON s.nid = n.nid ' .
      "WHERE IFNULL((s.publish_on = 0 OR s.publish_on <= $now) AND (s.unpublish_on = 0 OR $now < s.unpublish_on), 1)" .
      ') AS t';
    $count_sql = "SELECT COUNT(DISTINCT nid) FROM $inner_sql";
    $sql =       "SELECT * FROM $inner_sql " .
                   'GROUP BY nid ' .
                   'ORDER BY stuck DESC, weight ASC, created DESC';

    $out = '';
    $per_page = 0;
    switch ($node->mode) {
      case 'flow':
        $per_page = $node->flow_per_page;
        $out .= '<div class="mm-gallery-flow">';
        break;

      case 'grid':
        $per_page = $node->grid_rows * $node->grid_cols;
        $out .= '<table class="mm-gallery-table"><tbody>';
        break;

      case '1rand':
      case 'single':
        $single_total = db_result(db_query($count_sql));
        $single_list = '';
        $single_img = 0;
        if ($node->mode == '1rand') {
          $rand_img = rand(0, $single_total - 1);
          $sql = "SELECT * FROM $inner_sql " .
                   'GROUP BY nid ' .
                   'ORDER BY NULL ' .
                   "LIMIT $rand_img, 1";
        }
        else if ($node->single_start == 'random') {
          $single_img = rand(0, $single_total - 1);
        }
        else if ($node->single_start == 'last') {
          $single_img = $single_total - 1;
        }
        $out .= '<div class="mm-gallery-single">';
        break;
    }

    global $pager_total;
    $pager_elem = count($pager_total) + 1;
    $result = $per_page ? pager_query(db_rewrite_sql($sql), $per_page, $pager_elem, $count_sql) : db_query($sql);

    $i = 0;
    while ($n = db_fetch_object($result)) {
      $file_node = node_load($n->nid);
      // **** ONLY IMAGES ARE SUPPORTED (FOR NOW) ****
      if ($file_node->nid && $file_node->isimage) {
        $final_deriv = $node->final_deriv;  // $file_node->isimage ? $node->final_deriv : 'original';
        $thumb_deriv = $node->thumb_deriv;  // $file_node->isimage ? $node->thumb_deriv : 'original';

        switch ($node->mode) {
          case '1rand':
          case 'single':
            if ($i == $single_img) {
              $out .= _mm_node_gallery_image($file_node, $node, $thumb_deriv, $final_deriv, TRUE);
            }
            else if ($file_node->images[$final_deriv]) {
              $single_list .= _mm_node_gallery_image($file_node, $node, $thumb_deriv, $final_deriv, FALSE, '');
            }
            $i++;
            break;

          case 'link':
            if ($file_node->images[$final_deriv]) {
              if ($i == 1) $out .= '<div style="display: none">';
              $out .= _mm_node_gallery_image($file_node, $node, $link_title, $final_deriv, FALSE, $i ? '' : $node->link_text);
              $i++;
            }
            break;

          case 'flow':
            if ($file_node->images[$thumb_deriv]) {
              $thumb = _mm_node_gallery_image($file_node, $node, $thumb_deriv, $final_deriv);
              $out .= '<div class="mm-gallery-img mm-gallery-img-' . $thumb_deriv . '">' . $thumb . '</div>';
            }
            break;

          case 'grid':
            if ($file_node->images[$thumb_deriv]) {
              $thumb = _mm_node_gallery_image($file_node, $node, $thumb_deriv, $final_deriv);
              if ($i % $node->grid_cols == 0) $out .= '<tr>';
              $out .= '<td class="mm-gallery-img mm-gallery-img-' . $thumb_deriv . '">' . $thumb . '</td>';
              if ($i % $node->grid_cols == $node->grid_cols-1) $out .= '</tr>';
              $i++;
            }
            break;
        }
      }
    }

    switch ($node->mode) {
      case 'link':
        if ($i >= 2) $out .= '</div>';
        break;

      case 'flow':
        $out .= '</div><div class="mm-gallery-img-clear"></div>';
        $out .= theme('pager', NULL, $per_page, $pager_elem);
        break;

      case 'grid':
        if ($i % $node->grid_cols)
          if ($i < $node->grid_cols) $out .= '</tr>';
          else $out .= '<td colspan="' . ($node->grid_cols - ($i % $node->grid_cols)) . '" class="mm-gallery-empty"></td></tr>';

        $out .= '</tbody></table>';
        $out .= theme('pager', NULL, $per_page, $pager_elem);
        break;

      case '1rand':
      case 'single':
        if ($single_list) $out .= '<div style="display: none">' . $single_list . '</div>';
        $out .= '</div>';
        break;
    }

    $node->content['gallery'] = array(
      '#value' => $out,
      '#weight' => 1,
    );
  }
  return $node;
}

function _mm_node_gallery_image($file_node, $node, $thumb_deriv, $final_deriv, $hide_thumb_comment = FALSE, $link = NULL) {
  if (!$hide_thumb_comment && $file_node->comment > 0 && $node->show_comment) {
    $comments = $file_node->comment_count ? format_plural($file_node->comment_count, t('1 comment'), t('@count comments')) : t('No comments yet');
    $comments = '<div class="links">' . l($comments, 'node/' . $file_node->nid) . '</div>';
  }
  $file_node_title = check_plain(mm_ui_hide_node_title($file_node->title));

  $small_url = $file_node->isimage ? 'media/view/' . $file_node->nid . '/' . $thumb_deriv . '/' . urlencode($file_node->uploadedfilename) : base_path() . drupal_get_path('module', 'media') . '/icons/' . _media_get_icon($file_node->filemime);
  $small_img = '<img class="image ' . $thumb_deriv . '" src="' . check_url(url($small_url)) . '" alt="' . $file_node_title . '" title="';

  if (!empty($file_node_title) && $node->show_title) $title = "<h2>$file_node_title</h2>";
  if (!empty($file_node->teaser) && $node->show_caption) $caption = '<div class="mm-gallery-caption">' . $file_node->teaser . '</div>';

  if ($file_node->images[$final_deriv]) {
    $img_title = t('!size; last edited on !date', array('!date' => format_date($file_node->changed), '!size' => format_size($file_node->filesize[$final_deriv])));
    $small_img .= check_plain($img_title) . '"/>';
    $link_title = $title . $caption . $comments;
    $options = array('attributes' => array('class' => 'thickbox', 'rel' => 'node' . $node->nid, 'title' => $link_title), 'html' => TRUE);
    // The l() function calls strip_tags() on the title, so it can't be used here
    $thumb = '<a href="' .  check_url(url('media/view/' . $file_node->nid . '/' . $final_deriv . '/' . urlencode($file_node->uploadedfilename), $options)) . '"' . drupal_attributes($options['attributes']) . '>' . (isset($link) ? $link : $small_img) . '</a>';
    if (isset($link)) return $thumb;
  }
  else {
    $thumb = $small_img . $file_node_title . '"/>';
  }

  return $title . $thumb . $caption . $comments;
}

/******************** Subpage List functions start here *********************/

/**
 * Implementation of hook_access().
 */
function mm_node_subpglist_access($op, $node, $account) {
  if ($op == 'create') {
    // Only users with permission to do so may create this node type.
    return user_access('create subpage lists', $account);
  }

  // Users who create a node may edit or delete it later, assuming they have the
  // necessary permissions.
  if ($op == 'update' || $op == 'delete') {
    if (user_access('edit own subpage lists', $account) && ($account->uid == $node->uid)) {
      return TRUE;
    }
  }
}

/**
 * Implementation of hook_form().
 */
function mm_node_subpglist_form(&$node, &$form_state) {
  drupal_set_title($node->nid ? t('Edit a subpage list') : t('Create a subpage list'));
  $defaults = _mm_node_subpglist_settings();

  $type = node_get_types('type', $node);
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => check_plain($type->title_label),
    '#required' => FALSE,
    '#default_value' => $node->title,
    '#weight' => -5
  );
  $form['body_filter']['body'] = array(
    '#type' => 'textarea',
    '#title' => t('Preamble'),
    '#default_value' => $node->body,
    '#rows' => 5
  );
  $form['body_filter']['filter'] = filter_form($node->format);

  $use_node = $node->nid || isset($form_state['values']);

  $form['appearance'] = array(
    '#type' => 'fieldset',
    '#title' => t('Appearance'),
    '#collapsible' => FALSE
  );
  $form['appearance']['columns'] = array(
    '#type' => 'select',
    '#title' => t('Columns'),
    '#options' => drupal_map_assoc(range(1, 10)),
    '#default_value' => $use_node ? $node->columns : $defaults->columns,
  );
  $form['appearance']['depth'] = array(
    '#type' => 'select',
    '#title' => t('Depth'),
    '#options' => drupal_map_assoc(range(1, 10)),
    '#default_value' => $use_node ? $node->depth : $defaults->depth,
  );
  foreach (array_keys((array)$defaults) as $key) {
    if ($key != 'columns' && $key != 'depth') {
      $form['appearance'][$key] = array(
        '#type' => 'textfield',
        '#title' => drupal_ucfirst(str_replace('_', ' ', $key)),
        '#default_value' => $use_node ? $node->$key : $defaults->$key,
        '#size' => 50,
        '#maxlength' => 255,
      );
    }
  }

  $form['postamble'] = array(
    '#type' => 'textarea',
    '#title' => t('Postamble'),
    '#default_value' => $node->postamble,
    '#rows' => 5
  );

  return $form;
}

function _mm_node_subpglist_settings() {
  return (object)array(
    'columns' => 2,
    'depth' => 1,
    'outer_prefix' => '<table border="0"><tbody><tr valign="top">',
    'column_prefix' => '<td><ul>',
    'section_prefix' => '<ul>',
    'row_prefix' => '<li>',
    'row_suffix' => '</li>',
    'section_suffix' => '</ul>',
    'column_suffix' => '</ul></td>',
    'outer_suffix' => '</tr></tbody></table>',
  );
}

/**
 * Implementation of hook_insert().
 */
function mm_node_subpglist_insert($node) {
  db_query("INSERT INTO {mm_node_subpglist} (nid, vid, columns, depth, outer_prefix, column_prefix, section_prefix, row_prefix, row_suffix, section_suffix, column_suffix, outer_suffix, postamble) VALUES (%d, %d, %d, %d, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s')",
      $node->nid, $node->vid, $node->columns, $node->depth, $node->outer_prefix, $node->column_prefix, $node->section_prefix, $node->row_prefix, $node->row_suffix, $node->section_suffix, $node->column_suffix, $node->outer_suffix, $node->postamble);
}

/**
 * Implementation of hook_update().
 */
function mm_node_subpglist_update($node) {
  // if this is not a new node or we're adding a new revision
  if (!$node->revision) {
    db_query('DELETE FROM {mm_node_subpglist} WHERE vid = %d', $node->vid);
  }
  mm_node_subpglist_insert($node);
}

/**
 * Implementation of hook_nodeapi(), actually called by monster_menus_nodeapi()
 */
function mm_node_subpglist_nodeapi(&$node, $op, $teaser, $page) {
  switch ($op) {
    case 'delete revision':
      // When a node revision is deleted, we need to remove the corresponding
      // record from our table. Notice that we're matching a single revision
      // based on the node's vid.
      db_query('DELETE FROM {mm_node_subpglist} WHERE vid = %d', $node->vid);
      break;
  }
}

/**
 * Implementation of hook_delete().
 */
function mm_node_subpglist_delete($node) {
  // Notice that we're matching all revision, by using the node's nid.
  db_query('DELETE FROM {mm_node_subpglist} WHERE nid = %d', $node->nid);
}

/**
 * Implementation of hook_load().
 */
function mm_node_subpglist_load($node) {
  return db_fetch_object(db_query('SELECT * FROM {mm_node_subpglist} WHERE vid = %d', $node->vid));
}

/**
 * Implementation of hook_view().
 */
function mm_node_subpglist_view($node, $teaser = FALSE, $page = FALSE) {
  global $user;

  if ($teaser) {
    $node = node_prepare($node, $teaser);
    $node->readmore = TRUE;
    return $node;
  }

  mm_parse_args($mmtids, $oarg_list, $this_mmtid);
  if (!isset($this_mmtid)) {
    $with_node = mm_content_get_by_nid($node->nid);
    if (!count($with_node)) {
      return node_prepare($node, $teaser);
    }
    $this_mmtid = $with_node[0];
  }

  $tree_unfiltered = mm_content_get_tree($this_mmtid,
    array(
      MM_GET_TREE_RETURN_PERMS => TRUE,
      MM_GET_TREE_RETURN_BLOCK => TRUE,
      MM_GET_TREE_DEPTH        => $node->depth,
      MM_GET_TREE_ADD_TO_CACHE => TRUE,
    )
  );
  array_shift($tree_unfiltered);

  // There are two types of filtering that need to happen. One: Completely
  // remove (prune) all items (and their kids) that the user can't possibly see.
  // Two: Separate out the items that most users can't see, but the current
  // user can. Keep the topmost parent only.
  $shown = $hidden = array();
  $hide_skip_level = $prune_skip_level = 0;
  foreach ($tree_unfiltered as $item) {
    if ($item->level <= $prune_skip_level) $prune_skip_level = 0;
    if ($item->level <= $hide_skip_level) $hide_skip_level = 0;
    if (!$prune_skip_level) {
      if (($item->bid == '' || $item->bid == MM_MENU_BID || $item->bid < 0 ||
          $item->hidden && ($item->perms['w'] || $item->perms['a'] || $item->perms['u'])) && ($item->name != MM_ENTRY_NAME_RECYCLE || mm_content_user_can_recycle($item->mmtid, 'r'))) {
        if (!$hide_skip_level) {
          if (!$item->perms['IS_RECYCLED'] && !$item->hidden) {
            $shown[] = $item;
          }
          else {
            $hide_skip_level = $item->level;
            $hidden[] = $item;
          }
        }
      }
      else {
        $prune_skip_level = $item->level;
      }
    }
  }
  unset($tree_unfiltered);

  $node = node_prepare($node, $teaser);
  $link = "mm/$this_mmtid/";
  foreach (array($shown, $hidden) as $tree) {
    if (count($tree)) {
      if (count($hidden) && $tree[0]->mmtid == $hidden[0]->mmtid)
        $out .= t('<p>The following item(s) are normally hidden:</p>');
      $cols = min($node->columns, count($tree));
      $tree = array_values($tree);
      $branches = array();
      $cur_branch = $this_mmtid;
      foreach($tree as $leaf) {
        if ($leaf->parent == $this_mmtid) $cur_branch = $leaf->mmtid;
        $branches[$cur_branch][] = $leaf;
      }

      $out .= $node->outer_prefix;
      $per_col = floor(count($tree) / $cols);
      $remainder = count($tree) - $per_col * $cols;
      $col = 0;
      $cur_in_col = 0;
      $rem_in_col = $remainder > 0;
      $cur_branch = 0;
      foreach ($branches as $branch) {
        if ($cur_in_col == 0) {
          $out .= $node->column_prefix;
        }
        else if (($cur_in_col >= $per_col + $rem_in_col && $col != $cols - 1) ||
                 (count($branches) - $cur_branch == $cols - $col - 1)) {
          $out .= $node->column_suffix . $node->column_prefix;
          $cur_in_col = 0;
          $rem_in_col = --$remainder > 0;
          $col++;
        }
        $index = 0;
        $out .= mm_node_subpglist_render_branch($branch, $index, $link, $node->section_prefix, $node->section_suffix, $node->row_prefix, $node->row_suffix);
        $cur_in_col += count($branch);
        $cur_branch++;
      }

      $out .= $node->column_suffix . $node->outer_suffix;
    }
  }

  $node->content['subpglist'] = array(
    '#value' => $out,
    '#weight' => 1,
  );
  if (!empty($node->postamble)) {
    if (substr($node->postamble, 0, 3) != '<p>') {
      $node->postamble = "<p>$node->postamble</p>";
    }
    $node->content['postamble'] = array(
      '#value' => $node->postamble,
      '#weight' => 2,
    );
  }
  return $node;
}

function mm_node_subpglist_render_branch($branch, &$index, $link, $section_prefix, $section_suffix, $row_prefix, $row_suffix) {
  $out = '';
  $start_level = $branch[$index]->level;
  $max = count($branch);

  while ($index < $max && $branch[$index]->level == $start_level) {
    $leaf = $branch[$index++];

    $out .= $row_prefix . l(mm_content_expand_name($leaf->name), $link . $leaf->mmtid);

    while ($index < $max && $branch[$index]->level > $start_level) {
      $out .= $section_prefix;
      $out .= mm_node_subpglist_render_branch($branch, $index, $link, $section_prefix, $section_suffix, $row_prefix, $row_suffix);
      $out .= $section_suffix;
    }

    $out .= $row_suffix;
  }

  return $out;
}

/******************** Alert functions start here *********************/

/**
 * Implementation of hook_access().
 */
function mm_node_alert_access($op, $node, $account) {
  if ($op == 'create') {
    // Only users with permission to do so may create this node type.
    return user_access('create alerts', $account);
  }

  // Users who create a node may edit or delete it later, assuming they have the
  // necessary permissions.
  if ($op == 'update' || $op == 'delete') {
    if (user_access('edit own alerts', $account) && ($account->uid == $node->uid)) {
      return TRUE;
    }
  }
}

function monster_menus_form_alert_node_form_alter(&$form) {
  $form['scheduling']['#collapsed'] = FALSE;
  $form['scheduling']['publish_on']['#required'] = TRUE;
  $form['scheduling']['publish_on']['#description'] = '';
  $form['scheduling']['publish_on']['#default_value'] = _mm_ui_date_to_string(time());
}

/**
 * Implementation of hook_form().
 */
function mm_node_alert_form(&$node) {
  drupal_set_title($node->nid ? t('Edit a system alert') : t('Create a system alert'));

  $type = node_get_types('type', $node);
  $form['title'] = array('#type' => 'textfield', '#title' => check_plain($type->title_label), '#required' => TRUE, '#default_value' => $node->title, '#weight' => -5);
  $form['body_filter']['body'] = array('#type' => 'textarea', '#title' => check_plain($type->body_label), '#default_value' => $node->body, '#rows' => 20);
  $form['body_filter']['filter'] = filter_form($node->format);

  return $form;
}

function mm_node_alert_add_to_footer() {
  global $user;
  if (!$user->uid) return '';

  $output = '';
  $now = time();

  if (!empty($_SESSION["mm_node_alert_avoid_nid"]))
    $nid_skip_where = ' n.nid NOT IN (' . implode(',', $_SESSION["mm_node_alert_avoid_nid"]) . ') AND ';
  $frequency = variable_get('mm_node_alert_frequency', 'once');
  if ($frequency == 'once') {
    $last = $user->timestamp ? $user->timestamp : $user->access;
  }
  else if ($frequency == 'login') {
    if (!$_SESSION['mm_node_displayed_alert']) {
      $last = 0;
      $_SESSION['mm_node_displayed_alert'] = TRUE;
    }
    else {
      return;
    }
  }
  else if ($frequency == 'constantly') {
    $last = 0;
  }

  // Note: If this method proves not to work well, go to using the history table
  // instead. Just remember to call node_tag_new($node->nid) here and clear-out
  // any entries when the node is edited.
  $result = db_query("SELECT n.nid FROM {node} n ".
    "INNER JOIN {mm_node_schedule} s ON s.nid = n.nid ".
    "WHERE n.status = 1 AND n.type = '%s' AND %d >= s.publish_on AND ". $nid_skip_where .
      "(s.unpublish_on = 0 OR %d < unpublish_on) AND ".
      "(%d = 0 OR (%d BETWEEN s.publish_on AND n.changed - 1) OR %d < s.publish_on) ".
    "ORDER BY n.sticky DESC, IF(s.set_change_date = 1 AND s.publish_on > 0, s.publish_on, n.changed) DESC",
    'alert', $now, $now, $last, $last, $last, $last);
  while ($nid = db_result($result))
    if (mm_content_user_can_node($nid, 'r') && ($node = node_load($nid)) && !mm_content_node_is_recycled($node)) {
      $display = node_view($node, FALSE, FALSE, FALSE);
      if (strpos($display, '!!!Do not display this page!!!') === FALSE) {
        $output .= '<div style="display:none" id="mm-alert-' . $node->nid . '" class="mm-alert">' . $display . '<div class="mm-alert-close"><input type="button" value="' . t('Close') . '" /></div></div>';
        mm_static('node_alert', TRUE);
      }
      else {
        $_SESSION["mm_node_alert_avoid_nid"][] = $node->nid;
      }
    }
  return $output;
}