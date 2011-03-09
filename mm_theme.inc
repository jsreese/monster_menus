<?php
// $Id: mm_theme.inc 4937 2011-01-21 21:40:04Z dan $

/**
 * @file
 * Theme functions for Monster Menus
 */

global $_mmlist_instance;
$_mmlist_instance = 0;

/**
 * Theme a category list form element.
 */
function theme_mm_catlist($elt) {
  module_load_include('inc', 'monster_menus', 'mm_browser');
  return _mm_theme_mm_list(MM_BROWSER_MODE_PAGE, $elt, $elt['#mm_list_popup_root'],
      $elt['#title'] ? $elt['#title'] :
      ($elt['#mm_list_max'] == 1 ? t('Page:') : t('Pages:')),
      t('Path:'), '_mm_theme_list_info_cat');
}

/**
 * Theme a group list form element.
 */
function theme_mm_grouplist($elt) {
  module_load_include('inc', 'monster_menus', 'mm_browser');
  return _mm_theme_mm_list(MM_BROWSER_MODE_GROUP, $elt, mm_content_groups_mmtid(),
      $elt['#title'] ? $elt['#title'] :
      ($elt['#mm_list_max'] == 1 ? t('Group:') : t('Groups:')),
      t('Users in selected group:'), '_mm_theme_list_info_group');
}

/**
 * Theme a user list form element.
 */
function theme_mm_userlist($elt) {
  module_load_include('inc', 'monster_menus', 'mm_browser');
  return _mm_theme_mm_list(MM_BROWSER_MODE_USER, $elt, 0,
      $elt['#title'] ? $elt['#title'] :
      ($elt['#mm_list_max'] == 1 ? t('User:') : t('Users:')),
      t('Path:'), '_mm_theme_list_info_cat');
}

/**
 * Theme a node list form element.
 */
function theme_mm_nodelist($elt) {
  module_load_include('inc', 'monster_menus', 'mm_browser');
  return _mm_theme_mm_list(MM_BROWSER_MODE_NODE, $elt, 0,
      $elt['#title'] ? $elt['#title'] :
      ($elt['#mm_list_max'] == 1 ? t('Node:') : t('Nodes:')),
      t('Path:'), '_mm_theme_list_info_cat');
}

/**
 * Generate the form code to allow the user to add categories/groups to a node
 *
 * @param $elt
 *   Form object to create
 * @return
 *   The HTML code for the results
 */
