<?php
// $Id: mm_schedule_theme.inc 4225 2010-07-02 18:35:51Z dan $

/**
 * @file
 * Schedule/Calendar theme functions
 */

function theme_mm_schedule_date_combo_all_day($element) {
  return theme_date_combo($element);
}

function theme_calendar_year__mm_calendar($view, $options) {
  _mm_schedule_theme_setup_events($view);
  _mm_schedule_theme_setup_events_external($view);

  return theme('calendar_year', $view);
}

function theme_calendar_month__mm_calendar($view, $options) {
  _mm_schedule_theme_setup_events($view);
  _mm_schedule_theme_setup_events_external($view);

  return _mm_schedule_theme_fix_popups($view, theme('calendar_month', $view));
}

function theme_calendar_week__mm_calendar($view, $options) {
  list($fields, $time_inc, $wd_start, $wd_end, $min_time, $max_time) = _mm_schedule_theme_setup_events($view);
  return _mm_schedule_theme_fix_popups($view, _mm_schedule_theme_output_day_or_week($view, $fields, $time_inc, $wd_start, $wd_end, $min_time, $max_time));
}

function theme_calendar_day__mm_calendar($view, $options) {
  include_once(drupal_get_path('module', 'academics') . '/academics_schedule.inc');
  list($fields, $time_inc, $wd_start, $wd_end, $min_time, $max_time) = _mm_schedule_theme_setup_events($view);
  return _mm_schedule_theme_fix_popups($view, _mm_schedule_theme_output_day_or_week($view, $fields, $time_inc, $wd_start, $wd_end, $min_time, $max_time, TRUE));
}

function theme_calendar_view_ical__mm_calendar($view, $options) {
  _mm_schedule_theme_setup_events($view, TRUE);
  _mm_schedule_theme_setup_events_external($view, TRUE);

  return theme('calendar_view_ical', $view, $options, $view->items);
}

function theme_mm_schedule_list($view) {
  include_once(drupal_get_path('module', 'academics') . '/academics_schedule.inc');
  list($fields, $time_inc, $wd_start, $wd_end, $min_time, $max_time) = _mm_schedule_theme_setup_events($view);
  return _mm_schedule_theme_fix_popups($view, _mm_schedule_theme_output_list($view, $fields));
}

/**
 * This function overrides template_date_nav_title() in date/theme/theme.inc
 */
function phptemplate_date_nav_title($granularity, $view, $link = FALSE, $format = NULL) {
  if ($view->date_info->granularity == 'year' && $granularity == 'year' && $view->mm_first_month > 1) {
    return substr($view->date_info->min_date_date, 0, 4) . ' - ' . substr($view->date_info->max_date_date, 0, 4);
  }

  return theme_date_nav_title($granularity, $view, $link, $format);
}

/**
 * This function overrides template_preprocess_calendar_year() in
 * calendar/theme/theme.inc
 */
function phptemplate_preprocess_calendar_year(&$vars) {
  // Add in all the $vars added by the main calendar preprocessor.
  $vars['view']->date_info->style_with_weekno = FALSE;
  template_preprocess_calendar($vars);

  // Get the url of the year view and remove the year argument from it.
  // TODO clean this up in case there is another arg that looks like
  // the year to make sure only the year gets removed.
  $view = $vars['view'];
  $year = date_format($view->date_info->min_date, 'Y');

  // Construct a calendar for each month, adjusting the $view passed
  // to the theme so it will produce the right results.
  $view = drupal_clone($vars['view']);
  $rows = $vars['rows'];
  $months = array();
  foreach ($rows as $month => $month_rows) {
    $view->date_info->month = $month;
    $view->date_info->granularity = 'month';
    $view->date_info->mini = TRUE;
    $view->date_info->hide_nav = TRUE;
    $view->date_info->show_title = TRUE;
    $view->date_info->url = date_real_url($view, NULL, date_pad($year, 4) .'-'. date_pad($month));
    $view->date_info->min_date = date_make_date($view->date_info->year .'-'. date_pad($month) .'-01 00:00:00', date_default_timezone_name());
    if ($view->mm_first_month > 1) {
      $offset = '+' . ($view->mm_first_month - 1) . ' month';
      date_modify($view->date_info->min_date, $offset);
    }
    $view->date_info->max_date = drupal_clone($view->date_info->min_date);
    date_modify($view->date_info->max_date, '+1 month');
    date_modify($view->date_info->max_date, '-1 second');
    $months[$month] = theme('calendar_mini', $view, $vars['options'], $month_rows);
  }
  $vars['months'] = $months;
  $vars['view']->date_info->hide_nav = FALSE;
  $vars['view']->date_info->granularity = 'year';
}

