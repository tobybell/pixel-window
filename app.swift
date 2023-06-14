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
      contentRect: NSRect(x: 0, y: 0, width: 511, height: 512),
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
}

func makeImageBuffer(_ scaleFactor: Int) -> vImage_Buffer {
  do {
    return try vImage_Buffer(width: scaleFactor * 511, height: scaleFactor * 512, bitsPerPixel: 32)
  } catch {
    print("failed to make image buffer")
    abort()
  }
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

  override func mouseDown(with event: NSEvent) {
    if (event.type == .leftMouseDown) {
      let location = event.locationInWindow;
      let scale = Float(scaleFactor)
      sysMouseDown(sys, Float(location.x) * scale, Float(512 - location.y) * scale)
    } else {
      print("got event \(event)")
    }
  }

  let sys = sysInit();

  init(_ scaleFactor: Int) {
    self.scaleFactor = scaleFactor
    self.imageBuffer = makeImageBuffer(scaleFactor)
    super.init(frame: NSRect())
  }

  required init?(coder: NSCoder) {
    self.scaleFactor = 1
    self.imageBuffer = makeImageBuffer(self.scaleFactor)
    super.init(coder: coder)
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
      DispatchQueue.main.async {
        self.needsDisplay = true
      }
    } catch {
      print("error creating CGImage from buffer")
    }
  }
}
