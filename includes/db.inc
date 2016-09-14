<?php
/**
 * @file
 * database calls
 */

/**
 * Queries for the total size of the database.
 *
 * May only work on mysql with innodb
 *
 * @return int
 *   The size used by the database in bytes.
 */
function upei_roblib_reports_get_db_size(){
  global $databases;
  $result = 0;
  // We are assuming there is only one database
  $database = reset($databases);
  $default = reset($database);
  $db_name = $default['database'];
  $sql_query = 'SELECT SUM(TABLES.data_length + TABLES.index_length) ';
  $sql_query .= 'FROM information_schema.TABLES ';
  $sql_query .= 'WHERE TABLES.table_schema = :dbname ';
  $sql_query .= 'GROUP BY TABLES.table_schema ';
  try {
    $result = db_query($sql_query, array(
      ':dbname' => $db_name,
    ))->fetchField();
  } catch (Exception $e) {
    watchdog('upei_roblib_reports', 'Failed getting the database size', NULL, WATCHDOG_ERROR);
  }
  return $result;
}