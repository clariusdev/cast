//
//  Scanner.swift
//  Solum Example
//
//  Clarius scanner details
//

import Foundation

/// Clarius scanner details
struct Scanner: Identifiable {
    /// Serial number
    var serial: String
    /// Scanner's IP address (v4 or v6)
    var ip: String
    /// Scanner's TCP port for control connections
    var tcpPort: UInt32
    /// ID provided for use in collections
    var id: String { serial }
}
