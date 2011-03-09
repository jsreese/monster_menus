$content[type]  = array (
  'name' => 'Event calendar',
  'type' => 'mm_calendar',
  'description' => 'Calendars display events in a date-based grid',
  'title_label' => 'Title',
  'body_label' => 'Body',
  'min_word_count' => '0',
  'help' => '',
  'node_options' => 
  array (
    'status' => true,
    'promote' => false,
    'sticky' => false,
    'revision' => false,
  ),
  'upload' => '0',
  'show_preview_changes' => 0,
  'old_type' => 'mm_calendar',
  'orig_type' => 'mm_calendar',
  'module' => 'mm_schedule_mm_calendar',
  'custom' => '0',
  'modified' => '1',
  'locked' => '1',
  'reset' => 'Reset to defaults',
  'comment' => '2',
  'comment_default_mode' => '4',
  'comment_default_order' => '1',
  'comment_default_per_page' => '50',
  'comment_controls' => '3',
  'comment_anonymous' => 0,
  'comment_subject_field' => '1',
  'comment_preview' => '1',
  'comment_form_location' => '0',
);
$content[groups]  = array (
  0 => 
  array (
    'label' => 'Events',
    'group_type' => 'standard',
    'settings' => 
    array (
      'form' => 
      array (
        'style' => 'fieldset',
        'description' => '',
      ),
      'display' => 
      array (
        'description' => '',
        'teaser' => 
        array (
          'format' => 'fieldset',
          'exclude' => 1,
        ),
        'full' => 
        array (
          'format' => 'fieldset',
          'exclude' => 1,
        ),
        4 => 
        array (
          'format' => 'fieldset',
          'exclude' => 0,
        ),
        'label' => 'above',
      ),
    ),
    'weight' => '-2',
    'group_name' => 'group_events',
  ),
  1 => 
  array (
    'label' => 'Appearance',
    'group_type' => 'standard',
    'settings' => 
    array (
      'form' => 
      array (
        'style' => 'fieldset',
        'description' => '',
      ),
      'display' => 
      array (
        'description' => '',
        'teaser' => 
        array (
          'format' => 'fieldset',
          'exclude' => 1,
        ),
        'full' => 
        array (
          'format' => 'fieldset',
          'exclude' => 1,
        ),
        4 => 
        array (
          'format' => 'fieldset',
          'exclude' => 0,
        ),
        'label' => 'above',
      ),
    ),
    'weight' => '-1',
    'group_name' => 'group_appearance',
  ),
  2 => 
  array (
    'label' => 'Push to Personal Calendars',
    'group_type' => 'standard',
    'settings' => 
    array (
      'form' => 
      array (
        'style' => 'fieldset',
        'description' => '',
      ),
      'display' => 
      array (
        'description' => '',
        'teaser' => 
        array (
          'format' => 'fieldset',
          'exclude' => 1,
        ),
        'full' => 
        array (
          'format' => 'fieldset',
          'exclude' => 1,
        ),
        4 => 
        array (
          'format' => 'fieldset',
          'exclude' => 0,
        ),
        'label' => 'above',
      ),
    ),
    'weight' => '0',
    'group_name' => 'group_push',
  ),
  3 => 
  array (
    'label' => 'Week/Day Grid Settings',
    'group_type' => 'standard',
    'settings' => 
    array (
      'form' => 
      array (
        'style' => 'fieldset_collapsed',
        'description' => '',
      ),
      'display' => 
      array (
        'description' => '',
        'teaser' => 
        array (
          'format' => 'fieldset',
          'exclude' => 1,
        ),
        'full' => 
        array (
          'format' => 'fieldset',
          'exclude' => 1,
        ),
        4 => 
        array (
          'format' => 'fieldset',
          'exclude' => 0,
        ),
        'label' => 'above',
      ),
    ),
    'weight' => '1',
    'group_name' => 'group_week_day',
  ),
  4 => 
  array (
    'label' => 'Year Settings',
    'group_type' => 'standard',
    'settings' => 
    array (
      'form' => 
      array (
        'style' => 'fieldset_collapsed',
        'description' => '',
      ),
      'display' => 
      array (
        'description' => '',
        'teaser' => 
        array (
          'format' => 'fieldset',
          'exclude' => 1,
        ),
        'full' => 
        array (
          'format' => 'fieldset',
          'exclude' => 1,
        ),
        4 => 
        array (
          'format' => 'fieldset',
          'exclude' => 0,
        ),
        'label' => 'above',
      ),
    ),
    'weight' => '14',
    'group_name' => 'group_year',
  ),
);
$content[fields]  = array (
  0 => 
  array (
    'label' => 'All events on these page(s):',
    'field_name' => 'field_mm_pages',
    'type' => 'mm_list',
    'widget_type' => 'mm_catlist',
    'change' => 'Change basic information',
    'weight' => '6',
    'description' => '',
    'default_value' => 
    array (
    ),
    'default_value_php' => 'mm_parse_args($term_ids, $oarg_list, $this_tid);
if (!isset($this_tid)) return array();
return array(array(\'value\' => $this_tid));',
    'default_value_widget' => NULL,
    'group' => 'group_events',
    'required' => 0,
    'multiple' => '1',
    'mm_list_readonly' => 0,
    'mm_list_show_info' => 1,
    'mm_list_min' => '0',
    'mm_list_max' => '0',
    'mm_list_popup_start' => 
    array (
      7 => 'Home',
    ),
    'mm_list_enabled' => 'r',
    'mm_list_selectable' => 'u',
    'op' => 'Save field settings',
    'module' => 'mm_cck',
    'widget_module' => 'mm_cck',
    'columns' => 
    array (
      'value' => 
      array (
        'type' => 'int',
        'not null' => false,
        'sortable' => true,
      ),
    ),
    'display_settings' => 
    array (
      'weight' => '6',
      'parent' => 'group_events',
      'label' => 
      array (
        'format' => 'above',
      ),
      'teaser' => 
      array (
        'format' => 'hidden',
        'exclude' => 1,
      ),
      'full' => 
      array (
        'format' => 'hidden',
        'exclude' => 1,
      ),
      4 => 
      array (
        'format' => 'default',
        'exclude' => 0,
      ),
    ),
  ),
  1 => 
  array (
    'label' => 'Individual events',
    'field_name' => 'field_mm_events',
    'type' => 'nodereference',
    'widget_type' => 'nodereference_autocomplete',
    'change' => 'Change basic information',
    'weight' => '7',
    'autocomplete_match' => 'contains',
    'description' => 'To add an event, type part of its title, then choose it from the resulting list.',
    'default_value' => 
    array (
      0 => 
      array (
        'nid' => NULL,
        '_error_element' => 'default_value_widget][field_mm_events][0][nid][nid',
      ),
    ),
    'default_value_php' => '',
    'default_value_widget' => 
    array (
      'field_mm_events' => 
      array (
        0 => 
        array (
          'nid' => 
          array (
            'nid' => '',
            '_error_element' => 'default_value_widget][field_mm_events][0][nid][nid',
          ),
          '_error_element' => 'default_value_widget][field_mm_events][0][nid][nid',
        ),
      ),
    ),
    'group' => 'group_events',
    'required' => 0,
    'multiple' => '1',
    'referenceable_types' => 
    array (
      'story' => 0,
      'crs_guide' => 0,
      'mm_event' => 0,
      'mm_calendar' => 0,
      'media' => 0,
      'gallery' => 0,
      'gradebook' => 0,
      'quiz_instance' => 0,
      'quiz_tool' => 0,
      'rss_page' => 0,
      'redirect' => 0,
      'roster' => 0,
      'subpglist' => 0,
      'alert' => 0,
      'viewref' => 0,
      'webform' => 0,
      'portal_page' => 0,
      'course' => false,
      'course_xlist' => false,
      'biblio' => false,
      'it_project' => false,
      'course_noncourse' => false,
    ),
    'advanced_view' => 'mm_calendar_events_list',
    'advanced_view_args' => '',
    'op' => 'Save field settings',
    'module' => 'nodereference',
    'widget_module' => 'nodereference',
    'columns' => 
    array (
      'nid' => 
      array (
        'type' => 'int',
        'unsigned' => true,
        'not null' => false,
      ),
    ),
    'display_settings' => 
    array (
      'weight' => '7',
      'parent' => 'group_events',
      'label' => 
      array (
        'format' => 'above',
      ),
      'teaser' => 
      array (
        'format' => 'default',
        'exclude' => 1,
      ),
      'full' => 
      array (
        'format' => 'default',
        'exclude' => 1,
      ),
      4 => 
      array (
        'format' => 'default',
        'exclude' => 0,
      ),
    ),
  ),
  2 => 
  array (
    'label' => 'Show links to change mode',
    'field_name' => 'field_allow_mode_change',
    'type' => 'number_integer',
    'widget_type' => 'optionwidgets_onoff',
    'change' => 'Change basic information',
    'weight' => '3',
    'description' => '',
    'default_value' => 
    array (
      0 => 
      array (
        'value' => 1,
      ),
    ),
    'default_value_php' => '',
    'default_value_widget' => 
    array (
      'field_allow_mode_change' => 
      array (
        'value' => true,
      ),
    ),
    'group' => 'group_appearance',
    'required' => 0,
    'multiple' => '0',
    'min' => '',
    'max' => '',
    'prefix' => '',
    'suffix' => '',
    'allowed_values' => '0|no
1|Show links to change the display mode',
    'allowed_values_php' => '',
    'op' => 'Save field settings',
    'module' => 'number',
    'widget_module' => 'optionwidgets',
    'columns' => 
    array (
      'value' => 
      array (
        'type' => 'int',
        'not null' => false,
        'sortable' => true,
      ),
    ),
    'display_settings' => 
    array (
      'weight' => '3',
      'parent' => 'group_appearance',
      'label' => 
      array (
        'format' => 'above',
      ),
      'teaser' => 
      array (
        'format' => 'default',
        'exclude' => 1,
      ),
      'full' => 
      array (
        'format' => 'default',
        'exclude' => 1,
      ),
      4 => 
      array (
        'format' => 'default',
        'exclude' => 1,
      ),
    ),
  ),
  3 => 
  array (
    'label' => 'Show the Export link',
    'field_name' => 'field_show_export',
    'type' => 'number_integer',
    'widget_type' => 'optionwidgets_onoff',
    'change' => 'Change basic information',
    'weight' => '4',
    'description' => '',
    'default_value' => 
    array (
      0 => 
      array (
        'value' => 1,
      ),
    ),
    'default_value_php' => '',
    'default_value_widget' => 
    array (
      'field_show_export' => 
      array (
        'value' => true,
      ),
    ),
    'group' => 'group_appearance',
    'required' => 0,
    'multiple' => '0',
    'min' => '',
    'max' => '',
    'prefix' => '',
    'suffix' => '',
    'allowed_values' => '0|no
1|Show the Export link',
    'allowed_values_php' => '',
    'op' => 'Save field settings',
    'module' => 'number',
    'widget_module' => 'optionwidgets',
    'columns' => 
    array (
      'value' => 
      array (
        'type' => 'int',
        'not null' => false,
        'sortable' => true,
      ),
    ),
    'display_settings' => 
    array (
      'weight' => '4',
      'parent' => 'group_appearance',
      'label' => 
      array (
        'format' => 'above',
      ),
      'teaser' => 
      array (
        'format' => 'default',
        'exclude' => 1,
      ),
      'full' => 
      array (
        'format' => 'default',
        'exclude' => 1,
      ),
      4 => 
      array (
        'format' => 'default',
        'exclude' => 1,
      ),
    ),
  ),
  4 => 
  array (
    'label' => 'Default display mode',
    'field_name' => 'field_default_display',
    'type' => 'text',
    'widget_type' => 'optionwidgets_select',
    'change' => 'Change basic information',
    'weight' => '5',
    'description' => '',
    'default_value' => 
    array (
      0 => 
      array (
        'value' => 'month',
      ),
    ),
    'default_value_php' => '',
    'default_value_widget' => 
    array (
      'field_default_display' => 
      array (
        'value' => 'month',
      ),
    ),
    'group' => 'group_appearance',
    'required' => 1,
    'multiple' => '0',
    'text_processing' => '0',
    'max_length' => '',
    'allowed_values' => 'day|Day
week|Week
month|Month
year|Year',
    'allowed_values_php' => '',
    'op' => 'Save field settings',
    'module' => 'text',
    'widget_module' => 'optionwidgets',
    'columns' => 
    array (
      'value' => 
      array (
        'type' => 'text',
        'size' => 'big',
        'not null' => false,
        'sortable' => true,
      ),
    ),
    'display_settings' => 
    array (
      'weight' => '5',
      'parent' => 'group_appearance',
      'label' => 
      array (
        'format' => 'above',
      ),
      'teaser' => 
      array (
        'format' => 'default',
        'exclude' => 1,
      ),
      'full' => 
      array (
        'format' => 'default',
        'exclude' => 1,
      ),
      4 => 
      array (
        'format' => 'default',
        'exclude' => 1,
      ),
    ),
  ),
  5 => 
  array (
    'label' => 'List/Grid',
    'field_name' => 'field_default_view',
    'type' => 'text',
    'widget_type' => 'optionwidgets_select',
    'change' => 'Change basic information',
    'weight' => '6',
    'description' => '',
    'default_value' => 
    array (
    ),
    'default_value_php' => '',
    'default_value_widget' => 
    array (
      'field_default_view' => 
      array (
        'value' => '',
      ),
    ),
    'group' => 'group_appearance',
    'required' => 1,
    'multiple' => '0',
    'text_processing' => '0',
    'max_length' => '',
    'allowed_values' => 'grid|Grid
list|List',
    'allowed_values_php' => '',
    'op' => 'Save field settings',
    'module' => 'text',
    'widget_module' => 'optionwidgets',
    'columns' => 
    array (
      'value' => 
      array (
        'type' => 'text',
        'size' => 'big',
        'not null' => false,
        'sortable' => true,
      ),
    ),
    'display_settings' => 
    array (
      'weight' => '6',
      'parent' => 'group_appearance',
      'label' => 
      array (
        'format' => 'above',
      ),
      'teaser' => 
      array (
        'format' => 'default',
        'exclude' => 1,
      ),
      'full' => 
      array (
        'format' => 'default',
        'exclude' => 1,
      ),
      4 => 
      array (
        'format' => 'default',
        'exclude' => 0,
      ),
    ),
  ),
  6 => 
  array (
    'label' => 'Group(s):',
    'field_name' => 'field_mm_groups',
    'type' => 'mm_list',
    'widget_type' => 'mm_grouplist',
    'change' => 'Change basic information',
    'weight' => '11',
    'description' => 'Members of the selected group(s) will see events from this calendar on their personal "My Calendar" page.',
    'default_value' => 
    array (
    ),
    'default_value_php' => '',
    'default_value_widget' => 
    array (
      'field_mm_groups' => 
      array (
      ),
    ),
    'group' => 'group_push',
    'required' => 0,
    'multiple' => '1',
    'mm_list_readonly' => 0,
    'mm_list_show_info' => 0,
    'mm_list_min' => '0',
    'mm_list_max' => '0',
    'mm_list_popup_start' => 
    array (
    ),
    'op' => 'Save field settings',
    'module' => 'mm_cck',
    'widget_module' => 'mm_cck',
    'columns' => 
    array (
      'value' => 
      array (
        'type' => 'int',
        'not null' => false,
        'sortable' => true,
      ),
    ),
    'display_settings' => 
    array (
      'weight' => '11',
      'parent' => 'group_push',
      'label' => 
      array (
        'format' => 'above',
      ),
      'teaser' => 
      array (
        'format' => 'hidden',
        'exclude' => 1,
      ),
      'full' => 
      array (
        'format' => 'hidden',
        'exclude' => 1,
      ),
      4 => 
      array (
        'format' => 'default',
        'exclude' => 0,
      ),
    ),
  ),
  7 => 
  array (
    'label' => 'Start time',
    'field_name' => 'field_wd_start',
    'type' => 'number_integer',
    'widget_type' => 'optionwidgets_select',
    'change' => 'Change basic information',
    'weight' => 0,
    'description' => '',
    'default_value' => 
    array (
      0 => 
      array (
        'value' => '540',
      ),
    ),
    'default_value_php' => '',
    'default_value_widget' => 
    array (
      'field_wd_start' => 
      array (
        'value' => '540',
      ),
    ),
    'group' => 'group_week_day',
    'required' => 1,
    'multiple' => '0',
    'min' => '',
    'max' => '',
    'prefix' => '',
    'suffix' => '',
    'allowed_values' => '',
    'allowed_values_php' => 'return mm_ui_hour_list();',
    'op' => 'Save field settings',
    'module' => 'number',
    'widget_module' => 'optionwidgets',
    'columns' => 
    array (
      'value' => 
      array (
        'type' => 'int',
        'not null' => false,
        'sortable' => true,
      ),
    ),
    'display_settings' => 
    array (
      'weight' => 0,
      'parent' => 'group_week_day',
      'label' => 
      array (
        'format' => 'above',
      ),
      'teaser' => 
      array (
        'format' => 'default',
        'exclude' => 1,
      ),
      'full' => 
      array (
        'format' => 'default',
        'exclude' => 1,
      ),
      4 => 
      array (
        'format' => 'default',
        'exclude' => 1,
      ),
    ),
  ),
  8 => 
  array (
    'label' => 'End time',
    'field_name' => 'field_wd_end',
    'type' => 'number_integer',
    'widget_type' => 'optionwidgets_select',
    'change' => 'Change basic information',
    'weight' => '1',
    'description' => '',
    'default_value' => 
    array (
      0 => 
      array (
        'value' => '1020',
      ),
    ),
    'default_value_php' => '',
    'default_value_widget' => 
    array (
      'field_wd_end' => 
      array (
        'value' => '1020',
      ),
    ),
    'group' => 'group_week_day',
    'required' => 1,
    'multiple' => '0',
    'min' => '',
    'max' => '',
    'prefix' => '',
    'suffix' => '',
    'allowed_values' => '',
    'allowed_values_php' => 'return mm_ui_hour_list();',
    'op' => 'Save field settings',
    'module' => 'number',
    'widget_module' => 'optionwidgets',
    'columns' => 
    array (
      'value' => 
      array (
        'type' => 'int',
        'not null' => false,
        'sortable' => true,
      ),
    ),
    'display_settings' => 
    array (
      'weight' => '1',
      'parent' => 'group_week_day',
      'label' => 
      array (
        'format' => 'above',
      ),
      'teaser' => 
      array (
        'format' => 'default',
        'exclude' => 1,
      ),
      'full' => 
      array (
        'format' => 'default',
        'exclude' => 1,
      ),
      4 => 
      array (
        'format' => 'default',
        'exclude' => 1,
      ),
    ),
  ),
  9 => 
  array (
    'label' => 'Time increment',
    'field_name' => 'field_wd_increment',
    'type' => 'number_integer',
    'widget_type' => 'optionwidgets_select',
    'change' => 'Change basic information',
    'weight' => '2',
    'description' => '',
    'default_value' => 
    array (
      0 => 
      array (
        'value' => '30',
      ),
    ),
    'default_value_php' => '',
    'default_value_widget' => 
    array (
      'field_wd_increment' => 
      array (
        'value' => '30',
      ),
    ),
    'group' => 'group_week_day',
    'required' => 1,
    'multiple' => '0',
    'min' => '',
    'max' => '',
    'prefix' => '',
    'suffix' => '',
    'allowed_values' => '5|5 minutes
10|10 minutes
15|15 minutes
30|30 minutes
60|1 hour',
    'allowed_values_php' => '',
    'op' => 'Save field settings',
    'module' => 'number',
    'widget_module' => 'optionwidgets',
    'columns' => 
    array (
      'value' => 
      array (
        'type' => 'int',
        'not null' => false,
        'sortable' => true,
      ),
    ),
    'display_settings' => 
    array (
      'weight' => '2',
      'parent' => 'group_week_day',
      'label' => 
      array (
        'format' => 'above',
      ),
      'teaser' => 
      array (
        'format' => 'default',
        'exclude' => 1,
      ),
      'full' => 
      array (
        'format' => 'default',
        'exclude' => 1,
      ),
      4 => 
      array (
        'format' => 'default',
        'exclude' => 1,
      ),
    ),
  ),
  10 => 
  array (
    'label' => 'First month of the year',
    'field_name' => 'field_first_month',
    'type' => 'number_integer',
    'widget_type' => 'optionwidgets_select',
    'change' => 'Change basic information',
    'weight' => '15',
    'description' => 'Choose the starting month for year views. Use September if your calendar should be displayed based on the school year.',
    'default_value' => 
    array (
      0 => 
      array (
        'value' => '1',
      ),
    ),
    'default_value_php' => '',
    'default_value_widget' => 
    array (
      'field_first_month' => 
      array (
        'value' => '1',
      ),
    ),
    'group' => 'group_year',
    'required' => 1,
    'multiple' => '0',
    'min' => '',
    'max' => '',
    'prefix' => '',
    'suffix' => '',
    'allowed_values' => '',
    'allowed_values_php' => 'return date_month_names(TRUE);',
    'op' => 'Save field settings',
    'module' => 'number',
    'widget_module' => 'optionwidgets',
    'columns' => 
    array (
      'value' => 
      array (
        'type' => 'int',
        'not null' => false,
        'sortable' => true,
      ),
    ),
    'display_settings' => 
    array (
      'weight' => '15',
      'parent' => 'group_year',
      'label' => 
      array (
        'format' => 'above',
      ),
      'teaser' => 
      array (
        'format' => 'default',
        'exclude' => 1,
      ),
      'full' => 
      array (
        'format' => 'default',
        'exclude' => 1,
      ),
      4 => 
      array (
        'format' => 'default',
        'exclude' => 0,
      ),
    ),
  ),
);
$content[extra]  = array (
  'title' => '-5',
  'body_field' => '-4',
  'menu' => '-3',
);
