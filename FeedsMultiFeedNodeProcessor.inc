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
class FeedsMultiFeedNodeProcessor extends FeedsNodeProcessor {

  /**
   * Implementation of FeedsProcessor::clear().
   */
  public function clear(FeedsBatch $batch, FeedsSource $source) {
    // Do not support deleting imported items as we would have to delete all
    // items of the content type we imported which may contain nodes that a
    // user created by hand.
    throw new Exception(t('This configuration does not support deleting imported items.'));
  }
  
  /**
   * Execute mapping on an item.
   */
  protected function map($source_item, $target_node) {
    // Prepare node object.
    static $included;
    if (!$included) {
      module_load_include('inc', 'node', 'node.pages');
      $included = TRUE;
    }
    $target_node->feeds = array();
    // Suppress auto import, we may be creating many feeds
    $target_node->feeds['suppress_import'] = TRUE;
    node_object_prepare($target_node); // double object prepare...

    /*
    Assign an aggregated node always to current user.
    @todo This won't work in all cases as the assumption here is that
    import is happening one off when user is logged in. Assumption breaks if
    feed node processor is being used for aggregation on cron time and a
    specific user should still be the owner of the imported feed nodes.
    */
    global $user;
    $target_node->uid = $user->uid;

    // Have parent class do the iterating.
    return parent::map($source_item, $target_node);
  }
  
  /**
   * Override parent::configDefaults().
   */
  public function configDefaults() {
    $defaults = parent::configDefaults();
    $defaults['content_type'] = ''; // reset content type
    return $defaults;
  }
  
  /**
   * Override parent::configForm().
   */
  public function configForm(&$form_state) {
    $form = parent::configForm($form_state);
    
    $feeds = feeds_importer_load_all();
    $types = array();
    foreach ($feeds as $feed) {
      if ($feed->id != $this->id && !empty($feed->config['content_type'])) {
        $types[$feed->config['content_type']] = node_get_types('name', $feed->config['content_type']);
      }
    }
    if (empty($types)) {
      $types[''] = t('No feed node content type available');
    }
    else {
      $types = array(
        '' => t('Select'),
      ) + $types;
    }

    // Overwrite content type
    $form['content_type'] = array(
      '#type' => 'select',
      '#title' => t('Content type'),
      '#description' => t('Choose <strong>default</strong> node type to create from this feed. This can be overwritten if the mapping target "node type" is assigend. Only node types with attached importer configurations are listed here. <strong>Note:</strong> Users with "import !feed_id feeds" permissions will be able to <strong>import</strong> nodes of the content type selected here regardless of the node level permissions. However, users with "clear !feed_id permissions" need to have sufficient node level permissions to delete the imported nodes.', array('!feed_id' => $this->id)),
      '#options' => $types,
      '#default_value' => $this->config['content_type'],
    );
    return $form;
  }

  /**
   * Return available mapping targets.
   */
  public function getMappingTargets() {
    // Note: source is now called url to be consistent with FeedsNodeProcessor
    $targets = parent::getMappingTargets() + array(
      'node_type' => array(
        'name' => t('Feed node type'),
        'callback' => 'feeds_multifeeds_set_node_type',
        'description' => t('The node type of the feed node. If the node type does not have an attached importer configuration, it will replaced by the default node type specified in the settings.'),
      ),
    );
    return $targets;
  }

  /**
   * Override setTargetElement to operate on a target item that is a node.
   */
  public function setTargetElement($target_node, $target_element, $value) {
    parent::setTargetElement($target_node, $target_element, $value);
    if ($target_element == 'url') {
      // Get the class of the feed node importer's fetcher and set the source
      // property. See feeds_nodeapi() how $node->feeds gets stored.
      $class = get_class($this->feedNodeImporter()->fetcher);
      $target_node->feeds[$class]['source'] = $value;
    }
  }
  
  /**
   * Helper for retrieving the importer object for the feed nodes to produce.
   */
  protected function feedNodeImporter() {
   if ($id = feeds_get_importer_id($this->config['content_type'])) {
      return feeds_importer($id);
    }
    else {
      throw new Exception(t('Content type to be created is not a valid Feed content type.'));
    }
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