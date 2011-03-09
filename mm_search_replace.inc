<?php
// $Id: mm_search_replace.inc 4975 2011-01-26 22:21:29Z dan $

/**
 * @file
 * Search for nodes/MM categories with certain attributes, and perform certain
 * actions on them
 */

// Set this value to TRUE to enable some debugging messages and to see the search
// query submitted via AJAX.
define('MMSR_debug', FALSE);

// _mm_theme_mm_list in mm_theme.inc depends on $_mmsr_regexp
global $_mmsr_vars, $_mmsr_query_segments, $_mmsr_regexp;

require_once 'mm_search_replace_alter.inc';

function _mm_search_form_walk_page($item, $key, &$data) {
  static $last_fieldset;

  if (is_array($item)) {
    if ($item['#type'] == 'fieldset') {
      $last_fieldset = $item['#title'];
    }
    else if ($item['#type'] == 'textarea') {
      _mm_search_disable_wysiwyg($item);
    }
    if (isset($item['#mm-search'])) {
      if (is_array($item['#mm-search'])) {
        $i = 0;
        foreach ($item['#mm-search'] as $k => $v) {
          $this_key = "$key-$i";
          if (isset($last_fieldset)) {
            $data->form['search-page-wheres']['#options'][$last_fieldset][$this_key] = $k;
          }
          else {
            $data->form['search-page-wheres']['#options'][$this_key] = $k;
          }
          _mm_search_form_opts($data, $item, $this_key, $v);
          $i++;
        }
      }
      else {
        if (isset($last_fieldset)) {
          $data->form['search-page-wheres']['#options'][$last_fieldset][$key] = $item['#mm-search'];
        }
        else {
          $data->form['search-page-wheres']['#options'][$key] = $item['#mm-search'];
        }

        if (isset($item['#mm-search-opt-check'])) {
          _mm_search_form_opts($data, $item, $key, $item['#mm-search-opt-check'], FALSE, '#mm-search-opt-check');
        }
        if (isset($item['#mm-search-opt-optgroup'])) {
          _mm_search_form_opts($data, $item, $key, $item['#mm-search-opt-optgroup'], FALSE, '#mm-search-opt-optgroup');
        }
        if (isset($item['#mm-search-opt-list'])) {
          _mm_search_form_opts($data, $item, $key, $item['#mm-search-opt-list'], FALSE, '#mm-search-opt-list');
        }
        if (isset($item['#mm-search-opt'])) {
          _mm_search_form_opts($data, $item, $key, $item['#mm-search-opt']);
        }
      }
    }
    else array_walk($item, '_mm_search_form_walk_page', $data);
  }
}

function _mm_search_form_walk_group($item, $key, &$data) {
  static $last_fieldset;

  if (is_array($item)) {
    if ($item['#type'] == 'fieldset') {
      $last_fieldset = $item['#title'];
    }
    else if ($item['#type'] == 'textarea') {
      _mm_search_disable_wysiwyg($item);
    }
    if (isset($item['#mm-search'])) {
      if (is_array($item['#mm-search'])) {
        $i = 0;
        foreach ($item['#mm-search'] as $k => $v) {
          $this_key = "$key-$i";
          if (isset($last_fieldset)) {
            $data->form['search-group-wheres']['#options'][$last_fieldset][$this_key] = $k;
          }
          else {
            $data->form['search-group-wheres']['#options'][$this_key] = $k;
          }
          _mm_search_form_opts($data, $item, $this_key, $v, TRUE);
          $i++;
        }
      }
      else {
        if (isset($last_fieldset)) {
          $data->form['search-group-wheres']['#options'][$last_fieldset][$key] = $item['#mm-search'];
        }
        else {
          $data->form['search-group-wheres']['#options'][$key] = $item['#mm-search'];
        }

        if (isset($item['#mm-search-opt-check'])) {
          _mm_search_form_opts($data, $item, $key, $item['#mm-search-opt-check'], TRUE, '#mm-search-opt-check');
        }
        if (isset($item['#mm-search-opt-optgroup'])) {
          _mm_search_form_opts($data, $item, $key, $item['#mm-search-opt-optgroup'], TRUE, '#mm-search-opt-optgroup');
        }
        if (isset($item['#mm-search-opt-list'])) {
          _mm_search_form_opts($data, $item, $key, $item['#mm-search-opt-list'], TRUE, '#mm-search-opt-list');
        }
        if (isset($item['#mm-search-opt'])) {
          _mm_search_form_opts($data, $item, $key, $item['#mm-search-opt'], TRUE);
        }
      }
    }
    else if ($item['#type'] == 'mm_userlist') {
      if (is_array($data->form["search-$key-choose"]))
        $data->form["search-$key-choose"][$key] = $item;
    }
    else array_walk($item, '_mm_search_form_walk_group', $data);
  }
}