function _mm_theme_mm_list($mode, $elt, $start_mmtid, $cat_label, $info_label,
    $info_func = NULL) {
  global $_mmlist_instance, $_mmsr_regexp;

  if (!user_access('use tree browser')) $elt['#mm_list_readonly'] = TRUE;
  $max = intval($elt['#mm_list_max']);
  $min = intval($elt['#mm_list_min']);
  $exactly1 = $max == 1 && $min == 1;
  $flags = $elt['#mm_list_buttons_underneath'] ? drupal_to_js(array('action_under' => TRUE)) : '{}';
  $select_callback = 'catSelectCallback';
  if ($elt['#mm_list_readonly']) {
    $label_above_actions = $label_add_cat = $label_replace = $label_delete = "''";
    if ($elt['#mm_list_no_info']) $select_callback = 'null';
  }
  else {
    $label_above_actions = drupal_to_js(t('Action:'));
    $label_add_cat = drupal_to_js($max == 1 || $mode == MM_BROWSER_MODE_USER ? '' : t('Add...'));
    $label_replace = drupal_to_js($mode == MM_BROWSER_MODE_USER ? '' :
        ($max == 1 ? t('Choose...') : t('Replace...')));
    $label_delete = drupal_to_js($exactly1 ? '' : ($min == 0 && $max == 1 ? t('Clear') : t('Delete')));
  }
  $label_above_info = $elt['#mm_list_no_info'] ? "''" : drupal_to_js($info_label);

  if ($mode == MM_BROWSER_MODE_USER) $popup_base = '';
  else {
    $root = $elt['#mm_list_browser'];
    if ($elt['#type'] == 'mm_nodelist') {
      $popup_base = base_path() . "$root/$start_mmtid-$mode-$_mmlist_instance-" .
        $elt['#mm_list_enabled'] . '-' . $elt['#mm_list_selectable'] . '--' . $elt['#field_name'] . '/';
    }
    else {
      $popup_base = base_path() . "$root/$start_mmtid-$mode-$_mmlist_instance-" .
        $elt['#mm_list_enabled'] . '-' . $elt['#mm_list_selectable'] . '/';
    }
  }
  $popup_URL = $elt['#mm_list_popup_start'];

  if ($max == 1 && !$elt['#value'])
    $elt['#value'] = count($elt['#default_value']) ? $elt['#default_value'] :
        array('' => t('(choose a location)'));

  $adds = array();
  if (!isset($_mmsr_regexp)) {
    if (is_array($elt['#value'])) {
      foreach ($elt['#value'] as $mmtid => $name) {
        if (!$mmtid) {
          $url = $exactly1 ? $popup_URL : '';
          $info = '';
        }
        else {
          $info_func($mmtid, $name, $url, $info, $popup_URL);
        }

        $adds[] = _mm_theme_add_row_js(array($name, $mmtid, $url, $info));
      }
    }
    else {
      foreach (_mm_ui_split_mmlist($elt['#value']) as $m) {
        $info_func($m[1], $name = $m[2], $url, $info, $popup_URL);
        list($mmtid, $nid) = explode('/', $m[1]);

        if ($mode != MM_BROWSER_MODE_USER && !$nid && !mm_content_user_can($mmtid, $elt['#mm_list_selectable'])) {
          if ($exactly1) {
            $name = t('(choose)');
            $info = '';
          }
          else {
            if ($url == $popup_URL) unset($popup_URL);
            continue;
          }
        }

        $adds[] = _mm_theme_add_row_js(array($name, $mmtid, $url, $info));
      }
    }
  }
  else {
    if ($exactly1) $adds[] = _mm_theme_add_row_js(array('', '', '', ''));
    $adds[] = '$(obj.p.hiddenElt).change(function() { mmListImport(obj, this.value); });';
  }

  if (empty($popup_URL)) $popup_URL = $start_mmtid;
  $popup_label = t('Tree Browser');

  if (isset($elt['#name'])) $name = $elt['#name'];
  else {
    $name = $elt['#parents'][0];
    if (count($elt['#parents']) > 1)
        $name .= '[' . join('][', array_slice($elt['#parents'], 1)) . ']';
  }

  $other_name = $elt['#mm_list_other_name'];
  $other_callback = isset($elt['#mm_list_other_callback']) ? $elt['#mm_list_other_callback'] : 'null';

  $modpath = drupal_get_path('module', 'monster_menus');
  drupal_add_js("$modpath/mm.js", 'module', 'header');
  drupal_add_css("$modpath/monster_menus.css");

  $cat_label .= !empty($elt['#required']) ? ' <span class="form-required" title="' .
      t('This field is required.') . '">*</span>' : '';
  $cat_label = drupal_to_js($cat_label);

  $class = _mm_theme_add_class($elt);
  $desc = $elt['#description'] ?
      "\n" . '<div class="description">' . $elt['#description'] . '</div>' : '';
  $auto = $elt['#mm_list_autocomplete_name'];
  $tag = $exactly1 || $elt['#mm_list_readonly'] && $elt['#mm_list_no_info'] ? '<span></span>' : '<a></a>';
  if (isset($_mmsr_regexp)) {
    $startup = "\nDrupal.settings.MMSR.fixups['[name=\"mm_list_obj$_mmlist_instance\"]'] = function() { mmListInit$_mmlist_instance(this) };";
  }
  else {
    $startup = "\n\$(document).ready(mmListInit$_mmlist_instance);";
  }

  $adds = join("\n    ", $adds);
  $js = <<<EOJ
mmListInit$_mmlist_instance = function(where) {
  var obj = mmListGetObj(where,
    'div[name=mm_list_obj$_mmlist_instance]',
    'div[name=mm_list_obj$_mmlist_instance] + div[class="$class"]',
    '$name',
    '$auto', {
      minRows: $min,
      maxRows: $max,
      popupBase: '$popup_base',
      popupURL: '$popup_URL',
      popupLabel: '$popup_label',
      flags: $flags,
      addCallback: catAddCallback,
      replaceCallback: catReplCallback,
      selectCallback: $select_callback,
      dataCallback: catDataCallback,
      labelAboveList: $cat_label,
      labelAboveActions: $label_above_actions,
      labelAddCat: $label_add_cat,
      labelReplace: $label_replace,
      labelDelete: $label_delete,
      labelAboveInfo: $label_above_info,
      updateOnChangeName: '$other_name',
      updateOnChangeCallback: $other_callback
    }
  );
  if (obj && obj.addItem) {
    $adds
    obj.enableOpts();
    obj.setHiddenElt();
  }
} $startup
EOJ;
  drupal_add_js($js, 'inline');
  $output = <<<EOJ
\n<div name="mm_list_obj$_mmlist_instance" style="display: none">$tag</div><div class="$class"></div>$desc
EOJ;
  $_mmlist_instance++;

  return $output;
}

