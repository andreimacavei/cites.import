<?php

/**
 * File for drush commands from Add Consistency module
 */
function cites_addconsistency_drush_command() {
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

  $items['detect_links_for_news'] = array(
    'description' => 'detect links for news',
    'examples' => array('drush detect_links_for_news <path1>'),
    'arguments' => array('sourcePath'),
    'required-arguments' => 0,
    'aliases' => array('dlfn'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH
  );

  return $items;
}

function drush_cites_addconsistency_change_invalid_urls() {
  $consistencyObj = new AddConsistency();
  $consistencyObj->checkUrlsForNodes();
}

function drush_cites_addconsistency_change_node_aliases() {
  $obj = new AddConsistency();
  $obj->changeNodeAliases();
}

function drush_cites_addconsistency_detect_links_for_news($sourcePath) {
  if ($sourcePath[strlen($sourcePath) - 1] != "/") {
    $sourcePath .= "/";
  }
  $files = file_scan_directory($sourcePath, "/(php|html|shtml|htm)/", array(
    'recurse' => TRUE
      )
  );
  $newsFiles = array();
  foreach ($files as $file) {
    if (!strpos($file->uri, "/cop/") && !strpos($file->uri, "/com/")) {
      continue;
    }
    $relativePath = str_replace("/" . basename($file->uri), "", $file->uri);
    $relativePath = str_replace($sourcePath, "", $relativePath);
    $newsFiles[str_replace($sourcePath, "", $file->uri)] = reportLinks(file_get_contents($file->uri), $relativePath);
  }
  print_r($newsFiles);
}

function reportLinks($content, $relativePath) {
  $linksA = getLinksForTag($content, "a", $relativePath);
  $linksImg = getLinksForTag($content, "img", $relativePath);
  return array_merge($linksA, $linksImg);
}

function getLinksForTag($content, $tag, $relativePath) {
  switch ($tag) {
    case "a":
      $attribute = "href";
      break;
    case "img":
      $attribute = "src";
      break;
    default:
      $attribute = "href";
      break;
  }
  $links = array();
  $dom = new DOMDocument();
  libxml_use_internal_errors(true);
  $dom->loadHTML($content);
  libxml_use_internal_errors(false);
  $xpath = new DOMXPath($dom);
  $srcs = $xpath->evaluate("/html/body//$tag");
  for ($i = 0; $i < $srcs->length; $i++) {
    $src = $srcs->item($i);
    $link = $src->getAttribute($attribute);
    if (strpos($link, "javascript") === FALSE) {
      $links[] = preProcessLink($link, $relativePath);
    }
  }
  return $links;
}

function preProcessLink($link, $path) {
  if (strpos($link, "http") !== FALSE) {
    return $link;
  }
  if (strpos($link, "mailto") !== FALSE) {
    return $link;
  }
  if ($link[0] == "/") {
    return $link;
  }
  $linkArray = explode("/", $link);
  $pathArray = explode("/", $path);
  $i = 0;
  $j = 0;
  while ($linkArray[$i] == "..") {
    $i++;
    $j++;
  }
  $resultPathArray = array_slice($pathArray, 0, count($pathArray) - $j);
  $resultLinkArray = array_slice($linkArray, $i);
  return implode("/", array_merge($resultPathArray, $resultLinkArray));
}
