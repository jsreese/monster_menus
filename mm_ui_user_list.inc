<?php
// $Id: mm_ui_user_list.inc 4216 2010-06-25 18:36:37Z dan $

/**
 * @file
 * User interface routines for choosing a user from a list
 */

function mm_ui_user_list_form($mmtids) {
  if (!user_access('access user profiles'))
    return '';

  $title = t('User Search');
  drupal_set_title($title);
  $content = drupal_get_form('mm_ui_user_list');
  return "<div class=\"node\">
<h2>$title</h2>
<div class=\"content\"><p>$content</p></div>
</div>";
}

function mm_ui_user_list(&$form_state) {
  $form['#theme'] = 'node_form';
  _mm_ui_userlist_setup(array(0 => ''), $form, 'userlist', t("User's name:"), TRUE, '');
  $form['userlist-choose']['#title'] = '';
  $form['buttons']['submit'] = array('#type' => 'submit',
      '#value' => t('View user'));
  return $form;
}

function mm_ui_user_list_submit($form, &$form_state) {
  $uid = mm_ui_mmlist_key0($form_state['values']['userlist']);

  if (isset($uid) && ($home = mm_content_get(array('f.flag' => 'user_home', 'f.data' => $uid), MM_GET_FLAGS))) {
    mm_redirect_to_mmtid($home[0]->mmtid);
    $form_state['redirect'] = '';
  }
  else {
    form_set_error('userlist-choose', t('The selected user does not have a homepage.'));
  }
}