function _mm_search_form_walk_node($item, $key, &$data) {
  global $_mmsr_query_defaults;

  if (is_array($item)) {
    $item['#weight'] = 0;
    $item['#required'] = FALSE;
    $item['#description'] = isset($item['#mm-search-description']) ? $item['#mm-search-description'] : NULL;
    if ($item['#input']) {
      switch ($item['#type']) {
        case 'textarea':
          _mm_search_disable_wysiwyg($item);
          // no break
        case 'textfield':
        case 'date_popup':
          $mm_search = drupal_strtolower($item['#title']);
          $mm_search_opt = $_mmsr_query_defaults['s'];
          break;
      }
    }

    if (isset($item['#mm-search'])) $mm_search = $item['#mm-search'];
    if (isset($item['#mm-search-opt'])) $mm_search_opt = $item['#mm-search-opt'];

    if ($key == 'users_w-choose' || $key == 'users_w') {
      $data->form['search-groups_w'][$key] = $item;
    }
    else if (isset($mm_search)) {
      if (is_array($mm_search)) {
        $i = 0;
        foreach ($mm_search as $k => $v) {
          $this_key = "$key-$i";
          $data->form['search-node-wheres']['#options'][$this_key] = $k;
          _mm_search_form_opts($data, $item, $this_key, $v);
          $i++;
        }
      }
      else {
        $data->form['search-node-wheres']['#options'][$key] = $mm_search;
        _mm_search_form_opts($data, $item, $key, $mm_search_opt);
      }
    }
    else if ($item['#type'] == 'mm_userlist') {
      if (is_array($data->form["search-$key-choose"]))
        $data->form["search-$key-choose"][$key] = $item;
    }
    else array_walk($item, '_mm_search_form_walk_node', $data);
  }
}

