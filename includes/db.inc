<?php
/**
 * Created by IntelliJ IDEA.
 * User: ppound
 * Date: 2016-09-14
 * Time: 3:27 PM
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