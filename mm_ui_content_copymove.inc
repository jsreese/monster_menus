<?php
// $Id: mm_ui_content_copymove.inc 4953 2011-01-24 21:55:03Z dan $

/**
 * @file
 * User interface routines for copying/moving MM content
 */

/**
 * Display the form for copying/moving a tree entry
 */
function mm_ui_content_copymove(&$form_state, $item) {
  $x = mm_ui_strings($is_group = $item->is_group);

  if ($item->parent > 0) {
    $pitem = mm_content_get_tree($item->parent, array(
        MM_GET_TREE_DEPTH        => 0,
        MM_GET_TREE_RETURN_PERMS => TRUE,
      )
    );
  }

  $form['#attributes'] = array('class' => 'mm-ui-copymove');
  $form['mmtid'] = array(
    '#type' => 'value',
    '#value' => $item->mmtid
  );

  $form['mode'] = array(
    '#type' => 'fieldset',
    '#title' => t('Mode'),
    '#collapsible' => FALSE);
  $form['mode']['mode'] = array(
    '#type' => 'radios',
    '#default_value' => 'copy',
    '#options' => array('copy' => t('Copy'), 'move' => t('Move')));
  if ($item->perms['IS_RECYCLED']) {
    unset($form['mode']['mode']['#options']['move']);
    $form['mode']['mode']['#description'] = t('This @thing is in the recycle bin. You can copy it, but it cannot be moved.', $x);
  }

  $form['copy'] = array(
    '#type' => 'fieldset',
    '#attributes' => array('id' => 'copydiv'),
    '#title' => t('What to copy'),
    '#collapsible' => FALSE);
  $mustcheck = $is_group ? '' : "alert('" . t('You must choose "This @thing", "Contents", or both.', $x) . "');";
  $form['copy']['copy_page'] = array(
    '#type' => 'checkbox',
    '#default_value' => TRUE,
    '#title' => t('This @thing', $x));
  $form['copy']['copy_subpage'] = array(
    '#type' => 'checkbox',
    '#default_value' => FALSE,
    '#attributes' => array('style' => 'margin-left: 20px'),
    '#title' => t('and any @subthings', $x));
  if (!$is_group) {
    $form['copy']['copy_nodes'] = array(
      '#type' => 'checkbox',
      '#default_value' => FALSE,
      '#title' => t('Contents'));
    $form['copy']['desc'] = array(
      '#type' => 'item',
      '#description' => t('When contents are copied without their current @thing, the new copies take on the permissions of the destination @thing.', $x));
  }

  $form['move'] = array(
    '#type' => 'fieldset',
    '#attributes' => array('id' => 'movediv'),
    '#title' => t('What to move'),
    '#collapsible' => FALSE);
  $form['move']['move_mode'] = array(
    '#type' => 'radios',
    '#default_value' => 'page',
    '#description' => $is_group ? NULL : t('Moved contents keep their original permissions.'),
    '#options' => array(
      'page' => t($is_group ? 'This @thing and any @subthings' : 'This @thing, any @subthings, and all their contents', $x),
      'nodes' => t('Just the contents appearing on this @thing', $x)));

  $limit_move = isset($item->flags['limit_move']) && !user_access('administer all menus');
  if ($is_group) {
    unset($form['move']['move_mode']['#options']['nodes']);
    if ($limit_move) {
      unset($form['mode']['mode']['#options']['move']);
      $form['mode']['mode']['#description'] = t('You are not allowed to move this group.') . ' ' . $form['mode']['mode']['#description'];
    }
    else if (mm_content_is_vgroup($item->mmtid)) {
      $form['mode']['mode']['#default_value'] = 'move';
      unset($form['mode']['mode']['#options']['copy']);
    }
  }
  else if ($limit_move) {
    $form['move']['move_mode']['#default_value'] = 'nodes';
    unset($form['move']['move_mode']['#options']['page']);
    $form['move']['move_mode']['#description'] = t('You are not allowed to move this page, only its contents.') . ' ' . $form['move']['move_mode']['#description'];
  }

  $form['dest'] = array(
    '#type' => 'fieldset',
    '#title' => t('Destination'),
    '#collapsible' => FALSE);

  $parents = mm_content_get_parents($item->mmtid);
  if (!user_access('administer all menus')) array_shift($parents);  // skip root
  $pop_start = implode('/', $parents) . "/$item->mmtid";

  $form['dest']['dest'] = array(
    '#title' => t('Destination'),
    '#type' => $is_group ? 'mm_grouplist' : 'mm_catlist',
    '#required' => $item->parent <= 0,
    '#default_value' => $pitem ?
      array($pitem[0]->mmtid => mm_content_expand_name($pitem[0]->name)) :
      array('' => t('(choose the destination)')),
    '#mm_list_popup_start' => $pop_start,
    '#mm_list_min' => 1,
    '#mm_list_max' => 1,
    '#mm_list_selectable' => 'au',
    '#description' => t('Choose where to copy/move the @thing. The default value is the @thingpos current parent.',
        $x)
  );

  $form['name_alias'] = array(
    '#type' => 'fieldset',
    '#attributes' => array('id' => 'namediv'),
    '#title' => $is_group ? t('Name') : t('Name and URL name'),
    '#collapsible' => FALSE);
  $form['name_alias']['name'] = array(
    '#type' => 'mm_fake_required',
    '#mm_orig_type' => 'textfield',
    '#title' => t('New @thing name', $x),
    '#default_value' => $item->name,
    '#size' => 40, '#maxlength' => 128,
    '#description' => t('If you do not change the Destination above, you must modify the name here.')
  );

  if (!$is_group) {
    $form['name_alias']['alias'] = array(
      '#type' => $item->is_user_home || user_access('administer all menus') ? 'textfield' : 'mm_fake_required',
      '#mm_orig_type' => 'textfield',
      '#title' => t('New URL name'),
      '#default_value' => $item->alias,
      '#size' => 20, '#maxlength' => 128,
      '#description' => t('A name that will be used in URLs. '.
        'Try to limit this to something short, using only the lowercase characters '.
        '<code>a-z</code>, <code>0-9</code>, dot, dash, and underscore.')
    );
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Go!'),
  );
  mm_static('copymove', TRUE, $mustcheck);

  return $form;
}

