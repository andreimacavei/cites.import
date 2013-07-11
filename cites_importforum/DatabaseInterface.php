<?php

/**
 * Class as an interface with the cites forum database
 */
class DatabaseInterface {

  private static $dbObj = null;

  private function __construct($host, $username, $password, $database) {
    mysql_connect($host, $username, $password) or die('Error connecting to the database server \n');
    mysql_select_db($database) or die('Error connecting to the database \n');
  }

  public static function getInstance($host, $username, $password, $database) {
    if (DatabaseInterface::$dbObj == null)
      DatabaseInterface::$dbObj = new DatabaseInterface($host, $username, $password, $database);
    return DatabaseInterface::$dbObj;
  }

  public function getForums() {
    $query = "SELECT DISTINCT * 
              FROM `groups`
              ";
    $result = mysql_query($query);
    return $result;
  }
  public function getTopic($topicId){
    $query = "SELECT * 
              FROM `categs`
              WHERE `categ_id` = $topicId";
    return mysql_query($query);
  }
  public function getTopics($forumId) {
    $query = "SELECT * 
              FROM `categs`
              WHERE `group_id` = '$forumId'
              ";
    $result = mysql_query($query);
    return $result;
  }

  public function getPosts($topicId) {
    $query = "SELECT * 
              FROM `posts`
              WHERE `categ_id` = $topicId
              ORDER BY `data` ASC
              ";
    $result = mysql_query($query);
    return $result;
  }

  public function getPostsFiles($postId) {
    $query = "SELECT * 
              FROM `posts_files`
              WHERE `post_id` = $postId";
    $result = mysql_query($query);
    return $result;
  }

  public function getUsers() {
    $query = "SELECT * FROM `users` 
              WHERE `email` NOT LIKE '' 
              GROUP BY `email`";
    $result = mysql_query($query);
    return $result;
  }

  public function getUser($userId) {
    $query = "SELECT * 
              FROM `users`
              WHERE `user_id` = $userId";
    $result = mysql_query($query);
    return $result;
  }

  public function getGroupSubscriptions() {
    $query = "SELECT *
              FROM `users_groups`";
    $result = mysql_query($query);
    return $result;
  }

  public function getForum($id) {
    $query = "SELECT * 
              FROM `groups`
              WHERE `group_id` = $id";
    return mysql_query($query);
  }
  
  public function getStrangeUsers(){
    $query = "SELECT * 
              FROM `users` 
              WHERE `email` != ''
              AND
              `user_id` NOT IN 
              (SELECT `user_id` 
              FROM `users` 
              GROUP BY `email`
              )";
    return mysql_query($query);
  }
  
  public function getAllUsers(){
    $query = "SELECT * 
              FROM `users`
              WHERE `email` != ''";
    return mysql_query($query);
  }

}

?>