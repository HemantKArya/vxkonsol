// lib/core/window_setup.dart
import 'package:flutter/material.dart';
import 'package:flutter_acrylic/flutter_acrylic.dart';
import 'package:window_manager/window_manager.dart';

Future<void> setupWindow() async {
  await Window.initialize();
  await windowManager.ensureInitialized();
  const double screenWidth = 500;

  const windowOptions = WindowOptions(
    titleBarStyle: TitleBarStyle.hidden,
    center: true,
    title: 'VXKonsol',
    size: Size(screenWidth, 230), // Increased from 205
    minimumSize: Size(screenWidth, 230), // Increased from 205
    maximumSize: Size(screenWidth, 450), // Keep max size if needed
    skipTaskbar: false,
  );

  await windowManager.waitUntilReadyToShow(windowOptions, () async {
    await windowManager.setAsFrameless();
    await Window.setEffect(
      effect: WindowEffect.transparent,
      color: Colors.transparent,
    );
    await windowManager.setHasShadow(false);
    // await windowManager.hide();
  });
}

Future<void> resizeWindow(bool hasResults) async {
  // Adjust the collapsed height to match the new minimum
  // Keep expanded height as desired (e.g., 350 or adjust if needed)
  final double targetHeight =
      hasResults ? 350 : 230; // Use the new minimum height
  final Size currentSize = await windowManager.getSize();

  if (currentSize.height.round() != targetHeight.round()) {
    try {
      await windowManager.setSize(Size(currentSize.width, targetHeight),
          animate: true);
      // await windowManager.center(); // Optional
    } catch (e) {
      debugPrint("Error resizing window: $e");
    }
  }
}
