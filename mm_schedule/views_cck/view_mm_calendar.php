$view = new view;
$view->name = 'mm_calendar';
$view->description = 'A multi-dimensional calendar view with back/next navigation.';
$view->tag = 'Calendar, amherst';
$view->view_php = '';
$view->base_table = 'node';
$view->is_cacheable = FALSE;
$view->api_version = 2;
$view->disabled = FALSE; /* Edit this to true to make a default view disabled initially */
$handler = $view->new_display('default', 'Defaults', 'default');
$handler->override_option('relationships', array(
  'field_mm_events_nid' => array(
    'label' => 'Individual events',
    'required' => 0,
    'delta' => '-1',
    'id' => 'field_mm_events_nid',
    'table' => 'node_data_field_mm_events',
    'field' => 'field_mm_events_nid',
    'override' => array(
      'button' => 'Override',
    ),
    'relationship' => 'none',
  ),
  'field_mm_pages_value' => array(
    'label' => 'All events on these page(s)',
    'required' => 0,
    'delta' => '-1',
    'id' => 'field_mm_pages_value',
    'table' => 'node_data_field_mm_pages',
    'field' => 'field_mm_pages_value',
    'relationship' => 'none',
  ),
  'mmtid' => array(
    'label' => 'Nodes on MM page(s)',
    'required' => 0,
    'id' => 'mmtid',
    'table' => 'mm_node2tree',
    'field' => 'mmtid',
    'relationship' => 'field_mm_pages_value',
  ),
));
$handler->override_option('fields', array(
  'field_start_datetime_value_1' => array(
    'label' => '',
    'link_to_node' => 1,
    'label_type' => 'none',
    'format' => 'time',
    'multiple' => array(
      'multiple_number' => '',
      'multiple_from' => '',
      'multiple_to' => '',
      'group' => 0,
    ),
    'repeat' => array(
      'show_repeat_rule' => 'hide',
    ),
    'fromto' => array(
      'fromto' => 'both',
    ),
    'exclude' => 0,
    'id' => 'field_start_datetime_value_1',
    'table' => 'node_data_field_start_datetime',
    'field' => 'field_start_datetime_value',
    'override' => array(
      'button' => 'Override',
    ),
    'relationship' => 'field_mm_events_nid',
  ),
  'field_start_datetime_value' => array(
    'label' => '',
    'link_to_node' => 1,
    'label_type' => 'none',
    'format' => 'time',
    'multiple' => array(
      'multiple_number' => '',
      'multiple_from' => '',
      'multiple_to' => '',
      'group' => 0,
    ),
    'repeat' => array(
      'show_repeat_rule' => 'hide',
    ),
    'fromto' => array(
      'fromto' => 'both',
    ),
    'exclude' => 0,
    'id' => 'field_start_datetime_value',
    'table' => 'node_data_field_start_datetime',
    'field' => 'field_start_datetime_value',
    'relationship' => 'mmtid',
  ),
  'title_1' => array(
    'label' => '',
    'link_to_node' => 1,
    'exclude' => 0,
    'id' => 'title_1',
    'table' => 'node',
    'field' => 'title',
    'override' => array(
      'button' => 'Override',
    ),
    'relationship' => 'field_mm_events_nid',
  ),
  'title' => array(
    'label' => '',
    'link_to_node' => 1,
    'exclude' => 0,
    'id' => 'title',
    'table' => 'node',
    'field' => 'title',
    'relationship' => 'mmtid',
  ),
  'body_1' => array(
    'label' => 'Body',
    'exclude' => 1,
    'id' => 'body_1',
    'table' => 'node_revisions',
    'field' => 'body',
    'relationship' => 'field_mm_events_nid',
  ),
  'body' => array(
    'label' => 'Body',
    'exclude' => 1,
    'id' => 'body',
    'table' => 'node_revisions',
    'field' => 'body',
    'relationship' => 'mmtid',
  ),
  'vid_1' => array(
    'label' => 'Vid',
    'exclude' => 1,
    'id' => 'vid_1',
    'table' => 'node_revisions',
    'field' => 'vid',
    'override' => array(
      'button' => 'Override',
    ),
    'relationship' => 'field_mm_events_nid',
  ),
  'vid' => array(
    'label' => 'Vid',
    'exclude' => 1,
    'id' => 'vid',
    'table' => 'node_revisions',
    'field' => 'vid',
    'override' => array(
      'button' => 'Override',
    ),
    'relationship' => 'mmtid',
  ),
));
$handler->override_option('arguments', array(
  'nid' => array(
    'default_action' => 'not found',
    'style_plugin' => 'default_summary',
    'style_options' => array(),
    'wildcard' => 'all',
    'wildcard_substitution' => 'All',
    'title' => '',
    'default_argument_type' => 'fixed',
    'default_argument' => '',
    'validate_type' => 'none',
    'validate_fail' => 'not found',
    'break_phrase' => 1,
    'not' => 0,
    'id' => 'nid',
    'table' => 'node',
    'field' => 'nid',
    'relationship' => 'none',
    'default_options_div_prefix' => '',
    'default_argument_user' => 0,
    'default_argument_fixed' => '',
    'default_argument_php' => '',
    'validate_argument_node_type' => array(
      'webform' => 0,
      'crs_guide' => 0,
      'roster' => 0,
      'gradebook' => 0,
      'media' => 0,
      'quiz_tool' => 0,
      'quiz_instance' => 0,
      'rss_page' => 0,
      'portal_page' => 0,
      'biblio' => 0,
      'mm_calendar' => 0,
      'redirect' => 0,
      'gallery' => 0,
      'subpglist' => 0,
      'alert' => 0,
      'course' => 0,
      'course_noncourse' => 0,
      'course_xlist' => 0,
      'it_project' => 0,
      'mm_event' => 0,
      'story' => 0,
    ),
    'validate_argument_node_access' => 0,
    'validate_argument_nid_type' => 'nid',
    'validate_argument_vocabulary' => array(
      '2' => 0,
      '1' => 0,
    ),
    'validate_argument_type' => 'tid',
    'validate_argument_php' => '',
  ),
  'date_argument' => array(
    'default_action' => 'default',
    'style_plugin' => 'default_summary',
    'style_options' => array(),
    'wildcard' => 'all',
    'wildcard_substitution' => 'All',
    'title' => '',
    'default_argument_type' => 'date',
    'default_argument' => '',
    'validate_type' => 'none',
    'validate_fail' => 'not found',
    'date_fields' => array(
      'node_data_field_start_datetime.field_start_datetime_value' => 'node_data_field_start_datetime.field_start_datetime_value',
    ),
    'year_range' => '-3:+3',
    'date_method' => 'OR',
    'granularity' => 'month',
    'id' => 'date_argument',
    'table' => 'node',
    'field' => 'date_argument',
    'relationship' => 'mmtid',
    'default_argument_user' => 0,
    'default_argument_fixed' => '',
    'default_argument_php' => '',
    'validate_argument_node_type' => array(
      'webform' => 0,
      'crs_guide' => 0,
      'roster' => 0,
      'gradebook' => 0,
      'media' => 0,
      'quiz_tool' => 0,
      'quiz_instance' => 0,
      'rss_page' => 0,
      'portal_page' => 0,
      'mm_calendar' => 0,
      'redirect' => 0,
      'gallery' => 0,
      'subpglist' => 0,
      'alert' => 0,
      'course' => 0,
      'course_meeting' => 0,
      'course_noncourse' => 0,
      'course_xlist' => 0,
      'date' => 0,
      'mm_event' => 0,
      'story' => 0,
      'viewref' => 0,
    ),
    'validate_argument_node_access' => 0,
    'validate_argument_nid_type' => 'nid',
    'validate_argument_vocabulary' => array(
      '1' => 0,
    ),
    'validate_argument_type' => 'tid',
    'validate_argument_php' => '',
    'override' => array(
      'button' => 'Override',
    ),
    'default_options_div_prefix' => '',
  ),
  'date_argument_1' => array(
    'default_action' => 'default',
    'style_plugin' => 'default_summary',
    'style_options' => array(),
    'wildcard' => 'all',
    'wildcard_substitution' => 'All',
    'title' => '',
    'default_argument_type' => 'date',
    'default_argument' => '',
    'validate_type' => 'none',
    'validate_fail' => 'not found',
    'date_fields' => array(
      'node_data_field_start_datetime.field_start_datetime_value' => 'node_data_field_start_datetime.field_start_datetime_value',
    ),
    'year_range' => '-3:+3',
    'date_method' => 'OR',
    'granularity' => 'month',
    'id' => 'date_argument_1',
    'table' => 'node',
    'field' => 'date_argument',
    'relationship' => 'field_mm_events_nid',
    'default_options_div_prefix' => '',
    'default_argument_user' => 0,
    'default_argument_fixed' => '',
    'default_argument_php' => '',
    'validate_argument_node_type' => array(
      'webform' => 0,
      'crs_guide' => 0,
      'roster' => 0,
      'gradebook' => 0,
      'media' => 0,
      'quiz_tool' => 0,
      'quiz_instance' => 0,
      'rss_page' => 0,
      'portal_page' => 0,
      'mm_calendar' => 0,
      'redirect' => 0,
      'gallery' => 0,
      'subpglist' => 0,
      'alert' => 0,
      'course' => 0,
      'course_meeting' => 0,
      'course_noncourse' => 0,
      'course_xlist' => 0,
      'date' => 0,
      'mm_event' => 0,
      'story' => 0,
      'viewref' => 0,
    ),
    'validate_argument_node_access' => 0,
    'validate_argument_nid_type' => 'nid',
    'validate_argument_vocabulary' => array(
      '1' => 0,
    ),
    'validate_argument_type' => 'tid',
    'validate_argument_php' => '',
  ),
));
$handler->override_option('filters', array(
  'views_or_begin' => array(
    'operator' => '=',
    'value' => '',
    'group' => '0',
    'exposed' => FALSE,
    'expose' => array(
      'operator' => FALSE,
      'label' => '',
    ),
    'id' => 'views_or_begin',
    'table' => 'node',
    'field' => 'views_or_begin',
    'relationship' => 'none',
  ),
  'status' => array(
    'operator' => '=',
    'value' => '1',
    'group' => '0',
    'exposed' => FALSE,
    'expose' => array(
      'operator' => FALSE,
      'label' => '',
    ),
    'id' => 'status',
    'table' => 'node',
    'field' => 'status',
    'relationship' => 'mmtid',
  ),
  'type' => array(
    'operator' => 'in',
    'value' => array(
      'mm_event' => 'mm_event',
    ),
    'group' => '0',
    'exposed' => FALSE,
    'expose' => array(
      'operator' => FALSE,
      'label' => '',
    ),
    'id' => 'type',
    'table' => 'node',
    'field' => 'type',
    'relationship' => 'mmtid',
  ),
  'views_or_next' => array(
    'id' => 'views_or_next',
    'table' => 'node',
    'field' => 'views_or_next',
  ),
  'status_1' => array(
    'operator' => '=',
    'value' => '1',
    'group' => '0',
    'exposed' => FALSE,
    'expose' => array(
      'operator' => FALSE,
      'label' => '',
    ),
    'id' => 'status_1',
    'table' => 'node',
    'field' => 'status',
    'relationship' => 'field_mm_events_nid',
  ),
  'type_1' => array(
    'operator' => 'in',
    'value' => array(
      'mm_event' => 'mm_event',
    ),
    'group' => '0',
    'exposed' => FALSE,
    'expose' => array(
      'operator' => FALSE,
      'label' => '',
    ),
    'id' => 'type_1',
    'table' => 'node',
    'field' => 'type',
    'relationship' => 'field_mm_events_nid',
  ),
  'recycled' => array(
    'operator' => '=',
    'value' => '0',
    'group' => '0',
    'exposed' => FALSE,
    'expose' => array(
      'operator' => FALSE,
      'label' => '',
    ),
    'id' => 'recycled',
    'table' => 'mm_recycle',
    'field' => 'recycled',
    'relationship' => 'field_mm_events_nid',
  ),
  'views_or_end' => array(
    'id' => 'views_or_end',
    'table' => 'node',
    'field' => 'views_or_end',
  ),
));
$handler->override_option('access', array(
  'type' => 'none',
  'role' => array(),
  'perm' => '',
));
$handler->override_option('title', 'Calendar');
$handler->override_option('header_empty', 1);
$handler->override_option('items_per_page', 0);
$handler->override_option('use_more', 0);
$handler->override_option('distinct', 0);
$handler->override_option('style_plugin', 'calendar_nav');
$handler = $view->new_display('calendar', 'Calendar page', 'calendar_1');
$handler->override_option('path', '.mm_calendar');
$handler->override_option('menu', array(
  'type' => 'none',
  'title' => '',
  'weight' => 0,
  'name' => 'navigation',
));
$handler->override_option('tab_options', array(
  'type' => 'none',
  'title' => '',
  'weight' => 0,
));
$handler->override_option('calendar_colors', array(
  '0' => array(),
));
$handler->override_option('calendar_colors_vocabulary', array(
  '2' => 2,
));
$handler->override_option('calendar_colors_taxonomy', array(
  '11306' => '#994130',
  '11307' => '#F0C1DD',
  '11308' => '#CC6600',
  '11309' => '#e6cf62',
  '11310' => '#669900',
  '11311' => '#A8CD6C',
  '11312' => '#538FD7',
  '11313' => '#A8B8DD',
  '11314' => '#665599',
  '11315' => '#cccccc',
));
$handler->override_option('calendar_popup', '0');
$handler->override_option('calendar_date_link', '');
$handler = $view->new_display('calendar_block', 'Calendar block', 'calendar_block_1');
$handler->override_option('block_description', 'Calendar');
$handler->override_option('block_caching', -1);
$handler = $view->new_display('calendar_period', 'Year view', 'calendar_period_1');
$handler->override_option('style_plugin', 'calendar_style');
$handler->override_option('style_options', array(
  'display_type' => 'year',
  'name_size' => 1,
  'max_items' => 0,
));
$handler->override_option('attachment_position', 'after');
$handler->override_option('inherit_arguments', TRUE);
$handler->override_option('inherit_exposed_filters', TRUE);
$handler->override_option('displays', array(
  'calendar_1' => 'calendar_1',
  'default' => 0,
  'calendar_block_1' => 0,
));
$handler->override_option('calendar_type', 'year');
$handler = $view->new_display('calendar_period', 'Month view', 'calendar_period_2');
$handler->override_option('style_plugin', 'calendar_style');
$handler->override_option('style_options', array(
  'name_size' => '99',
  'with_weekno' => '0',
  'max_items' => '5',
  'max_items_behavior' => 'more',
  'groupby_times' => 'hour',
  'groupby_times_custom' => '',
  'groupby_field' => '',
));
$handler->override_option('attachment_position', 'after');
$handler->override_option('inherit_arguments', TRUE);
$handler->override_option('inherit_exposed_filters', TRUE);
$handler->override_option('displays', array(
  'calendar_1' => 'calendar_1',
  'default' => 0,
  'calendar_block_1' => 0,
));
$handler->override_option('calendar_type', 'month');
$handler = $view->new_display('calendar_period', 'Day view', 'calendar_period_3');
$handler->override_option('style_plugin', 'calendar_style');
$handler->override_option('style_options', array(
  'name_size' => '99',
  'with_weekno' => 0,
  'max_items' => 0,
  'max_items_behavior' => 'more',
  'groupby_times' => 'hour',
  'groupby_times_custom' => '',
  'groupby_field' => '',
));
$handler->override_option('attachment_position', 'after');
$handler->override_option('inherit_arguments', TRUE);
$handler->override_option('inherit_exposed_filters', TRUE);
$handler->override_option('displays', array(
  'calendar_1' => 'calendar_1',
  'default' => 0,
  'calendar_block_1' => 0,
));
$handler->override_option('calendar_type', 'day');
$handler = $view->new_display('calendar_period', 'Week view', 'calendar_period_4');
$handler->override_option('style_plugin', 'calendar_style');
$handler->override_option('style_options', array(
  'name_size' => '99',
  'with_weekno' => 0,
  'max_items' => 0,
  'max_items_behavior' => 'more',
  'groupby_times' => 'half',
  'groupby_times_custom' => '',
  'groupby_field' => '',
));
$handler->override_option('attachment_position', 'after');
$handler->override_option('inherit_arguments', TRUE);
$handler->override_option('inherit_exposed_filters', TRUE);
$handler->override_option('displays', array(
  'calendar_1' => 'calendar_1',
  'default' => 0,
  'calendar_block_1' => 0,
));
$handler->override_option('calendar_type', 'week');
$handler = $view->new_display('calendar_period', 'Block view', 'calendar_period_5');
$handler->override_option('style_plugin', 'calendar_style');
$handler->override_option('style_options', array(
  'display_type' => 'month',
  'name_size' => '1',
));
$handler->override_option('attachment_position', 'after');
$handler->override_option('inherit_arguments', TRUE);
$handler->override_option('inherit_exposed_filters', TRUE);
$handler->override_option('displays', array(
  'calendar_1' => 0,
  'default' => 0,
  'calendar_block_1' => 'calendar_block_1',
));
$handler->override_option('calendar_type', 'month');
$handler = $view->new_display('calendar_ical', 'iCal feed', 'calendar_ical_1');
$handler->override_option('style_plugin', 'mm_ical');
$handler->override_option('style_options', array(
  'mission_description' => FALSE,
  'description' => '',
  'summary_field' => 'title',
  'description_field' => 'body',
  'location_field' => '',
));
$handler->override_option('row_plugin', '');
$handler->override_option('path', '.mm_calendar/ical');
$handler->override_option('menu', array(
  'type' => 'none',
  'title' => '',
  'weight' => 0,
  'name' => 'navigation',
));
$handler->override_option('tab_options', array(
  'type' => 'none',
  'title' => '',
  'weight' => 0,
));
$handler->override_option('displays', array(
  'calendar_1' => 'calendar_1',
  'default' => 0,
  'calendar_block_1' => 'calendar_block_1',
));
$handler->override_option('sitename_title', FALSE);
$handler = $view->new_display('block', 'Upcoming', 'block_1');
$handler->override_option('fields', array(
  'field_start_datetime_value' => array(
    'label' => '',
    'link_to_node' => 0,
    'label_type' => 'none',
    'format' => 'short',
    'multiple' => array(
      'multiple_number' => '',
      'multiple_from' => '',
      'multiple_to' => '',
      'group' => 0,
    ),
    'repeat' => array(
      'show_repeat_rule' => 'hide',
    ),
    'fromto' => array(
      'fromto' => 'both',
    ),
    'exclude' => 1,
    'id' => 'field_start_datetime_value',
    'table' => 'node_data_field_start_datetime',
    'field' => 'field_start_datetime_value',
    'relationship' => 'none',
    'override' => array(
      'button' => 'Use default',
    ),
  ),
  'title' => array(
    'label' => '',
    'link_to_node' => 1,
    'exclude' => 0,
    'id' => 'title',
    'field' => 'title',
    'table' => 'node',
    'relationship' => 'none',
  ),
));
$handler->override_option('sorts', array(
  'field_start_datetime_value' => array(
    'order' => 'ASC',
    'delta' => '-1',
    'id' => 'field_start_datetime_value',
    'table' => 'node_data_field_start_datetime',
    'field' => 'field_start_datetime_value',
    'override' => array(
      'button' => 'Use default',
    ),
    'relationship' => 'none',
  ),
));
$handler->override_option('arguments', array(
  'mmtid' => array(
    'default_action' => 'empty',
    'style_plugin' => 'default_summary',
    'style_options' => array(),
    'wildcard' => '',
    'wildcard_substitution' => 'All',
    'title' => '',
    'default_argument_type' => 'fixed',
    'default_argument' => '',
    'validate_type' => 'none',
    'validate_fail' => 'not found',
    'break_phrase' => 1,
    'add_table' => 0,
    'require_value' => 0,
    'reduce_duplicates' => 0,
    'id' => 'mmtid',
    'table' => 'mm_node2tree',
    'field' => 'mmtid',
    'relationship' => 'none',
    'default_options_div_prefix' => '',
    'default_argument_user' => 0,
    'default_argument_fixed' => '',
    'default_argument_php' => '',
    'validate_argument_node_type' => array(
      'webform' => 0,
      'crs_guide' => 0,
      'roster' => 0,
      'gradebook' => 0,
      'media' => 0,
      'quiz_tool' => 0,
      'quiz_instance' => 0,
      'rss_page' => 0,
      'portal_page' => 0,
      'redirect' => 0,
      'gallery' => 0,
      'subpglist' => 0,
      'alert' => 0,
      'mm_calendar' => 0,
      'course' => 0,
      'course_meeting' => 0,
      'course_noncourse' => 0,
      'course_xlist' => 0,
      'date' => 0,
      'event' => 0,
      'event_all' => 0,
      'story' => 0,
      'viewref' => 0,
    ),
    'validate_argument_node_access' => 0,
    'validate_argument_nid_type' => 'nid',
    'validate_argument_vocabulary' => array(
      '1' => 0,
    ),
    'validate_argument_type' => 'tid',
    'validate_argument_php' => '',
  ),
));
$handler->override_option('filters', array(
  'status' => array(
    'operator' => '=',
    'value' => 1,
    'group' => '0',
    'exposed' => FALSE,
    'expose' => array(
      'operator' => FALSE,
      'label' => '',
    ),
    'id' => 'status',
    'table' => 'node',
    'field' => 'status',
    'relationship' => 'none',
  ),
  'type' => array(
    'operator' => 'in',
    'value' => array(
      'mm_event' => 'mm_event',
    ),
    'group' => '0',
    'exposed' => FALSE,
    'expose' => array(
      'operator' => FALSE,
      'label' => '',
    ),
    'id' => 'type',
    'table' => 'node',
    'field' => 'type',
    'relationship' => 'none',
    'override' => array(
      'button' => 'Use default',
    ),
  ),
  'date_filter' => array(
    'operator' => '>',
    'value' => array(
      'min' => NULL,
      'max' => NULL,
      'value' => NULL,
      'default_date' => 'now',
      'default_to_date' => '',
    ),
    'group' => '0',
    'exposed' => FALSE,
    'expose' => array(
      'operator' => FALSE,
      'label' => '',
    ),
    'date_fields' => array(
      'node_data_field_start_datetime.field_start_datetime_value2' => 'node_data_field_start_datetime.field_start_datetime_value2',
    ),
    'date_method' => 'OR',
    'granularity' => 'minute',
    'form_type' => 'date_text',
    'default_date' => 'now',
    'default_to_date' => '',
    'year_range' => '-3:+3',
    'id' => 'date_filter',
    'table' => 'node',
    'field' => 'date_filter',
    'override' => array(
      'button' => 'Use default',
    ),
    'relationship' => 'none',
  ),
));
$handler->override_option('title', 'Upcoming');
$handler->override_option('items_per_page', 10);
$handler->override_option('use_pager', 'mini');
$handler->override_option('use_more', 1);
$handler->override_option('style_plugin', 'list');
$handler->override_option('style_options', array(
  'grouping' => 'field_start_datetime_value',
  'type' => 'ul',
));
$handler->override_option('block_description', 'Upcoming');
$handler->override_option('block_caching', -1);
$handler = $view->new_display('attachment', 'List view', 'attachment_1');
$handler->override_option('style_plugin', 'mm_schedule_list');
$handler->override_option('attachment_position', 'after');
$handler->override_option('inherit_arguments', TRUE);
$handler->override_option('inherit_exposed_filters', 1);
$handler->override_option('displays', array(
  'default' => 'default',
  'calendar_1' => 0,
  'calendar_block_1' => 0,
  'block_1' => 0,
));