function theme_mm_admin_list_blocks($form) {
  if (isset($form['name']) && is_array($form['name'])) {
    foreach (element_children($form['name']) as $key) {
      $row = array();
      $row[] = drupal_render($form['name'][$key]);
      $row[] = drupal_render($form['title'][$key]);
      $row[] = drupal_render($form['flags'][$key]);
      $row[] = drupal_render($form['operations'][$key][0]);
      $row[] = drupal_render($form['operations'][$key][1]);
      $rows[] = $row;
    }
  }
  else {
    $rows[] = array(array('data' => t('No blocks defined.'), 'colspan' => '5'));
  }

  $output .= theme('table', $form['header']['#value'], $rows);
  if ($form['pager']['#value']) {
    $output .= drupal_render($form['pager']);
  }

  $output .= drupal_render($form);
  $output .= t('<p>To control where these blocks appear on the page, you must visit !link.',
    array('!link' => l(t('the main Blocks editor'), 'admin/build/block')));

  return $output;
}

function theme_mm_admin_verify_integrity($form) {
  $passed = t('All tests passed.');
  foreach (element_children($form) as $fieldset) {
    if (substr($fieldset, 0, 2) == 'fs') {
      $rows = array();
      $collapsed = TRUE;
      foreach (element_children($form[$fieldset]) as $key) {
        if (is_numeric($key)) {
          $append = '';
          if ($form[$fieldset][$key]['result']['#value'] === '0') {
            $form[$fieldset][$key]['result']['#value'] = t('PASS');
          }
          else {
            $collapsed = $passed = FALSE;
            $append = '<input type="text" size="80" value="SELECT * FROM ' . drupal_render($form[$fieldset][$key]['test']) . '">';
            $form[$fieldset][$key]['result']['#value'] = t('<strong>FAIL</strong>');
          }

          $row = array();
          $row[] = '<div class="description">' . drupal_render($form[$fieldset][$key]['description']) . '</div>' . $append;
          $row[] = drupal_render($form[$fieldset][$key]['result']);
          $rows[] = $row;
        }
      }

      $form[$fieldset] = array(
        '#type' => 'fieldset',
        '#value' => theme('table', $form[$fieldset]['header']['#value'], $rows),
        '#title' => $form[$fieldset]['#title'],
        '#collapsible' => TRUE,
        '#collapsed' => $collapsed,
      );
    }
  }

  return $passed . drupal_render($form);
}

/**
 * Format a tooltip activation string
 *
 * @param $text
 *   The text of the link that starts the tooltip
 * @param $title
 *   The popup tooltip's title
 * @param $tip
 *   The popup tooltip's text
 * @param $html
 *   If TRUE, $text should be raw HTML, and check_plain() is not run
 * @return
 *   The HTML code for the results
 */
function theme_tooltip($text, $title, $tip, $html = FALSE) {
  if (!function_exists('over_text_make')) return '';

  drupal_add_css(drupal_get_path('module', 'monster_menus') .'/monster_menus.css');
  $attrib = over_text_make($title, $tip);
  $attrib['onclick'] = 'return false';
  if (!$html) $text = check_plain($text);
  return '<a href="#" class="tooltip-link" ' . drupal_attributes($attrib) . ">$text</a>";
}

function theme_mm_repeatlist_div($elt) {
  $elt['#attributes']['style'] .= '; display: none';
  return '<div id="' . $elt['#parents'][0] . '"' . drupal_attributes($elt['#attributes']) . '>' . $elt['#children'] . "</div>\n";
}

/**
 * Theme a form input element with the 'This field is required' notation, but
 * which does not get the built-in validation. Instead, it is assumed that
 * validation code will be provided in a custom function. This is helpful when
 * the element is dynamically hidden to the user using Javascript, because it
 * prevents validation errors on elements which aren't visible.
 */
function theme_mm_fake_required($elt) {
  $elt['#required'] = TRUE;
  $elt['#type'] = $elt['#mm_orig_type'];
  unset($elt['#mm_orig_type']);
  return theme($elt['#type'], $elt);
}

