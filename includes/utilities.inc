<?php
/**
 * @file
 * Implementation of robertson library reports utility functions.
 */

/**
 * Returns the disk usage for both the private and public files directory.
 * TODO test on large file system to see how this behaves.
 *
 * @return array
 *   An array formatted as such
 *   'public' => 19626
 *   'private' => 0
 */
function upei_roblib_reports_get_local_disk_usage() {
  module_load_include('inc', 'upei_roblib_reports', 'includes/utilities');
  $public = $private = 0;
  if ($wrapper = file_stream_wrapper_get_instance_by_uri('public://')) {
    $public = upei_roblib_reports_get_disk_usage($wrapper);
  }
  if ($wrapper = file_stream_wrapper_get_instance_by_uri('private://')) {
    $private = upei_roblib_reports_get_disk_usage($wrapper);
  }
  $output = array('public' => $public, 'private' => $private);
  return $output;
}

/**
 * Gets the size in bytes used by this object, including datastream revisions.
 *
 * @param $pid
 *   The objects PID
 * @return int
 *   The size in Bytes
 */
function upei_roblib_reports_get_object_size($pid) {
  $size = 0;
  $object = islandora_object_load($pid);
  if (!$object) {
    watchdog('upei_roblib_reports', 'Error loading %object', array('%object' => $pid), WATCHDOG_NOTICE);
    return 0;
  }
  $apim = $object->repository->api->m;
  foreach ($object as $datastream) {
    $ds_history = $apim->getDatastreamHistory($object->id, $datastream->id);
    foreach ($ds_history as $hist) {
      $size += $hist['dsSize'];
    }
  }
  unset($object);
  return $size;
}

/**
 * Returns the number of bytes used by the given file directory.
 *
 * @param DrupalStreamWrapperInterface $wrapper
 * @return array|mixed|string
 */
function upei_roblib_reports_get_disk_usage($wrapper) {
  $realpath = $wrapper->realpath();
  $total = 0;
  if ($realpath !== FALSE) {
    try {
      foreach (new RecursiveIteratorIterator(new RecursiveDirectoryIterator($realpath, FilesystemIterator::SKIP_DOTS)) as $object) {
        $total += $object->getSize();
      }
    }
    catch (Exception $e) {
      // do nothing but continue with other files, this could be a bad symlink
    }
  }
  return $total;
}

/**
 * Convert a given number to bytes from the format given in $from.
 *
 * @param $number
 *   The number to convert to bytes
 * @param $from
 *   How the number was orginally formated (GB, MB etc.)
 * @return mixed
 *   The number now as bytes.
 */
function upei_roblib_reports_convert_to_bytes($number, $from) {
  switch (strtoupper($from)) {
    case "KB":
      return $number * 1024;
    case "MB":
      return $number * pow(1024, 2);
    case "GB":
      return $number * pow(1024, 3);
    case "TB":
      return $number * pow(1024, 4);
    case "PB":
      return $number * pow(1024, 5);
    default:
      return $from;
  }
}

/**
 * Calculates the total storage used by a site (Drupal and Islandora content).
 *
 * @return int/string
 *   The total storage used
 */
function upei_roblib_reports_get_total_storage() {
  $drupal_sizes = variable_get('upei_roblib_reports_drupal_file_size', array(
    'public' => 0,
    'private' => 0
  ));
  $islandora_storage = variable_get('upei_roblib_reports_islandora_size', 0);
  $db_storage = variable_get('upei_roblib_reports_db_size', 0);
  return $islandora_storage + $drupal_sizes['public'] + $drupal_sizes['private'] +
  $db_storage;
}

/**
 * Checks to see if a site is using too much storage, if it is sends an email to admin.
 *
 * Also logs the info in watchdog.
 */
function upei_roblib_reports_check_storage_and_notify() {
  $total_storage_used = upei_roblib_reports_get_total_storage();
  $max_storage_allowed = variable_get('upei_roblib_reports_max_storage', 1000);
  $max_bytes_allowed = upei_roblib_reports_convert_to_bytes($max_storage_allowed, 'GB');
  if ($total_storage_used > $max_bytes_allowed) {
    $account = user_load(1);
    $sitename = variable_get('site_name', 'undefined site name');
    // send message to admin user
    $message = t('Site %sitename is over their allowed storage threshold, they are using %total_storage but their threshold is %max_storage',
      array(
        '%sitename' => $sitename,
        '%total_storage' => format_size($total_storage_used),
        '%max_storage' => format_size($max_bytes_allowed)
      ));
    $params = array(
      'subject' => t('%sitename site is over it\'s storage threshold', array('%sitename' => $sitename)),
      'body' => check_markup($message, 'plain_text'),
    );
    drupal_mail('upei_roblib_reports', 'over_storage', $account->mail, language_default(), $params);
    watchdog('upei_roblib_reports', $message, NULL, WATCHDOG_NOTICE);
  }
}