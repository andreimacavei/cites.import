<?php
/**
 * Implementation of hook_drush_command().
 *
 * @return
 *   An associative array describing available command(s).
 */
function initialize_drush_command() {
  $items = array();

  // The 'initialize' command.
  $items['initialize'] = array(
    'aliases'             => array('init'),
    'arguments'           => array(
      'type' => 'Type of the initialization. Omit this argument to choose from available options.'
    ),
    'bootstrap'           => DRUSH_BOOTSTRAP_DRUPAL_FULL,
    'description'         => 'Initializes your site.',
    'drupal dependencies' => array('context', 'ctools', 'i18n_menu', 'l10n_update', 'locale'),
    'examples'            => array(
      'drush init languages --config-file=config.json' => 'Adds languages defined in the config.json configuration file.'
    ),
    'options'             => array(
      'config-file' => array(
        'description'   => 'Path to the configuration file.',
        'example-value' => 'config.json',
        'value'         => 'optional'
      )
    )
  );

  return $items;
}


/**
 * Implementation of hook_drush_help().
 *
 * @param $section
 *   A string with the help section (prepend with 'drush:').
 *
 * @return
 *   A string with the help text for the command.
 */
function initialize_drush_help($section) {
  switch ($section) {
    case 'drush:initialize':
      return dt('This command will initialize your site.');

    case 'meta:initialize:title':
      return dt('Initialize commands');

    case 'meta:initialize:summary':
      return dt('Initializes your site.');
  }
}


/**
 * All types available for initialization. Contrib commands can alter in their
 * own.
 *
 * @return
 *   An associative array of initialization types.
 */
function drush_initialize_types() {
  $types = array(
    'all'              => 'drush_initialize_all',
    'languages'        => 'drush_initialize_languages',
    'menus'            => 'drush_initialize_menus',
    'contextual-menus' => 'drush_initialize_contextual_menus',
    'blocks'           => 'drush_initialize_blocks'
  );

  // Command files may customize types as desired.
  drush_command_invoke_all_ref('drush_init', $types);

  return $types;
}


/**
 * Command argument complete callback. Provides argument values for shell
 * completion.
 *
 * @return
 *   An associative array of initialization types.
 */
function initialize_initialize_complete() {
  return array('values' => array_keys(drush_initialize_types()));
}


/**
 * Calls all available initializations.
 *
 * @param $config
 *   An associative array with the configuration data.
 *
 * @return
 *   Nothing.
 */
function drush_initialize_all($config) {
  // Retrieves all types available for initialization.
  $types = drush_initialize_types();

  unset($types['all']);

  foreach ($types as $callback)
    call_user_func($callback, $config);
}


/**
 * Sanitizes a language definition.
 *
 * @param $language
 *   An associative array describing a language.
 *
 * @return
 *   An associative array with the sanitized language.
 */
function drush_initialize_sanitize_language($language) {
  if (!isset($language['langcode']))
    $language['name'] = NULL;

  if (!isset($language['name']))
    $language['name'] = NULL;

  if (!isset($language['native']))
    $language['native'] = NULL;

  if (!isset($language['direction']) || $language['direction'] == 'LANGUAGE_LTR')
    $language['direction'] = LANGUAGE_LTR;
  else
    $language['direction'] = LANGUAGE_RTL;

  if (!isset($language['domain']))
    $language['domain'] = '';

  if (!isset($language['prefix']))
    $language['prefix'] = '';

  if (!isset($language['enabled']))
    $language['enabled'] = TRUE;

  if (!isset($language['default']))
    $language['default'] = FALSE;

  return $language;
}


/**
 * Initializes languages.
 *
 * @param $config
 *   An associative array with the configuration data.
 */
