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
      styleMask: [.titled, .closable, .miniaturizable, .fullSizeContentView],
      backing: .buffered, defer: false)
    window.center()
    window.setFrameAutosaveName("Pixel Window")
    window.contentView = PixelView()
    window.makeKeyAndOrderFront(nil)
    window.delegate = self
    NSApp.activate(ignoringOtherApps: true)
  }
}

func makeImageBuffer() -> vImage_Buffer {
  do {
    return try vImage_Buffer(width: 511, height: 512, bitsPerPixel: 32)
  } catch {
    print("failed to make image buffer")
    abort()
  }
}

class PixelView: NSView {

  let imageFormat = vImage_CGImageFormat(
      bitsPerComponent: 8,
      bitsPerPixel: 32,
      colorSpace: CGColorSpaceCreateDeviceRGB(),
      bitmapInfo: CGBitmapInfo(rawValue: CGImageAlphaInfo.first.rawValue),
      renderingIntent: .defaultIntent)!

  let imageBuffer = makeImageBuffer()

  override func draw(_ dirtyRect: NSRect) {
    do {
      let data = imageBuffer.data!
      let pixels = data.bindMemory(to: UInt32.self, capacity: 0)
      paint(pixels, UInt32(imageBuffer.width), UInt32(imageBuffer.height), UInt32(imageBuffer.rowBytes) / 4)
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
