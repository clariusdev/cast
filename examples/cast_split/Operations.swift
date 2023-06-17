//
//  Operations.swift
//  SideBySide
//

import SwiftUI

func +(lhs: CGPoint, rhs: CGPoint) -> CGPoint {
    return CGPoint(x: lhs.x + rhs.x, y: lhs.y + rhs.y)
}
func -(lhs: CGPoint, rhs: CGPoint) -> CGPoint {
    return CGPoint(x: lhs.x - rhs.x, y: lhs.y - rhs.y)
}
func /(lhs: CGPoint, rhs: CGFloat) -> CGPoint {
    return CGPoint(x: lhs.x / rhs, y: lhs.y / rhs)
}
func *(lhs: CGPoint, rhs: CGSize) -> CGPoint {
    return CGPoint(x: lhs.x * rhs.width, y: lhs.y * rhs.height)
}
func /(lhs: CGPoint, rhs: CGSize) -> CGPoint {
    return CGPoint(x: lhs.x / rhs.width, y: lhs.y / rhs.height)
}