/**
 * Validate the form for copying/moving a tree entry
 */
function mm_ui_content_copymove_validate($form, &$form_state, $restore = FALSE) {
  _mm_ui_content_copymove_get_values($form_state['values'], $src_mmtid, $modes, $src_parent_mmtid, $dest_mmtid);
  $nodes_only = $modes['move_nodes'] || $modes['copy_nodes'] && !$modes['copy_page'];

  if ($modes['copy_page'] || $modes['move_page']) {
    mm_ui_validate_fake_required($form['name_alias']['name'], trim($form_state['values']['name']));
    mm_ui_validate_fake_required($form['name_alias']['alias'], trim($form_state['values']['alias']));
  }

  if (!$src_mmtid || !is_numeric($src_mmtid)) {
    form_set_error('', t('Missing one or more required fields'));
    return;
  }

  $x = mm_ui_strings(mm_content_is_group($src_mmtid));

  if ($modes['move_page'] && !mm_content_user_can($src_mmtid, 'w'))
    $message = t('You do not have permission to modify this @thing.', $x);

  if ($modes['move_page'] && !$restore && !mm_content_user_can($src_parent_mmtid, 'a'))
    $message = t('You do not have permission to modify the source @thingpos parent.', $x);

  if (!is_numeric($dest_mmtid) || !mm_content_user_can($dest_mmtid, $nodes_only ? 'u' : 'a'))
    $message = t('You do not have permission to modify the destination @thing.', $x);

  if (mm_content_is_normal($src_mmtid) != mm_content_is_normal($dest_mmtid) || mm_content_is_group($src_mmtid) != mm_content_is_group($dest_mmtid))
      $message = t('Source and destination are not of the same type.');

  if (mm_content_user_can($dest_mmtid, 'IS_RECYCLED'))
    if ($modes['copy_nodes'] || $modes['copy_page']) {
      $message = 'You cannot copy to a recycle bin.';
    }
    else {
      $message = 'You cannot move to a recycle bin using this option. Please use the Delete option, instead.';
    }

  if ($message) {
    watchdog('mm', $message, array(), WATCHDOG_WARNING, "source: $src_mmtid, dest: $dest_mmtid");
    form_set_error('', t($message));
    return;
  }

  if (mm_content_is_vgroup($src_mmtid) != mm_content_is_vgroup($dest_mmtid)) {
    form_set_error('dest', mm_content_is_vgroup($src_mmtid) ? t('Pre-defined groups can only be moved to create sub-groups of other pre-defined groups.') : t('You cannot copy/move a regular group inside the pre-defined groups list.'));
  }

  if (!$nodes_only && !_mm_ui_validate_entry($src_mmtid, $dest_mmtid, $form_state['values'], TRUE, TRUE)) {
    return;
  }

  if ($dest_mmtid == $src_parent_mmtid && $modes['move_page']) {
    form_set_error('', t('Instead of moving a @thing within the same parent @thing, rename it using Settings-&gt;Edit.', $x));
  }
  else if ($dest_mmtid == $src_mmtid && $nodes_only && $modes['move_nodes']) {
    form_set_error('', t('Moving just a @thingpos contents to itself results in no change.', $x));
  }
  else if ($dest_mmtid == $src_mmtid && !$modes['move_nodes'] && ($modes['move_page'] || $modes['copy_recur'])) {
    form_set_error('', t('You cannot copy or move a @thing within itself.', $x));
  }
  else if (mm_content_is_child($dest_mmtid, $src_mmtid)) {
    if ($modes['move_page']) {
      form_set_error('dest', t('You cannot move a @thing into a @subthing of itself. Please choose another destination.', $x));
    }
    else if ($modes['copy_recur']) {
      form_set_error('dest', t('You cannot copy a @thing and any @subthings into a child of itself. Please choose another destination.', $x));
    }
  }
  else if ($modes['move_nodes'] && mm_content_is_archive($dest_mmtid)) {
    form_set_error('dest', t('You cannot move content into an archive. Move the content into the main @thing, instead, and the archive will be updated automatically.', $x));
  }
}

