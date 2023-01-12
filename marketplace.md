# Marketplace Partners

Marketplace partners will take advantage of the Cast API and receive the ultrasound stream directly to their App without any complex integration. Users on the Clarius platform can gain access to 3rd party Apps, after subscribing, and launch directly from the Clarius App interface.

Marketplace Workflow:

- User subscribes to the 3rd party App
- From the interface, upon choosing the associated probe and application, an icon will be displayed
- Tapping the icon will launch the partner App and provide the TCP connection information
- Once connected, the ultrasound stream is received in real-time alongside the Clarius App
- Partner Apps can send data back to the Clarius App for storing within the examination, and can include:
  - Measurements
  - Annotations
  - Overlay/Segmentations

## Enrolment

Potential partners can reach out directly to hello@clarius.com and pitch their ideas for coming aboard the Clarius Marketplace and joining our rich ecosystem of technologies and clinical applications.

## Launcher Setup

Once enrolled, the following information will need to be sent to us:

- Logo
  - PNG with transparencies is preferred, however JPEG is also possible
  - Ideally a circular icon to align with existing Clarius bitmaps used within the App
- App Name
  - The name of your App, for example `SonoAI Ejection Fraction Calculator`
- App Description
  - The description of your App, for example: `SonoAI's EF calculator will automatically calculate and output ejection fraction in a 4-chamber view after 2 valid cardiac cycles`
- iOS URL
  - The URL to launch the iOS App, for example: `com.clarius.sidebyside://cast`
- Android Package Name
  - The package name specified in the manifest, for example `me.clarius.sdk.cast.example`
  - Optional intent configuration:
    - Component name: String representation of a component name as defined in `android/content/ComponentName`, for example `com.example/.MainActivity`
    - Action: the intent action as defined in `android/content/Intent`
    - Categories: list of strings representing the intent categories as defined in `android/content/Intent`
- Product Associations
  - The probe model(s) that your technology works with, for example: `C3 and PA`
  - The clinical application(s) that your technology works with, for example: `Cardiac and Vascular`