function mm_search_form(&$form_state, $item, $mmtid) {
  global $_mmsr_query_defaults;

  $modpath = drupal_get_path('module', 'monster_menus');
  // Use the 'theme' setting, so that this script loads after autocomplete.js
  drupal_add_js("$modpath/mm_search_replace.js", 'theme', 'header', FALSE, TRUE, FALSE);

  $item = (object)array('mmtid' => $mmtid, 'flags' => array());
  module_load_include('inc', 'monster_menus', 'mm_ui_content_edit');
  $form = mm_ui_content_edit($form_state, $item, $mmtid, FALSE, TRUE, TRUE);

  $data = (object)array();
  $data->form['search-type'] = array(
    '#type' => 'select',
    '#title' => t('Find all'),
    '#default_value' => 1,
    '#options' => array(t('contents'), t('pages'), t('contents on pages'), t('groups')),
  );
  $tree = mm_content_get($item->mmtid);
  $tree->name = mm_content_expand_name($tree->name);
  $data->form['search-group-catlist'] = array(
    '#prefix' => '<div id="search-group-catlist">',
    '#suffix' => '</div>',
  );
  $data->form['search-group-catlist']['search-group-cat'] = array(
    '#type' => 'mm_grouplist',
    '#mm_list_min' => 1,
    '#mm_list_max' => 1,
    '#mm_list_selectable' => '',
    '#title' => t('starting at:'),
    '#default_value' => array($tree->mmtid => $tree->name),
    '#description' => t('Search down the tree starting at this location.'),
  );
  $data->form['search-page-catlist'] = array(
    '#prefix' => '<div id="search-page-catlist">',
    '#suffix' => '</div>',
  );
  $data->form['search-page-catlist']['search-page-cat'] = array(
    '#type' => 'mm_catlist',
    '#mm_list_min' => 1,
    '#mm_list_max' => 1,
    '#mm_list_selectable' => '',
    '#title' => t('starting at:'),
    '#default_value' => array($tree->mmtid => $tree->name),
    '#description' => t('Search down the tree starting at this location.'),
  );
  $data->form['search-logic'] = array(
    '#type' => 'select',
    '#default_value' => 'and',
    '#id' => 'search-logic',
    '#attributes' => array('style' => 'display: none'),
    '#options' => array('and' => t('and'), 'or' => t('or')),
  );
  $data->form['search-page-wheres'] = array(
    '#type' => 'select',
    '#default_value' => '',
    '#id' => 'search-page-wheres',
    '#attributes' => array('style' => 'display: none'),
    '#options' => array('' => '(choose a property)'),
  );
  $data->form['search-group-wheres'] = array(
    '#type' => 'select',
    '#default_value' => '',
    '#id' => 'search-group-wheres',
    '#attributes' => array('style' => 'display: none'),
    '#options' => array('' => '(choose a property)'),
  );

  $node_types = array('' => t('(any type)'));
  foreach (node_get_types() as $type)
    if (node_access('create', $type->type))
      $node_types[$type->type] = $type->name;
  natcasesort($node_types);
  $data->form['search-node-type'] = array(
    '#type' => 'select',
    '#id' => 'search-node-type',
    '#options' => $node_types,
  );

  mm_search_replace_alter_mm($form, FALSE);
  array_walk($form, '_mm_search_form_walk_page', $data);

  $form = mm_ui_content_edit($form_state, $item, $mmtid, TRUE, TRUE, TRUE);
  mm_search_replace_alter_mm($form, TRUE);
  array_walk($form, '_mm_search_form_walk_group', $data);

  $data->form['search-node-wheres'] = array(
    '#type' => 'select',
    '#id' => 'search-node-wheres',
    '#options' => array('' => '(choose a property)'),
  );
  $data->form['data'] = array(
    '#type' => 'hidden',
  );
  $data->form['reset'] = array(
    '#type' => 'submit',
    '#value' => t('Reset'),
  );
  $data->form['result'] = array(
    '#type' => 'submit',
    '#value' => t('Show Results'),
  );
  global $user;
  $old_user = $user;
  session_save_session(FALSE);  // in case of error, don't save session as wrong user
  $user = user_load(array('uid' => 1));
  $node = (object)array('uid' => 1, 'name' => '', 'type' => 'story');
  $form_id = $node->type .'_node_form';
  $form_state = array('storage' => NULL, 'submitted' => FALSE);
  module_load_include('inc', 'node', 'node.pages');
  $form = drupal_retrieve_form($form_id, $form_state, $node);
  $user = $old_user;
  session_save_session(TRUE);  // re-enable session saving
  drupal_prepare_form($form_id, $form, $form_state);
  $form['mm_appearance']['changed'] = $form['mm_appearance']['date'];
  _mm_search_disable_wysiwyg($form['body_field']['body']);
  $form = form_builder($form_id, $form, $form_state);
  mm_search_replace_alter_node($form);
  mm_content_restore_breadcrumb();
//  debug_add_dump($form);
  array_walk($form, '_mm_search_form_walk_node', $data);

  $mmlist_name = $item->mmtid . '{' . $tree->name . '}';
  if (mm_content_is_group($item->mmtid)) {
    $home = mm_content_get(mm_home_mmtid());
    $node_page = mm_home_mmtid() . '{' . mm_content_expand_name($home->name) . '}';
    $group_cat = $mmlist_name;
  }
  else {
    $node_page = $mmlist_name;
    $group_cat = mm_content_groups_mmtid() . '{' . mm_content_expand_name(MM_ENTRY_NAME_GROUPS) . '}';
  }

  $reset = $startup = array(
    'search-type' => 0,
    'search-page-cat' => $node_page,
    'search-group-cat' => $group_cat,
    'search-node-type' => '',
    'mmsr-cont-row' => array(array('search-node-wheres' => '')),
  );
  if (isset($_SESSION['mmsr-data'])) {
    $row = -1;
    $startup = array();
    foreach (explode('&', $_SESSION['mmsr-data']) as $arg) {
      list($key, $val) = explode('=', $arg, 2);
      $val = urldecode($val);
      if ($key == 'search-node-wheres') $row_type = 'mmsr-cont-row';
      else if ($key == 'search-page-wheres') $row_type = 'mmsr-page-row';
      else if ($key == 'search-group-wheres') $row_type = 'mmsr-group-row';

      if ($key == 'search-logic')
        if ($row < 0) continue;
        else $row++;
      else if ($row < 0 && ($key == 'search-node-wheres' || $key == 'search-page-wheres' || $key == 'search-group-wheres'))
        $row++;

      if ($row >= 0 && $key != 'search-page-cat' && $key != 'search-group-cat' && $key != 'search-node-cat') $startup[$row_type][$row][$key] = $val;
      else $startup[$key] = $val;
    }
    $startup['search-page-cat'] = $reset['search-page-cat'];
    $startup['search-group-cat'] = $reset['search-group-cat'];
    $startup = array_merge($reset, $startup);
  }

  drupal_add_js(
    array('MMSR' =>
      array('fixups' => array(),
            'get_path' => base_path() . 'mmsr-get',
            'loading' => base_path() . drupal_get_path('module', 'thickbox') . '/loading_animation.gif',
            'startup' => $startup,
            'reset' => $reset,
            't_where' => t('where'),
            't_of_type' => t('of type'),
            't_plus' => t('Add a row'),
            't_minus' => t('Delete this row'),
      )
    ), 'setting');
  drupal_add_js('Drupal.settings.MMSR.fixups["input.autocomplete"] = MMSR_fixup_autocomplete;', 'inline');
  $_SESSION['mmsr-query'] = $data->query;
  if (MMSR_debug) debug_add_dump($data);
  return $data->form;
}

