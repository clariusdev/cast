# Clarius Cast Example

1. Copy the `aar` package into the `libs` subfolder
2. Re-sync Gradle
3. Build

## Important Notes

* Set the DSL option `useLegacyPackaging = true` in the application's `build.gradle`, this ensures the native libraries are extracted and can be loaded at runtime because the current implementation does not support loading uncompressed libraries from the APK:

## Marketplace Integration

### Launcher

The Clarius app can launch partner apps with an Android intent containing the probe connection info in the intent's extras:

| Key | Description | Type |
|-----|-------------|------|
| `cus_probe_serial` | probe serial | UTF-8 string in a `byte[]` |
| `cus_ip_address` | probe's IP address | UTF-8 string in a `byte[]` |
| `cus_cast_port` | probe's TCP port | Integer encoded in a `byte[]` |
| `cus_network_id` | probe's network ID | Integer encoded in a `byte[]` |

### Visibility

The Clarius app attempts to detect installed partner apps by querying the `PackageManager` to adapt its GUI.
However, starting with Android 11 (API 30), results are filtered by the system, preventing detection.
To allow detection, ensure your target activity declares an intent filter in the manifest file with either of the following actions:

* `android.intent.action.MAIN`
* `me.clarius.sdk.action.LAUNCH`

For example:

    <application>
        <activity>
            <intent-filter>
                <action android:name="android.intent.action.MAIN" />
                <action android:name="me.clarius.sdk.action.LAUNCH" />
                <category android:name="android.intent.category.LAUNCHER" />
            </intent-filter>
        </activity>
    </application>

## TODO

* Provide a multi-ABI package
