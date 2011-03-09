<?php
// $Id: mm_taxonomy.inc 4216 2010-06-25 18:36:37Z dan $

/**
 * @file
 * Versions of taxonomy module functions which take MM permissions into account.
 * Without these, the user gets results that can't be viewed.
 */

/**
 * Finds all nodes that match selected taxonomy conditions. Originally from
 * taxonomy_select_nodes() in taxonomy.module.
 *
 * @param $tids
 *   An array of term IDs to match.
 * @param $operator
 *   How to interpret multiple IDs in the array. Can be "or" or "and".
 * @param $depth
 *   How many levels deep to traverse the taxonomy tree. Can be a nonnegative
 *   integer or "all".
 * @param $pager
 *   Whether the nodes are to be used with a pager (the case on most Drupal
 *   pages) or not (in an XML feed, for example).
 * @param $order
 *   The order clause for the query that retrieve the nodes.
 * @param $mmtid
 *   (optional) LOCALMOD: Limit the results to a particular mmtid and its
 *   children.
 * @return
 *   A resource identifier pointing to the query results.
 */
function mm_taxonomy_select_nodes($tids = array(), $operator = 'or', $depth = 0, $pager = TRUE, $order = 'n.sticky DESC, n.created DESC', $mmtid = NULL) {
  if (count($tids) > 0) {
    // For each term ID, generate an array of descendant term IDs to the right depth.
    $descendant_tids = array();
    if ($depth === 'all') {
      $depth = NULL;
    }
    foreach ($tids as $index => $tid) {
      $term = taxonomy_get_term($tid);
      $tree = taxonomy_get_tree($term->vid, $tid, -1, $depth);
      $descendant_tids[] = array_merge(array($tid), array_map('_taxonomy_get_tid_from_term', $tree));
    }

    // LOCALMOD: Added
    db_query('DROP TEMPORARY TABLE IF EXISTS temp_search_results');

    if ($operator == 'or') {
      $args = call_user_func_array('array_merge', $descendant_tids);
      $placeholders = db_placeholders($args, 'int');
      // Removed by LOCALMOD
      //$sql = 'SELECT DISTINCT(n.nid), n.sticky, n.title, n.created FROM {node} n INNER JOIN {term_node} tn ON n.vid = tn.vid WHERE tn.tid IN ('. $placeholders .') AND n.status = 1 ORDER BY '. $order;
      //$sql_count = 'SELECT COUNT(DISTINCT(n.nid)) FROM {node} n INNER JOIN {term_node} tn ON n.vid = tn.vid WHERE tn.tid IN ('. $placeholders .') AND n.status = 1';
      // LOCALMOD: Start replacement code
      db_query_temporary("SELECT DISTINCT(n.nid), n.sticky, n.title, n.created FROM {node} n INNER JOIN {term_node} tn ON n.nid = tn.nid LEFT JOIN {mm_recycle} t ON t.id = n.nid AND t.type = 'node' WHERE tn.tid IN ($placeholders) AND n.status = 1 AND t.id IS NULL ORDER BY $order LIMIT 500", $args, 'temp_search_results');
      // LOCALMOD: End replacement code
    }
    else {
      $joins = '';
      $wheres = '';
      $args = array();
      foreach ($descendant_tids as $index => $tids) {
        $joins .= ' INNER JOIN {term_node} tn'. $index .' ON n.vid = tn'. $index .'.vid';
        $wheres .= ' AND tn'. $index .'.tid IN ('. db_placeholders($tids, 'int') .')';
        $args = array_merge($args, $tids);
      }
      // Removed by LOCALMOD
      // $sql = 'SELECT DISTINCT(n.nid), n.sticky, n.title, n.created FROM {node} n '. $joins .' WHERE n.status = 1 '. $wheres .' ORDER BY '. $order;
      // $sql_count = 'SELECT COUNT(DISTINCT(n.nid)) FROM {node} n '. $joins .' WHERE n.status = 1 '. $wheres;
      // LOCALMOD: Start replacement code
      db_query_temporary("SELECT DISTINCT(n.nid), n.sticky, n.title, n.created FROM {node} n $joins LEFT JOIN {mm_recycle} t ON t.id = n.nid AND t.type = 'node' WHERE n.status = 1 $wheres AND t.id IS NULL ORDER BY $order LIMIT 500", $args, 'temp_search_results');
      // LOCALMOD: End replacement code
    }

    // LOCALMOD: Start added code
    $delete_node = array();
    // look for nodes that aren't readable by the user
    $result = db_query("SELECT t.mmtid, i.nid FROM temp_search_results i INNER JOIN {mm_node2tree} t ON i.nid = t.nid GROUP BY t.nid");
    while ($item = db_fetch_object($result)) {
      if (!mm_content_user_can($item->mmtid, 'r')) {
        $delete_node[] = $item->nid;
      }
    }

    if (!is_null($mmtid)) {
      // limit the results to a particular mmtid and its children
      $keep_node = array();
      $result = db_query("SELECT t.mmtid, i.nid FROM temp_search_results i INNER JOIN {mm_node2tree} t ON i.nid = t.nid INNER JOIN {mm_tree} m ON t.mmtid = m.mmtid INNER JOIN {mm_tree_parents} p ON p.mmtid = m.mmtid WHERE m.mmtid = %d OR p.parent = %d", array($mmtid, $mmtid));
      while ($item = db_fetch_object($result)) {
        $keep_node[] = $item->nid;
      }

      if (count($keep_node)) {
        db_query('DELETE FROM temp_search_results WHERE nid NOT IN (' . db_placeholders($keep_node, 'int') . ') ', $keep_node);
      }
      else {
        db_query('DELETE FROM temp_search_results');
      }
    }

    if (count($delete_node)) {
      db_query('DELETE FROM temp_search_results WHERE nid IN (' . db_placeholders($delete_node, 'int') . ') ', $delete_node);
    }

    $sql = 'SELECT DISTINCT(nid), sticky, title, created FROM temp_search_results';
    $sql_count = 'SELECT COUNT(DISTINCT nid) FROM temp_search_results';
    $args = NULL;
    // LOCALMOD: End added code

    $sql = db_rewrite_sql($sql);
    $sql_count = db_rewrite_sql($sql_count);
    if ($pager) {
      $result = pager_query($sql, variable_get('default_nodes_main', 10), 0, $sql_count, $args);
    }
    else {
      $result = db_query_range($sql, $args, 0, variable_get('feed_default_items', 10));
    }
  }

  return $result;
}