function _mm_schedule_theme_fix_popups($view, $output) {
  if (!function_exists('_mm_schedule_theme_preg_replace_callback')) {
    function _mm_schedule_theme_preg_replace_callback($matches) {
      return $matches[1] . 'view-nonnode' . $matches[2] .'">' . _academics_schedule_thickbox_link($matches[5], 'mm_schedule/view_event/' . $matches[3] . '/' . $matches[4], t('Course Details'));
    }
  }

  include_once(drupal_get_path('module', 'academics') . '/academics_schedule.inc');

  $output = preg_replace_callback('{(class=")view-field(.*?)">\s*<a href=".*?/node/node_(\w+-\d+-\d+-\d{4}[FS])_([A-Z]+)_\d+?.*?">(.*?)</a>}i', '_mm_schedule_theme_preg_replace_callback', $output);
  if (!empty($view->mm_came_from)) {
    // This is a hack to append the destination onto the URL, so that the user
    // is returned to the same view of the calendar they started at, after
    // clicking on the Edit or Delete link.
    return preg_replace('{(id="calendar:.*?)"}', '\1:' . $view->mm_came_from . '"', $output);
  }

  return $output;
}

function _mm_schedule_theme_setup_events($view, $ical = FALSE) {
  global $user;
  $fields = array(
    'start' => array(
      $view->field['field_start_datetime_value']->field_alias,
      $view->field['field_start_datetime_value_1']->field_alias,
      preg_replace('/^.*?__/', '', $view->field['field_start_datetime_value']->field_alias),
    ),
    'end' => array(
      $view->field['field_start_datetime_value']->field_alias . '2',
      $view->field['field_start_datetime_value_1']->field_alias . '2',
      preg_replace('/^.*?__/', '', $view->field['field_start_datetime_value']->field_alias) . '2',
    ),
    'title' => array(
      $view->field['title']->field_alias,
      $view->field['title_1']->field_alias,
      'title',
    ),
    'body' => array(
      $view->field['body']->field_alias,
      $view->field['body_1']->field_alias,
      'node_revisions_body',
    ),
    'delta' => array(
      $view->field['field_start_datetime_value']->aliases['delta'],
      $view->field['field_start_datetime_value_1']->aliases['delta'],
      preg_replace('/^.*?__/', '', $view->field['field_start_datetime_value']->aliases['delta']),
    ),
    'nid' => array(
      $view->field['field_start_datetime_value']->aliases['nid'],
      $view->field['field_start_datetime_value_1']->aliases['nid'],
      'nid',
    ),
    'vid' => array(
      $view->field['vid']->field_alias,
      $view->field['vid_1']->field_alias,
      'node_vid',
    ),
  );
  $node     = $view->mm_node;
  $time_inc = empty($node->field_wd_increment[0]['value']) ? 30 : $node->field_wd_increment[0]['value'];
  $wd_start = empty($node->field_wd_start[0]['value']) ? 9 * 60 : $node->field_wd_start[0]['value'];
  $wd_end   = empty($node->field_wd_end[0]['value']) ? 17 * 60 : $node->field_wd_end[0]['value'];
  $min_time = _mm_schedule_date_timestamp_get($view->date_info->min_date);
  $max_time = _mm_schedule_date_timestamp_get($view->date_info->max_date) - 59;

  if ($node->mycalendar) {
    $week_day = array_flip(array_keys(academics_day_list()));

    $start_date = date_format($view->date_info->min_date, DATE_FORMAT_DATE);
    $start_date_array = explode('-', $start_date);

    $end_date = date_format($view->date_info->max_date, DATE_FORMAT_DATE);
    $end_date_array = explode('-', $end_date);

    $day_of_week_first = date('w', strtotime($start_date));

    $courses = db_query(
      'SELECT * FROM (' .
        'SELECT subjectid, course_num, d.sectionid, d.instr_method, s.title, t.start_date AS term_start, t.end_date AS term_end, start_time, end_time, weekday ' .
        'FROM {univ_term} t ' .
          'INNER JOIN {univ_section} s ON t.termid = s.termid ' .
          'INNER JOIN {univ_section_enrollment} e ON e.sectionid = s.sectionid ' .
          'INNER JOIN {univ_section_detail} d ON s.sectionid = d.sectionid ' .
        "WHERE t.start_date <= '%s' AND t.end_date >= '%s' AND e.idnumber = '%s' " .
      ') AS y ' .
      'GROUP BY subjectid, course_num, instr_method, weekday',
      $end_date, $start_date, $user->idnumber);

    $fake_nid = 1;
    while ($course = db_fetch_object($courses)) {
      $first_class = $start_date_array[2] + $week_day[$course->weekday] - $day_of_week_first;
      if ($first_class <= 0) $first_class = $first_class + 7;

      $month = (int)$start_date_array[1];
      $year = $start_date_array[0];
      $last_day_of_month = date('t', strtotime(sprintf('%d-%02d-01', $year, $month)));

      if ($first_class > $last_day_of_month) {
        $first_class -= $last_day_of_month;
        if (++$month == 13) {
          $month = 1;
          $year++;
        }
      }

      $temp_date = sprintf('%d-%02d-%02d', $year, $month, $first_class);
      for ($i = $first_class; $temp_date <= $end_date; $i += 7) {
        // Handle times when the events go outside of a month (week and year view)
        $last_day_of_month = date('t', strtotime(sprintf('%d-%02d-01', $year, $month)));
        if ($i > $last_day_of_month) {
          $i -= $last_day_of_month;
          if (++$month == 13) {
            $month = 1;
            $year++;
          }
        }

        $temp_date = sprintf('%d-%02d-%02d', $year, $month, $i);
        if ($temp_date >= $course->term_start && $temp_date <= $course->term_end) {
          $start = date('Y-m-d H:i:s', strtotime($temp_date . ' ' . $course->start_time));
          $obj = date_convert($start, DATE_DATETIME, DATE_OBJECT, date_default_timezone_name()); date_timezone_set($obj, timezone_open('UTC'));
          $start = date_format($obj, DATE_FORMAT_DATETIME);

          $end = date('Y-m-d H:i:s', strtotime($temp_date . ' ' . $course->end_time));
          $obj = date_convert($end, DATE_DATETIME, DATE_OBJECT, date_default_timezone_name()); date_timezone_set($obj, timezone_open('UTC'));
          $end = date_format($obj, DATE_FORMAT_DATETIME);

          $nid_str = 'node_' . $course->sectionid . '_' . str_replace('/', MM_SCHEDULE_INSTRUCTION_SLASH, $course->instr_method) . '_' . $fake_nid;
          $title = $course->title . ' (' . $course->instr_method . ')';
          $view->result[] = (object)array(
            'nid' => $node->nid,
            'node_type' => 'mm_calendar',
            'node_mm_node2tree_nid' => $nid_str,
            'node_mm_node2tree_type' => 'mm_event',
            $fields['nid'][0] => $nid_str,
            $fields['delta'][0] => 0,
            $fields['start'][0] => $start,
            $fields['start'][1] => $start,
            $fields['end'][0] => $end,
            $fields['end'][1] => $end,
            $fields['title'][0] => $title,
            $fields['title'][1] => $title,
          );
          $fake_nid++;
        }
      }
    }
  }

  // Adjust overall start/end and remove redundant results
  $used = array();
  $to_zone = date_default_timezone();

  foreach ($view->result as $num => $result) {
    $skip = FALSE;
    $field = $result->node_mm_node2tree_type == 'mm_event' ? 0 : 1;
    if (empty($fields['nid'][$field]) || empty($fields['delta'][$field])) {
      // content_permissions has disallowed field_start_datetime
      $skip = TRUE;
    }
    else {
      $nid   = $result->{$fields['nid'][$field]};
      $delta = $result->{$fields['delta'][$field]};

      $start_field = $fields['start'][$field];
      $end_field   = $fields['end'][$field];

      $start_time = strtotime($result->$start_field . ' UTC');
      $end_time   = strtotime($result->$end_field . ' UTC');

      if ($no_start_adj = $start_time < $min_time) $start_time = $min_time;
      if ($no_end_adj   = $end_time > $max_time)   $end_time   = $max_time;

      if ($start_time > $max_time || $end_time < $min_time) {
        $skip = TRUE;
      }
      else if ($ical) {
        // for iCal output, only keep the earliest occurrence in the series
        if (isset($used[$nid])) {
          if ($start_time < $used[$nid][0]) {
            unset($view->result[$used[$nid][1]]);
            $used[$nid] = array($start_time, $num);
          }
          else {
            $skip = TRUE;
          }
        }
        else {
          $used[$nid] = array($start_time, $num);
        }
      }
      else {
        if (isset($used[$nid][$delta])) {
          $skip = TRUE;
        }
        else {
          $used[$nid][$delta] = TRUE;
        }
      }
    }

    if ($skip) {
      unset($view->result[$num]);
      continue;
    }

    foreach (array_values($fields) as $value) {
      if ($ical) {
        $result->{$value[$field]} = strip_tags($result->{$value[$field]});
        // Copy the data to the other version of the fields
        $result->{$value[1 - $field]} = $result->{$value[$field]};
      }
      else {
        // Clear the unused fields
        unset($result->{$value[1 - $field]});
      }

      // Copy values for the calendar module's theme functions
      $result->{$value[2]} = $result->{$value[$field]};
    }

    $start      = localtime($start_time, TRUE);
    $end        = localtime($end_time, TRUE);
    $start_mins = floor(($start['tm_hour'] * 60 + $start['tm_min']) / $time_inc) * $time_inc;
    $end_mins   = ceil(($end['tm_hour'] * 60 + $end['tm_min']) / $time_inc) * $time_inc;
    $all_day    = $start['tm_sec'] == 0 && $start['tm_min'] == 0 && $start['tm_hour'] == 0 && $end['tm_sec'] == 0 && $end['tm_min'] == 59 && $end['tm_hour'] == 23;

    if ($start['tm_wday'] == $end['tm_wday'] && !$all_day) {
      if (!$no_start_adj) $wd_start = min($start_mins, $wd_start);
      if (!$no_end_adj)   $wd_end   = max($end_mins, $wd_end);
    }

    $result->event = array(
      'start'      => $start,
      'end'        => $end,
      'end_time'   => $end_time,
      'start_mins' => $start_mins,
      'end_mins'   => $end_mins,
      'all_day'    => $all_day
    );

    if ($ical) {
      $start_date = date_create($result->$start_field, timezone_open('UTC'));
      $end_date   = date_create($result->$end_field, timezone_open('UTC'));
      date_timezone_set($start_date, $to_zone);
      date_timezone_set($end_date, $to_zone);
      $result->calendar_start      = date_format($start_date, DATE_FORMAT_DATETIME);
      $result->calendar_start_date = $start_date;
      $result->calendar_end        = date_format($end_date, DATE_FORMAT_DATETIME);
      $result->calendar_end_date   = $end_date;
      $result->datefield           = $start_field;
      $result->date_id             = $num;
    }
  }

  return array($fields, $time_inc, $wd_start, $wd_end, $min_time, $max_time);
}

