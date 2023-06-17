//
//  Bluetooth.swift
//  Solum Example
//
//  Class for finding scanners over bluetooth
//

import Foundation
import CoreBluetooth
import Yams

/// Holder for UUIDs relevant to the Wi-Fi info service
struct WifiService {
    /// The UUID for the service itself
    public static let serviceUUID = CBUUID.init(string: "f9eb3fae-947a-4e5b-ab7c-c799e91ed780")
    /// Characteristic for reading the value (read/subscribe)
    public static let infoCharacteristic = CBUUID.init(string: "f9eb3fae-947a-4e5b-ab7c-c799e91ed781")
}

/// Holder for UUIDs relevant to the power service
struct PowerService {
    /// The UUID for the service itself
    public static let serviceUUID = CBUUID.init(string: "8c853b6a-2297-44c1-8277-73627c8d2abc")
    /// Characteristic for reading the value (read/subscribe)
    public static let infoCharacteristic = CBUUID.init(string: "8c853b6a-2297-44c1-8277-73627c8d2abd")
}

/// Details about a scanner found over bluetooth
struct DeviceFound {
    /// Serial number
    var serial: String
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
    /// Bluetooth RSSI
    var rssi: Int
}

/// Try to parse out the serial number of a bluetooth peripheral
/// - Parameter peripheral: Bluetooth peripheral discovered during a scan
/// - Returns: The serial number if the device is a Clarius scanner, otherwise nil
func serialForDevice(peripheral: CBPeripheral) -> String? {
    guard let deviceName = peripheral.name else {
        return nil
    }
    if !deviceName.starts(with: "CUS-") {
        return nil
    }
    return String(deviceName.dropFirst(4))
}

/// Parse out the scanner details from a bluetooth device
/// - Parameters:
///   - peripheral: Bluetooth peripheral discovered during a scan
///   - advertisementData: Data advertised over BLE
///   - rssi: Bluetooth RSSI
/// - Returns: Device details if the given peripheral is a Clarius scanner, otherwise nil
func parseDevice(peripheral: CBPeripheral, advertisementData: [String : Any], rssi: Int) -> DeviceFound? {
    guard let serial = serialForDevice(peripheral: peripheral) else {
        return nil
    }
    guard let manufacturerAny = advertisementData[CBAdvertisementDataManufacturerDataKey] else {
        return nil
    }
    guard let manufacturerData = manufacturerAny as? NSData else {
        return nil
    }
    if manufacturerData.length < 6 {
        return nil
    }
    // The first 2 bytes are just to identify Clarius
    if manufacturerData[0] != 0xFB || manufacturerData[1] != 0x02 {
        return nil
    }
    let batt = min(Int(manufacturerData[2]), 100)
    let temp = min(Int(manufacturerData[3]), 100)
    let packedBits = Int(manufacturerData[4])
    guard let avail = Availability(rawValue: min(packedBits & 0x07, Availability.allCases.count)) else { return nil
    }
    guard let listen = ListenPolicy(rawValue: min((packedBits >> 3) & 0x03, ListenPolicy.allCases.count)) else {
        return nil
    }
    guard let charging = ChargingStatus(rawValue: min((packedBits >> 6) & 0x03, ChargingStatus.allCases.count)) else {
        return nil
    }
    let powered = (Int(manufacturerData[5]) & 0x01) != 0;
    return DeviceFound(serial: serial, battery: batt, temperature: temp, powered: powered, availability: avail, listenPolicy: listen, chargingStatus: charging, rssi: rssi)
}

/// Wi-Fi information which is delivered over Bluetooth
struct WifiInfo {
    /// SSID of the scanner's current Wi-Fi network
    var ssid: String
    /// Password of the scanner's current Wi-Fi network (if Wi-Fi direct)
    var password: String
    /// Scanner's IP address (v4 or v6)
    var ip: String
    /// Scanner's TCP port for control connections
    var port: UInt32
}