function _mm_search_disable_wysiwyg(&$item) {
  $item['#rows'] = 3;
  $item['#tinymce_disabled'] = TRUE;
}

function mm_search_form_submit($form, &$form_state) {
  $_SESSION['mmsr-data'] = $form_state['values']['data'];
  $form_state['redirect'] = $_GET['q'] . '/result';
}

function mm_search_results() {
  $output = l(t('Export as CSV'), $_GET['q'] . '.csv', array('query' => drupal_query_string_encode($_GET, array('q'))));

  $count_sql = _mm_search_result_query($_SESSION['mmsr-data'], $_SESSION['mmsr-query']['queries'], TRUE, $result_sql, $header);
  $result = pager_query($result_sql, 25, 0, $count_sql);

  while ($row = db_fetch_object($result)) {
    if (isset($row->nid)) {
      $rows[] = array('data' =>
        array(
          l(trim($row->title) == '' ? t('(unknown)') : $row->title, 'node/' . $row->nid),
          node_get_types('name', $row),
          format_date($row->changed, 'small'),
          format_date($row->created, 'small'),
          mm_content_uid2name($row->uid, 'fmlu', $row, $hover)
        )
      );
    }
    else {
      $rows[] = array('data' =>
        array(
          l(trim($row->pgname) == '' ? t('(unknown)') : $row->pgname, mm_content_get_mmtid_url($row->mmtid)),
          mm_content_uid2name($row->uid, 'fmlu', $row, $hover)
        )
      );
    }
  }

  if (!$rows) {
    $rows[] = array(array('data' => t('No matches'), 'colspan' => count($header)));
  }

  $output .= theme('table', $header, $rows);
  $output .= theme('pager', NULL, 25, 0);

  return $output;
}

function mm_search_results_csv() {
  header('Content-type: text/plain');

  $count_sql = _mm_search_result_query($_SESSION['mmsr-data'], $_SESSION['mmsr-query']['queries'], TRUE, $result_sql, $header);
  $result = pager_query($result_sql, 1000000, 0, $count_sql);

  $hdrs = array();
  foreach ($header as $row) {
    $hdrs[] = $row['data'];
    if ($row['data'] == t('Title')) $hdrs[] = t('URL');
  }
  $fp = fopen('php://output', 'w');
  fputcsv($fp, $hdrs);

  while ($row = db_fetch_object($result)) {
    if (isset($row->nid)) {
      $title = trim($row->title);
      fputcsv($fp, array(
        $title == '' ? t('(unknown)') : $title,
        url('node/' . $row->nid, array('absolute' => TRUE)),
        node_get_types('name', $row),
        format_date($row->changed, 'small'),
        format_date($row->created, 'small'),
        mm_content_uid2name($row->uid, 'fmlu', $row, $hover)
      ));
    }
    else {
      $title = trim($row->pgname);
      fputcsv($fp, array(
        $title == '' ? t('(unknown)') : $title,
        url(mm_content_get_mmtid_url($row->mmtid), array('absolute' => TRUE)),
        mm_content_uid2name($row->uid, 'fmlu', $row, $hover)
      ));
    }
  }

  $GLOBALS['devel_shutdown'] = FALSE;
  fclose($fp);
  exit();
}