/**
 * Menu callback; displays all nodes associated with a term. Originally from
 * taxonomy_term_page() in taxonomy.pages.inc.
 */
function mm_taxonomy_term_page($str_tids = '', $depth = 0, $op = 'page') {
  $terms = taxonomy_terms_parse_string($str_tids);
  if ($terms['operator'] != 'and' && $terms['operator'] != 'or') {
    drupal_not_found();
  }

  if ($terms['tids']) {
    $result = db_query(db_rewrite_sql('SELECT t.tid, t.name FROM {term_data} t WHERE t.tid IN ('. db_placeholders($terms['tids']) .')', 't', 'tid'), $terms['tids']);
    $tids = array(); // we rebuild the $tids-array so it only contains terms the user has access to.
    $names = array();
    while ($term = db_fetch_object($result)) {
      $tids[] = $term->tid;
      $names[] = $term->name;
    }

    if ($names) {
      // LOCALMOD: added
      module_load_include('inc', 'taxonomy', 'taxonomy.pages');
      $title = check_plain(implode(', ', $names));
      drupal_set_title($title);

      switch ($op) {
        case 'page':
          // Build breadcrumb based on first hierarchy of first term:
          $current->tid = $tids[0];
          $breadcrumb = array();
          while ($parents = taxonomy_get_parents($current->tid)) {
            $current = array_shift($parents);
            $breadcrumb[] = l($current->name, 'taxonomy/term/'. $current->tid);
          }
          $breadcrumb[] = l(t('Home'), NULL);
          $breadcrumb = array_reverse($breadcrumb);
          drupal_set_breadcrumb($breadcrumb);

          // LOCALMOD: was taxonomy_select_nodes()
          $output = theme('taxonomy_term_page', $tids, mm_taxonomy_select_nodes($tids, $terms['operator'], $depth, TRUE));
          drupal_add_feed(url('taxonomy/term/'. $str_tids .'/'. $depth .'/feed'), 'RSS - '. $title);
          return $output;
          break;

        case 'feed':
          $channel['link'] = url('taxonomy/term/'. $str_tids .'/'. $depth, array('absolute' => TRUE));
          $channel['title'] = variable_get('site_name', 'Drupal') .' - '. $title;
          // Only display the description if we have a single term, to avoid clutter and confusion.
          if (count($tids) == 1) {
            $term = taxonomy_get_term($tids[0]);
            // HTML will be removed from feed description, so no need to filter here.
            $channel['description'] = $term->description;
          }

          // LOCALMOD: was taxonomy_select_nodes()
          $result = mm_taxonomy_select_nodes($tids, $terms['operator'], $depth, FALSE);
          $items = array();
          while ($row = db_fetch_object($result)) {
            $items[] = $row->nid;
          }

          node_feed($items, $channel);
          break;

        default:
          drupal_not_found();
      }
    }
    else {
      drupal_not_found();
    }
  }
}