/**
 * Add Javascript code to a page, allowing the user to repeat a set of form
 * fields any number of times without having to submit the form multiple times.
 *
 * To specify which data is repeatable, use the mm_repeatlist_div type to set
 * up a new form group and pass its ID in the mm_repeatlist form element using
 * the #mm_list_id parameter. In order for the code to work properly, the
 * mm_repeatlist_div MUST come before the mm_repeatlist form element in the
 * form array.
 *
 * Default values for the data fields can be passed using the #default_value
 * parameter. It should contain an array of arrays, one per row of data to
 * pre-populate in the form. To parse the data after it has been submitted by
 * the user, call mm_ui_parse_repeatlist().
 *
 * Required fields in the form element:
 *   #mm_list_id                 Form ID of repeatable DIV (mm_repeatlist_div
 *                               type)
 *   #mm_list_inputs_per_row     Number of values per row in repeatable DIV and
 *                               #default_value
 *
 * Optional fields in the form element:
 *   #mm_list_min                Min number of rows (default: 1)
 *   #mm_list_max                Max number of rows (default: 0)
 *   #mm_list_buttons_underneath Show the action buttons underneath the data
 *                               rows, instead of to the right (default: FALSE)
 *   #mm_list_reorder            Show up/down arrows in each row, allowing user
 *                               to change the order (default: FALSE)
 *
 * Caveats:
 *   - In order for the code to work properly, the mm_repeatlist_div MUST come
 *     before the mm_repeatlist form element in the form array
 *   - The '#multiple' option is not supported for the 'select' type.
 *   - The 'file' and 'password' types are not supported.
 *   - If using the 'date' type, you must be sure to allocate three elements per
 *     row in the '#default_value' field, and increase the
 *     '#mm_list_inputs_per_row' accordingly.
 *
 * Example: Create a form element that allows the user to enter up to 10 rows of
 * data in two textfields per row. The form is pre-populated with some default
 * values:
 *
 *     $form['name_age'] = array(
 *       '#type' => 'mm_repeatlist_div',
 *       '#title' => t('Name and age'));
 *     $form['name_age']['name'] = array(
 *       '#type' => 'textfield',
 *       '#title' => t('Your first name'),
 *       '#description' => t('What is your first name?'));
 *     $form['name_age']['age'] = array(
 *       '#type' => 'textfield',
 *       '#title' => t('Your age'),
 *       '#description' => t('What is your age?'));
 *
 *     $form['grouping'] = array(
 *       '#type' => 'fieldset',
 *       '#collapsible' => TRUE, '#collapsed' => FALSE);
 *     $form['grouping']['data'] = array(
 *       '#type' => 'mm_repeatlist',
 *       '#title' => t('Tell us about yourself'),
 *       '#mm_list_id' => 'name_age',
 *       '#mm_list_inputs_per_row' => 2,
 *       '#default_value' => array(
 *           array('name1', '18'),
 *           array('name2', '26')));
 *
 * @param $elt
 *   The form element to display
 * @return
 *   The HTML code for the form element
 */
function theme_mm_repeatlist($elt) {
  global $_mmlist_instance, $_mmsr_regexp;

  $max = intval($elt['#mm_list_max']);
  $min = intval($elt['#mm_list_min']);
  $exactly1 = $max == 1 && $min == 1;

  $label_above_actions = $elt['#mm_list_buttons_underneath'] ? '&nbsp;' : '';
  $cat_label = $elt['#title'] . (!empty($elt['#required']) ? ' <span class="form-required" title="'.
      t('This field is required.') .'">*</span>' : '');
  $cat_label = drupal_to_js($cat_label);

  $flags = array('narrow_actions' => TRUE);
  if ($elt['#mm_list_buttons_underneath']) $flags['action_under'] = TRUE;
  if ($elt['#mm_list_readonly']) {
    $labels = array();
    $label_add_set = "''";
    $flags['readonly'] = TRUE;
  }
  else {
    $labels = mm_ui_mmlist_labels();
    $labels[5] = '';  // no edit
    if (!$elt['#mm_list_reorder']) $labels[0] = $labels[1] = $labels[3] = $labels[4] = '';
    $label_add_set = drupal_to_js($max == 1 ? '' : $elt['#mm_list_add_button']);
  }
  $flags = drupal_to_js($flags);

  if (isset($elt['#name'])) $name = $elt['#name'];
  else {
    $name = $elt['#parents'][0];
    if (count($elt['#parents']) > 1)
        $name .= '[' . join('][', array_slice($elt['#parents'], 1)) . ']';
  }

  $class = _mm_theme_add_class($elt);
  $desc = $elt['#description'] ?
      "\n".'<div class="description">'. $elt['#description'] .'</div>' : '';
  $objname = $elt['#mm_list_id'];

  $modpath = drupal_get_path('module', 'monster_menus');
  drupal_add_js("$modpath/mm.js", 'module', 'header');
  drupal_add_css("$modpath/monster_menus.css");

  $imgpath = base_path() . $modpath . '/images';
  $del_confirm = t("Are you sure you want to delete this row?\\n\\n(You can skip this alert in the future by holding the Shift key while clicking the Delete button.)");

  $adds = array();
  if (isset($_mmsr_regexp)) {
    $startup = "\nDrupal.settings.MMSR.fixups['[name=\"mm_list_obj$_mmlist_instance\"]'] = 'function() { mmListInit$_mmlist_instance(this); }';";
  }
  else {
    $value = $elt['#value'];
    if (!is_array($value))
      if ($value[0] == '{') $value = mm_ui_parse_repeatlist($value, $elt['#mm_list_inputs_per_row']);
      else $value = array();
    if ($min) $value = array_pad($value, $min, array());
    foreach ($value as $row)
      $adds[] = _mm_theme_add_row_js($row);
    $startup = "\n\$(document).ready(mmListInit$_mmlist_instance);";
  }
  $adds = join("\n    ", $adds);

  $js = <<<EOJ
mmListInit$_mmlist_instance = function(where) {
  var obj = mmListGetObj(where,
    'div#$objname',
    'div[name=mm_list$_mmlist_instance]',
    '$name',
    null, {
      minRows: $min,
      maxRows: $max,
      flags: $flags,
      addCallback: listAddCallback,
      dataCallback: listDataCallback,
      labelAboveList: $cat_label,
      labelAboveActions: '$label_above_actions',
      labelAddList: $label_add_set,
      imgPath: '$imgpath',
      delConfirmMsg: '$del_confirm',
      labelTop: '$labels[0]',
      labelUp: '$labels[1]',
      labelX: '$labels[2]',
      labelBott: '$labels[3]',
      labelDown: '$labels[4]',
      labelEdit: '$labels[5]'
    }
  );
  if (obj && obj.addItem) {
    $adds
    obj.enableOpts();
    obj.setHiddenElt();
  }
} $startup
EOJ;
  drupal_add_js($js, 'inline');
  $output = "\n<div name=\"mm_list$_mmlist_instance\" class=\"$class\"></div>$desc";
  $_mmlist_instance++;

  return $output;
}

