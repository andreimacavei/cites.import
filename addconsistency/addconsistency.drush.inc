<?php

function addconsistency_drush_command() {
  $items['change_invalid_urls'] = array(
    'description' => 'change all invalid url from content',
    'examples' => array('drush change_invalid_urls'),
    'arguments' => array(),
    'required-arguments' => 0,
    'aliases' => array('ciu'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH
  );
  
  $items['change_node_aliases'] = array(
    'description' => 'change all node aliases and use EntityTranslation Module',
    'examples' => array('drush change_node_aliases'),
    'arguments' => array(),
    'required-arguments' => 0,
    'aliases' => array('cna'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH
  );
  return $items;
}

function drush_addconsistency_change_invalid_urls(){  
  $consistencyObj = new AddConsistency();
  $consistencyObj->checkUrlsForNodes();
}

function drush_addconsistency_change_node_aliases(){
  $obj = new AddConsistency();
  $obj->changeNodeAliases();
}