function _mm_search_form_opts(&$data, $item, $key, $opt, $is_group = FALSE, $type = 'select') {
  global $_mmsr_query_defaults;
  $segs = $is_group ? 'grp_segs' : 'segs';
  $do_form = !isset($data->form["search-$key"]);

  if ($do_form) {
    $data->form["search-$key"] = array(
      '#type' => 'markup',
      '#prefix' => "<div id=\"search-$key\" style=\"display: none\">",
      '#suffix' => '</div>',
    );
  }
  $used_subpanels_outer = FALSE;
  if (is_array($opt[0]) && $type != '#mm-search-opt-optgroup' && $type != '#mm-search-opt-list') {
    $i = 0;
    foreach ($opt as $title => $o) {
      $k = "$key-$i";
      $options = array_keys($o);
      $sp_options = array();
      $used_subpanels = FALSE;
      foreach ($options as $o2)
        if (preg_match('/^\[(.*?)\]$/', $o2, $matches)) {
          $subpanel = $matches[1];
          $sp_options[$subpanel] = $item[$subpanel]['#title'];
          $item[$subpanel]['#type'] = 'markup';
          $item[$subpanel]['#prefix'] = '<div class="subpanel" name="' . "search-$k-$subpanel" . '">';
          $item[$subpanel]['#suffix'] = '</div>';
          if ($do_form) $data->form["search-$key"]["search-$k-$subpanel"] = $item[$subpanel];
          unset($item[$subpanel]);
          $used_subpanels = $used_subpanels_outer = TRUE;
        }

      if ($do_form) {
        $data->form["search-$key"]["search-$k"] = array(
          '#type' => 'select',
          '#title' => is_numeric($title) ? NULL : $title,
          '#options' => $used_subpanels ? $sp_options : $options,
          '#weight' => $item['#mm-search-weight'],
          '#attributes' => (isset($item['#mm-search-attr']) ? $item['#mm-search-attr'] : ($used_subpanels ? array('class' => 'subpanel-select') : NULL)),
        );
      }
      $data->query[$segs][$k] = array_values($o);
      $i++;
    }
    if (isset($item['#mm-search-query'])) {
      $data->query['queries'][$key][0] = $item['#mm-search-joins'];
      $data->query['queries'][$key][1] = $item['#mm-search-query'];
    }
  }
  else if (is_array($opt)) {
    if ($type == '#mm-search-opt-list') {   // arbitrary list of other form elements and/or selects
      foreach ($opt as $k => $v)
        if (isset($v['#type'])) {
          if ($do_form) $data->form["search-$key"]["search-$key-$k"] = $v;
        }
        else {
          if ($do_form) {
            $data->form["search-$key"]["search-$key-$k"] = array(
              '#type' => 'select',
              '#title' => is_numeric($k) ? NULL : $k,
              '#options' => array_keys($v),
              '#weight' => $item['#mm-search-weight'],
              '#attributes' => (isset($item['#mm-search-attr']) ? $item['#mm-search-attr'] : $item['#attributes']),
            );
          }

          foreach ($v as $k2 => $v2)
            if (isset($_mmsr_query_defaults[$v2][$k2])) $data->query[$segs]["$key-$k"][] = $_mmsr_query_defaults[$v2][$k2];
            else $data->query[$segs]["$key-$k"][] = $v2;
        }
    }
    else {
      if ($type == '#mm-search-opt-optgroup') {   // categorized select list (<optgroup>)
        $keys = array();
        foreach ($opt as $cat => $v)
          foreach ($v as $k2 => $v2)
            foreach ($v2 as $k3 => $v3) {
              $keys[$cat][$k2] = $k3;
              if (isset($_mmsr_query_defaults[$v3][$k2])) $data->query[$segs][$key][] = $_mmsr_query_defaults[$v3][$k2];
              else $data->query[$segs][$key][] = $v3;
            }
      }
      else {
        $keys = array_keys($opt);
        foreach ($opt as $k => $v)
          if (isset($_mmsr_query_defaults[$v][$k])) $data->query[$segs][$key][] = $_mmsr_query_defaults[$v][$k];
          else $data->query[$segs][$key][] = $v;
      }

      if ($do_form) {
        $data->form["search-$key"]["search-$key-0"] = array(
          '#title' => $type == '#mm-search-opt-check' ? $keys[1] : NULL,
          '#type' => $type == '#mm-search-opt-check' ? 'checkbox' : 'select',
          '#options' => $keys,
          '#weight' => $item['#mm-search-weight'],
          '#attributes' => (isset($item['#mm-search-attr']) ? $item['#mm-search-attr'] : $item['#attributes']),
        );
      }
    }
    if (isset($item['#mm-search-query'])) {
      unset($item['#description']);
      $data->query['queries'][$key][0] = $item['#mm-search-joins'];
      $data->query['queries'][$key][1] = $item['#mm-search-query'];
    }
  }
  else {
    if (isset($item['#mm-search-query'])) {
      $data->query['queries'][$key][0] = $item['#mm-search-joins'];
      $data->query['queries'][$key][1][$key] = $item['#mm-search-query'][$key];
    }
    return;
  }

  unset($item['#title']);
//  unset($item['#description']);
//  debug_add_dump($key, $item);
  if (!$used_subpanels_outer && $type != '#mm-search-opt-check')
    $data->form["search-$key"][$key] = $item;
}