/**
 * Process the form for copying/moving a tree entry
 */
function mm_ui_content_copymove_submit($form, &$form_state, $restore = FALSE) {
  global $user;

  _mm_ui_content_copymove_get_values($form_state['values'], $src_mmtid, $modes, $src_parent_mmtid, $dest_mmtid);

  $name = trim($form_state['values']['name']);
  $alias = trim($form_state['values']['alias']);
  $x = mm_ui_strings(mm_content_is_group($src_mmtid));

  if ($modes['copy_page'] || $modes['copy_nodes']) {
    if ($modes['copy_page'] && $modes['copy_nodes']) {
      $msg = $modes['copy_recur'] ?
        t('The @thing, any @subthings, and their contents were successfully copied.', $x) :
        t('The @thing and its contents were successfully copied.', $x);
    }
    else if ($modes['copy_nodes']) {
      $msg = t('The @thingpos contents were successfully copied.', $x);
    }
    else {
      $msg = $modes['copy_recur'] ?
        t('The @thing and any @subthings were successfully copied.', $x) :
        t('The @thing was successfully copied.', $x);
    }

    $copy_params = array(
      MM_COPY_ALIAS =>    $alias,
      MM_COPY_CONTENTS => $modes['copy_nodes'],
      MM_COPY_NAME =>     $name,
      MM_COPY_OWNER =>    $user->uid,
      MM_COPY_READABLE => TRUE,
      MM_COPY_RECUR =>    $modes['copy_recur'],
      MM_COPY_TREE =>     $modes['copy_page'],
    );
    $new_mmtid = mm_content_copy($src_mmtid, $dest_mmtid, $copy_params);

    if (is_numeric($new_mmtid)) {
      if ($new_mmtid != $src_mmtid) $msg .= ' ' . t('You are now viewing the destination.');
      drupal_set_message($msg);
      $form_state['redirect'] = mm_content_get_mmtid_url($new_mmtid);
    }
    else {
      form_set_error('', $new_mmtid);
    }
  }
  else if ($modes['move_page']) {
    $old_tree = mm_content_get($src_mmtid);
    $error = mm_content_move($src_mmtid, $dest_mmtid, $restore ? 'restore' : '');

    if (!$error) {
      if ($name != $old_tree->name || $alias != $old_tree->alias) {
        mm_content_update_quick(array('name' => $name, 'alias' => $alias), array('mmtid' => $src_mmtid), $dest_mmtid);
      }
      drupal_set_message($restore ?
        t('The @thing has been restored.', $x) :
        t('The @thing was successfully moved.', $x));
      $form_state['redirect'] = mm_content_get_mmtid_url($src_mmtid);
    }
    else {
      form_set_error('', $error);
    }
  }
  else if ($modes['move_nodes']) {
    $ok = $total = 0;
    foreach (mm_content_get_nids_by_mmtid($src_mmtid) as $nid) {
      $total++;
      $uid = db_result(db_query('SELECT uid FROM {node} WHERE nid = %d', $nid));
      if (!empty($uid) || $uid === 0) {
        $node = (object)array('nid' => $nid, 'uid' => $uid);
        if (mm_content_node_access('update', $node)) {
          db_query('UPDATE {mm_node2tree} SET mmtid = %d WHERE mmtid = %d AND nid = %d',
              $dest_mmtid, $src_mmtid, $nid);
          db_query('DELETE FROM {mm_node_reorder} WHERE nid = %d AND mmtid = %d',
              $nid, $dest_mmtid);
          $ok++;
        }
      }
    }

    $failed = $total - $ok;
    $x += array('@total' => $total, '@ok' => $ok, '@failed' => $failed);
    if ($failed) {
      if ($failed == $total) {
        drupal_set_message(t('You do not have permission to move any of the @total piece(s) of content on the @thing.', $x));
        $form_state['redirect'] = mm_content_get_mmtid_url($src_mmtid);
      }
      else {
        drupal_set_message(t('Only @ok of the @total piece(s) of content on the @thing could be moved, due to lack of permission.', $x));
        $form_state['redirect'] = mm_content_get_mmtid_url($dest_mmtid);
      }
    }
    else if (!$total) {
      drupal_set_message(t('There are no contents on this @thing to move.', $x));
      $form_state['redirect'] = mm_content_get_mmtid_url($src_mmtid);
    }
    else {
      drupal_set_message(t('@total piece(s) of content were successfully moved.', $x));
      $form_state['redirect'] = mm_content_get_mmtid_url($dest_mmtid);
    }
  }
}

function _mm_ui_content_copymove_get_values($form_values, &$src_mmtid, &$modes, &$src_parent_mmtid, &$dest_mmtid) {
  $src_mmtid =      $form_values['mmtid'];
  $copy_page =      $form_values['mode'] == 'copy' && $form_values['copy_page'];
  $modes = array(
    'copy_page' =>  $copy_page,
    'copy_recur' => $copy_page && $form_values['copy_subpage'],
    'copy_nodes' => $form_values['mode'] == 'copy' && $form_values['copy_nodes'],
    'move_page' =>  $form_values['mode'] == 'move' && $form_values['move_mode'] == 'page',
    'move_nodes' => $form_values['mode'] == 'move' && $form_values['move_mode'] == 'nodes'
  );
  $src_parent_mmtid = mm_content_get_parent($src_mmtid);
  $dest_mmtid = mm_ui_mmlist_key0($form_values['dest']);
}