function _mm_schedule_theme_setup_events_external($view, $ical = FALSE) {
  $date_fields = date_api_fields($view->base_table);
  $calendar_fields = array_keys($date_fields['alias']);

  $view->items = array();
  foreach ($view->result as $pos => $result) {
    $item = drupal_clone($result);
    foreach ($item as $key => $value) {
      if (in_array($key, $calendar_fields)) {
        $item->calendar_fields->$key = $value;
      }
    }

    if (!$ical) {
      foreach ($view->field as $name => $field_data) {
        // Some fields, like the node edit and delete links, have no alias.
        $field_alias = $field_data->field_alias != 'unknown' ? $field_data->field_alias : $name;
        if (!empty($field_data->options['exclude'])) {
          unset($item->$field_alias);
        }
      }
    }

    $view->items[] = $item;
  }

  $calendar_mmtid = isset($view->mm_calendar_mmtid) ? $view->mm_calendar_mmtid : $view->mmtid;
  $view->can_add_event = $calendar_mmtid && _mm_schedule_can_add_event($calendar_mmtid);
}

function _mm_schedule_theme_output_list($view, $fields) {
  foreach ($view->result as $pos => $result) {
    $field = $result->node_mm_node2tree_type == 'mm_event' ? 0 : 1;
    $start_field = $fields['start'][$field];
    $end_field   = $fields['end'][$field];
    $result->start_datetime = $result->$start_field;
    $result->end_datetime = $result->$end_field;
  }

  usort($view->result, create_function('$a, $b', 'return strcmp($a->start_datetime, $b->start_datetime);'));

  $output = array();
  $all_day = date_t('All day', 'datetime');
  $base_date_field = $view->field['field_start_datetime_value']->content_field['field_name'];
  $to_zone = date_default_timezone();
  foreach ($view->result as $pos => $result) {
    $start = date_convert($result->start_datetime, DATE_DATETIME, DATE_OBJECT);
    $end = date_convert($result->end_datetime, DATE_DATETIME, DATE_OBJECT);
    date_timezone_set($start, $to_zone);
    date_timezone_set($end, $to_zone);
    if ($result->event['all_day']) {
      if (date_format($start, 'Ymd') == date_format($end, 'Ymd')) {
        // all day on a single day
        $time = $all_day;
      }
      else {
        // all day, spanning multiple days
        $same_year = date_format($start, 'Y') == date_format($end, 'Y');
        $time = t('until !date', array('!date' => date_format($end, $same_year ? 'F j' : 'F j, Y')));
      }
    }
    else {
      $time = date_format($start, 'g:i a');
      if ($result->start_datetime != $result->end_datetime)
        if (date_format($start, 'Ymd') == date_format($end, 'Ymd')) {
          // various times within the same day
          $time .= ' - ' . date_format($end, 'g:i a');
        }
        else {
          // various times on different days
          $date_fmt = date_format($start, 'Y') == date_format($end, 'Y') ? 'F j' : 'F j, Y';
          $time = t('!start_time on !start_date, to !end_time on !end_date', array('!start_time' => date_format($start, 'g:i a'), '!start_date' => date_format($start, $date_fmt), '!end_time' => date_format($end, 'g:i a'), '!end_date' => date_format($end, $date_fmt)));
        }
    }

    $field = $result->node_mm_node2tree_type == 'mm_event' ? 0 : 1;
    $nid_field = $fields['nid'][$field];
    $id = views_css_safe('calendar:' . $result->$nid_field . ":$base_date_field:0:$pos");
    $div = '<div id="' . $id . '"><div class="view-field">';

    if ($view->date_info->granularity == 'day') {
      if ($result->event['all_day']) $key = '';
      else $key = $time;
      $output[$key][] = $div . l($result->title, 'node/' . $result->$nid_field) . '</div></div>';
    }
    else {
      $key = date_format($start, 'F j, Y');
      $output[$key][] = $div . $time . ': ' . l($result->title, 'node/' . $result->$nid_field) . '</div></div>';
    }
  }

  if (!$output) return '';

  $html = '<dl>';
  foreach ($output as $period => $events)
    if (empty($period) && count($output) == 1) {
      $html .= '<dt>' . join('</dt><dt>', $events) . '</dt>';
    }
    else {
      if (empty($period)) $period = $all_day;
      $html .= '<dt>' . $period . '</dt>';
      $html .= '<dd>' . join('</dd><dd>', $events) . '</dd>';
    }
  $html .= '</dl>';
  return $html;
}

