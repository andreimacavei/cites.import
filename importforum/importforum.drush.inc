<?php

function importforum_drush_command() {
  $items['import_forum'] = array(
    'description' => 'import forum  from old site to drupal',
    'examples' => array('drush import_forum localhost root root forumDatabase'),
    'arguments' => array('host', 'username', 'password', 'database'),
    'required-arguments' => 0,
    'aliases' => array('if'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH
  );
  
  $items['remove_roles'] = array(
    'description' => 'delete all custom roles',
    'examples' => array('drush remove_roles'),
    'arguments' => array(),
    'required-arguments' => 0,
    'aliases' => array('rr'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH
  );
  return $items;
}

function drush_importforum_import_forum($host = 'localhost', $username = 'root', $password = '', $database = 'cites.forum') { 
  foreach (taxonomy_get_tree(2) as $term) {   taxonomy_term_delete($term->tid); }
  
  
  $dataBaseObj = DatabaseInterface::getInstance('localhost', 'root','andrei','cites.forum');
  $forumObj = new ForumManagement($dataBaseObj);
  // stage I - add users
  //$forumObj->addUsers($databaseObj);
  $forumObj->addForums();
  //$forumObj->createRoles();
}

/**
 * command to remove custom roles
 */
function drush_importforum_remove_roles(){
  $obj = new ForumManagement();
  $obj->deleteRoles();
}
