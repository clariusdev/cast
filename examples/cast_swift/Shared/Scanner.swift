//
//  Scanner.swift
//  Solum Example
//
//  Clarius scanner details
//

import Foundation

/// Enumerations of scanner availabilities
enum Availability: Int, CaseIterable {
    case available = 0, listenOnly, notAvailable
}

/// Enumeration of scanner listen policies
enum ListenPolicy: Int, CaseIterable {
    case disabled = 0, institution, global, research
}

/// Enumeration of charging states
enum ChargingStatus: Int, CaseIterable {
    case none = 0, pre, fast, done
}

/// Clarius scanner details
struct Scanner: Identifiable {
    /// Serial number
    var serial: String
    /// SSID of the scanner's current Wi-Fi network
    var ssid: String
    /// Password of the scanner's current Wi-Fi network (if Wi-Fi direct)
    var password: String
    /// Scanner's IP address (v4 or v6)
    var ip: String
    /// Scanner's TCP port for control connections
    var tcpPort: UInt32
    /// Bluetooth RSSI
    var rssi: Int
    /// Battery level in 0-100%
    var battery: Int
    /// Temperature in degrees Celsius
    var temperature: Int
    /// Turn if currently powered on
    var powered: Bool
    /// Current availability
    var availability: Availability
    /// Listen policy (for Clarius Cast)
    var listenPolicy: ListenPolicy
    /// Charging status
    var chargingStatus: ChargingStatus
    /// ID provided for use in collections
    var id: String { serial }
}
