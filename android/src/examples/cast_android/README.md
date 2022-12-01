# Clarius Cast Example

1. Copy the `aar` package into the `libs` subfolder
2. Re-sync Gradle
3. Build

## TODO

* Provide a multi-ABI package

## Marketplace Integration

### Launcher

The Clarius app can launch partner apps with an Android intent containing the probe connection info in the intent's extras:

| Key | Description | Type |
|-----|-------------|------|
| `cus_probe_serial` | probe serial | UTF-8 string in a `byte[]` |
| `cus_ip_address` | probe's IP address | UTF-8 string in a `byte[]` |
| `cus_cast_port` | probe's TCP port | Java `int` |

### Visibility

The Clarius app attempts to detected installed partner apps by querying the `PackageManager` to adapt its GUI.
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
