<?php
// $Id$

/**
 * @file
 * Class definition of FeedsMultiFeedNodeProcessor.
 */

/**
 * Creates *feed* nodes from feed items. The difference to FeedsFeedNodeProcessor is
 * that this plugin allows creation of feed nodes of different type.
 */
class FeedsMultiFeedNodeProcessor extends FeedsFeedNodeProcessor {

  /**
   * Override parent::configForm().
   */
  public function configForm(&$form_state) {
    $form = parent::configForm($form_state);
    $form['content_type']['#description'] = t('Choose <strong>default</strong> node type to create from this feed. This can be overwritten if the mapping target "node type" is assigend. Only node types with attached importer configurations are listed here. <strong>Note:</strong> Users with "import !feed_id feeds" permissions will be able to <strong>import</strong> nodes of the content type selected here regardless of the node level permissions. However, users with "clear !feed_id permissions" need to have sufficient node level permissions to delete the imported nodes.', array('!feed_id' => $this->id));
    return $form;
  }

  /**
   * Return available mapping targets.
   */
  public function getMappingTargets() {
    $targets = parent::getMappingTargets() + array(
      'node_type' => array(
        'name' => t('Feed node type'),
        'callback' => 'feeds_multifeeds_set_node_type',
        'description' => t('The node type of the feed node. If the node type does not have an attached importer configuration, it will replaced by the default node type specified in the settings.'),
      ),
    );
    return $targets;
  }

}
  
/**
 * Callback method to set the feed node type.
 * 
 * @see getMappingTargets()
 *
 * @param $node
 *   The target node.
 * @param $field_name
 *   The name of field on the target node to map to.
 * @param $feed_element
 *   The value to be mapped. Should be a feed content type.
 */
function feeds_multifeeds_set_node_type($node, $target, $feed_element) {
  if ($feed_element != $node->type) {
    if (feeds_get_importer_id($feed_element)) {
      $node->type = $feed_element;
      // we need to prepare the node again now its type has changed
      node_object_prepare($node);
    }
  }
}