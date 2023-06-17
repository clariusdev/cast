//
//  ContentView.swift
//  Shared
//
//  Main application view
//

import SwiftUI

struct ContentView: View {
    @EnvironmentObject private var cast: CastModel
    @EnvironmentObject private var bluetooth: BluetoothModel
    @EnvironmentObject private var scannerModel: ScannersModel
    func displayImage() -> Image {
        if cast.image != nil {
            return Image(cast.image!, scale: 1.0, label: Text("Ultrasound"))
        } else {
            return Image("BlankImage")
        }
    }
    var body: some View {
        ScrollView {
            VStack(spacing: 0) {
                GroupBox(label: Label("Bluetooth", systemImage: "antenna.radiowaves.left.and.right").font(.title3)) {
                    Toggle(isOn: $bluetooth.scanActive) {
                        Label("Searching", systemImage: "magnifyingglass")
                    }
                    ScannerList(scanners: scannerModel.scanners, selectedSerial: bluetooth.selectedSerial)
                    HStack {
                        Button(action: {
                            if bluetooth.selectedSerial.isEmpty {
                                print("Cannot load details when no scanner selected")
                                return
                            }
                            scannerModel.sendDetails(serial: bluetooth.selectedSerial)
                        }) {
                            Label("Load Selected Scanner Details", systemImage: "square.and.arrow.down.on.square")
                        }
                    }
                }.padding()
                GroupBox(label: Label("Wi-Fi", systemImage: "wifi").font(.title3)) {
                    VStack {
                        HStack {
                            Text("SSID")
                            TextField("SSID", text: $cast.ssid)
                        }
                        HStack {
                            Text("Password")
                            TextField("Password", text: $cast.password)
                        }
                        HStack {
                            Text("IP Address")
                            TextField("IP Address", text: $cast.address)
                        }
                        HStack {
                            Text("Port")
                            TextField("Port", value: $cast.port, format: .number)
                        }
                        HStack {
                            Text("Certificate")
                            TextField("Certificate", text: $cast.certificate)
                        }
                        HStack {
                            Button(action: cast.connectToScanner) {
                                Text("Connect")
                            }
                            Button(action: cast.disconnect) {
                                Text("Disconnect")
                            }
                        }
                    }
                }.padding()
                GroupBox(label: Label("Imaging", systemImage: "waveform").font(.title3)) {
                    VStack {
                        HStack {
                            Button(action: cast.toggleFreeze) {
                                Text("Toggle Freeze")
                            }
                            Text("Imaging is \(cast.frozen ? "frozen" : "live")")
                                .frame(maxWidth: .infinity)
                        }
                        displayImage()
                            .resizable()
                            .aspectRatio(contentMode: .fit)
                        }
                }.padding()
            }
        }
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
            .environmentObject(CastModel())
            .environmentObject(BluetoothModel())
            .environmentObject(ScannersModel())
    }
}
