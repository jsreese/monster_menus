<?php
// $Id: mm_query.inc 4770 2010-11-10 22:17:37Z dan $

/**
 * @file
 * Cached data retrieval functions
 */

define('MM_QUERY_PAGE_SIZE', 50);

/**
 * Classes for mm_query
 *
 * The mmqCache class contains the SQL statement handle and cached data. Only
 * one instance of this class is created per unique SQL statement.
 *
 * The mmqIterator class maintains a pointer ($count) into the cached data. A
 * new instance of this class is instantiated whenever mm_query is called. In
 * this way, multiple callers can read different parts of the data cache for a
 * given statement simultaneously.
 */
class mmqIterator {
  private $count, $cache;

  public function __construct($query, $args, $mmtid) {
    global $_mmq_cache;

    _db_query_callback($args, TRUE);
    $query = preg_replace_callback(DB_QUERY_REGEXP, '_db_query_callback', $query);

    $this->count = 0;
    if (isset($_mmq_cache[$query])) {
      $this->cache = $_mmq_cache[$query];
      return $_mmq_cache[$query]->reset($query);
    }

    return $this->cache = $_mmq_cache[$query] = new mmqCache($query, $mmtid);
  }

  public function next() {
    return $this->cache->next($this->count);
  }

  public function back() {
    $this->cache->back($this->count);
  }
}

class mmqCache {
  public $mmtid;
  private $stmt, $data, $error, $overflow, $query, $prev, $prev_buffer;

  public function __construct($query, $mmtid) {
    $this->overflow = FALSE;
    $this->data = array();
    $this->mmtid = $mmtid;
    $this->all_cols = !strncasecmp($query, 'SELECT * FROM {mm_tree}', 23);
    $this->stmt = $mmtid < 0 ? NULL : db_query($this->query = $query);
    if (!$this->stmt) $this->error = TRUE;
  }

  public function reset($query) {
    if ($this->overflow) $this->query = $query;
    $this->prev = FALSE;
    $this->prev_buffer = NULL;

    return $this;
  }

  public function back($count) {
    if (!$count) {
      $this->error = TRUE;
    }
    else {
      $this->prev = TRUE;
    }
  }

  public function next(&$count) {
    if ($this->error) return NULL;

    if ($this->prev) {
      $this->prev = FALSE;
      // intentionally don't clear prev_buffer, so that if another back()
      // follows, it will work
      return $this->prev_buffer;
    }
    else if ($count < count($this->data)) {
      return $this->prev_buffer = clone $this->data[$count++];
    }
    else if ($this->stmt) {
      $count++;

      if ($r = db_fetch_object($this->stmt)) {
        if ($count > MM_QUERY_PAGE_SIZE) {    // don't waste too much memory
          $this->overflow = TRUE;
          return $this->prev_buffer = $r;
        }

        $this->data[] = $r;

        if ($this->all_cols) {
          global $_mmtbt_cache;
          $_mmtbt_cache[$r->mmtid] =& $this->data[count($this->data) - 1];
        }

        return $this->prev_buffer = $r;
      }

      $this->stmt = NULL;
    }
    else if ($this->overflow && !($count % MM_QUERY_PAGE_SIZE)) {
      if (stripos($this->query, ' limit ') > 0) {
        $this->stmt = db_query($this->query);

        if (!$this->stmt) $this->error = TRUE;
        else {
          $c = 0;
          $count++;

          while ($c++ < $count) // skip past cached results, saving last
            if (!($r = db_fetch_object($this->stmt))) break;

          if (!$r) $this->stmt = NULL;

          return $this->prev_buffer = $r;
        }
      }
      else {
        $this->stmt = db_query_range($this->query, $count, 0x7fffffff);

        if (!$this->stmt) $this->error = TRUE;
        else {
          $count++;
          $r = db_fetch_object($this->stmt);
          if (!$r) $this->stmt = NULL;

          return $this->prev_buffer = $r;
        }
      }
    }

    return $this->prev_buffer = NULL;
  }
}

/**
 * Start a new DB query, caching the results, or return an object to a
 * previously cached set of values.
 *
 * @param $mmtid
 *   The optional tree ID of the entry being retrieved. This allows
 *   mm_content_clear_caches() to flush the cache only when needed. This
 *   parameter can be completely omitted, in which case the query's cache will
 *   be flushed whenever mm_content_clear_caches() is called.
 * @param $query
 *   The SQL query
 * @param ...
 *   A variable number of arguments which are substituted into the query
 *   using printf() syntax. The query arguments can be enclosed in one
 *   array instead.
 *   Valid %-modifiers are: %s, %d, %f, %b (binary data, do not enclose
 *   in '') and %%.
 *
 *   NOTE: using this syntax will cast NULL and FALSE values to decimal 0,
 *   and TRUE values to decimal 1.
 * @return
 *   If this function returns NULL, an error occurred. Otherwise, to obtain each
 *   row, call the returned value's next() function. If next() returns NULL, the
 *   end of the data set has been reached.
 */
