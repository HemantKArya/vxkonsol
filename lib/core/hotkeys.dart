// lib/core/hotkeys.dart
import 'dart:developer';
// Remove Flutter Material import if no longer needed for FocusNode type directly
// import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:get/get.dart'; // Import GetX
import 'package:hotkey_manager/hotkey_manager.dart';
import 'package:window_manager/window_manager.dart';
import 'package:vxkonsol/core/global_controller.dart'; // Import the controller

// Remove old FocusNode management
// FocusNode? _searchFocusNode;
// void setSearchFocusNode(FocusNode node) {
//   _searchFocusNode = node;
//   log("Global searchFocusNode set in hotkeys.dart");
// }
// FocusNode? get searchFocusNode => _searchFocusNode;

// Define HotKeys (remains the same)
final kToggleWindowHotKey = HotKey(
  key: PhysicalKeyboardKey.keyQ,
  modifiers: [HotKeyModifier.alt],
  scope: HotKeyScope.system,
);

// --- Hotkey Registration --- (remains the same)
Future<void> registerHotkeys() async {
  await hotKeyManager.unregisterAll(); // Ensure clean state

  await hotKeyManager.register(
    kToggleWindowHotKey,
    keyDownHandler: _handleToggleWindowHotkey,
    // ... other handlers
  );
  log('Hotkey ${kToggleWindowHotKey.key.keyLabel} registered.');
}

// --- Hotkey Handlers ---
void _handleToggleWindowHotkey(HotKey hotKey) async {
  // Access the controller and FocusNode via GetX
  final GlobalController globalController = Get.find<GlobalController>();
  final searchFocusNode = globalController.searchFocusNode;

  // Remove the null check for the old _searchFocusNode
  // if (_searchFocusNode == null) { ... }

  try {
    final isVisible = await windowManager.isVisible();
    final isFocused = await windowManager.isFocused();
    log("Hotkey triggered. Visible: $isVisible, Focused: $isFocused");

    if (isVisible && isFocused) {
      log("Hiding window.");
      await windowManager.hide();
    } else if (isVisible && !isFocused) {
      log("Focusing existing visible window and text field.");
      await windowManager.focus();
      searchFocusNode.requestFocus(); // Use the focus node from the controller
    } else {
      // Not visible
      log("Showing and focusing window and text field.");
      await windowManager.show();
      await windowManager.focus();
      searchFocusNode.requestFocus(); // Use the focus node from the controller
    }
  } catch (e) {
    log("Error handling hotkey: $e", error: e, level: 1000);
  }
}