function _mm_schedule_theme_output_day_or_week($view, $fields, $time_inc, $wd_start, $wd_end, $min_time, $max_time, $one_day = FALSE) {
  // Convert $view->result into $events and theme with mm_schedule

  $date_fields = date_api_fields($view->base_table);
  $calendar_fields = array_keys($date_fields['alias']);

  $events = array();
  $theme = 'calendar_' . $view->date_info->granularity . '_node';
  $tz = timezone_open(date_default_timezone_name());
  $base_date_field = $view->field['field_start_datetime_value']->content_field['field_name'];

  foreach ($view->result as $pos => $result) {
    $field = $result->node_mm_node2tree_type == 'mm_event' ? 0 : 1;
    $start_field = $fields['start'][$field];
    $end_field   = $fields['end'][$field];
    $nid_field   = $fields['nid'][$field];

    $event = $result->event;
    $start_mins = $event['start_mins'];

    $item = drupal_clone($result);
    foreach ($item as $key => $value) {
      if (in_array($key, $calendar_fields)) {
        $item->calendar_fields->$key = $value;
      }
    }

    for ($wday = $event['start']['tm_wday']; $wday <= $event['end']['tm_wday']; $wday++) {
      $copy = drupal_clone($result);
      unset($copy->event);    // save a little memory

      if ($wday > $event['start']['tm_wday'] || $start_mins < $wd_start) {
        $start_mins = $wd_start;
        $copy->$start_field = date_convert($min_time + $wday * 24 * 60 * 60, DATE_UNIX, DATE_DATETIME);
      }

      if ($wday == $event['end']['tm_wday'] && $event['end_mins'] <= $wd_end) {
        $end_mins = $event['end_mins'];
      }
      else {
        $end_mins = $wd_end;
        $copy->$end_field = date_convert($min_time + ($wday + 1) * 24 * 60 * 60, DATE_UNIX, DATE_DATETIME);
      }

      foreach ($view->field as $name => $field_data) {
        // Some fields, like the node edit and delete links, have no alias.
        $field_alias = $field_data->field_alias != 'unknown' ? $field_data->field_alias : $name;
        if (!empty($field_data->options['exclude'])) {
          unset($item->$field_alias);
        }
        else if (!empty($field_data) && is_object($field_data)) {
          // Theme the copy instead of the original row so duplicate date
          // fields each get a fresh copy of the original data to theme.
          $item->$field_alias = $field_data->theme($copy);
        }
      }

      $temp_item = (object)array_merge((array)$item, array(
        'node_type' => $view->mm_node->type,
        'calendar_all_day' => $event['all_day'],
        'calendar_start' => $result->$start_field,
        'calendar_end' => $result->$end_field,
        'calendar_start_date' => date_create($result->$start_field, $tz),
        'calendar_end_date' => date_create($result->$end_field, $tz),
        'date_id' => 'calendar:' . $result->$nid_field . ":$base_date_field:$wday:$pos",
        'raw' => $copy,
      ));

      $alias = $fields['start'][2];
      calendar_node_stripe($view, $temp_item, $alias, $alias);
      calendar_node_taxonomy_stripe($view, $temp_item, $alias, $alias);

      $events[] = (object)array(
        'body' => theme($theme, $temp_item, $view),
        'weekday' => $wday,
        'start_mins' => $start_mins,
        'duration' => $end_mins - $start_mins,
        'all_day' => $event['all_day'],
      );
    }
  }

  // Generate column headings
  $days = mm_ui_day_list();
  if ($one_day) {
    $wday = localtime($min_time, TRUE);
    $wday = $wday['tm_wday'];
    $days = array($wday => t('Items'));
  }
  else {
    foreach ($days as $num => $name) {
      $date = strtotime("+$num day", $min_time);
      $days[$num] = '<div class="schedule-day">' . $name . '</div><div class="schedule-date"><a href="' . date_real_url($view, NULL, date('Y-m-d', $date)) . '">' . date('j', $date) . '</a></div>';
    }
  }

  $options = array();
  $calendar_mmtid = isset($view->mm_calendar_mmtid) ? $view->mm_calendar_mmtid : $view->mmtid;
  if (_mm_schedule_can_add_event($calendar_mmtid)) {
    $options['add_link_func'] = '_mm_schedule_theme_add_link';
    $options['add_link_param'] = array(
      'title' => t('add'),
      'attributes' => array(
        'title' => t('Add a new event at this time'),
      ),
      'href' => "mm/$calendar_mmtid/contents/add/mm_event",
      'query' => array(
        'destination' => "mm/$view->mmtid%3F" . drupal_query_string_encode($_GET, array('q')),
      ),
    );
    $options['min_date'] = $view->date_info->min_date;
  }
  $options['granularity'] = $time_inc;

  return theme('mm_schedule', mm_ui_hour_list($wd_start, $wd_end, $time_inc), $days, $events, $options);
}

