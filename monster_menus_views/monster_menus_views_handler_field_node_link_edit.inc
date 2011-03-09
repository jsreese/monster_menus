<?php
// $Id: monster_menus_views_handler_field_node_link_edit.inc 4216 2010-06-25 18:36:37Z dan $
/**
 * Field handler to present a link node edit which is MM-aware.
 */
class monster_menus_views_handler_field_node_link_edit extends views_handler_field_node_link {
  function construct() {
    parent::construct();
    $this->additional_fields['uid'] = 'uid';
    $this->additional_fields['type'] = 'type';
    $this->additional_fields['format'] = array('table' => 'node_revisions', 'field' => 'format');
  }

  function render($values) {
    // ensure user has access to edit this node.
    $node = new stdClass();
    $node->nid = $values->{$this->aliases['nid']};
    $node->uid = $values->{$this->aliases['uid']};
    $node->type = $values->{$this->aliases['type']};
    $node->format = $values->{$this->aliases['format']};
    $node->status = 1; // unpublished nodes ignore access control
    if (!node_access('update', $node)) {
      return;
    }

    $text = !empty($this->options['text']) ? $this->options['text'] : t('edit');
    $link = "node/$node->nid/edit";
    $mmtids = mm_content_get_by_nid($node->nid);
    if (count($mmtids) == 1) {
      $link = "mm/$mmtids[0]/$link";
    }
    return l($text, $link, array('query' => drupal_get_destination()));
  }
}

