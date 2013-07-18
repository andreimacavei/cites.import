<?php

function cites_importforum_drush_command() {
  $items['import_forum'] = array(
    'description' => 'import forum  from old site to drupal',
    'examples' => array('drush import_forum localhost root root forumDatabase /home/andrei/cites/attachments/'),
    'arguments' => array('host', 'username', 'password', 'database', 'pathToAttachments'),
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
  $items['create_profile2'] = array(
    'description' => 'create a profile2 type',
    'examples' => array('drush create_profile2'),
    'arguments' => array(),
    'required-arguments' => 0,
    'aliases' => array('cprof2'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH
  );
  return $items;
}

function drush_cites_importforum_create_profile2() {
  $profileObj = profile2_type_load('extendedprofile');
  if (field_info_field('extendedprofile_country')) {
  }
  if ($profileObj) {
    profile2_type_delete($profileObj);
  }
  $profileObj = new ProfileType(array(
    'type' => 'extendedprofile',
    'label' => 'Extended Profile',
    'weight' => 0
  ));
  profile2_type_save($profileObj);
  if (field_info_field("extendedprofile_firstname")) {
    field_delete_field("extendedprofile_firstname");
  }
  if (field_info_field("extendedprofile_lastname")) {
    field_delete_field("extendedprofile_lastname");
  }
  if (field_info_field("extendedprofile_organization")) {
    field_delete_field("extendedprofile_organization");
  }
  if (field_info_field("extendedprofile_telephone")) {
    field_delete_field("extendedprofile_telephone");
  }
  if (field_info_field("extendedprofile_country")) {
    field_delete_field("extendedprofile_country");
  }

  $field = array(
    'field_name' => 'extendedprofile_country',
    'type' => 'entityreference',
    'cardinality' => 1,
    'settings' => array(
      'target_type' => 'node',
      'handler_settings' => array('target_bundles' => array('country'))
    ),
    'columns' => array('target_id' => array('#default_value' => 833)),
  );
  field_create_field($field);
  $instance = array(
    'field_name' => 'extendedprofile_country',
    'entity_type' => 'profile2',
    'bundle' => 'extendedprofile',
    'label' => 'Country',
    'widget' => array(
      'type' => 'options_select',
    ),
    'settings' => array(
      'target_type' => 'node',
      'handler_settings' => array('target_bundles' => array('country')),
    ),
    '#default_value' => '701'
  );
  field_create_instance($instance);

  $field = array(
    'field_name' => 'extendedprofile_telephone',
    'type' => 'text',
    'cardinality' => 1,
    'translatable' => FALSE,
  );
  field_create_field($field);
  $instance = array(
    'entity_type' => 'profile2',
    'field_name' => 'extendedprofile_telephone',
    'bundle' => 'extendedprofile',
    'label' => 'Telephone',
    'widget' => array(
      'type' => 'text_textfield',
      'weight' => 0,
    ),
  );
  field_create_instance($instance);

  $field = array(
    'field_name' => 'extendedprofile_organization',
    'type' => 'text',
    'cardinality' => 1,
    'translatable' => FALSE,
  );
  field_create_field($field);
  $instance = array(
    'entity_type' => 'profile2',
    'field_name' => 'extendedprofile_organization',
    'bundle' => 'extendedprofile',
    'label' => 'Organization',
    'widget' => array(
      'type' => 'text_textfield',
      'weight' => 0,
    ),
  );
  field_create_instance($instance);


  $field = array(
    'field_name' => 'extendedprofile_lastname',
    'type' => 'text',
    'cardinality' => 1,
    'translatable' => FALSE,
  );
  field_create_field($field);
  $instance = array(
    'entity_type' => 'profile2',
    'field_name' => 'extendedprofile_lastname',
    'bundle' => 'extendedprofile',
    'label' => 'Lastname',
    'widget' => array(
      'type' => 'text_textfield',
      'weight' => 0,
    ),
  );
  field_create_instance($instance);

  $field = array(
    'field_name' => 'extendedprofile_firstname',
    'type' => 'text',
    'cardinality' => 1,
    'translatable' => FALSE,
  );
  field_create_field($field);
  $instance = array(
    'entity_type' => 'profile2',
    'field_name' => 'extendedprofile_firstname',
    'bundle' => 'extendedprofile',
    'label' => 'Firstname',
    'widget' => array(
      'type' => 'text_textfield',
      'weight' => 0,
    ),
  );
  field_create_instance($instance);
}

function drush_cites_importforum_import_forum($host = 'localhost', $username = 'root', $password = '', $database = 'cites', $pathToAttachments = '/') {
  $taxonomyObj = taxonomy_vocabulary_machine_name_load("Forums");
  // delete previous created forums
  if ($taxonomyObj) {
    foreach (taxonomy_get_tree($taxonomyObj->vid) as $term) {
      taxonomy_term_delete($term->tid);
    }
  }
  $dataBaseObj = DatabaseInterface::getInstance($host, $username, $password, $database);
  $forumObj = new ForumManagement($dataBaseObj, $pathToAttachments);
  // stage I - add users
  $forumObj->addUsers($databaseObj);
  // stage II - add forum content
  $forumObj->addForums();
  // stage III - add custom roles
  $forumObj->addRoles();
  // stage IV - assign roles to users
  $forumObj->assignRolesToUsers();
  // stage V - add subscriptions
  $forumObj->addSubscriptionToUsers();
}

/**
 * command to remove custom roles
 */
function drush_cites_importforum_remove_roles($host = 'localhost', $username = 'root', $password = '', $database = 'cites', $pathToAttachments = '/') {
  $databaseObj = DatabaseInterface::getInstance($host, $username, $password, $database);
  $obj = new ForumManagement($databaseObj, $pathToAttachments);
  $obj->deleteRoles();
}

function drush_cites_importforum_delete_users($host = 'localhost', $username = 'root', $password = '', $database = 'cites', $pathToAttachments = '/') {
  $databaseObj = DatabaseInterface::getInstance($host, $username, $password, $database);
  $obj = new ForumManagement($databaseObj, $pathToAttachments);
  $obj->deleteUsers();
}

?>