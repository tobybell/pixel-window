import Cocoa
import Accelerate

let application = NSApplication.shared
let delegate = AppDelegate()
application.delegate = delegate
NSApp.setActivationPolicy(.regular)
application.run()

class AppDelegate: NSObject, NSApplicationDelegate, NSWindowDelegate {
  var window: NSWindow!
  func applicationDidFinishLaunching(_ notification: Notification) {
    window = NSWindow(
      contentRect: NSRect(x: 0, y: 0, width: 512, height: 512),
      styleMask: [.titled, .closable, .miniaturizable],
      backing: .buffered, defer: false)
    window.center()
    window.setFrameAutosaveName("Pixel Window")

    assert(window.backingScaleFactor == floor(window.backingScaleFactor))

    window.contentView = PixelView(Int(window.backingScaleFactor))
    window.makeKeyAndOrderFront(nil)
    window.delegate = self
    print(window.backingScaleFactor)
    NSApp.activate(ignoringOtherApps: true)
  }
  
  func windowWillClose(_: Notification) {
    NSApplication.shared.stop(nil)
  }
}

func makeImageBuffer(_ scaleFactor: Int) -> vImage_Buffer {
  do {
    return try vImage_Buffer(width: 512, height: 512, bitsPerPixel: 32)
  } catch {
    print("failed to make image buffer")
    abort()
  }
}

func redraw(user: UnsafeRawPointer?) {
  let view = user!.bindMemory(to: PixelView.self, capacity: 1)
  view[0].needsDisplay = true
}

class PixelView: NSView {
  let scaleFactor: Int
  let imageFormat = vImage_CGImageFormat(
      bitsPerComponent: 8,
      bitsPerPixel: 32,
      colorSpace: CGColorSpaceCreateDeviceRGB(),
      bitmapInfo: CGBitmapInfo(rawValue: CGImageAlphaInfo.first.rawValue),
      renderingIntent: .defaultIntent)!

  let imageBuffer: vImage_Buffer

  override var acceptsFirstResponder: Bool { return true }

  func systemLocationFor(_ point: CGPoint) -> (Float, Float) {
    let x = Float(point.x / frame.width) * Float(imageBuffer.width)
    let y = (1 - Float(point.y / frame.height)) * Float(imageBuffer.height)
    return (x, y)
  }

  override func mouseDown(with event: NSEvent) {
    guard event.type == .leftMouseDown else {
      return print("mouse down event \(event)")
    }
    let (x, y) = systemLocationFor(event.locationInWindow)
    withUnsafePointer(to: self) { sysMouseDown(sys, $0, x, y) }
  }

  override func mouseUp(with event: NSEvent) {
    guard event.type == .leftMouseUp else {
      return print("mouse up event \(event)")
    }
    let (x, y) = systemLocationFor(event.locationInWindow)
    withUnsafePointer(to: self) { sysMouseUp(sys, $0, x, y) }
  }

  override func mouseDragged(with event: NSEvent) {
    guard event.type == .leftMouseDragged else {
      return print("mouse dragged event \(event)")
    }
    let (x, y) = systemLocationFor(event.locationInWindow)
    withUnsafePointer(to: self) { sysMouseMoved(sys, $0, x, y) }
  }

  override func mouseMoved(with event: NSEvent) {
    let (x, y) = systemLocationFor(event.locationInWindow)
    withUnsafePointer(to: self) { sysMouseMoved(sys, $0, x, y) }
  }

  let sys: UnsafeMutableRawPointer

  init(_ scaleFactor: Int) {
    self.sys = sysInit(redraw);
    self.scaleFactor = scaleFactor
    self.imageBuffer = makeImageBuffer(scaleFactor)
    super.init(frame: NSRect())

    self.addTrackingArea(NSTrackingArea(rect: NSRect(), options: [.mouseMoved, .inVisibleRect, .activeInKeyWindow, .enabledDuringMouseDrag], owner: self))
  }

  required init?(coder: NSCoder) {
    return nil;
  }

  override func draw(_ dirtyRect: NSRect) {
    do {
      let data = imageBuffer.data!
      let pixels = data.bindMemory(to: UInt32.self, capacity: 0)
      sysPaint(sys, pixels, UInt32(imageBuffer.width), UInt32(imageBuffer.height), UInt32(imageBuffer.rowBytes) / 4)
      let image = try imageBuffer.createCGImage(format: imageFormat)
      let context = NSGraphicsContext.current!.cgContext
      context.draw(image, in: dirtyRect)

      // Uncomment this to continuously re-render.
      // DispatchQueue.main.async {
      //   self.needsDisplay = true
      // }
    } catch {
      print("error creating CGImage from buffer")
    }
  }
}