function theme_mm_schedule($hours, $days, $events, $options = array()) {
  // Find the range of hours, based on the passed labels
  $granularity = isset($options['granularity']) && $options['granularity'] < 60 ? $options['granularity'] : 60;
  $min = 24 * 60;
  $max = 0;
  foreach (array_keys($hours) as $hour)
    _mm_schedule_theme_date_compare($hour, $hour, $granularity, $min, $max);
  $max += $granularity;

  // Remove any bad entries and expand the range of hours, if needed
  $all_day_row = array();
  foreach ($events as $index => $event)
    if ($event->all_day) {
      if (!isset($all_day_row[$event->group])) $all_day_row[$event->group] = count($all_day_row);
    }
    else if (empty($event->start_mins) && !is_numeric($event->start_mins) && $event->start_mins !== '0' || $event->duration <= 0) {
      unset($events[$index]);
    }
    else {
      if ($event->duration > 24 * 60) $event->duration = 24 * 60;
      _mm_schedule_theme_date_compare($event->start_mins, $event->start_mins + $event->duration, $granularity, $min, $max);
    }

  $all_day_rows = count($all_day_row);

  // Add hours to the first column
  $cols = array();
  if (count($hours) || $all_day_rows) {
    $cols['times'][0] = array();
    $prev_row = -1;

    if ($all_day_rows) {
      $prev_row += $all_day_rows;
      $cols['times'][0][0] = array('class' => 'schedule-hour schedule-all-day', 'data' => '<span class="schedule-hour schedule-all-day">' . t('All day') . '</span>', 'hour' => FALSE, 'colspan' => 1, 'rowspan' => $all_day_rows);
      $temp_row = 0;
      while (++$temp_row < $all_day_rows)
        $cols['times'][0][$temp_row] = TRUE;
    }

    $prev_span = 1;
    foreach ($hours as $hour => $title) {
      $row = ($hour - $min) / $granularity + $all_day_rows;
      if ($prev_row >= $all_day_rows) {
        $cols['times'][0][$prev_row]['rowspan'] = $prev_span = $row - $prev_row;
        $temp_row = $prev_row;
        while (++$temp_row < $row)
          $cols['times'][0][$temp_row] = TRUE;
      }
      $cols['times'][0][$row] = array('class' => 'schedule-hour', 'data' => '<span class="schedule-hour">' . $title . '</span>', 'hour' => $title, 'colspan' => 1);
      $prev_row = $row;
    }

    if (count($hours)) {
      $span = ($max - $min + 2) / $granularity - $prev_row + $all_day_rows;
      $cols['times'][0][$prev_row]['rowspan'] = $prev_span;
      $temp_row = $prev_row;
      while (++$temp_row < $prev_row + $prev_span)
        $cols['times'][0][$temp_row] = TRUE;
    }
  }

  // Find the first day (column) and initialize arrays
  foreach (array_keys($days) as $day) {
    if (!isset($first_day)) $first_day = $day;
    $cols[$day] = array(array());
  }

  // Populate the $cols array with events
  $colors = $conflict = array();
  foreach ($events as $event) {
    $day = $event->weekday;
    $start = $event->start_mins;
    $row = $event->all_day ? $all_day_row[$event->group] : ($start - $min) / $granularity + $all_day_rows;
    $span = $event->all_day ? 1 : $event->duration / $granularity;

    // Find this event's conflicts
    $col = 0;
    while (is_array($cols[$day][$col]) && ($in_use = array_intersect_key(array_fill($row, $span, TRUE), $cols[$day][$col]))) {
      if (!empty($options['show_conflicts'])) {
        $conflict += $in_use;
      }
      $col++;
    }

    // Add a new column, if needed
    if (!is_array($cols[$day][$col])) $cols[$day][$col] = array();

    // Figure out what color to make the cell
    if (isset($colors[$event->group])) $color = $colors[$event->group];
    else {
      $color = count($colors);
      $colors[$event->group] = $color;
    }

    // Store the data and mark any spanned cells as in use, for later conflict
    // detection
    $class = 'schedule-items schedule-color-' . $color;
    if ($event->all_day && $row == $all_day_rows - 1) $class .= ' schedule-all-day';
    $cols[$day][$col][$row] = array('class' => $class, 'data' => $event->body, 'rowspan' => $span, 'colspan' => 1);
    while (--$span > 0)
      $cols[$day][$col][$row + $span] = 1;
  }

  foreach ($cols as $day => $day_col)
    foreach ($day_col as $col => $col_data) {
      // Auto-expand each scheduled item as far to the right as possible
      foreach ($col_data as $row => $row_data)
        if (is_array($row_data) && $col < count($day_col) - 1) {
          $auto_span_col = $col;
          $auto_compare = array_fill($row, $row_data['rowspan'], TRUE);
          while (is_array($day_col[++$auto_span_col]) && !array_intersect_key($auto_compare, $day_col[$auto_span_col])) {
            $cols[$day][$col][$row]['colspan']++;
            $cols[$day][$auto_span_col] += $auto_compare;
          }
        }

      // Fill-in any remaining gaps
      $cols[$day][$col] += array_fill(0, ($max - $min) / $granularity + $all_day_rows, FALSE);
      ksort($cols[$day][$col]);
    }

  // Auto-expand all-day events across days to the right when possible
  $days_idx = array_keys($cols);
  for ($row = 0; $row < $all_day_rows; $row++)
    for ($day = 1; $day < count($days_idx) - 1; $day++) {
      $idx = $days_idx[$day];
      if (is_array($cols[$idx][0][$row])) {
        $this_day = $day;
        while (is_array($cols[$days_idx[++$day]][0][$row])) {
          $cols[$idx][0][$row]['colspan'] += $cols[$days_idx[$day]][0][$row]['colspan'];
          $cols[$days_idx[$day]][0][$row] = TRUE;
        }
      }
    }

  // Figure out the header cells and colgroup values
  $th = $colgroups = '';
  foreach ($cols as $day => $day_col) {
    if ($day === 'times') {
      $th .= '<th class="schedule-dayview-hour">' . t('Time') . '</th>';
      $colgroups .= '<col width="1"></col>';
    }
    else {
      $th .= '<th class="schedule-items" colspan="' . count($day_col) . '">' . $days[$day] . '</th>';
      if (count($day_col))
        $colgroups .= '<colgroup span="' . count($day_col) . '" width="' . intval(100 / count($days) / count($day_col)) . '%"></colgroup>';
    }
  }
  // filler
  $th .= '<th class="schedule-items"></th>';
  $colgroups .= '<col width="1"></col>';

  $tbody = '';
  $in_use = array();
  foreach (array_keys($cols[$first_day][0]) as $row) {
    $tbody .= $row > $all_day_rows && ($row % (60 / $granularity) - $all_day_rows) == 0 ? '<tr class="schedule-major-hdiv">' : '<tr>';
    foreach ($cols as $day => $day_col) {
      $major_vdiv = $day === 'times' ? '' : ' schedule-major-vdiv';
      foreach ($day_col as $col => $col_data) {
        $row_data = $col_data[$row];
        $add_link = '';
        if (isset($options['add_link_func']) && isset($options['min_date']) && $day !== 'times' && $col == count($day_col) - 1 && ($row < $all_day_rows || $cols['times'][0][$row]['hour'] !== FALSE)) {
          $add_mins = ($day - $first_day) * 24 * 60;
          if ($row < $all_day_rows) {
            $add_end = 24 * 60 - 1;
          }
          else {
            $add_mins += ($row - $all_day_rows) * $granularity + $min;
            $add_end = $granularity;
          }

          $add_start_date = drupal_clone($options['min_date']);
          date_modify($add_start_date, "+$add_mins minutes");
          $add_end_date = drupal_clone($add_start_date);
          date_modify($add_end_date, "+$add_end minutes");
          $add_link = $options['add_link_func']($add_start_date, $add_end_date, $options['add_link_param']);
        }

        if (is_array($row_data)) {
          $span = '';
          if ($row_data['rowspan'] > 1) $span .= ' rowspan="' . $row_data['rowspan'] . '"';
          if ($row_data['colspan'] > 1) $span .= ' colspan="' . $row_data['colspan'] . '"';
          $not_last = $col + $row_data['colspan'] >= count($day_col) ? '' : ' schedule-items-not-last';
          $tbody .= '<td class="' . $row_data['class'] . $not_last . $major_vdiv . '"' . $span . '>';
          $tbody .= $add_link . $row_data['data'];
          $tbody .= '</td>';
          for ($irow = $row; $irow < $row + $row_data['rowspan']; $irow++)
            $in_use[$day][$irow] = TRUE;
        }
        else if ($row_data === FALSE) {
          $find_link = '&nbsp;';
          if ($options['editable'] && !$in_use[$day][$row] && $col == count($day_col) - 1 && !empty($cols['times'][0][$row]['data']) && is_object($options['schedule']) && $cols['times'][0][$row]['hour'] !== FALSE) {
            $find_link = '<div>' . _academics_schedule_thickbox_link(t('find'), 'course_scheduler/daytime/' . join('/', array($options['schedule']->schedid, $day, ($row - $all_day_rows) * $granularity + $min)), t('Find courses meeting at @time on @day', array('@time' => $cols['times'][0][$row]['hour'], '@day' => $days[$day]))) . '</div>';
          }
          $class = 'schedule-empty' . $major_vdiv;
          if ($row == $all_day_rows - 1) $class .= ' schedule-all-day';
          $tbody .= '<td class="' . $class . '">' . $add_link . $find_link . '</td>';
        }
        $major_vdiv = '';
      }
    }

    $class = $conflict[$row] ? 'schedule-conflict' : 'schedule-filler';
    if ($row == $all_day_rows - 1) $class .= ' schedule-all-day';
    $tbody .= '<td class="' . $class . ' schedule-major-vdiv">&nbsp;</td>';
    $tbody .= '</tr>';
  }

  return <<<EOH
<table class="schedule">
  $colgroups
  <thead>
    <tr>
      $th
    </tr>
  </thead>
  <tbody>
    $tbody
  </tbody>
</table>
EOH;
}

