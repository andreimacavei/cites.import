<?php

/**
 * Actual drush command implementation.
 * Function name is modulename_drush_command()
 */
function importdata_drush_command() {
  $items['import_static'] = array(
    'description' => 'import static files from old site to drupal',
    'examples' => array('drush import_static /var/www/site'),
    'arguments' => array('source'),
    'required-arguments' => 1,
    'aliases' => array('is'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH
  );

  $items['import_html'] = array(
    'description' => 'import .html,.php,.shtml files from old site to drupal',
    'examples' => array('drush import_html /var/www/site'),
    'arguments' => array('source'),
    'required-arguments' => 1,
    'aliases' => array('ih'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );
  
  $items['clear_all_files'] = array(
    'description' => 'delete all files from public://',
    'examples' => array('drush clear_all_files'),
    'arguments' => array(0),
    'required-arguments' => 0,
    'aliases' => array("caf"),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH
  );
  return $items;
}

/**
 * This is the callback function for drush task 'import_static'
 * @param $path - absolute path to directory where static files are located
 */
function drush_importdata_import_static($path) {
  $obj = new DataImport();
  $obj->copyStaticFiles($path);
}

/**
 * This is the callback function for drush task 'import_html'
 * @param $path - absolute path to directory where html files are located
 */
function drush_importdata_import_html($path) {
  $obj = new DataImport();
  $obj->copyHtmlFiles($path);
}

/**
 * This is the callback function for task 'clear_all_files'
 */
function drush_importdata_clear_all_files(){
  file_unmanaged_delete_recursive("public://");
  $query = 'DELETE FROM `file_managed` WHERE 1 > 0';
  db_query($query);
}