function drush_initialize_languages($config) {
  // Returns a list of installed languages, indexed by the specified key.
  $languages = language_list();

  foreach ($config['languages'] as $index => $language) {
    // Sanitizes the language definition.
    $language = drush_initialize_sanitize_language($language);

    if (array_key_exists($language['langcode'], $languages)) {
      drush_log(dt('!langcode: Language already exists', array('!langcode' => $language['langcode'])), 'warning');

      $properties = array('name', 'native', 'direction', 'domain', 'prefix');

      $options = array();

      // Keeps only valid properties that are defined in the configuration file.
      foreach (array_keys($config['languages'][$index]) as $key) {
        if (in_array($key, $properties))
          $options[$key] = $language[$key];
      }

      if (!empty($options)) {
        // Returns a new UpdateQuery object for the active database.
        db_update('languages')->fields($options)->condition('language', $language['langcode'])->execute();
      }

      drush_log(dt('!langcode: Language successfully updated', array('!langcode' => $language['langcode'])), 'ok');

      continue;
    }

    // Predefined language selection.
    include_once DRUPAL_ROOT . '/includes/iso.inc';

    $predefined = _locale_get_predefined_list();

    if (!isset($predefined[$language['langcode']])) {
      drush_log(dt('!langcode: Invalid language code', array('!langcode' => $language['langcode'])), 'warning');

      continue;
    }

    // Adds the language definition.
    locale_add_language(
      $language['langcode'],
      $language['name'],
      $language['native'],
      $language['direction'],
      $language['domain'],
      $language['prefix'],
      $language['enabled'],
      $language['default']
    );

    // Checks whether there are language files to import for the newly added
    // language, collects and imports them.
    if ($batch = locale_batch_by_language($language['langcode'], '_locale_batch_language_finished')) {
      // Adds a new batch.
      batch_set($batch);

      // Retrieves the current batch.
      $batch =& batch_get();

      $batch['progressive'] = FALSE;

      // Processes the batch.
      drush_backend_batch_process();
    }

    drush_log(dt('!langcode: Language successfully added', array('!langcode' => $language['langcode'])), 'ok');
  }

  // Invoke drush api calls, including all hooks.
  drush_invoke('l10n-update');
}


/**
 * Sanitizes a menu definition.
 *
 * @param $menu
 *   An associative array describing a menu.
 *
 * @return
 *   An associative array with the sanitized menu.
 */
function drush_initialize_sanitize_menu($menu) {
  if (!isset($menu['i18n_mode']))
    $menu['i18n_mode'] = I18N_MODE_NONE;
  else
    $menu['i18n_mode'] = defined($menu['i18n_mode']) ? constant($menu['i18n_mode']) : I18N_MODE_NONE;

  return $menu;
}


/**
 * Initializes links.
 *
 * @param $menu_name
 *   A string with the menu name.
 * @param $links
 *   An associative array describing a links hierarchy.
 * @param $plid
 *   An integer with the menu link identifier of the parent.
 *
 * @return
 *   Nothing.
 */
function drush_initialize_links($menu_name, $links, $plid = 0) {
  $weight = -50;

  foreach ($links as $link) {
    $link['menu_name'] = $menu_name;
    $link['plid']      = $plid;

    if (!isset($link['link_path']) || empty($link['link_path']))
      continue;

    if (!isset($link['link_title']) || empty($link['link_title']))
      continue;

    $link['external'] = url_is_external($link['link_path']) || $link['link_path'] == '<front>' ? 1 : 0;

    if (!$link['external']) {
      $link['link_path']   = drupal_get_normal_path($link['link_path']);
      $link['router_path'] = _menu_find_router_path($link['link_path']);

      if (!$link['link_path'] || !$link['router_path']) {
        drush_log(
          dt(
            '!link-path: Invalid path for \'!link-title\'',
            array(
              '!link-path'  => $link['link_path'],
              '!link-title' => $link['link_title']
            )
          ),
          'warning'
        );

        continue;
      }
    }

    $link['weight']     = $weight++;
    $link['customized'] = 1;

    if (isset($link['translations'])) {
      $translations = $link['translations'];

      unset($link['translations']);
    }

    if (isset($link['links'])) {
      $children = $link['links'];

      unset($link['links']);
    }

    // Saves a menu link.
    $mlid = menu_link_save($link);

    if (!$mlid)
      continue;

    if (isset($translations)) {
      foreach ($translations as $langcode => $translation) {
        // Updates / creates translation for a certain source.
        i18n_string_translation_update('menu:item:' . $mlid . ':title', $translation, $langcode);
      }

      unset($translations);
    }

    if (!isset($children) || empty($children))
      continue;

    // Initializes links.
    drush_initialize_links($menu_name, $children, $mlid);

    unset($children);
  }
}


/**
 * Initializes menus.
 *
 * @param $config
 *   An associative array with the configuration data.
 *
 * @return
 *   Nothing.
 */
