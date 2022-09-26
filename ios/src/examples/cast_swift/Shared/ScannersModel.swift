//
//  ScannersModel.swift
//  Solum Example
//
//  Holds a list model for presenting the found scanners
//

import Foundation

/// A list model for presenting the found scanners
class ScannersModel: ObservableObject {
    /// The stored list of scanners with properties collected from elsewhere
    @Published var scanners: [Scanner] = []

    /// Send out the current stored details for the scanner with the given serial
    /// - Parameter serial: Serial of the scanner to send details
    func sendDetails(serial: String) {
        guard let row = self.scanners.firstIndex(where: {$0.serial == serial}) else {
            print("Cannot find details for scanner with serial \(serial)")
            return
        }
        let userInfo = ["scanner": scanners[row]]
        NotificationCenter.default.post(name: .scannerDetails, object: nil, userInfo: userInfo)
    }

    /// Initialization to listen to notifications from other classes
    init() {
        // Listen for notifications from the bluetooth model about found scanners
        NotificationCenter.default.addObserver(forName: .deviceFound, object: nil, queue: nil) { [weak self] notification in
            guard let self = self else {
                return
            }
            guard let userInfo = notification.userInfo else {
                return
            }
            guard let deviceFound = userInfo["device"] as? DeviceFound else {
                return
            }
            if let row = self.scanners.firstIndex(where: {$0.serial == deviceFound.serial}) {
                self.scanners[row].rssi = deviceFound.rssi
                self.scanners[row].battery = deviceFound.battery
                self.scanners[row].temperature = deviceFound.temperature
                self.scanners[row].availability = deviceFound.availability
                self.scanners[row].listenPolicy = deviceFound.listenPolicy
                self.scanners[row].chargingStatus = deviceFound.chargingStatus
            } else {
                self.scanners.append(Scanner(serial: deviceFound.serial, ssid: "-", password: "password", ip: "-", tcpPort: 0, rssi: deviceFound.rssi, battery: deviceFound.battery, temperature: deviceFound.temperature, powered: deviceFound.powered, availability: deviceFound.availability, listenPolicy: deviceFound.listenPolicy, chargingStatus: deviceFound.chargingStatus))
            }
        }
        // Listen for notifications from the bluetooth model about Wi-Fi info from scanners
        NotificationCenter.default.addObserver(forName: .wifiInfoReceived, object: nil, queue: nil) { [weak self] notification in
            guard let self = self else {
                return
            }
            guard let userInfo = notification.userInfo else {
                return
            }
            guard let serial = userInfo["serial"] as? String else {
                return
            }
            guard let wifiInfo = userInfo["wifiInfo"] as? WifiInfo else {
                return
            }
            guard let row = self.scanners.firstIndex(where: {$0.serial == serial}) else {
                return
            }
            self.scanners[row].ssid = wifiInfo.ssid
            self.scanners[row].password = wifiInfo.password
            self.scanners[row].ip = wifiInfo.ip
            self.scanners[row].tcpPort = wifiInfo.port
        }
        // Listen for notifications from the bluetooth model about powered status
        NotificationCenter.default.addObserver(forName: .poweredChanged, object: nil, queue: nil) { [weak self] notification in
            guard let self = self else {
                return
            }
            guard let userInfo = notification.userInfo else {
                return
            }
            guard let serial = userInfo["serial"] as? String else {
                return
            }
            guard let powered = userInfo["powered"] as? Bool else {
                return
            }
            guard let row = self.scanners.firstIndex(where: {$0.serial == serial}) else {
                return
            }
            self.scanners[row].powered = powered
        }
    }
}
