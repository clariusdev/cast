//
//  ContentView.swift
//  SideBySide
//
//  Created by Ben Kerby on 2022-10-05.
//

import SwiftUI

struct ContentView: View {
    @EnvironmentObject private var cast: CastModel
    func displayImage() -> Image {
        if cast.image != nil {
            return Image(cast.image!, scale: 1.0, label: Text("Ultrasound"))
        } else {
            return Image("BlankImage")
        }
    }
    var body: some View {
        VStack {
            GroupBox(label: Label("Wi-Fi", systemImage: "wifi").font(.title3)) {
                VStack {
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

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
            .environmentObject(CastModel())
    }
}
