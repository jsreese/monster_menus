<?php
// $Id: mm_ui_node_reorder.inc 4216 2010-06-25 18:36:37Z dan $

/**
 * @file
 * User interface routines for reordering nodes on a page
 */

function mm_ui_node_reorder(&$form_state, $mmtid) {
  $where = '';
  $hidden = mm_get_node_info(MM_NODE_INFO_NO_REORDER);
  if ($hidden) {
    $where = "AND n.type NOT IN('" . join("', '", $hidden) . "')";
  }
  $node_sql = mm_content_get_accessible_nodes_by_mmtid_query($mmtid, $count_sql, ', n.type, nr.title', ' LEFT JOIN {node_revisions} nr ON nr.vid = n.vid', $where);
  $count = db_result(db_query($count_sql));

  mm_content_add_breadcrumb(t('Reorder contents'));
  $form = array('#tree' => TRUE);
  if ($count > 1) {
    $form['mmtid'] = array(
      '#type' => 'value',
      '#value' => $mmtid,
    );
    if ($count > MM_UI_MAX_REORDER_ITEMS) {
      $form['empty_menu'] = array('#value' => t('<p>There are too many pieces of content on this @thing to make reordering feasible.</p>', mm_ui_strings(FALSE)));
    }
    else {
      $alt = t('drag handle');
      $clear = drupal_get_path('module', 'monster_menus') . '/images/clear.gif';
      $form['#prefix'] = t('<div id="help"><p>To reorder a piece of content, grab the !image and drag it to a new location. Be sure to <em>Save configuration</em> when done.</p></div>', array('!image' => '<a href="#" class="tabledrag-handle" style="text-decoration: none; cursor: text"><span class="handle">' . theme('image', $clear, '', '', array('width' => 13, 'height' => 13), FALSE) . '</span></a>'));
      $form['#suffix'] = t('<p /><div id="help"><p>If a custom content order is being used, any new contents you add will appear at the top of the list. You may want to return here to change their location, afterward.</p><p>Use the <em>Remove custom ordering and save</em> button to revert to the default, reverse-chronological order.</p></div>');

      $q = mm_query($node_sql);

      $new_weight = 1;
      while ($row = ($q->next())) {
        $id = 'nid:' . $row->nid;
        $form[$id]['#attributes'] = array('class' => 'menu-enabled');
        $title = trim($row->title);
        if (empty($title))
          $title = t('Title not provided');
        $form[$id]['title']['#value'] = check_plain($title);
        $form[$id]['type']['#value'] = node_get_types('name', $row);
        $form[$id]['weight'] = array(
          '#type' => 'weight',
          '#delta' => $count,
          '#default_value' => isset($form_state[$id]['weight']) ? $form_state[$id]['weight'] : $new_weight++,
        );
        $form[$id]['nid'] = array(
          '#type' => 'hidden',
          '#value' => $row->nid,
        );
        if ($row->stuck) $form[$id]['stuck'] = array(
          '#type' => 'value',
          '#value' => TRUE,
        );
      }

      $form['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Save configuration'),
      );
      $form['spacer'] = array(
        '#value' => '&nbsp;&nbsp;&nbsp;&nbsp;',
      );
    }
    $form['reorder_normal'] = array(
      '#type' => 'submit',
      '#value' => t('Remove custom ordering and save'),
    );
  }
  else if ($count == 1) {
    $form['empty_menu'] = array('#value' => t('There is only one piece of content on this @thing, so it cannot be reordered.', mm_ui_strings(FALSE)));
  }
  else {
    $form['empty_menu'] = array('#value' => t('There is no content on this @thing to reorder.', mm_ui_strings(FALSE)));
  }
  return $form;
}

function mm_ui_node_reorder_submit($form, &$form_state) {
  // Reset all children, including hidden and recycle bins.
  // If 'Remove custom ordering' was clicked, this is all we need to do.
  db_query('DELETE FROM {mm_node_reorder} WHERE mmtid = %d', $form_state['values']['mmtid']);
  if ($form_state['clicked_button']['#id'] != 'edit-reorder-normal') {
    $result = &$form_state['mm']['result'];
    $weight = 1;
    foreach ($form['#post'] as $key => $entry) {
      // make sure these are integers
      if (is_array($entry) && isset($entry['nid']) && intval($entry['nid'])) {
        db_query('INSERT INTO {mm_node_reorder} (mmtid, nid, weight) VALUES(%d, %d, %d)', $form_state['values']['mmtid'], $entry['nid'], $weight++);
      }
    }
  }
  // Make sure the page draws with the new order
  mm_content_clear_caches($parent);
  if ($form_state['clicked_button']['#id'] == 'edit-reorder-normal') {
    drupal_set_message(t('Custom ordering has been removed.'));
  }
  else {
    drupal_set_message(t('The contents have been reordered.'));
  }
}
