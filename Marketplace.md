# Marketplace Workflow

The marketplace workflow allows integrating partner applications into the Clarius App on released platforms.

Features:

- Third party apps can be launched directly from the Clarius App.
- Third party apps will receive the probe TCP connection info with the launcher, removing the need for manual input or Bluetooth scanning.

## Enrolment

TODO

## Launcher

Once enrolled in the marketplace program, send the following information to Clarius to setup a launcher:

- Logo: a URL to a custom logo hosted on your web server. It must be a bitmap (PNG or JPEG for example, no SVG). It should be square for nicer display.
- App name: The app name
- App description: Short description of the app
- iOS launcher (only if an iOS version is available):
  - URL: The url to launch the iOS app, for example: `com.clarius.sidebyside://cast`
- Android launcher (only if an Android version is available):
  - Package name: The package name specified in the manifest, for example `me.clarius.sdk.cast.example`
  - Optional intent configuration:
    - Component name: String representation of a component name as defined in `android/content/ComponentName`, for example `com.example/.MainActivity`
    - Action: the intent action as defined in `android/content/Intent`
    - Categories: list of strings representing the intent categories as defined in `android/content/Intent`
- Optional filters to display the launcher:
  - Probe model filters: list of compatible probe models, wildcard (`*`) allowed, for example: `PA*` for phased array only
  - Application filters: list of compatible applications/workflows, wildcard (`*`) allowed, for example: `cardiac` for cardiac workflow only
