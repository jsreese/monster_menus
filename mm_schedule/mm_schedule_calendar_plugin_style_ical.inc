<?php
//$Id: mm_schedule_calendar_plugin_style_ical.inc 3600 2009-11-24 16:32:56Z dan $

/**
 * @file
 * Override class calendar_plugin_style_ical in
 * calendar/calendar_ical/calendar_plugin_style_ical.inc because it cannot
 * handle relationships
 */

class mm_schedule_calendar_plugin_style_ical extends calendar_plugin_style_ical {
  function render() {
    // Transfer the style options to the view object so they
    // can be easily accessed in the theme.
    $style_options = $this->options;
    $this->view->date_info->summary_field = $style_options['summary_field'];
    $this->view->date_info->description_field = $style_options['description_field'];
    $this->view->date_info->location_field = $style_options['location_field'];

    // Evaluate our argument values and figure out which
    // calendar display we need to create.
    $i = 0;
    foreach ($this->view->argument as $id => $argument) {
      if ($argument->field == 'date_argument') {
        // TODO Decide if we want to provide a date here or not.
        // Adding this now is to prevent fatal errors later if the
        // view is used in unexpected ways without a date being set.
        if (empty($argument->min_date)) {
          $value = $argument->get_default_argument();
          $range = $argument->date_handler->arg_range($value);
          $argument->min_date = $range[0];
          $argument->max_date = $range[1];
        }
        $this->view->date_info->granularity = !empty($argument->granularity) ? $argument->granularity : $argument->options['granularity'];
        $this->view->date_info->date_arg = !empty($this->view->args) ? $this->view->args[$argument->position] : '';
        $this->view->date_info->date_arg_pos = $i;
        $this->view->date_info->year = isset($argument->year) ? $argument->year : NULL;
        $this->view->date_info->month = isset($argument->month) ? $argument->month: NULL;
        $this->view->date_info->day = isset($argument->day) ? $argument->day : NULL;
        $this->view->date_info->week = isset($argument->week) ? $argument->week : NULL;
        $this->view->date_info->min_date = $argument->min_date;
        $this->view->date_info->max_date = $argument->max_date;

        // Stop after the first date argument, if there is more than one.
        break;
      }
      $i++;
    }

    // The ical display might have date filters instead of arguments.
    // If we missed getting a min date from date arguments, try date filters.
    if (empty($this->view->date_info->min_date)) {
      foreach ($this->view->filter as $id => $filter) {
        if ($filter->field == 'date_filter') {
          // TODO Decide if we want to provide a date here or not.
          // Adding this now is to prevent fatal errors later if the
          // view is used in unexpected ways without a date being set.
          if (empty($filter->min_date)) {
            $value = $filter->default_value('value');
            $range = $filter->date_handler->arg_range($value);
            $filter->min_date = $range[0];
            $filter->max_date = $range[1];
          }
          $this->view->date_info->granularity = !empty($filter->granularity) ? $filter->granularity : $filter->options['granularity'];
          $this->view->date_info->year = isset($filter->year) ? $filter->year : NULL;
          $this->view->date_info->month = isset($filter->month) ? $filter->month: NULL;
          $this->view->date_info->day = isset($filter->day) ? $filter->day : NULL;
          $this->view->date_info->week = isset($filter->week) ? $filter->week : NULL;
          $this->view->date_info->min_date = $filter->min_date;
          $this->view->date_info->max_date = $filter->max_date;
          if (empty($this->view->date_info->date_fields)) {
            $this->view->date_info->date_fields = array();
          }
          $this->view->date_info->date_fields = array_merge($this->view->date_info->date_fields, array_keys($filter->options['date_fields']));

          // Stop after the first date filter, if there is more than one.
          break;
        }
      }
      $i++;
    }

    return theme($this->theme_functions(), $this->view, $this->options);
  }
}
