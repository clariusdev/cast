//
//  SideBySideApp.swift
//  SideBySide
//

import SwiftUI

extension Notification.Name {
    /// Notification that the scanner details should be should populated, user data: "scanner" as Scanner struct
    static let scannerDetails = Notification.Name("scannerDetails")
}

func parseScanner(url: URL) -> Scanner? {
    guard let components = URLComponents(url: url, resolvingAgainstBaseURL: false) else {
        return nil
    }
    guard let queryItems = components.queryItems else {
        return nil
    }
    var ipAddress: String?
    var serial: String?
    var tcpPort: UInt32?
    for item in queryItems {
        switch (item.name) {
        case "serial":
            serial = item.value
        case "address":
            ipAddress = item.value
        case "port":
            tcpPort = UInt32(item.value ?? "")
        default:
            print("Unknown parameter \(item.name)")
        }
    }
    guard let serial = serial, let ipAddress = ipAddress, let tcpPort = tcpPort else {
        return nil
    }
    return Scanner(serial: serial, ip: ipAddress, tcpPort: tcpPort)
}

@main
struct SideBySideApp: App {
    @StateObject private var castModel = CastModel()
    var body: some Scene {
        WindowGroup {
            ContentView()
                .environmentObject(castModel)
                .onOpenURL { url in
                    guard let scanner = parseScanner(url: url) else {
                        return
                    }
                    let userInfo = ["scanner": scanner]
                    NotificationCenter.default.post(name: .scannerDetails, object: nil, userInfo: userInfo)
                }
        }
    }
}
