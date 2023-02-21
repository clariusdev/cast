//
//  CastModel.swift
//  Cast Example
//
//  Class to wrap the Cast framework for Swift
//

import Foundation
import CoreGraphics

/// Class to wrap the Cast framework for Swift
class CastModel: ObservableObject {

    /// Last image received from the scanner
    @Published var image: CGImage?
    /// The frozen state of the connected scanner
    @Published var frozen: Bool = true

    /// Scanner's SSID (informational)
    @Published var ssid: String = ""
    /// Scanner's Wi-Fi direct password (informational)
    @Published var password: String = ""
    /// Scanner's IP address
    @Published var address: String = ""
    /// Scanner's TCP port
    @Published var port: UInt32 = 0
    /// Certificate to allow scanner use (provide before connecting)
    @Published var certificate: String = "research"

    /// Connect to the scanner at the given address and port
    func connectToScanner() {
        cast.connect(address, port: port, cert: certificate) { (succeeded: Bool, port: Int32, swRevMatch: Bool) -> Void in
            print("Connection to \(self.address):\(self.port) \(succeeded ? "succeeded" : "failed")")
            if (succeeded) {
                print("UDP stream will be on port \(port)")
                print("App software \(swRevMatch ? "matches" : "does not match")")
            }
        }
    }
    /// Disconnect from the current scanner
    func disconnect() {
        cast.disconnect() {
            print("Disconnection \($0 ? "succeeded" : "failed")")
        }
    }
    /// Send a freeze toggle command to the connected scanner
    func toggleFreeze() {
        cast.userFunction(Freeze, value: 0.0, callback: {
            print("User function freeze \($0 ? "succeeded" : "failed")")
        })
    }
    /// Framework instance
    private let cast = CusCast()
    /// Initialize the framework and register for callbacks
    init() {
        cast.setErrorCallback({ (errorString: String) -> Void in
            print(errorString)
        })
        // The framework requires a directory for storing security keys.
        // Using the application support path for this application.
        let appSupportPaths = FileManager.default.urls(for: .applicationSupportDirectory, in: .userDomainMask)
        let appSupportPath = appSupportPaths[0].path
        cast.initialize(appSupportPath) { (result: Bool) -> Void in
            print("Initialization \(result ? "succeeded" : "failed")")
        }
        cast.setFreezeCallback({ (frozen: Bool) -> Void in
            self.frozen = frozen
        })
        cast.setNewProcessedImageCallback({ (imageData: Data, imageInfo: UnsafePointer<CusProcessedImageInfo>, npos: Int32, positions: UnsafePointer<CusPosInfo>) -> Void in
            // Converting the raw image data into a CGImage which can be displayed
            let nfo = imageInfo.pointee
            let rowBytes = nfo.width * nfo.bitsPerPixel / 8
            let totalBytes = Int(nfo.height * rowBytes)
            let rawBytes = UnsafeMutableRawPointer.allocate(byteCount: totalBytes, alignment: 1)
            let bmpInfo = CGImageAlphaInfo.premultipliedFirst.rawValue | CGBitmapInfo.byteOrder32Little.rawValue
            imageData.copyBytes(to: UnsafeMutableRawBufferPointer(start: rawBytes, count: totalBytes))
            guard let colorspace = CGColorSpace(name: CGColorSpace.sRGB) else {
                return
            }
            guard let ctxt = CGContext(data: rawBytes, width: Int(nfo.width), height: Int(nfo.height), bitsPerComponent: 8, bytesPerRow: Int(rowBytes), space: colorspace, bitmapInfo: bmpInfo) else {
                return
            }
            self.image = ctxt.makeImage()
        })
        // Listen for notifications from the scanners model about scanner details
        NotificationCenter.default.addObserver(forName: .scannerDetails, object: nil, queue: nil) { [weak self] notification in
           guard let self = self else {
               return
           }
           guard let userInfo = notification.userInfo else {
               return
           }
           guard let scanner = userInfo["scanner"] as? Scanner else {
               return
           }
           self.ssid = scanner.ssid
           self.password = scanner.password
           self.address = scanner.ip
           self.port = scanner.tcpPort
       }
    }
}