function mm_search_result_count() {
//  $f = fopen('/tmp/xx','a');fwrite($f,print_r($_POST['data']."\n",1));
  $query = _mm_search_result_query($_SESSION['mmsr-data'] = $_POST['data'], $_SESSION['mmsr-query']['queries'], FALSE, $result_query, $header);
  $result = db_result(db_query($query));
  if (isset($result)) $result = format_plural($result, '@count match', '@count matches');
//  fwrite($f,print_r($result."\n",1));
  $out = array('result' => $result);
  if (MMSR_debug) {
    $out['query'] = '<p>' . $_POST['data'] . '</p><p>' . db_prefix_tables($query) . '</p>' . var_dump_ret($args) . monster_menus_footer();
  }
  print drupal_to_js($out);
  $GLOBALS['devel_shutdown'] = FALSE;
//  fclose($f);
  exit();
}

function _mm_search_result_query($data, $query_info, $results, &$result_query, &$header) {
  global $_mmsr_node_queries, $_mmsr_query_segments;

  $row = 0;
  $visited = array('search-logic' => TRUE);
  foreach (explode('&', $data) as $arg) {
    list($key, $val) = explode('=', $arg, 2);
    $val = urldecode($val);
    if (!$visited[$key]) $visited[$key] = TRUE;
    else {
      $row++;
      $visited = array('search-logic' => TRUE);
    }
    $args[$row][$key] = $val;
  }
  $joins = array();
  $wheres = $logic = '';
  $wlist2 = array();
  $cat_key = 'search-page-cat';
  foreach ($args as $row) {
    $wlist = $vars = array();
    $qlist = array();
    foreach ($row as $key => $val) {// debug_add_dump("$key=$val");
      if ($key == 'search-type') {
        $_mmsr_query_segments = $_SESSION['mmsr-query']['segs'];
        switch ($val) {
          case 0:   // contents
            $cat_key = '';
          case 2:   // contents on pages
            $count_query = 'SELECT COUNT(DISTINCT {node}.nid) FROM {node}';
            if ($results) {
              $joins['node_revisions'] = '{node_revisions}.nid = {node}.nid AND {node_revisions}.vid = {node}.vid';
              $header = array(
                array('data' => t('Title'), 'field' => '{node}.title'),
                array('data' => t('Type'), 'field' => '{node}.type'),
                array('data' => t('Modified'), 'field' => '{node}.changed', 'sort' => 'DESC'),
                array('data' => t('Created'), 'field' => '{node}.created'),
              );
              if (module_exists('amherstprofile')) {
                $joins['eduprofile'] = '{eduprofile}.uid = {node}.uid';
                $header[] = array('data' => t('Owner'), 'field' => '{eduprofile}.lastname');
                $result_query = 'SELECT {node}.title, {node}.nid, {node}.type, {node}.changed, {node}.created, {eduprofile}.pref_fml, {eduprofile}.pref_lfm, {eduprofile}.lastname, {eduprofile}.firstname, {eduprofile}.username AS name, {eduprofile}.middlename, {eduprofile}.hover, {node}.uid FROM {node}';
              }
              else {
                $joins['users'] = '{users}.uid = {node}.uid';
                $header[] = array('data' => t('Owner'), 'field' => '{users}.name');
                $result_query = 'SELECT {node}.title, {node}.nid, {node}.type, {node}.changed, {node}.created, {node}.uid, {users}.name FROM {node}';
              }
              $result_groupby = ' GROUP BY {node}.nid ';
            }
            $joins['mm_node2tree'] = '{mm_node2tree}.nid = {node}.nid';
            $joins['mm_tree'] = '{mm_tree}.mmtid = {mm_node2tree}.mmtid';
            break;

          case 3:   // groups
            $cat_key = 'search-group-cat';
            $_mmsr_query_segments = $_SESSION['mmsr-query']['grp_segs'];
            // no break

          case 1:   // pages
            $count_query = 'SELECT COUNT(DISTINCT {mm_tree}.mmtid) FROM {mm_tree}';
            if ($results) {
              $header = array(
                array('data' => t('Page'), 'field' => '{mm_tree}.name'),
              );
              if (module_exists('amherstprofile')) {
                $joins['eduprofile'] = '{eduprofile}.uid = {mm_tree}.uid';
                $result_query = 'SELECT {mm_tree}.name AS pgname, {mm_tree}.mmtid, {eduprofile}.pref_fml, {eduprofile}.pref_lfm, {eduprofile}.lastname, {eduprofile}.firstname, {eduprofile}.username AS name, {eduprofile}.middlename, {eduprofile}.hover, {mm_tree}.uid FROM {mm_tree}';
                $header[] = array('data' => t('Owner'), 'field' => '{eduprofile}.lastname');
              }
              else {
                $joins['users'] = '{users}.uid = {mm_tree}.uid';
                $result_query = 'SELECT {mm_tree}.name AS pgname, {mm_tree}.mmtid, {users}.name, {mm_tree}.uid FROM {mm_tree}';
                $header[] = array('data' => t('Owner'), 'field' => '{users}.name');
              }
              $result_groupby = ' GROUP BY {mm_tree}.mmtid ';
            }
            break;
        } // switch
      }
      else if ($key == 'search-node-type' && $val) {
        $w = _mm_search_parse('{node}.type=${qval}', 'node', $val);
        if ($w != '') $wlist2[] = $w;
      }
      else if ($key == $cat_key) {
        if (intval($val)) {
          $joins['mm_tree_parents'] = '{mm_tree_parents}.mmtid = {mm_tree}.mmtid';
          $w = _mm_search_parse('({mm_tree}.mmtid = ${ival} OR {mm_tree_parents}.parent = ${ival})', 'mm_node2tree', intval($val));
          if ($w != '') $wlist2[] = $w;
        }
      }
      else if ($key == 'search-logic') {
        if ($wheres) $logic = $val == 'and' ? ' AND ' : ' OR ';
      }
      else if ($key == 'search-node-wheres') {
        if (isset($_mmsr_node_queries[$val])) $qlist[] = &$_mmsr_node_queries[$val];
      }
      else if ($key == 'search-page-wheres' || $key == 'search-group-wheres') {
        if (isset($query_info[$val])) $qlist[] = &$query_info[$val];
      }
      else {
        $vars[$key] = $val;
      }
    } // foreach

    foreach ($qlist as $q) {
      if (isset($q[0]))
        foreach ($q[0] as $table => $join_seg)
          if (!isset($joins[$table]))
            $joins[$table] = _mm_search_parse($join_seg, $table, '', $vars);

      $w = '';
      foreach ($q[1] as $varname => $seg)
        $w .= _mm_search_parse($seg, $table, $vars[$varname], $vars);

      if ($w != '') $wlist[] = "($w)";
    }

    if ($wlist) $wheres .= $logic . join(' AND ', $wlist);
  }   // foreach

  if ($wlist2) {
    if ($wheres) $wheres = '(' . $wheres . ') AND ';
    $wheres .= join(' AND ', $wlist2);
  }
  foreach ($joins as $table => $on)
    $query_joins .= ' LEFT JOIN {' . $table . '} ON ' . $on;

  if ($wheres) $query_joins .= ' WHERE ' . $wheres;
  $result_query .= $query_joins;
  if ($results) $result_query .= $result_groupby . tablesort_sql($header);
  return $count_query . $query_joins;
}

