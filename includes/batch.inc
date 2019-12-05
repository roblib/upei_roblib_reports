<?php

/**
 * @file
 * Batch definition and callbacks.
 */

/**
 * Get the our basic batch structure.
 *
 * @param int $jump
 *   The number of records to grab each iteration. Defaults to 100.
 * @param int $cutoff
 *   The number of records to process per batch. -1 means to attempt to process
 *   ALL records.
 *
 * @return array
 *   An array representing the batch.
 */
function upei_roblib_reports_get_batch($jump = 100, $cutoff = -1, $account = NULL) {
  $path = drupal_get_path('module', 'upei_roblib_reports');

  $batch = array(
    'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaining @estimate.'),
    'operations' => array(
      array('upei_roblib_reports_batch_operation', array($jump, $cutoff, $account)),
    ),
    'file' => "$path/includes/batch.inc",
  );
  return $batch;
}

/**
 * Batch operation.
 *
 * Grab a number of records from Solr at a time, until we hit the cutoff
 * number.
 *
 * @param int $jump
 *   The number of records to grab each iteration.
 * @param int $cutoff
 *   The number of records to process per batch.
 */
function upei_roblib_reports_batch_operation($jump, $cutoff, $account, &$context) {
  $sandbox =& $context['sandbox'];

  if (!isset($sandbox['offset'])) {
    $sandbox['offset'] = 0;

    $sandbox['last_pid'] = variable_get('upei_roblib_reports_last_pid', NULL);

    $qp = $sandbox['query_processor'] = new IslandoraSolrQueryProcessor();
    $qp->solrLimit = $cutoff > 0 ?
      min($jump, $cutoff) :
      $jump;
    $qp->solrQuery = 'PID:*';
    $qp->solrParams['sort'] = "PID asc";
    $qp->solrParams['fq'] = array();
    $qp->solrParams['fl'] = 'PID';

    $namespaces_enforced = variable_get('islandora_namespace_restriction_enforced', FALSE);
    if ($namespaces_enforced) {
      $namespace_map = function ($namespace) {
        return 'PID:' . Apache_Solr_Service::escape("$namespace:") . '*';
      };
      module_load_include('inc', 'islandora', 'includes/utilities');
      $qp->solrParams['fq'][] = implode(' OR ', array_map($namespace_map, islandora_get_allowed_namespaces()));
    }
  }
  else {
    $sandbox['offset'] += $jump;
    $qp = $sandbox['query_processor'];
  }

  $qp->solrStart = $sandbox['offset'];

  // Query for $count PIDs, starting from $offset.
  $qp->executeQuery(FALSE, TRUE);
  $results = $qp->islandoraSolrResult;

  $sandbox['total'] = $cutoff > 0 ?
    min((int) $results['response']['numFound'], $cutoff) :
    (int) $results['response']['numFound'];
  if ($results === NULL || $sandbox['total'] === 0) {
    $context['message'] = t('No results selected, or error...');
    // Stash the most current value, so we can hopefully pick up where we left
    // off last time.
    variable_set('upei_roblib_reports_last_pid', $sandbox['last_pid']);
    return;
  }
  if(empty($sandbox['size'])) {
    $sandbox['size'] = 0;
  }
  foreach ($results['response']['objects'] as $result) {
    $sandbox['size'] += upei_roblib_reports_get_object_size($result['PID'], $account);
    $sandbox['last_pid'] = $result['PID'];
  }
  $qp->resetResults();

  $context['message'] = t('Processed @count of @total.', array(
    '@count' => min($sandbox['offset'] + $jump, $sandbox['total']),
    '@total' => $sandbox['total'],
  ));
  $context['finished'] = $sandbox['offset'] / $sandbox['total'];
  if ($context['finished'] >= 1) {
    variable_set('upei_roblib_reports_last_pid', $sandbox['last_pid']);
    variable_set('upei_roblib_reports_islandora_size', $sandbox['size']);
  }
}

/**
 * Run a  batch operation.
 * Borrowed from xmlsitemap module.
 */
function upei_roblib_reports_run_unprogressive_batch() {
  $batch = batch_get();
  if (!empty($batch)) {
    // If there is already something in the batch, don't run.
    return FALSE;
  }

  $args = func_get_args();
  $batch_callback = array_shift($args);

  if (!lock_acquire($batch_callback)) {
    return FALSE;
  }

  // Attempt to increase the execution time.
  drupal_set_time_limit(0);

  // Build the batch array.
  $batch = call_user_func_array($batch_callback, $args);
  batch_set($batch);

  // We need to manually set the progressive variable again.
  // @todo Remove when http://drupal.org/node/638712 is fixed.
  $batch =& batch_get();
  $batch['progressive'] = FALSE;

  // Run the batch process.
  batch_process();

  lock_release($batch_callback);
  return TRUE;
}