/**
 * Theme one radio button with a Help tooltip.
 */
function theme_mm_help_radio($element) {
  _form_set_class($element, array('form-radio'));
  $output = '<input type="radio" ';
  $output .= 'name="' . $element['#name'] . '" ';
  $output .= 'value="'. $element['#return_value'] .'" ';
  $output .= ($element['#value'] == $element['#return_value']) ? ' checked="checked" ' : ' ';
  $output .= drupal_attributes($element['#attributes']) . ' />';

  if (!is_null($element['#title'][0])) {
    $output = '<label class="option">' . $output . ' ' . $element['#title'][0] .'</label>';
  }

  if (function_exists('over_text_params') && !empty($element['#title'][1])) {
    over_text_params('wrap', 1);
    $output .= '&nbsp;&nbsp;&nbsp;' . theme('tooltip', t('help'), $element['#title'][0], $element['#title'][1]);
  }

  return theme('form_element', NULL, $output);
}

/**
 * Theme a list of radio buttons, each with a Help tooltip.
 */
function theme_mm_help_radios($element) {
  if ($element['#title'] || $element['#description']) {
    return theme('form_element', $element, $element['#children']);
  }

  return $element['#children'];
}

function theme_mm_ui_menu_reorder($form) {
  drupal_add_tabledrag('mm-reorder', 'order', 'sibling', 'menu-weight');

  $rows = array();
  foreach (element_children($form) as $mmtid) {
    $element = &$form[$mmtid];
    if (isset($element['mmtid'])) {
      // Add special classes to be used for tabledrag.js.
      $element['mmtid']['#attributes']['class'] = 'menu-mmtid';
      $element['weight']['#attributes']['class'] = 'menu-weight';

      $row = array();
      $row[] = drupal_render($element['title']);
      $row[] = drupal_render($element['weight']) . drupal_render($element['mmtid']);

      $row = array_merge(array('data' => $row), $element['#attributes']);
      $row['class'] = !empty($row['class']) ? $row['class'] .' draggable' : 'draggable';
      $rows[] = $row;
    }
  }

  $output = '';
  if ($rows) {
    $output .= theme('table', array(), $rows, array('id' => 'mm-reorder'));
  }
  $output .= drupal_render($form);
  return $output;
}

function theme_mm_ui_node_reorder($form) {
  drupal_add_tabledrag('mm-reorder1', 'order', 'sibling', 'sticky-weight');
  drupal_add_tabledrag('mm-reorder2', 'order', 'sibling', 'unsticky-weight');

  $sticky_rows = $unsticky_rows = array();
  foreach (element_children($form) as $mmtid) {
    $element = &$form[$mmtid];
    if (isset($element['nid'])) {
      // Add special classes to be used for tabledrag.js.
      $element['nid']['#attributes']['class'] = 'menu-nid';
      $element['weight']['#attributes']['class'] = isset($element['stuck']) ? 'sticky-weight' : 'unsticky-weight';

      $row = array();
      $row[] = drupal_render($element['title']);
      $row[] = drupal_render($element['type']);
      $row[] = drupal_render($element['weight']) . drupal_render($element['nid']);

      $row = array_merge(array('data' => $row), $element['#attributes']);
      $row['class'] = !empty($row['class']) ? $row['class'] .' draggable' : 'draggable';
      if (isset($element['stuck'])) {
        $sticky_rows[] = $row;
      }
      else {
        $unsticky_rows[] = $row;
      }
    }
  }

  $output = '';
  $header = array(t('Title'), t('Type'), t('Weight'));
  if ($sticky_rows) {
    if ($unsticky_rows) {
      $output .= t('<h3>Sticky content</h3>');
    }
    $output .= theme('table', $header, $sticky_rows, array('id' => 'mm-reorder1'));
  }
  if ($unsticky_rows) {
    if ($sticky_rows) {
      $output .= t('<p /><h3>Non-sticky content</h3>');
    }
    $output .= theme('table', $header, $unsticky_rows, array('id' => 'mm-reorder2'));
  }
  $output .= drupal_render($form);
  return $output;
}

