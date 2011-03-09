<?php
// $Id: mm_sitemap.inc 4216 2010-06-25 18:36:37Z dan $
/**
 * @file
 * Generate an XML sitemap based on Monster Menu permissions.
 */

define('MM_SITEMAP_MAX_LEVEL', 6);
global $_mm_sitemap_exclude_list;
$_mm_sitemap_exclude_list = array(
  '/.system',
  '/go',
  '/launch',
  '/myamherst',
);

/**
 * Use an iterator to generate the sitemap
 */
function mm_sitemap() {
  class dumpIter extends getTreeIterator {
    protected $fp, $file_path, $tree_path;

    public function __construct() {
      $this->file_path = file_directory_path() . '/sitemap.xml';
      $this->fp = fopen($this->file_path . '.temp', 'w');
      if ($this->fp) {
        $this->tree_path = array();
        fwrite($this->fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<urlset xmlns=\"http://www.sitemaps.org/schemas/sitemap/0.9\">\n");
      }
      else {
        watchdog('mm', 'Could not create !path', array('!path' => $this->file_path . '.temp'), WATCHDOG_ERROR);
        exit();
      }
    }

    public function iterate($item) {
      global $base_url, $_mm_sitemap_exclude_list;

      // root level?
      if ($item->level <= 0)
        return 1;     // no error

      // not publicly readable, hidden, or recycled? not the normal menu block?
      if ($item->level > MM_SITEMAP_MAX_LEVEL || !$item->perms['r'] || $item->hidden || $item->perms[IS_RECYCLED] || $item->bid > 1)
        return -1;    // skip children

      $node_name = empty($item->alias) ? $item->mmtid : $item->alias;
      if ($item->level >= 2) {
        array_splice($this->tree_path, $item->level - 2);
        $this->tree_path[] = $node_name;
      }
      $path = '/' . join('/', $this->tree_path);

      // check the exclusions list
      if (array_search($path, $_mm_sitemap_exclude_list) !== FALSE)
        return -1;    // skip children

      // Figure out if the node or container is newer and use that date
      // Comment this out if it is too intensive (not required for the xml spec)
      $mod_time = db_fetch_array(db_query(
        'SELECT r.mtime AS t_changed, MAX(n.changed) AS n_changed '.
        'FROM {mm_tree} t '.
        'LEFT JOIN {mm_tree_revisions} r ON r.vid = t.vid '.
        'LEFT JOIN {mm_node2tree} m ON t.mmtid = m.mmtid '.
        'LEFT JOIN {node} n ON m.nid = n.nid '.
        'WHERE t.mmtid = %d GROUP BY t.mmtid, t_changed',
        $item->mmtid));
      // End of last updated section
      $max_mod_time = max($mod_time['t_changed'], $mod_time['n_changed']);

      fwrite($this->fp, "<url>\n");
      fwrite($this->fp, "\t<loc>" . htmlspecialchars($base_url . $path, ENT_QUOTES) . "</loc>\n");
      if (!is_null($max_mod_time)) {
        fwrite($this->fp, "\t<lastmod>" . date('Y-m-d', $max_mod_time) . "</lastmod>\n");
      }
      // fwrite($this->fp, "\t<priority>".round(1/$item->level, 2)."</priority>\n");
      fwrite($this->fp, "</url>\n");

      return 1;       // no error
    }

    public function finish() {
      fwrite($this->fp, "</urlset>\n");
      fclose($this->fp);
      $source = $this->file_path . '.temp';   // necessary for pass-by-reference
      file_move($source, $this->file_path, FILE_EXISTS_REPLACE);
    }
  }

  $iter = new dumpIter();
  // Use the anonymous user, so permissions tests are valid
  $params = array(
    MM_GET_TREE_FILTER_NORMAL => TRUE,
    MM_GET_TREE_ITERATOR      => $iter,
    MM_GET_TREE_RETURN_BLOCK  => TRUE,
    MM_GET_TREE_RETURN_PERMS  => TRUE,
    MM_GET_TREE_USER          => user_load(0),
  );
  mm_content_get_tree(1, $params);
  $iter->finish();
  exit();
}

function mm_sitemap_show() {
  file_transfer('sitemap.xml', array('Content-Type: text/xml'));
}
