<?php
// $Id$

/**
 * @file
 * OPML Multi Parser plugin.
 */

/**
 * Feeds parser plugin that parses OPML feeds.
 */
class FeedsOPMLMultiParser extends FeedsParser {

  /**
   * Implementation of FeedsParser::parse().
   */
  public function parse(FeedsImportBatch $batch, FeedsSource $source) {
    include_once(drupal_get_path('module', 'feeds_multifeeds') .'/opmlmulti_parser.inc');
    $result = opmlmulti_parser_parse($batch->getRaw());
    $batch->setTitle($result['title']);
    $batch->setItems($result['items']);
  }

  /**
   * Return mapping sources.
   */
  public function getMappingSources() {
    return array(
      'title' => array(
        'name' => t('Feed title'),
        'description' => t('Title of the feed.'),
      ),
      'xmlurl' => array(
        'name' => t('Feed URL'),
        'description' => t('URL of the feed.'),
      ),
      'type' => array(
        'name' => t('Feed type'),
        'description' => t('Node type of the feed.'),
      ),
    );
  }
}