/// Parse Wi-Fi info that is received from the scanner
/// - Parameter value: Data which was read over bluetooth
/// - Returns: The parsed Wi-Fi info, if successful
func parseWifiInfo(value: Data) -> WifiInfo? {
    guard let dataString = String(data: value, encoding: .utf8) else {
        return nil
    }
    print("WifiInfo received: \(dataString)")
    guard let object = try? Yams.load(yaml: dataString) else {
        return nil
    }
    guard let mapped = object as? [String: Any] else {
        return nil
    }
    // All of these are technically optional, but
    // the SSID and password should be provided for
    // Wi-Fi direct networks
    let ssid = mapped["ssid"] as? String
    let password = mapped["pw"] as? String
    let ipv4 = mapped["ip4"] as? String
    let ipv6 = mapped["ip6"] as? String
    // This is the TCP port which should be used for cast connections
    let port = mapped["cast"] as? Int
    let ip = ipv4 ?? ipv6 ?? ""
    return WifiInfo(ssid: ssid ?? "", password: password ?? "", ip: ip, port: UInt32(port ?? 0))
}

/// Class for finding scanners over bluetooth
class BluetoothModel: NSObject, ObservableObject, CBCentralManagerDelegate, CBPeripheralDelegate {
    /// Toggle to make the model continuously search for scanners
    @Published var scanActive: Bool = false  {
        didSet { checkScan() }
    }
    /// Serial of the scanner which we should connect to. Technically multiple
    /// simultaneous connections are possible, but it is simpler to only keep
    /// one connection active.
    @Published var selectedSerial: String = "" {
        didSet {
            if selectedSerial.isEmpty {
                return
            }
            guard let peripheral = peripherals[selectedSerial] else {
                return
            }
            centralManager.connect(peripheral)
        }
    }
    /// Cache of all Clarius scanners which have been found over Bluetooth
    private var peripherals: [String: CBPeripheral] = [:]
    /// Class for activating Bluetooth scans
    private var centralManager: CBCentralManager!
    /// True if the CBCentralManager is currently running a scan
    private var centralScanning = false
    /// Power state of local Bluetooth
    private var centralState = CBManagerState.unknown
    /// Initialize the CBCentralManager and subscribe for notifications
    override init() {
        super.init()
        // Bluetooth manager
        centralManager = CBCentralManager(delegate: self, queue: nil)
        // Listen for notifications from the scanner list about changing scanners
        NotificationCenter.default.addObserver(forName: .changeScanner, object: nil, queue: nil) { [weak self] notification in
            guard let self = self else {
                return
            }
            guard let userInfo = notification.userInfo else {
                return
            }
            guard let index = userInfo.index(forKey: "serial") else {
                return
            }
            guard let newSerial = userInfo[index].value as? String else {
                return
            }
            self.selectedSerial = newSerial
        }
    }
    /// Callback for CBCentralManager state notifications
    /// - Parameter central: CBCentralManager which is notifying
    internal func centralManagerDidUpdateState(_ central: CBCentralManager) {
        centralScanning = central.isScanning
        centralState = central.state
        checkScan()
    }
    /// Callback for device discovery notifications
    /// - Parameters:
    ///   - central: CBCentralManager
    ///   - peripheral: Device which was discovered
    ///   - advertisementData: Accompanying advertisement
    ///   - RSSI: Current signal strength
    internal func centralManager(_ central: CBCentralManager,
                        didDiscover peripheral: CBPeripheral,
                        advertisementData: [String : Any],
                        rssi RSSI: NSNumber) {
        // This also acts as a filter to ignore non-Clarius devices
        guard let deviceFound = parseDevice(peripheral: peripheral, advertisementData: advertisementData, rssi: RSSI.intValue) else {
            return
        }
        // Check if it is new device
        if peripherals.index(forKey: deviceFound.serial) == nil {
            // Tell it to give callbacks to us
            peripheral.delegate = self
            peripherals[deviceFound.serial] = peripheral
            // Connect if it is the selected scanner
            if selectedSerial == deviceFound.serial {
                centralManager.connect(peripheral)
            }
        }
        // Notify the other models that a scanner was found
        let userInfo = ["device": deviceFound]
        NotificationCenter.default.post(name: .deviceFound, object: nil, userInfo: userInfo)
    }
    /// Callback for successful device connection
    /// - Parameters:
    ///   - central: CBCentralManager
    ///   - peripheral: Device connected to
    internal func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        guard let serial = serialForDevice(peripheral: peripheral) else {
            return
        }
        // If it is still the selected scanner, discover its services
        if serial == selectedSerial {
            peripheral.discoverServices([PowerService.serviceUUID, WifiService.serviceUUID])
        }
    }
    /// Callback for device service discovery
    /// - Parameters:
    ///   - peripheral: Device with services discovered
    ///   - error: Error if a problem occurred
    internal func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        if let error = error {
            print("Bluetooth error \(error.localizedDescription)")
            return
        }
        guard let serial = serialForDevice(peripheral: peripheral) else {
            return
        }
        // Check if it is still the selected scanner
        if serial != selectedSerial {
            return
        }
        guard let services = peripheral.services else {
            print("Scanner with serial \(serial) has no services")
            return
        }
        // Attempt to discover characteristics for any relevant services
        if let service = services.first(where: {$0.uuid == WifiService.serviceUUID}) {
            peripheral.discoverCharacteristics([WifiService.infoCharacteristic], for: service)
        }
        if let service = services.first(where: {$0.uuid == PowerService.serviceUUID}) {
            peripheral.discoverCharacteristics([PowerService.infoCharacteristic], for: service)
        }
    }
    /// Callback for characteristic discovery
    /// - Parameters:
    ///   - peripheral: Device which has the characteristic
    ///   - service: Service containing the characteristic
    ///   - error: Error, if one occurred
    internal func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?) {
        if let error = error {
            print("Bluetooth error \(error.localizedDescription)")
            return
        }
        guard let serial = serialForDevice(peripheral: peripheral) else {
            return
        }
        guard let characteristics = service.characteristics else {
            print("Scanner with serial \(serial) has no characteristics for \(service.uuid)")
            return
        }
        // Get the info characteristic for either the Wi-Fi service or the power service
        let uuid = service.uuid == WifiService.serviceUUID ? WifiService.infoCharacteristic : PowerService.infoCharacteristic
        guard let characteristic = characteristics.first(where: {$0.uuid == uuid}) else {
            return
        }
        // Try to register for notifications
        if characteristic.properties.contains(.notify) {
            peripheral.setNotifyValue(true, for: characteristic)
        }
        // Try to read the current value
        if characteristic.properties.contains(.read) {
            peripheral.readValue(for: characteristic)
        }
    }
    /// Callback for characteristic value updates
    /// - Parameters:
    ///   - peripheral: Device which has the characteristic
    ///   - characteristic: Characteristic with the updated value
    ///   - error: Error, if one occurred
    internal func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: Error?) {

        if let error = error {
            print("Bluetooth error \(error.localizedDescription)")
            return
        }
        guard let serial = serialForDevice(peripheral: peripheral) else {
            return
        }
        guard let value = characteristic.value else {
            return
        }
        // If it is Wi-Fi info, attempt to parse
        if characteristic.uuid == WifiService.infoCharacteristic {
            guard let wifiInfo = parseWifiInfo(value: value) else {
                return
            }
            // Notify the other classes that Wi-Fi info was received
            let userInfo: [String: Any] = ["serial": serial,
                            "wifiInfo": wifiInfo]
            NotificationCenter.default.post(name: .wifiInfoReceived, object: nil, userInfo: userInfo)
        }
        // If it is power info, attempt to parse
        if characteristic.uuid == PowerService.infoCharacteristic {
            if value.count < 1 {
                return
            }
            let powered = (value[0] == 1)
            // Notify the other classes that the powered state was received
            let userInfo: [String: Any] = ["serial": serial,
                            "powered": powered]
            NotificationCenter.default.post(name: .poweredChanged, object: nil, userInfo: userInfo)
        }
    }
    /// Check if the bluetooth scan should be re-triggered
    private func checkScan() {
        if scanActive {
            if centralState == .poweredOn && !centralScanning {
                centralManager.scanForPeripherals(withServices: nil, options: [CBCentralManagerScanOptionAllowDuplicatesKey : true])
            }
        } else {
            if centralScanning {
                centralManager.stopScan()
            }
        }
    }
}
