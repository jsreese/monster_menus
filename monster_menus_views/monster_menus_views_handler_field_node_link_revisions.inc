<?php
// $Id: monster_menus_views_handler_field_node_link_revisions.inc 3092 2009-07-22 19:36:35Z dan $

/**
 * Field handler to present a link to a node's revision list.
 */
class monster_menus_views_handler_field_node_link_revisions extends views_handler_field_node_link {
  function construct() {
    parent::construct();
    $this->additional_fields['type'] = array('table' => 'node', 'field' => 'type');
    $this->additional_fields['uid'] = array('table' => 'node', 'field' => 'uid');
    $this->additional_fields['format'] = 'format';
    $this->additional_fields['nid'] = 'nid';
  }

  function query() {
    parent::query();
    $this->query->add_field(NULL, 'COUNT(mm_node_revisions.nid)', 'count_revisions');
    $this->query->add_groupby('mm_node_revisions.nid');
  }

  function access() {
    return user_access('view revisions') || user_access('administer nodes');
  }

  function render($values) {
    if ($values->count_revisions <= 1) {
      return;
    }

    // ensure user has access to edit this node.
    $node = new stdClass();
    $node->nid = $values->{$this->aliases['nid']};
    $node->uid = $values->{$this->aliases['uid']};
    $node->type = $values->{$this->aliases['type']};
    $node->format = $values->{$this->aliases['format']};
    $node->status = 1; // unpublished nodes ignore access control
    if (!node_access('update', $node) || !node_access('view', $node)) {
      return;
    }

    $text = !empty($this->options['text']) ? $this->options['text'] : t('revisions');
    return l($text, "node/$node->nid/revisions");
  }
}

