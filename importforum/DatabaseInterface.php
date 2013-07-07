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
    $query = "SELECT *
              FROM `users`";
    $result = mysql_query($query);
    return $result;
  }

  public function getUser($userId) {
    $query = "SELECT * 
              FROM `users`
              WHERE `user_id` = $userId";
    $result = mysql_query($query);
    return mysql_fetch_object($result);
  }

  public function getGroupSubscriptions() {
    $query = "SELECT *
              FROM `users_groups`";
    $result = mysql_query($query);
    return $result;
  }
  
  public function getForum($id){
    $query = "SELECT * 
              FROM `groups`
              WHERE `group_id` = $id";
    return mysql_query($query);
  }
  
  

}

?>
