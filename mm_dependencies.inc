<?php
// $Id: mm_dependencies.inc 4965 2011-01-26 19:22:36Z dan $

function mm_dep_add_libs($lib_type) {
  $dep_path = drupal_get_path('module', 'monster_menus') . '/dependencies/';
  switch ($lib_type) {
    case 'jsTree':
      // used in monster_menus/mm_browser.inc
      drupal_add_js($dep_path . 'jsTree/jquery.jstree.js');
      drupal_add_css($dep_path . 'jsTree/source/tree_component.css');
      return;

    case 'splitter':
      // used in monster_menus/mm_browser.inc
      drupal_add_js(JQUERY_UI_PATH . '/external/jquery.cookie.js');
      drupal_add_js($dep_path . 'splitter.js');
      return;

    case 'fg.menu':
      // used in monster_menus/mm_browser.inc
      drupal_add_js($dep_path . 'fg.menu.js');
      drupal_add_css($dep_path . 'fg.menu.css');
      jquery_ui_add('ui.core');
      return;

    case 'dataTables':
      // used in academics/{acad_library.inc, acad_lib_eresv.inc, academics.module}
      drupal_add_js($dep_path . 'jquery.dataTables.min.js');
      return;

    case 'php-captcha':
      // used in monster_menus/misc.inc
      $captcha = $dep_path . 'php-captcha';
      require_once("$captcha/php-captcha.inc.php");
      $fonts = array("$captcha/fonts/VeraBd.ttf", "$captcha/fonts/VeraMono.ttf", "$captcha/fonts/VeraIt.ttf");
      $images = array("$captcha/images/1.jpg", "$captcha/images/2.jpg", "$captcha/images/3.jpg");
      return array($fonts, $images);

    case 'xregexp':
      // used in monster_menus/mm_static.inc
      drupal_add_js($dep_path . 'xregexp.js');
      return;
  }
  die("mm_dep_add_libs() called with unknown parameter '$lib_type'");
}

function mm_dep_add_js_dep_path() {
  drupal_add_js(array('dependencyPath' => drupal_get_path('module', 'monster_menus') . '/dependencies'), 'setting');
}