function _mm_search_set_vars($val) {
  global $_mmsr_vars;

  $_mmsr_vars['val'] = $val;
  $_mmsr_vars['ival'] = intval($val);
  $_mmsr_vars['qval'] = "'" . db_escape_string($val) . "'";
}

$_mmsr_regexp = '/
  ([$|=]) \{
    ( (
      (?> (?: \{[^{}]*?\} | [^$|={}]++ | [$|=][^{] )+ ) |
      (?R)
    )* )
  \}/xs';

function _mm_search_parse($seg, $table, $val, $vars2 = NULL) {
  global $_mmsr_vars, $_mmsr_regexp;

  $_mmsr_vars = array('table' => '{' . $table . '}');
  _mm_search_set_vars($val);
  if (is_array($vars2)) $_mmsr_vars = array_merge($_mmsr_vars, $vars2);

  return trim(preg_replace_callback($_mmsr_regexp, '_mm_search_preg', $seg));
}

function _mm_search_preg($matches) {
  global $_mmsr_vars, $_mmsr_regexp, $_mmsr_query_segments;

// debug_add_dump($matches,$_mmsr_vars);
  $query_segments = $_mmsr_query_segments;   // set for use within pseudocode
  if ($matches[1] == '=') {          //  ={something}
    $e = preg_replace_callback($_mmsr_regexp, '_mm_search_preg', $matches[2]) . ';';
    return preg_replace_callback($_mmsr_regexp, '_mm_search_preg', eval($e));
  }

  if ($matches[1] == '|') {          //  |{something}
    $e = array();
    foreach (split(',', $old = $_mmsr_vars['val']) as $v) {
      _mm_search_set_vars($v);
      $e[] = preg_replace_callback($_mmsr_regexp, '_mm_search_preg', $matches[2]);
    }
    _mm_search_set_vars($old);
    return preg_replace_callback($_mmsr_regexp, '_mm_search_preg', join(', ', $e));
  }

  if ($matches[3][0] == "'") {       //  ${'something'}
    return "'" . db_escape_string($_mmsr_vars[substr($matches[3], 1, -1)]) . "'";
  }

  return $_mmsr_vars[$matches[3]];   //  ${something}
}

function _mm_search_split_mmlist() {
  $count = 0;
  $out = array();
  foreach (func_get_args() as $a) {
    $element = _mm_ui_process_mmlist(array(), $a);
    if (is_array($element['#value'])) {
      $o = array_keys($element['#value']);
      sort($o);
      $out[] = join(',', $o);
      $count += count($o);
    }
  }
  return array($out, $count);
}

function _mm_search_split_mmlist_perms($mode, &$others) {
  global $_mmsr_vars;

  if ($others = $_mmsr_vars["others_$mode"]) return array('', '');
  $others = '';
  list($a) = _mm_search_split_mmlist($_mmsr_vars["groups_$mode"], $_mmsr_vars["users_$mode"]);
  return $a;
}

function _mm_search_date($item_id, $field) {
  global $_mmsr_vars, $_mmsr_query_segments;

  $date = module_exists('date_popup') ? $_mmsr_vars[$item_id . '[date]'] . ' ' . $_mmsr_vars[$item_id . '[time]'] : $_mmsr_vars[$item_id];
  $dv = preg_match('/\d/', $date) ? date_make_date($date) : '';
  return $field . $_mmsr_query_segments[$item_id][intval($_mmsr_vars["search-$item_id-0"])] . ($dv ? $dv->timestamp : 0);
}
