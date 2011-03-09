<?php
// $Id: mm_ui_solver.inc 4528 2010-09-10 20:28:17Z dan $

/**
 * @file
 * Permissions solver
 */

function mm_ui_solver($mmtid, $uid = NULL) {
  if (empty($uid) && $uid !== '0') {
    mm_static('solver_mm_list_callback', TRUE, $mmtid);
    $title = t('Solve Permissions Issues');
    drupal_set_title($title);
    $content = drupal_get_form('mm_ui_solver_form');
    // This is a dummy call, to make sure the over_text.module inserts its JS
    theme('tooltip', '', '', '');
    return "<div class=\"node\">
<h2>$title</h2>
<div class=\"content\">$content<div id=\"mm-solver-table\"></div></div>
</div>";
  }

  $acct = user_load($uid);
  if ($acct === FALSE) {
    print '<p>Unknown user</p>';
  }
  else {
    print mm_ui_solver_table($mmtid, $acct);
  }
  $GLOBALS['devel_shutdown'] = FALSE;
  exit();
}

function mm_ui_solver_form() {
  $form = array();
  _mm_ui_userlist_setup(array(-1 => ''), $form, 'solver', t("User's name:"), TRUE, '');
  $form['solver-choose']['#title'] = '';
  $form['solver']['#mm_list_other_callback'] = 'Drupal.mmSolverCallback';
  $form['solver']['#description'] = t('Choose a user to see their actual permissions for this page, taking into account the permissions of all parent pages.');

  return $form;
}

/**
 * Show permissions for a tree entry
 *
 * @param $mmtid
 *   MM tree ID of entry to query
 * @return
 *   The HTML code for the results
 */
function mm_ui_solver_table($mmtid, $acct) {
  $list = mm_content_get_parents_with_self($mmtid, FALSE, FALSE);

  $user_access = array();
  $is_admin = $acct->uid == 1;
  if (!$is_admin) {
    foreach (array('administer all menus', 'administer all users', 'administer all groups', 'view all menus') as $access_mode)
      if ($acct->uid || substr($access_mode, 0, 4) == 'view')
        $user_access[$access_mode] = user_access($access_mode, $acct);
  }
  $is_admin |= $user_access['administer all menus'];

  $is_user = $is_group = $is_recycled = FALSE;
  $can_read = $can_recycle = TRUE;

  $header = array(t('Page'), t('Edit'), t('Add subpages'), t('Add content'), t('Read'));
  $rows = array();
  $all_modes = array('w' => 1, 'a' => 1, 'u' => 1, 'r' => 1);
  $everyone = array(
    'w' => t('everyone can edit'),
    'a' => t('everyone can add subpages'),
    'u' => t('everyone can add content'),
    'r' => t('everyone can read')
  );
  $readable = array(
    'w' => t('can edit this page'),
    'a' => t('can add subpages to this page'),
    'u' => t('can add content to this page'),
  );
  $yes = theme('mm_ui_mark_yesno', TRUE);
  $no = theme('mm_ui_mark_yesno', FALSE);

  foreach ($list as $entry) {
    $modes = array();
    $reasons = array('w' => array(), 'a' => array(), 'u' => array(), 'r' => array());
    $item = mm_content_get($entry);

    if ($entry == mm_content_users_mmtid()) $is_user = TRUE;
    else if ($entry == mm_content_groups_mmtid()) $is_group = TRUE;

    if ($item->name == MM_ENTRY_NAME_RECYCLE) $is_recycled = TRUE;

    $name = mm_content_expand_name($item->name);
    if ($entry != 1) {
      $url = mm_content_user_can($entry, 'w') ? "mm/$entry" : "mm/$entry/settings";
      $name = l($name, $url);
    }
    $row = array($name);

    if ($is_admin || ($is_user && $user_access['administer all users'] || $is_group && $user_access['administer all groups'])) {
      $modes = $all_modes;
      $reasons['w'][] = $reasons['a'][] = $reasons['u'][] = $reasons['r'][] = t('is an administrator');
    }

    if (!$is_admin && $user_access['view all menus']) {
      $modes['r'] = 1;
      $reasons['r'][] = t('can view all pages');
    }

    if (!$can_read) {
      $reasons['w'][] = $reasons['a'][] = $reasons['u'][] = $reasons['r'][] = t('cannot read parent');
    }
    else {
      if ($item->uid == $acct->uid) {
        $modes = $all_modes;
        $reasons['w'][] = $reasons['a'][] = $reasons['u'][] = $reasons['r'][] = t('is the owner');
      }

      foreach ($everyone as $mode => $reason)
        if (strpos($item->default_mode, $mode) !== FALSE) {
          $modes[$mode] = 1;
          $reasons[$mode][] = $reason;
        }

      $result = db_query(
        'SELECT a.mode, a.gid, t.name FROM {mm_tree_access} a ' .
          'INNER JOIN {mm_group} g ON g.gid = a.gid ' .
          'LEFT JOIN {mm_tree} t ON t.mmtid = g.gid ' .
          'LEFT JOIN {mm_virtual_group} v ON v.vgid = g.vgid ' .
        'WHERE a.mmtid = %d AND (v.uid = %d OR g.vgid = 0 AND g.uid = %d)',
        $entry, $acct->uid, $acct->uid);
      while ($mode_obj = db_fetch_object($result)) {
        $modes[$mode_obj->mode] = 1;
        $reasons[$mode_obj->mode][] = $mode_obj->gid < 0 ? t('is a listed user') : t('is in the group %name', array('%name' => mm_content_expand_name($mode_obj->name)));
      }

      if ($item->name == MM_ENTRY_NAME_RECYCLE) {
        $can_recycle = $can_recycle && mm_content_user_can_recycle($entry, 'r', $acct);
        if ($can_recycle) {
          $reasons['r'][] = t('can edit something in bin');
          $modes['r'] = 1;
        }
        else {
          $reasons['r'][] = t('cannot edit anything in bin');
        }
      }
      else {
        if (isset($modes['w'])) {
          $modes['a'] = $modes['u'] = $modes['r'] = 1;
          $reasons['a'][] = $reasons['u'][] = $reasons['r'][] = $readable['w'];
        }
        else {
          foreach (array('a', 'u') as $mode) {
            if (isset($modes[$mode])) {
              $modes['r'] = 1;
              $reasons['r'][] = $readable[$mode];
            }
          }
        }

        if ($is_recycled) {
          if (!isset($modes['r']) || !$can_recycle) {
            $reasons['r'][] = t('cannot edit anything in bin');
          }
        }
      }
    }

    foreach (array_keys($all_modes) as $mode) {
      if (!$reasons[$mode]) $reasons[$mode][] = t('not permitted');
      $tip = '<ul><li>' . join('</li><li>', $reasons[$mode]) . '</li></ul>';
//      $row[] = (isset($modes[$mode]) ? $yes : $no) . '<br />' . $tip;
      $row[] = theme('tooltip', isset($modes[$mode]) ? $yes : $no, t('reason(s)'), $tip, TRUE);
    }

    $rows[] = $row;
    $can_read = isset($modes['r']);
  }

  return theme('table', $header, $rows, array('id' => 'mm-solver')) . '<div class="description">' . t('Put the mouse over any !yes or !no symbol above to see the reason(s).<br /><b>Note:</b> The table above does not take into account any unsaved changes to settings.', array('!yes' => $yes, '!no' => $no)) . '</div>';
}
