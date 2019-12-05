<?php

/**
 * @file
 * Drush integration functions for the upei_roblbi_reports module.
 *
 */

/**
 * Implements hook_drush_command().
 */
function upei_roblib_reports_drush_command() {
  $items['upei-roblib-reports-run'] = array(
    'description' => 'Regenerate the Storage usage data.',
    'callback' => 'drush_upei_roblib_reports_run',
    'drupal dependencies' => array('upei_roblib_reports'),
  );
  return $items;
}

/**
 * Regenerate the Islandora Storage report data.
 */
function drush_upei_roblib_reports_run() {
  module_load_include('inc', 'upei_roblib_reports', 'includes/batch');
  module_load_include('inc', 'upei_roblib_reports', 'includes/utilities');
  module_load_include('inc', 'upei_roblib_reports', 'includes/db');
  // Save time to DB in case anything fails we have a last time run.
  // try and prevent multiple cron runs the very first time this is run on
  // large Islandora sites.
  variable_set('upei_roblib_reports_last_execution', time());
  timer_start('upei-roblib-reports');
  // Run the batch process.
  upei_roblib_reports_run_unprogressive_batch('upei_roblib_reports_get_batch');
  // Get Drupal File and DB size
  variable_set('upei_roblib_reports_drupal_file_size', upei_roblib_reports_get_local_disk_usage());
  variable_set('upei_roblib_reports_db_size', upei_roblib_reports_get_db_size());

  $vars = array(
    '@timer' => format_interval(timer_read('upei-roblib-reports') / 1000),
    '@memory-peak' => format_size(memory_get_peak_usage(TRUE)),
  );
  timer_stop('upei-roblib-reports');
  variable_set('upei_roblib_reports_last_execution', time());
  drush_print(dt('UPEI Roblib Report generated in @timer. Peak memory usage: @memory-peak.', $vars));
}