function _mm_schedule_theme_date_compare($start, $end, &$granularity, &$min, &$max) {
  static $grans;

  if ($start < $min) $min = $start;
  if ($end > $max) $max = $end;

  if ($granularity == 5) return;

  if (!isset($grans)) {
    $grans = array(60, 30, 20, 15, 10, 5);
  }
  else if ($grans[0] != $granularity) {
    $grans = array_slice($grans, array_search($granularity, $grans));
  }

  foreach (array($start, $end) as $time)
    foreach ($grans as $new_gran)
      if ($new_gran <= $granularity && ($time % $new_gran) == 0 && ($granularity % $new_gran) == 0) {
        $granularity = $new_gran;
        break;
      }
}

function _mm_schedule_theme_add_link($start_date, $end_date, $param) {
  $param['query']['mm_event_start'] = date_format($start_date, DATE_FORMAT_DATETIME);
  if (!empty($end_date)) $param['query']['mm_event_end'] = date_format($end_date, DATE_FORMAT_DATETIME);
  return '<div class="calendar-add">' . l($param['title'], $param['href'], $param) . '</div>';
}

function _mm_schedule_theme_month_add_link($view, $date) {
  static $param = array();

  if ($view->can_add_event) {
    $calendar_mmtid = isset($view->mm_calendar_mmtid) ? $view->mm_calendar_mmtid : $view->mmtid;
    if (!isset($param[$calendar_mmtid])) {
      $param[$calendar_mmtid] = array(
        'title' => t('add'),
        'attributes' => array(
          'title' => t('Add a new event on this day'),
        ),
        'href' => "mm/$calendar_mmtid/contents/add/mm_event",
        'query' => array(
          'destination' => "mm/$view->mmtid%3F" . drupal_query_string_encode($_GET, array('q')),
        ),
      );
    }

    $start_date = date_create(substr($date, 0, 11) . '09:00:00');
    $end_date   = date_create(substr($date, 0, 11) . '10:00:00');
    return _mm_schedule_theme_add_link($start_date, $end_date, $param[$calendar_mmtid]);
  }
}
