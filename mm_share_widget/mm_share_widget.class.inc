<?php
// $Id: mm_share_widget.class.inc 3024 2009-06-19 18:51:51Z dan $

module_load_include('inc', 'share_widget', 'share_widget.class');

class mm_share_widget extends share_widget {
  var $module = 'mm_share_widget';

  function visible($node, $teaser, $page = TRUE) {
    $visible = parent::visible($node, $teaser, $page) && db_result(db_query('SELECT COUNT(*) FROM {mm_share_widget} WHERE vid = %d AND share_id = %d', $node->vid, $this->share_id)) > 0;
    if ($visible) {
      $settings[] = array(
        'nid' => $node->nid,
        'shareID' => $this->share_id,
        'widget' => $this->render($node, 'content')
      );
      drupal_add_js(array('share' => $settings), 'setting');
    }
    return $visible;
  }
}
