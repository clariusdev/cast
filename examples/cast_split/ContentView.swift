//
//  ContentView.swift
//  SideBySide
//

import SwiftUI

struct ContentView: View {
    @EnvironmentObject private var cast: CastModel
    @State private var labelToAdd: String = ""
    @State private var labels: [CaptureLabel] = []
    @State private var measurements: [Measurement] = []
    @State private var imagingSize: CGSize = .zero
    @State private var imagingPosition: CGPoint = .zero
    func labelPos(_ measurement: Measurement) -> CGPoint {
        let center = (measurement.position1 + measurement.position2) / 2
        return center * imagingSize + imagingPosition
    }
    func displayImage() -> Image {
        if cast.image != nil {
            return Image(cast.image!, scale: 1.0, label: Text("Ultrasound"))
        } else {
            return Image("BlankImage")
        }
    }
    func imageView(_ geometry: GeometryProxy) -> some View {
        DispatchQueue.main.async {
            let dimension = min(geometry.size.width, geometry.size.height)
            self.imagingSize = CGSize(width: dimension, height: dimension)
            self.imagingPosition = CGPoint(x: (geometry.size.width - dimension) / 2, y: (geometry.size.height - dimension) / 2)
        }
        return displayImage()
                .resizable()
    }
    struct Cursor: View {
        var body: some View {
            ZStack {
                Rectangle()
                    .opacity(0.0001)
                GeometryReader { geometry in
                    Path { path in
                        let frame = geometry.frame(in: .local)
                        let strokeY = frame.height / 4
                        path.move(to: CGPoint(x: frame.midX, y: frame.minY))
                        path.addLine(to: CGPoint(x: frame.midX, y: frame.minY + strokeY))
                        path.move(to: CGPoint(x: frame.midX, y: frame.maxY - strokeY))
                        path.addLine(to: CGPoint(x: frame.midX, y: frame.maxY))
                        let strokeX = frame.width / 4
                        path.move(to: CGPoint(x: frame.minX, y: frame.midY))
                        path.addLine(to: CGPoint(x: frame.minX + strokeX, y: frame.midY))
                        path.move(to: CGPoint(x: frame.maxX - strokeX, y: frame.midY))
                        path.addLine(to: CGPoint(x: frame.maxX, y: frame.midY))
                    }.stroke(.green, style: StrokeStyle(lineWidth: 1))
                }
            }
        }
    }
    var body: some View {
        ScrollView {
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
                GroupBox(label: Label("Capture", systemImage: "camera").font(.title3)) {
                    VStack {
                        HStack {
                            Button(action: {
                                labels = []
                                measurements = []
                            }) {
                                Text("Clear Screen")
                            }
                            Button(action: {
                                let mmtLabel = "D\(measurements.count + 1)"
                                measurements.append(Measurement(text: mmtLabel, position1: CGPoint(x: 0.4, y: 0.4), position2: CGPoint(x: 0.6, y: 0.6)))
                            }) {
                                Text("Add Measurement")
                            }
                            Button(action: {
                                cast.createCapture(labels: self.labels, measurements: self.measurements)
                            }) {
                                Text("Create Capture")
                            }
                        }
                        HStack {
                            TextField("Text", text: $labelToAdd)
                            Button(action: {
                                if labelToAdd.isEmpty {
                                    print("No label text provided")
                                    return
                                }
                                labels.append(CaptureLabel(text: labelToAdd, position: CGPoint(x: 0.5, y: 0.5)))
                            }) {
                                Text("Add Label")
                            }
                        }
                    }
                }.padding()
                GroupBox(label: Label("Imaging", systemImage: "waveform").font(.title3)) {
                    HStack {
                        Button(action: cast.toggleFreeze) {
                            Text("Toggle Freeze")
                        }
                        Text("Imaging is \(cast.frozen ? "frozen" : "live")")
                            .frame(maxWidth: .infinity)
                    }
                    GeometryReader { geometry in
                        ZStack(alignment: .center) {
                            self.imageView(geometry)
                            ForEach(Array(zip(measurements.indices, measurements)), id: \.0) { index, measurement in
                                Path { path in
                                    path.move(to: measurement.position1 * imagingSize + imagingPosition)
                                    path.addLine(to: measurement.position2 * imagingSize + imagingPosition)
                                }.stroke(.green, style: StrokeStyle(lineWidth: 1, dash: [imagingSize.height / 80, imagingSize.height / 80]))
                                Text(measurement.text)
                                    .position(labelPos(measurement))
                                Cursor()
                                    .frame(width: imagingSize.height / 20, height: imagingSize.height / 20)
                                    .position(measurement.position1 * imagingSize + imagingPosition)
                                    .gesture(DragGesture().onChanged{ state in
                                        measurements[index].position1 = (state.location - imagingPosition) / imagingSize
                                    })
                                Cursor()
                                    .frame(width: imagingSize.height / 20, height: imagingSize.height / 20)
                                    .position(measurement.position2 * imagingSize + imagingPosition)
                                    .gesture(DragGesture().onChanged{ state in
                                        measurements[index].position2 = (state.location - imagingPosition) / imagingSize
                                    })
                            }
                            ForEach(Array(zip(labels.indices, labels)), id: \.0) { index, label in
                                Text(label.text)
                                    .position(label.position * imagingSize + imagingPosition)
                                    .gesture(DragGesture().onChanged{ state in
                                        labels[index].position = (state.location - imagingPosition) / imagingSize
                                    })
                            }
                        }
                    }.aspectRatio(1, contentMode: .fit)

                }.padding()
            }
        }
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
            .environmentObject(CastModel())
    }
}