function theme_mm_ui_node_permissions($form) {
  if (!empty($form['everyone']['#post'])) {
    list($groups, $users, $default_modes) = _mm_ui_form_parse_perms($form['everyone']['#post'], FALSE);
    foreach ($users as $uid => $list)
      $users[$uid] = $list['name'];
    $item_uid = $form['everyone']['#post']['owner'];
  }

  $form['indiv_tbl']['#type'] = 'value';
  $form['indiv_tbl']['#value'] = array(
    'title' => t('Individuals'),
    'action' => mm_ui_add_user_subform('settings-perms-indiv-add', t('add'), t('User(s) to add to permissions:'), t('Add users to permissions'), 'Drupal.MMSettingsPermsAddUsers'),
  );

  foreach (array('indiv_tbl', 'groups_tbl') as $type) {
    $kids = array();
    foreach ($form[$type] as $key => $item) {
      if (is_array($item) && isset($item['#mm_delete_link'])) {
        $rows = array();
        if (!isset($users)) $users = $item['#mm_users'];
        if ($type == 'indiv_tbl') {
          if (!isset($item_uid)) $item_uid = $item['#mm_owner']['uid'];
          if (isset($form['everyone']['owner'])) $form['everyone']['owner']['#value'] = $item_uid;
          list($name, $msg, $owner_readonly) = mm_ui_owner_desc(array('@thing' => $item['#mm_owner']['type']), $item_uid);
          $kids[] = _mm_ui_perms_table_row(
            'user',
            'owner',
            t('<span class="settings-perms-owner-prefix">Owner: </span><span class="settings-perms-owner-name">!name</span>', array('!name' => $name)),
            $msg,
            $owner_readonly ? NULL : mm_ui_add_user_subform('settings-perms-indiv-owner', t('change'), t('Owner:'), t('Change the owner'), 'Drupal.MMSettingsPermsOwner', $item_uid, $name)
          );

          if (!$owner_readonly) {
            $kids[] = array('#type' => 'hidden', '#name' => 'uid', '#value' => $item_uid);
          }

          foreach ($users as $uid => $name) {
            $obj = array(
              "user-w-$uid" => array(
                '#name' => "user-w-$uid",
                '#type' => 'hidden',
                '#value' => $uid,
              ),
              array(
                '#type' => 'item',
                '#value' => $name,
              ),
            );
            $kids[] = _mm_ui_perms_table_row('user', $uid, $obj, '', $item['#mm_delete_link']);
          }

          // Empty row to be used when adding new users
          $obj = array(
            array(
              '#type' => 'item',
              '#value' => '.',
              '#id' => 'mm-permissions-user-new',
            ),
            'user-w-new' => array(
              '#name' => 'user-w-new',
              '#type' => 'hidden',
              '#value' => 0,
            ),
          );
          $kids[] = _mm_ui_perms_table_row('user', 'new', $obj, '', $item['#mm_delete_link']);
        }
        else {
          if (!isset($groups)) $groups = $item['#mm_groups'];
          foreach ($groups as $mmtid => $data) {
            $temp_elem = $item['#mm_groups_elem'];
            $temp_elem[0]['#title'] = $data['name'];
            $temp_elem[0][0]['#value'] = $data['members'];
            $temp_elem[0]["group-w-$mmtid"] = array(
              '#name' => "group-w-$mmtid",
              '#type' => 'hidden',
              '#value' => $mmtid,
            );
            $kids[] = _mm_ui_perms_table_row('group', $mmtid, $temp_elem, '', $item['#mm_delete_link']);
          }

          // Empty row to be used when adding new groups
          $temp_elem = $item['#mm_groups_elem'];
          $temp_elem[0][0]['#id'] = 'mm-permissions-group-new';
          $temp_elem[0]['group-w-new'] = array(
            '#name' => 'group-w-new',
            '#type' => 'hidden',
            '#value' => 0,
          );
          $kids[] = _mm_ui_perms_table_row('group', 'new', $temp_elem, '', $item['#mm_delete_link']);
        }
      }
      else if (is_numeric($key)) {
        $kids[] = $item;
      }
      else {
        $kids[$key] = $item;
      }
    }
    $form[$type] = $kids;
  }

  mm_static('settings_perms', TRUE);
  return theme_mm_ui_permissions($form);
}

