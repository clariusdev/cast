//
//  ScannerList.swift
//  Solum Example
//
//  List of found scanners
//

import SwiftUI

struct ScannerList: View {

    var scanners: [Scanner]
    var selectedSerial: String
    var body: some View {
        VStack {
            HStack {
                Text("Serial")
                    .bold()
                    .frame(maxWidth: .infinity)
                Text("SSID")
                    .bold()
                    .frame(maxWidth: .infinity)
                Text("Battery")
                    .bold()
                    .frame(maxWidth: .infinity)
                Text("Powered")
                    .bold()
                    .frame(maxWidth: .infinity)
            }
            HStack {
                Text("IP Address")
                    .bold()
                    .frame(maxWidth: .infinity)
                Text("Password")
                    .bold()
                    .frame(maxWidth: .infinity)
                Text("Temperature")
                    .bold()
                    .frame(maxWidth: .infinity)
                Text("Availability")
                    .bold()
                    .frame(maxWidth: .infinity)
            }
            HStack {
                Text("TCP Port")
                    .bold()
                    .frame(maxWidth: .infinity)
                Text("-")
                    .bold()
                    .frame(maxWidth: .infinity)
                Text("RSSI")
                    .bold()
                    .frame(maxWidth: .infinity)
                Text("Listen")
                    .bold()
                    .frame(maxWidth: .infinity)
            }
            Divider()
            ScrollView {
                VStack {
                    ForEach(scanners) { scanner in
                        let selected = scanner.serial == selectedSerial
                        #if os(iOS)
                        let bgColor = selected ? Color.accentColor : Color(uiColor: .tertiarySystemBackground)
                        #else
                        let bgColor = Color(selected ? NSColor.selectedContentBackgroundColor : NSColor.controlBackgroundColor)
                        #endif
                        VStack {
                            HStack {
                                Text(scanner.serial)
                                    .frame(maxWidth: .infinity)
                                Text(scanner.ssid)
                                    .frame(maxWidth: .infinity)
                                Text("\(scanner.battery)%")
                                    .frame(maxWidth: .infinity)
                                Text(scanner.powered ? "On" : "Off")
                                    .frame(maxWidth: .infinity)
                            }
                            HStack {
                                Text(scanner.ip)
                                    .frame(maxWidth: .infinity)
                                Text(scanner.password)
                                    .frame(maxWidth: .infinity)
                                Text("\(scanner.temperature)°C")
                                    .frame(maxWidth: .infinity)
                                Text(String(describing: scanner.availability))
                                    .frame(maxWidth: .infinity)
                            }
                            HStack {
                                Text(String(scanner.tcpPort))
                                    .frame(maxWidth: .infinity)
                                Text("-")
                                    .frame(maxWidth: .infinity)
                                Text(String(scanner.rssi))
                                    .frame(maxWidth: .infinity)
                                Text(String(describing: scanner.listenPolicy))
                                    .frame(maxWidth: .infinity)
                            }.padding([.bottom], 1)
                        }.background(bgColor).onTapGesture {
                            let userInfo = ["serial": scanner.serial]
                            NotificationCenter.default.post(name: .changeScanner, object: nil, userInfo: userInfo)
                        }
                    }
                }
            }.frame(maxHeight: 200)
        }
    }
}

struct ScannerList_Previews: PreviewProvider {
    static var previews: some View {
        ScannerList(scanners: [
            Scanner(serial: "Test1",
                    ssid: "DIRECT-C3HD3",
                    password: "Secure",
                    ip: "192.168.0.1",
                    tcpPort: 33546,
                    rssi: -59,
                    battery: 75,
                    temperature: 33,
                    powered: false,
                    availability: Availability.available,
                    listenPolicy: ListenPolicy.institution,
                    chargingStatus: ChargingStatus.none
            ),
            Scanner(serial: "Test2",
                    ssid: "DIRECT-L7HD3",
                    password: "Secure",
                    ip: "192.168.0.19",
                  	tcpPort: 37434,
                  	rssi: -30,
                  	battery: 70,
                  	temperature: 29,
                	powered: true,
                    availability: Availability.notAvailable,
                    listenPolicy: ListenPolicy.disabled,
                    chargingStatus: ChargingStatus.done
              )], selectedSerial: "Test2")
    }
}
