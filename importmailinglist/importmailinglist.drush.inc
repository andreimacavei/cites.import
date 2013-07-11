<?php function importmailinglist_drush_command() {
  $items['importml'] = array(
    'description' => 'import newsletter database from old cites site',
    'examples' => array('drush importml localhost root admin cites_db'),
    'arguments' => array('host', 'username', 'password', 'database'),
    'required-arguments' => 0,
    'aliases' => array(),
    'options'=>array('drupal dependencies'=>'simplenews'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH
  );
  return $items;
}

function drush_importmailinglist_importml($host = 'localhost', $username = 'root', $password = '', $database = 'edw_mailing_list') {
        /* Connect to an ODBC database using driver invocation */
        $dsn = "mysql:dbname=$database;host=$host"; 
       try {
            $db = new PDO($dsn, $username, $password);
        } catch (PDOException $e) {
            drush_print($e->getMessage());
        }
        $db->exec("set names utf8");
        $mlm=new MailingListManagement($db);
        //Import all at once
        $rez=$mlm->import();
        drush_print_r($rez);
        
}

?>