function theme_mm_ui_permissions($form) {
  $rows = array();
  foreach (element_children($form) as $section_id) {
    $colspan = 1;
    if (is_array($form[$section_id]['#value']['types'])) {
      $colspan = count($form[$section_id]['#value']['types']) + 1;
    }

    $row = array(
      'class' => 'mm-permissions-header-row',
      'data' => array(
        array(
          'header' => TRUE,
          'data' => '<h2>' . $form[$section_id]['#value']['title'] . '</h2>',
          'class' => 'first-col',
        ),
      ),
    );

    if (is_array($form[$section_id]['#value']['types']) && $form[$section_id]['#value']['headings']) {
      foreach ($form[$section_id]['#value']['types'] as $type => $vals) {
        $row['data'][] = array(
          'header' => TRUE,
          'data' => '<h4>' . $vals[0] . '</h4>',
        );
      }
      $row['data'][] = array(
        'header' => TRUE,
        'data' => '',
      );
    }
    else {
      $row['data'][] = array(
        'header' => TRUE,
        'data' => '',
        'colspan' => $colspan,
      );
    }

    $rows[] = $row;
    foreach (element_children($form[$section_id]) as $row_id) {
      $row = !empty($form[$section_id][$row_id]['#mm_is_data_row']) ? array('class' => 'mm-permissions-data-row') : array();
      foreach (element_children($form[$section_id][$row_id]) as $item_id) {
        $row['data'][] = drupal_render($form[$section_id][$row_id][$item_id]);
      }
      $rows[] = $row;
    }

    if (isset($form[$section_id]['#value']['action'])) {
      $rows[] = array(
        array('colspan' => $colspan, 'data' => ''),
        array('data' => $form[$section_id]['#value']['action']),
      );
    }
  }

  $output = '';
  if ($rows) {
    $output = theme('table', array(), $rows, array('id' => 'mm-permissions'));
  }
  $output .= drupal_render($form);
  return $output;
}

function theme_mm_archive_header($frequency, $date) {
  $format = array(
    'year' => t('Archive for %Y'),
    'month' => t('Archive for %B %Y'),
    'week' => t('Archive for the week of %B %e, %Y'),
    'day' => t('Archive for %A, %B %e, %Y'),
  );
  return strftime($format[$frequency], $date);
}

function theme_mm_archive($list, $frequency, $this_mmtid, $main_mmtid, $archive_mmtid, $date) {
  $format = array(
    '%Y',
    '%B',
    $frequency == 'week' ? t('week of %b %e') : '%e'
  );
  $link_attributes = array('attributes' => array('class' => 'archive-link'));
  $out = array('header' => '', 'links' => '');

  // remove unneeded upper levels
  $offset = 0;
  while (is_array($list) && count($list) == 1 && is_array($list[key($list)])) {
    $list = $list[key($list)];
    $offset++;
  }

  if (is_array($list) && count($list) == 0) return $out;
  if ($this_mmtid == $archive_mmtid) {
    $recent_link = '<span class="archive-link">' . l(t('See the most recent entries'), mm_content_get_mmtid_url($main_mmtid), $link_attributes) . '</span>';
    $out['header'] = '<h2 class="archive-header">' . theme('mm_archive_header', $frequency, $date) . '</h2>';
    if (count($list) == 1) {
      $out['links'] = '<div class="archive-links">' . $recent_link . '</div>';
      return $out;
    }
  }
  if (!is_array($list) || !mm_content_user_can($archive_mmtid, 'r')) return $out;
  if (count($list) == 1) {
    $out['links'] = '<div class="archive-links">' . l(t('See older entries'), "mm/$archive_mmtid/" . strftime('%Y-%m-%d', $list[key($list)]), $link_attributes) . '</div>';
    return $out;
  }

  _mm_theme_archive_links_recurs($list, $offset, $format, $archive_mmtid, $out['links']);

  if ($out['links']) {
    $out['links'] = '<ul class="select-menu">' . $out['links'] . '</ul>';
    if ($this_mmtid == $archive_mmtid) {
      $out['links'] = '<label class="archive-link">' . t('Choose another archive: ') . '</label>' . $out['links'] . $recent_link;
    }
    else {
      $out['links'] = '<label class="archive-link">' . t('Choose an archive: ') . '</label>' . $out['links'];
    }
    $out['links'] = '<div class="archive-links">' . $out['links'] . '</div>';

    mm_static('select_menu', TRUE);
  }
  return $out;
}

function theme_mm_page_wrapper($title, $body, $attributes = array('class' => 'mm-page-wrapper'), $wrap_body_with_div = TRUE) {
  $head = drupal_get_html_head() . drupal_get_css() . drupal_get_js();
  $body_att = drupal_attributes($attributes);
  if ($wrap_body_with_div) $body = "<div class=\"mm-page-wrapper\">$body</div>";
  $status = theme('status_messages');
  $closure = theme('closure');

  return <<<EOH
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" lang="$GLOBALS[locale]"
  xml:lang="$GLOBALS[locale]">
<head>
  <meta http-equiv="Content-Style-Type" content="text/css" />
  <title>$title</title>
  $head
</head>
<body$body_att>
$status$body$closure
</body>
</html>
EOH;
}

