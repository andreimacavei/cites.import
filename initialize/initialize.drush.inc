<?php
/**
 * Implementation of hook_drush_command().
 *
 * @return array An associative array describing available command(s).
 */
function initialize_drush_command() {
	$items = array();

	// The 'initialize' command.
	$items['initialize'] = array(
		'aliases'             => array( 'init' ),
		'arguments'           => array(
			'type' => 'Type of the initialization. Omit this argument to choose from available options.'
		),
		'bootstrap'           => DRUSH_BOOTSTRAP_DRUPAL_FULL,
		'description'         => 'Initializes your site.',
		'drupal dependencies' => array( 'locale' ),
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
 * @param string $section A string with the help section (prepend with 'drush:').
 *
 * @return string A string with the help text for the command.
 */
function initialize_drush_help( $section ) {
	switch ( $section ) {
		case 'drush:initialize':
			return dt( 'This command will initialize your site.' );

		case 'meta:initialize:title':
			return dt( 'Initialize commands' );

		case 'meta:initialize:summary':
			return dt( 'Initializes your site.' );
	}
}

/**
 * All types available for initialization. Contrib commands can alter in their
 * own.
 *
 * @return array An associative array of initialization types.
 */
function drush_initialize_types() {
	$types = array(
		'all'       => 'drush_initialize_all',
		'languages' => 'drush_initialize_languages',
		'menus'     => 'drush_initialize_menus'
	);

	// Command files may customize types as desired.
	drush_command_invoke_all_ref( 'drush_init', $types );

	return $types;
}


/**
 * Command argument complete callback. Provides argument values for shell
 * completion.
 *
 * @return array An associative array of initialization types.
 */
function initialize_initialize_complete() {
	return array( 'values' => array_keys( drush_initialize_types() ) );
}

/**
 * Calls all available initializations.
 *
 * @param $config An associative array with the configuration data.
 */
function drush_initialize_all( $config ) {
	// Retrieves all types available for initialization.
	$types = drush_initialize_types();

	unset( $types['all'] );

	foreach ( $types as $callback )
		call_user_func( $callback, $config );
}

/**
 * Sanitizes a language definition.
 *
 * @param array $language An associative array describing a language.
 *
 * @return array An associative array with the sanitized language.
 */
function drush_initialize_sanitize_language( $language ) {
	if ( ! isset( $language['langcode'] ) )
		$language['name'] = NULL;

	if ( ! isset( $language['name'] ) )
		$language['name'] = NULL;

	if ( ! isset( $language['native'] ) )
		$language['native'] = NULL;

	if ( ! isset( $language['direction'] ) || $language['direction'] == 'LANGUAGE_LTR' )
		$language['direction'] = LANGUAGE_LTR;
	else
		$language['direction'] = LANGUAGE_RTL;

	if ( ! isset( $language['domain'] ) )
		$language['domain'] = '';

	if ( ! isset( $language['prefix'] ) )
		$language['prefix'] = '';

	if ( ! isset( $language['enabled'] ) )
		$language['enabled'] = TRUE;

	if ( ! isset( $language['default'] ) )
		$language['default'] = FALSE;

	return $language;
}

/**
 * Initializes languages.
 *
 * @param $config An associative array with the configuration data.
 */
function drush_initialize_languages( $config ) {
	// Returns a list of installed languages, indexed by the specified key.
	$languages = language_list();

	foreach ( $config['languages'] as $index => $language ) {
		// Sanitizes the language definition.
		$language = drush_initialize_sanitize_language( $language );

		if ( array_key_exists( $language['langcode'], $languages ) ) {
			drush_log( dt( '!langcode: Language already exists', array( '!langcode' => $language['langcode'] ) ), 'warning' );

			$properties = array( 'name', 'native', 'direction', 'domain', 'prefix' );

			$options = array();

			// Keeps only valid properties that are defined in the configuration
			// file.
			foreach ( array_keys( $config['languages'][ $index ] ) as $key ) {
				if ( in_array( $key, $properties ) )
					$options[ $key ] = $language[ $key ];
			}

			if ( ! empty( $options ) ) {
				// Returns a new UpdateQuery object for the active database.
				db_update( 'languages' )->fields( $options )->condition( 'language', $language['langcode'] )->execute();
			}

			drush_log( dt( '!langcode: Language successfully updated', array( '!langcode' => $language['langcode'] ) ), 'ok' );

			continue;
		}

		// Predefined language selection.
		include_once DRUPAL_ROOT . '/includes/iso.inc';

		$predefined = _locale_get_predefined_list();

		if ( ! isset( $predefined[ $language['langcode'] ] ) ) {
			drush_log( dt( '!langcode: Invalid language code', array( '!langcode' => $language['langcode'] ) ), 'warning' );

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
		if ( $batch = locale_batch_by_language( $language['langcode'], '_l10n_update_batch_finished' ) ) {
			// Adds a new batch.
			batch_set( $batch );

			// Retrieves the current batch.
			$batch =& batch_get();

			$batch['progressive'] = FALSE;

			// Processes the batch.
			drush_backend_batch_process();
		}

		drush_log( dt( '!langcode: Language successfully added', array( '!langcode' => $language['langcode'] ) ), 'ok' );
	}
}

/**
 * Initializes menus.
 *
 * @param $config An associative array with the configuration data.
 */
function drush_initialize_menus( $config ) {

}

/**
 * Command callback for drush initialize.
 *
 * @see drush_invoke()
 * @see drush.api.php
 *
 * @param string $type A string with the type of the initialization.
 */
function drush_initialize( $type = NULL ) {
	// Gets the value for the configuration file option.
	$config_file = drush_get_option( 'config-file', dirname( __FILE__ ) . '/config.json' );

	if ( ! file_exists( $config_file ) )
		return drush_set_error( 'ERROR_FILE_NOT_FOUND', dt( '!config-file: No such file or directory', array( '!config-file' => $config_file ) ) );

	$config = json_decode( file_get_contents( $config_file ), TRUE );

	switch ( json_last_error() ) {
		case JSON_ERROR_DEPTH:
			return drush_set_error( 'ERROR_JSON_DEPTH', dt( '!config-file: The maximum stack depth has been exceeded', array( '!config-file' => $config_file ) ) );

		case JSON_ERROR_STATE_MISMATCH:
			return drush_set_error( 'ERROR_JSON_STATE_MISMATCH', dt( '!config-file: Invalid or malformed JSON', array( '!config-file' => $config_file ) ) );

		case JSON_ERROR_CTRL_CHAR:
			return drush_set_error( 'ERROR_JSON_CTRL_CHAR', dt( '!config-file: Control character error, possibly incorrectly encoded', array( '!config-file' => $config_file ) ) );

		case JSON_ERROR_SYNTAX:
			return drush_set_error( 'ERROR_JSON_SYNTAX', dt( '!config-file: Syntax error', array( '!config-file' => $config_file ) ) );

		case JSON_ERROR_UTF8:
			return drush_set_error( 'ERROR_JSON_UTF8', dt( '!config-file: Malformed UTF-8 characters, possibly incorrectly encoded', array( '!config-file' => $config_file ) ) );

		default:
			break;
	}

	// Retrieves all types available for initialization.
	$types = drush_initialize_types();

	// Validates provided initialization type.
	if ( $type && ! key_exists( $type, $types ) )
		return drush_set_error( 'ERROR_INVALID_TYPE', dt( '!type: Invalid initialization type.', array( '!type' => $type ) ) );

	if ( $type ) {
		// Calls a given function, passing through all arguments unchanged.
		drush_op( $types[ $type ], $config );
	} else {
		// Asks the user to select an item from a list.
		$type = drush_choice( $types, 'Enter a number to choose the initialization type.', '!key' );

		if ( $type !== FALSE )
			call_user_func( $types[ $type ], $config );
	}

	if ( $type !== FALSE ) {
		// Gets the name of the current bootstrapped site.
		$site_name = drush_sitealias_bootstrapped_site_name();

		drush_log( dt( '!site-name: \'!type\' successfully initialized', array('!site-name' => $site_name, '!type' => $type ) ), 'success' );
	}
}
