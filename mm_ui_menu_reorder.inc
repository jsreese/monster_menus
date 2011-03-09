<?php
// $Id: mm_ui_menu_reorder.inc 4216 2010-06-25 18:36:37Z dan $

/**
 * @file
 * User interface routines for reordering MM content in menus
 */

function mm_ui_menu_reorder(&$form_state, $mmtid) {
  if (!function_exists('_mm_ui_menu_reorder_filter')) {
    function _mm_ui_menu_reorder_filter($item) {
      return !$item->hidden && ($item->bid == '' || $item->bid == MM_MENU_BID || $item->bid < 0) && $item->name != MM_ENTRY_NAME_RECYCLE;
    }
  }

  mm_content_add_breadcrumb(t('Reorder menu'));
  $params = array(
    MM_GET_TREE_DEPTH         => 1,
    MM_GET_TREE_FILTER_NORMAL => TRUE,
    MM_GET_TREE_FILTER_USERS  => TRUE,
    MM_GET_TREE_RETURN_BLOCK  => TRUE,
    MM_GET_TREE_RETURN_PERMS  => TRUE,
  );
  $tree = mm_content_get_tree($mmtid, $params);
  // Skip root
  array_shift($tree);
  $tree = array_filter($tree, '_mm_ui_menu_reorder_filter');

  $form = array('#tree' => TRUE);
  $count = count($tree);
  if ($count > 1) {
    if ($count > MM_UI_MAX_REORDER_ITEMS) {
      $form['empty_menu'] = array('#value' => t('<p>There are too many @subthings of this @thing to make reordering feasible.</p>', mm_ui_strings(FALSE)));
    }
    else {
      $alt = t('drag handle');
      $clear = drupal_get_path('module', 'monster_menus') . '/images/clear.gif';
      $form['#prefix'] = t('<div id="help"><p>To reorder an item, grab the !image and drag it to a new location. Be sure to <em>Save configuration</em> when done.</p></div>', array('!image' => '<a href="#" class="tabledrag-handle" style="text-decoration: none; cursor: text"><span class="handle">' . theme('image', $clear, '', '', array('width' => 13, 'height' => 13), FALSE) . '</span></a>'));
      $form['#suffix'] = t('<p /><div id="help"><p>If a custom menu order is being used, any new items you add will appear at the top of the list. You will most likely have to return here to change their location.</p><p>Use the <em>Reorder Z to A and save</em> button to order the menu reverse-alphabetically.</p><p>Use the <em>Remove custom ordering and save</em> button to revert to the default, A to Z order. Any new items added later on will appear in their correct location, alphabetically.</p></div>');

      $new_weight = 1;
      foreach ($tree as $data) {
        $id = 'mmtid:' . $data->mmtid;
        $form[$id]['#attributes'] = array('class' => 'menu-enabled');
        $form[$id]['title']['#mm_orig_name'] = mm_content_expand_name($data->name);
        $form[$id]['title']['#value'] = check_plain($form[$id]['title']['#mm_orig_name']);
        $form[$id]['weight'] = array(
          '#type' => 'weight',
          '#delta' => $count,
          '#default_value' => isset($form_state[$id]['weight']) ? $form_state[$id]['weight'] : $new_weight++,
        );
        $form[$id]['mmtid'] = array(
          '#type' => 'hidden',
          '#value' => $data->mmtid,
        );
      }

      $form['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Save configuration'),
      );
      $form['spacer'] = array(
        '#value' => '&nbsp;&nbsp;&nbsp;&nbsp;',
      );
      $form['reorder_desc'] = array(
        '#type' => 'submit',
        '#value' => t('Reorder Z to A and save'),
      );
    }
    $form['reorder_asc'] = array(
      '#type' => 'submit',
      '#value' => t('Remove custom ordering and save'),
    );
  }
  else if ($count == 1) {
    $form['empty_menu'] = array('#value' => t('There is only one visible @subthing of this @thing, so it cannot be reordered.', mm_ui_strings(FALSE)));
  }
  else {
    $form['empty_menu'] = array('#value' => t('There are no visible @subthings to reorder.', mm_ui_strings(FALSE)));
  }
  return $form;
}

function mm_ui_menu_reorder_validate($form, &$form_state) {
  $form_state['mm']['result'] = array();
  $first = TRUE;
  foreach ($form['#post'] as $key => $entry) {
    // make sure these are integers
    if (is_array($entry) && isset($entry['mmtid']) && intval($entry['mmtid'])) {
      // Make sure there is only one parent for all (meaning, nothing has moved)
      $parent = mm_content_get_parent(intval($entry['mmtid']));
      if ($first) {
        // Speed up mm_content_get_parent() by pre-fetching the rest of the kids
        mm_content_get_tree($parent, array(MM_GET_TREE_DEPTH => 1, MM_GET_TREE_ADD_TO_CACHE => TRUE));
        $first = FALSE;
      }
      if (isset($parent) && (!isset($last_parent) || $parent == $last_parent)) {
        $last_parent = $parent;
        // Don't store if 'Remove custom ordering' was clicked
        if ($form_state['clicked_button']['#id'] != 'edit-reorder-asc') {
          $index = 'mmtid:' . $entry['mmtid'];
          $form_state['mm']['result'][] = array('mmtid' => $entry['mmtid'], 'name' => $form[$index]['title']['#mm_orig_name']);
        }
      }
      else {
        form_set_error('', t('The menu structure seems to have changed while you were editing it. Please try again.'));
        return;
      }
    }
  }

  if (isset($last_parent) && !mm_content_user_can($last_parent, 'w')) {
    form_set_error('', t('You do not have permission to modify this menu.'));
  }
  else {
    $form_state['mm']['parent'] = $last_parent;
  }
}

function mm_ui_menu_reorder_submit($form, &$form_state) {
  if (!function_exists('_mm_ui_menu_reorder_sort_desc')) {
    function _mm_ui_menu_reorder_sort_desc($arr1, $arr2) {
      return strnatcasecmp($arr2['name'], $arr1['name']);
    }
  }

  if (isset($form_state['mm']['parent'])) {
    $parent = $form_state['mm']['parent'];
    // Reset all children, including hidden and recycle bins.
    // If 'Remove custom ordering' was clicked, this is all we need to do.
    mm_content_update_quick(array('weight' => 0), array('parent' => $parent), $parent, FALSE);
    if (is_array($form_state['mm']['result'])) {
      $result = &$form_state['mm']['result'];
      if ($form_state['clicked_button']['#id'] == 'edit-reorder-desc') {
        // Z to A sorting
        usort($result, '_mm_ui_menu_reorder_sort_desc');
      }
      $weight = 1;
      foreach ($result as $array) {
        mm_content_update_quick(array('weight' => $weight++), array('mmtid' => $array['mmtid'], 'parent' => $parent), $parent, FALSE);
      }
    }
    // Make sure the page draws with the new order
    mm_content_clear_caches($parent);
    if ($form_state['clicked_button']['#id'] == 'edit-reorder-asc') {
      drupal_set_message(t('Custom ordering has been removed.'));
    }
    else {
      drupal_set_message(t('The menu has been reordered.'));
    }
  }
}