function drush_initialize_menus($config) {
  foreach ($config['menus'] as $menu) {
    $menu = drush_initialize_sanitize_menu($menu);

    if (!isset($menu['menu_name']) || empty($menu['menu_name'])) {
      drush_log(dt('!menu-name: \'menu_name\' cannot be null', array('!menu-name' => $menu['menu_name'])), 'warning');

      continue;
    }

    if (!isset($menu['title']) || empty($menu['title'])) {
      drush_log(dt('!menu-name: \'title\' cannot be null', array('!menu-name' => $menu['menu_name'])), 'warning');

      continue;
    }

    if (isset($menu['translations'])) {
      $translations = $menu['translations'];

      unset($menu['translations']);
    }

    if (isset($menu['links'])) {
      $links = $menu['links'];

      unset($menu['links']);
    }

    // Saves a custom menu.
    menu_save($menu);

    if (isset($translations)) {
      foreach ($translations as $langcode => $translation) {
        // Updates / creates translation for a certain source.
        i18n_string_translation_update('menu:menu:' . $menu['menu_name'] . ':title', $translation, $langcode);
      }

      unset($translations);
    }

    drush_log(dt('!menu-name: Menu successfully saved', array('!menu-name' => $menu['menu_name'])), 'ok');

    if (!isset($links) || empty($links))
      continue;

    // Deletes all links for a menu.
    menu_delete_links($menu['menu_name']);

    // Initializes links.
    drush_initialize_links($menu['menu_name'], $links);

    drush_log(dt('!menu-name: Links successfully saved', array('!menu-name' => $menu['menu_name'])), 'ok');
  }
}


/**
 * Initializes contextual menus.
 *
 * @param $config
 *   An associative array with the configuration data.
 *
 * @return
 *   Nothing.
 */
function drush_initialize_contextual_menus($config) {
  $vocabulary = (object) array(
    'name'         => t('Contextual menus'),
    'description'  => NULL,
    'machine_name' => 'contextual_menus',
    'help'         => NULL
  );

  taxonomy_vocabulary_save($vocabulary);

  $vocabulary->vid = db_query('SELECT vid FROM {taxonomy_vocabulary} WHERE machine_name = :machine_name', array(':machine_name' => $vocabulary->machine_name))->fetchField();

  foreach ($config['contextual_menus'] as $name) {
    $term = array(
      'name'        => t($name),
      'description' => '',
      'parent'      => array(0),
      'vid'         => $vocabulary->vid,
    );

    $term = (object) $term;

    taxonomy_term_save($term);
  }

  $field = array(
    'field_name'  => 'field_' . $vocabulary->machine_name,
    'type'        => 'taxonomy_term_reference',
    'cardinality' => FIELD_CARDINALITY_UNLIMITED,
    'settings'    => array(
      'allowed_values' => array(
        array(
          'vocabulary' => $vocabulary->machine_name,
          'parent'     => 0
        )
      )
    )
  );

  field_create_field($field);

  $bundles = array('article', 'page');

  foreach ($bundles as $bundle) {
    $instance = array(
      'field_name'  => 'field_' . $vocabulary->machine_name,
      'entity_type' => 'node',
      'label'       => 'Contextual menus',
      'bundle'      => $bundle,
      'description' => $vocabulary->help,
      'widget'      => array(
        'type'   => 'options_select',
        'weight' => -4
      ),
      'display'     => array(
        'default' => array(
          'type'   => 'hidden',
          'weight' => 10
        ),
        'teaser'  => array(
          'type'   => 'hidden',
          'weight' => 10
        )
      )
    );

    field_create_instance($instance);
  }

  foreach ($config['contextual_menus'] as $menu_name => $name) {
    $tid = db_query('SELECT tid FROM {taxonomy_term_data} WHERE vid = :vid AND name = :name', array(':vid' => $vocabulary->vid, ':name' => $name))->fetchField();

    ctools_include('export');

    $context = ctools_export_new_object('context');

    $context->name           = t($name);
    $context->tag            = t('Contextual menus');
    $context->conditions     = array('node_taxonomy' => array('values' => array($tid => $tid)));
    $context->reactions      = array(
      'block' => array(
        'blocks' => array(
          'menu-' . $menu_name => array(
            'module' => 'menu',
            'delta'  => $menu_name,
            'region' => 'sidebar_first',
            'weight' => '-10'
          )
        )
      )
    );
    $context->condition_mode = 1;

    context_save($context);
  }
}


