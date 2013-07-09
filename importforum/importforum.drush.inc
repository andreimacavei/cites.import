<?php

function importforum_drush_command() {
  $items['import_forum'] = array(
    'description' => 'import forum  from old site to drupal',
    'examples' => array('drush import_forum localhost root root forumDatabase /home/andrei/cites/attachments/'),
    'arguments' => array('pathToAttachments', 'host', 'username', 'password', 'database'),
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

  $items['delete_users'] = array(
    'description' => 'delete all custom users',
    'examples' => array('drush delete_users'),
    'arguments' => array('host', 'username', 'password', 'database'),
    'required-arguments' => 0,
    'aliases' => array('du'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH
  );
  return $items;
}

function drush_importforum_import_forum($host = 'localhost', $username = 'root', $password = '', $database = 'cites', $pathToAttachments) {
  foreach (taxonomy_get_tree(2) as $term) {
    taxonomy_term_delete($term->tid);
  }
  $dataBaseObj = DatabaseInterface::getInstance($host, $username, $password, $database);
  $forumObj = new ForumManagement($dataBaseObj, $pathToAttachments);
  // stage I - add users
  //$forumObj->addUsers($databaseObj);
  // stage II - add forum content
  //$forumObj->addForums();
  // stage III - add custom roles
  //$forumObj->addRoles();
  // stage IV - assign roles to users
  //$forumObj->assignRolesToUsers();  
  // stage V - add subscriptions
  $forumObj->addSubscriptionToUsers();
  }

/**
 * command to remove custom roles
 */
function drush_importforum_remove_roles() {
  $obj = new ForumManagement();
  $obj->deleteRoles();
}

function drush_importforum_delete_users($host = 'localhost', $username = 'root', $password = '', $database = 'cites') {
  $databaseObj = DatabaseInterface::getInstance($host, $username, $password, $database);
  $obj = new ForumManagement($databaseObj);
  $obj->deleteUsers();
}
