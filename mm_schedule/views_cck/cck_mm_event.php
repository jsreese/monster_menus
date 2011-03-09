$content[type]  = array (
  'name' => 'Event',
  'type' => 'mm_event',
  'description' => 'A date/time-based event, to appear on a calendar',
  'title_label' => 'Title',
  'body_label' => 'Description',
  'min_word_count' => '0',
  'help' => '',
  'node_options' => 
  array (
    'status' => true,
    'promote' => true,
    'revision' => true,
    'sticky' => false,
  ),
  'show_preview_changes' => 0,
  'old_type' => 'mm_event',
  'orig_type' => '',
  'module' => 'node',
  'custom' => '1',
  'modified' => '1',
  'locked' => '0',
  'comment' => '0',
  'comment_default_mode' => '4',
  'comment_default_order' => '1',
  'comment_default_per_page' => '50',
  'comment_controls' => '3',
  'comment_anonymous' => 0,
  'comment_subject_field' => '1',
  'comment_preview' => '1',
  'comment_form_location' => '0',
);
$content[fields]  = array (
  0 => 
  array (
    'label' => 'Date/time',
    'field_name' => 'field_start_datetime',
    'type' => 'datetime',
    'widget_type' => 'date_popup_repeat',
    'change' => 'Change basic information',
    'weight' => '1',
    'default_value' => 'now',
    'default_value2' => 'same',
    'default_value_code' => '',
    'default_value_code2' => '',
    'input_format' => 'm/d/Y - g:i:sa',
    'input_format_custom' => '',
    'year_range' => '0:+5',
    'increment' => '1',
    'advanced' => 
    array (
      'label_position' => 'above',
      'text_parts' => 
      array (
        'year' => 0,
        'month' => 0,
        'day' => 0,
        'hour' => 0,
        'minute' => 0,
        'second' => 0,
      ),
    ),
    'label_position' => 'above',
    'text_parts' => 
    array (
    ),
    'description' => '',
    'group' => false,
    'required' => 1,
    'multiple' => 1,
    'repeat' => 1,
    'todate' => 'required',
    'granularity' => 
    array (
      'year' => 'year',
      'month' => 'month',
      'day' => 'day',
      'hour' => 'hour',
      'minute' => 'minute',
    ),
    'default_format' => 'field_start_datetime_long',
    'tz_handling' => 'site',
    'timezone_db' => 'UTC',
    'repeat_collapsed' => '1',
    'op' => 'Save field settings',
    'module' => 'date',
    'widget_module' => 'date',
    'columns' => 
    array (
      'value' => 
      array (
        'type' => 'datetime',
        'not null' => false,
        'sortable' => true,
        'views' => true,
      ),
      'value2' => 
      array (
        'type' => 'datetime',
        'not null' => false,
        'sortable' => true,
        'views' => false,
      ),
      'rrule' => 
      array (
        'type' => 'text',
        'not null' => false,
        'sortable' => false,
        'views' => false,
      ),
    ),
    'display_settings' => 
    array (
      'weight' => '1',
      'parent' => '',
      'label' => 
      array (
        'format' => 'hidden',
      ),
      'teaser' => 
      array (
        'format' => 'field_start_datetime_default',
        'exclude' => 0,
      ),
      'full' => 
      array (
        'format' => 'field_start_datetime_default',
        'exclude' => 0,
      ),
      4 => 
      array (
        'format' => 'field_start_datetime_default',
        'exclude' => 0,
      ),
    ),
  ),
);
$content[extra]  = array (
  'title' => '-5',
  'body_field' => '0',
  'menu' => '-2',
);
