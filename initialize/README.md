# Initialize

Initialize commands initializes your site.

## Commands

The following commands are available:

```
initialize (init)     Initializes your site.
```

### initialize

This command will initialize your site.

#### Examples

```
drush init languages       Adds languages defined in the
--config-file=config.json  config.json configuration file.
```

#### Arguments

```
type                                      Type of the initialization. Omit
                                          this argument to choose from
                                          available options.
```

The following initialization types are available:

* all
* languages
* menus

Each type must be defined in the configuration file.

#### Options

```
--config-file=[config.json]               Path to the configuration file.
```

If set then the specified configuration file will be loaded. Otherwise the
default value will be used.

Default: _config.json_

To add a new language it must defined in the configuration file. The following
properties are available:

* langcode: _""_
* name: _""_
* native: _""_
* direction: _"LANGUAGE_LTR"_
* domain: _""_
* prefix: _""_
* enabled: _true_
* default: _false_

If a property is not set then its default value will be used. If a language
already exists it will be updated with the values defined in the configuration
files.

A sample configuration file is shown below:

```
{
	"languages":
	[
		{
			"langcode": "en",
			"name": "English",
			"native": "English",
			"direction": "LANGUAGE_LTR",
			"domain": "",
			"prefix": "",
			"enabled": true,
			"default": true
		}
	],
	"menus":
	[
		{
			"menu_name": "main-menu",
			"title": "Main menu",
			"description": "The <em>Main</em> menu is used on many sites to show the major sections of the site, often in a top navigation bar.",
			"i18n_mode": "I18N_MODE_MULTIPLE",
			"links":
			[
				{
					"link_path": "<front>",
					"link_title": "Home"
				}
			]
		}
	]
}
```

#### Aliases

```
init
```