function theme_mm_tabbed_ui($title, $outer_id, &$tabs, $selected = '', $before_tabs = '') {
  /* NOTE: Be careful not to have more than one form element with the same name
     on multiple tabs. If the element is required on one tab, and the user
     does not enter any value, it will be highlighted in red on EVERY tab on
     which it appears. */

  $output = "<div id=\"$outer_id\" class=\"mm-tabbed-ui\">";
  if (!empty($title)) $output .= "<h5><a href=\"#\">$title</a></h5>";
  $output .= "<div class=\"mm-tabs\" style=\"display: none\">$before_tabs<ul>";
  $forms = '';
  uasort($tabs, 'element_sort');  // sort by #weight
  $i = 0;
  foreach ($tabs as $id => $data) {
    $output .= '<li><a href="#' . $id . '"><span>' . $data['#title'] . '</span></a></li>';
    $forms .= '<div id="' . $id . '">' . $data['#data'] . '</div>';
    $tabs[$id]['#order'] = $i++;
  }

  jquery_ui_add(array('ui.widget', 'ui.tabs'), FALSE);

  $selected_tab = 0;
  if ($selected && isset($tabs[$selected])) $selected_tab = $tabs[$selected]['#order'];
  mm_static('tabbed_ui_js', TRUE, $selected_tab);
  drupal_add_css(drupal_get_path('module', 'monster_menus') . '/mm_tabbed_ui.css');

  return $output . "</ul>$forms</div></div>";
}

function theme_mm_ui_mark_yesno($yes) {
  drupal_add_css(drupal_get_path('module', 'monster_menus') . '/monster_menus.css');
  if ($yes) {
    return '<span class="mm-yesno-yes">&#10004;</span>';
  }
  return '<span class="mm-yesno-no">&#10007;</span>';
}

// ****************************************************************************
// * Private functions start here
// ****************************************************************************

function _mm_theme_archive_links_recurs($list, $offset, $format, $mmtid, &$out) {
  $return = NULL;
  ksort($list, SORT_NUMERIC);
  foreach ($list as $key => $item) {
    if (is_array($item)) {
      $inner = '';
      $return = _mm_theme_archive_links_recurs($item, $offset + 1, $format, $mmtid, $inner);
      $out .= '<li>' . trim(strftime($format[$offset], $return))  . '</li>';
      $out .= '<ul>' . $inner . '</ul>';
    }
    else {
      $out .= '<li>' . l(trim(strftime($format[$offset], $item)), "mm/$mmtid/" . strftime('%Y-%m-%d', $item)) . '</li>';
      $return = $item;
    }
  }
  return $return;
}

function _mm_theme_add_row_js($row) {
  $list = '';
  foreach ($row as $col)
    $list .= ', ' . drupal_to_js($col);

  return "obj.addItem(false$list);";
}

// helper function for theme_mm_catlist when it calls _mm_theme_mm_list
function _mm_theme_list_info_cat($mmtid, $name, &$url, &$info, &$popup_URL) {
  list($mmtid, $node) = explode('/', $mmtid);

  $parents = mm_content_get_parents($mmtid);
  array_shift($parents);  // skip root
  if ($node) $parents[] = $mmtid;

  $url = implode('/', $parents);
  if ($mmtid) $url .= "/$mmtid";

  $path = array();
  foreach ($parents as $par) {
    if (!($tree = mm_content_get($par)))
        break;
    $path[] = mm_content_expand_name($tree->name);
  }

  if (!$node) $path[] = $name;
  $info = implode('&nbsp;&raquo; ', $path);

  if (isset($popup_URL)) {
    $top = explode('/', $popup_URL, 2);
    if (($found = strstr($url, "/$top[0]/")) !== FALSE) {
      $url = substr($found, 1);
    }
  }
  else {
    $popup_URL = $url;
  }
}

// helper function for theme_mm_grouplist when it calls _mm_theme_mm_list
function _mm_theme_list_info_group($mmtid, $name, &$url, &$info, &$popup_URL) {
  $parents = mm_content_get_parents($mmtid);
  array_shift($parents);  // skip root
  $url = implode('/', $parents);

  if (!isset($popup_URL)) $popup_URL = $url;

  $url .= "/$mmtid";

  $info = mm_content_get_users_in_group($mmtid, '<br />', FALSE, 20, TRUE);
  if ($info == '') $info = t('(none)');
}

function _mm_theme_add_class($elt) {
  $list = explode(' ', $elt['#attributes']['class']);
  if (count($list) == 1 && $list[0] == '') unset($list[0]);
  array_unshift($list, 'form-item');
  if (form_get_error($elt)) array_push($list, 'mm-list-error');
  return implode(' ', $list);
}
