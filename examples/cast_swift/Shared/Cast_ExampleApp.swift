//
//  Cast_ExampleApp.swift
//  Shared
//
//  The main app which creates all class instances
//

import SwiftUI

extension Notification.Name {
    /// Notification that the selected scanner should change, user data: "serial" as String
    static let changeScanner = Notification.Name("changeScanner")
    /// Notification that a scanner was found over bluetooth, user data: "device" as DeviceFound struct
    static let deviceFound = Notification.Name("deviceFound")
    /// Notification that a scanner's powered state changed, user data: "serial" as String, "powered" as Bool
    static let poweredChanged = Notification.Name("poweredChanged")
    /// Notification that a scanner's Wi-Fi info changed, user data: "serial" as String, "wifiInfo" as WifiInfo struct
    static let wifiInfoReceived = Notification.Name("wifiInfoReceived")
    /// Notification that the scanner details should be should populated, user data: "scanner" as Scanner struct
    static let scannerDetails = Notification.Name("scannerDetails")
}

@main
/// The main app which creates all class instances
struct Cast_ExampleApp: App {
    @StateObject private var castModel = CastModel()
    @StateObject private var bluetooth = BluetoothModel()
    @StateObject private var scannersModel = ScannersModel()
    var body: some Scene {
        WindowGroup {
            ContentView()
                .environmentObject(castModel)
                .environmentObject(bluetooth)
                .environmentObject(scannersModel)
        }
    }
}