function mm_query() {
  $args = func_get_args();
  $mmtid = 0;
  if (is_numeric($args[0])) $mmtid = array_shift($args);
  $query = array_shift($args);
  if (isset($args[0]) && is_array($args[0])) { // 'All arguments in one array' syntax
    $args = $args[0];
  }

  return new mmqIterator($query, $args, $mmtid);
}

/**
 * Start a new DB query, caching the results, or return an object to a
 * previously cached set of values. This version allows a subset of the results
 * to be returned, using the LIMIT clause.
 *
 * @param $mmtid
 *   The optional tree ID of the entry being retrieved. This allows
 *   mm_content_clear_caches() to flush the cache only when needed. This
 *   parameter can be completely omitted, in which case the query's cache will
 *   be flushed whenever mm_content_clear_caches() is called.
 * @param $query
 *   The SQL query
 * @param ...
 *   A variable number of arguments which are substituted into the query
 *   using printf() syntax. The query arguments can be enclosed in one
 *   array instead.
 *   Valid %-modifiers are: %s, %d, %f, %b (binary data, do not enclose
 *   in '') and %%.
 *
 *   NOTE: using this syntax will cast NULL and FALSE values to decimal 0,
 *   and TRUE values to decimal 1.
 * @param $from
 *   The first result row to return.
 * @param $count
 *   The maximum number of result rows to return.
 * @return
 *   If this function returns NULL, an error occurred. Otherwise, to obtain each
 *   row, call the returned value's next() function. If next() returns NULL, the
 *   end of the data set has been reached.
 */
function mm_query_range() {
  $args = func_get_args();
  $mmtid = 0;
  if (is_numeric($args[0])) $mmtid = array_shift($args);
  $count = array_pop($args);
  $from = array_pop($args);
  $query = array_shift($args);
  if (isset($args[0]) && is_array($args[0])) { // 'All arguments in one array' syntax
    $args = $args[0];
  }
  $args[] = $from;
  $args[] = $count;
  $query .= ' LIMIT %d, %d';
  return new mmqIterator($query, $args, $mmtid);
}

/**
 * Start a new DB query, cache and return one result.
 *
 * @param $mmtid
 *   The optional tree ID of the entry being retrieved. This allows
 *   mm_content_clear_caches() to flush the cache only when needed. This
 *   parameter can be completely omitted, in which case the query's cache will
 *   be flushed whenever mm_content_clear_caches() is called.
 * @param $query
 *   The SQL query
 * @param ...
 *   A variable number of arguments which are substituted into the query
 *   using printf() syntax. The query arguments can be enclosed in one
 *   array instead.
 *   Valid %-modifiers are: %s, %d, %f, %b (binary data, do not enclose
 *   in '') and %%.
 *
 *   NOTE: using this syntax will cast NULL and FALSE values to decimal 0,
 *   and TRUE values to decimal 1.
 * @return
 *   The single result field, or NULL on error
 */
function mm_query_result() {
  $args = func_get_args();
  $mmtid = 0;
  if (is_numeric($args[0])) $mmtid = array_shift($args);
  $query = array_shift($args);
  if (isset($args[0]) && is_array($args[0])) { // 'All arguments in one array' syntax
    $args = $args[0];
  }

  if (!($q = new mmqIterator($query, $args, $mmtid)) || !($row = $q->next()))
    return NULL;

  $row = (array)$row;
  return array_shift($row);
}


/**
 * Start a new DB query, cache and return one row as an object.
 *
 * @param $mmtid
 *   The optional tree ID of the entry being retrieved. This allows
 *   mm_content_clear_caches() to flush the cache only when needed. This
 *   parameter can be completely omitted, in which case the query's cache will
 *   be flushed whenever mm_content_clear_caches() is called.
 * @param $query
 *   The SQL query
 * @param ...
 *   A variable number of arguments which are substituted into the query
 *   using printf() syntax. The query arguments can be enclosed in one
 *   array instead.
 *   Valid %-modifiers are: %s, %d, %f, %b (binary data, do not enclose
 *   in '') and %%.
 *
 *   NOTE: using this syntax will cast NULL and FALSE values to decimal 0,
 *   and TRUE values to decimal 1.
 * @return
 *   The single result field, or NULL on error
 */
function mm_query_result_object() {
  $args = func_get_args();
  $mmtid = 0;
  if (is_numeric($args[0])) $mmtid = array_shift($args);
  $query = array_shift($args);
  if (isset($args[0]) && is_array($args[0])) { // 'All arguments in one array' syntax
    $args = $args[0];
  }

  if (!($q = new mmqIterator($query, $args, $mmtid)) || !($row = $q->next()))
    return NULL;

  return $row;
}
