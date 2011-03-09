<?php
//$Id: mm_schedule.views.inc 3830 2010-02-04 21:40:39Z dan $

/**
 * @file
 */

/**
 * Implementation of hook_views_plugins (called from mm_schedule.module)
 */
function _mm_schedule_views_plugins() {
  $views_path = drupal_get_path('module', 'views');
  $sched_path = drupal_get_path('module', 'mm_schedule');
  $data = array(
    'module' => 'calendar_ical',
    'style' => array(
      // Override class calendar_plugin_style_ical in
      // calendar/calendar_ical/calendar_plugin_style_ical.inc because it cannot
      // handle relationships
      'mm_ical' => array(
        'title' => t('iCal feed (MM)'),
        'help' => t('Generates an iCal feed from a view.'),
        'handler' => 'mm_schedule_calendar_plugin_style_ical',
        'path' => $sched_path,
        'theme' => 'calendar_view_ical',
        'theme file' => 'theme.inc',
        'theme path' => drupal_get_path('module', 'calendar_ical'),
        'parent' => 'ical',
        'uses fields' => TRUE,
        'uses row plugin' => FALSE,
        'uses options' => TRUE,
        'type' => 'normal',
        'even empty' => TRUE,
      ),
      // Define a style class for <dl> lists
      'mm_schedule_list' => array(
        'title' => t('Date List (MM)'),
        'help' => t('Generates a list of dates from a view.'),
        'handler' => 'calendar_plugin_style',
        'path' => drupal_get_path('module', 'calendar') . '/includes',
        'theme' => 'mm_schedule_list',
        'theme file' => 'mm_schedule_theme.inc',
        'theme path' => $sched_path,
        'parent' => 'ical',
        'uses fields' => TRUE,
        'uses row plugin' => FALSE,
        'uses options' => FALSE,
        'type' => 'normal',
        'even empty' => TRUE,
      ),
    ),
  );
  return $data;
}