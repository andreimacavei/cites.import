# Initialize

Initialize commands initializes your site.

## Commands

The following commands are available:

* **initialize (init)** Initializes your site.

### initialize

This command will initialize your site.

#### Examples

```
drush init languages --config-file=config.json
```

Adds languages defined in the config.json configuration file.

#### Arguments

`type` Type of the initialization. Omit this argument to choose from available options.

The following initialization types are available:

* all
* languages
* menus

Each type must be defined in the configuration file.

#### Options

`--config-file=[config.json]` Path to the configuration file.

If set then the specified configuration file will be loaded. Otherwise the
default value will be used.

Default: config.json

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
	]
}
```

#### Aliases

`init`