/**
 * Sanitizes a block definition.
 *
 * @param $block
 *   An associative array describing a block.
 *
 * @return
 *   An associative array with the sanitized block.
 */
function drush_initialize_sanitize_block($block) {
  global $theme_key;

  if (isset($block['bid']))
      unset($block['bid']);

  if (!isset($block['theme']) || empty($block['theme']))
    $block['theme'] = $theme_key;

  if (!isset($block['status']) || $block['status'] != 0)
    $block['status'] = 1;

  if (isset($block['pages']) && is_array($block['pages']))
    $block['pages'] = implode(PHP_EOL, $block['pages']);

  return $block;
}


/**
 * Initializes blocks.
 *
 * @param $config
 *   An associative array with the configuration data.
 *
 * @return
 *   Nothing.
 */
function drush_initialize_blocks($config) {
  foreach ($config['blocks'] as $block) {
    $block = drush_initialize_sanitize_block($block);

    if (!isset($block['module']) || empty($block['module'])) {
      drush_log(dt('!module: \'module\' cannot be null', array('!module' => $block['module'])), 'warning');

      continue;
    }

    if (!isset($block['delta']) || empty($block['delta'])) {
      drush_log(dt('!delta: \'delta\' cannot be null', array('!delta' => $block['delta'])), 'warning');

      continue;
    }

    // Returns a new UpdateQuery object for the active database.
    db_update('block')->fields($block)->condition('module', $block['module'])->condition('delta', $block['delta'])->condition('theme', $block['theme'])->execute();

    drush_log(dt('!module: !delta: Block successfully saved', array('!module' => $block['module'], '!delta' => $block['delta'])), 'ok');
  }
}


/**
 * Command callback for drush initialize.
 *
 * @see drush_invoke()
 * @see drush.api.php
 *
 * @param $type
 *   A string with the type of the initialization.
 *
 * @return
 *   Nothing.
 */
function drush_initialize($type = NULL) {
  // Gets the value for the configuration file option.
  $config_file = drush_get_option('config-file', dirname(__FILE__) . '/config.json');

  if (!file_exists($config_file))
    return drush_set_error('ERROR_FILE_NOT_FOUND', dt('!config-file: No such file or directory', array('!config-file' => $config_file)));

  $config = json_decode(file_get_contents($config_file), TRUE);

  switch (json_last_error()) {
    case JSON_ERROR_DEPTH:
      return drush_set_error('ERROR_JSON_DEPTH', dt('!config-file: The maximum stack depth has been exceeded', array('!config-file' => $config_file)));

    case JSON_ERROR_STATE_MISMATCH:
      return drush_set_error('ERROR_JSON_STATE_MISMATCH', dt('!config-file: Invalid or malformed JSON', array('!config-file' => $config_file)));

    case JSON_ERROR_CTRL_CHAR:
      return drush_set_error('ERROR_JSON_CTRL_CHAR', dt('!config-file: Control character error, possibly incorrectly encoded', array('!config-file' => $config_file)));

    case JSON_ERROR_SYNTAX:
      return drush_set_error('ERROR_JSON_SYNTAX', dt('!config-file: Syntax error', array('!config-file' => $config_file)));

    case JSON_ERROR_UTF8:
      return drush_set_error('ERROR_JSON_UTF8', dt('!config-file: Malformed UTF-8 characters, possibly incorrectly encoded', array('!config-file' => $config_file)));

    default:
      break;
  }

  // Retrieves all types available for initialization.
  $types = drush_initialize_types();

  // Validates provided initialization type.
  if ($type && !key_exists($type, $types))
    return drush_set_error('ERROR_INVALID_TYPE', dt('!type: Invalid initialization type.', array('!type' => $type)));

  if ($type) {
    // Calls a given function, passing through all arguments unchanged.
    drush_op($types[$type], $config);
  } else {
    // Asks the user to select an item from a list.
    $type = drush_choice($types, 'Enter a number to choose the initialization type.', '!key');

    if ($type !== FALSE)
      call_user_func($types[$type], $config);
  }

  if ($type !== FALSE) {
    // Gets the name of the current bootstrapped site.
    $site_name = drush_sitealias_bootstrapped_site_name();

    drush_log(dt('!site-name: \'!type\' successfully initialized', array('!site-name' => $site_name, '!type' => $type)), 'success');
  }
}
