import 'dart:developer';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_bloc/flutter_bloc.dart';
import 'package:vxkonsol/cubits/search/search_cubit.dart';
import 'package:get/get.dart';
import 'package:hotkey_manager/hotkey_manager.dart';
import 'package:vxkonsol/app.dart';
import 'package:vxkonsol/core/global_controller.dart';
import 'package:vxkonsol/core/hotkeys.dart';
import 'package:vxkonsol/core/window_setup.dart';
import 'package:window_manager/window_manager.dart';

final searchCubit = SearchCubit();

bool _handleRawKeyEvent(KeyEvent event) {
  final globalController = Get.find<GlobalController>();

  if (event is KeyDownEvent) {
    // Only act on key down
    if (event.logicalKey == LogicalKeyboardKey.enter ||
        event.logicalKey == LogicalKeyboardKey.numpadEnter) {
      log("Enter captured -> Executing action, selecting text, hiding window");
      final controller = globalController.searchController;
      if (controller.text.isNotEmpty) {
        controller.selection = TextSelection(
          baseOffset: 0,
          extentOffset: controller.text.length,
        );
      }
      searchCubit.executeSelectedAction();
      windowManager.hide();
    } else if (event.logicalKey == LogicalKeyboardKey.arrowDown) {
      log("Arrow Down captured -> Delegating to Cubit selectNext");
      searchCubit.selectNext();
    }
    // --- Arrows ---
    else if (event.logicalKey == LogicalKeyboardKey.arrowUp) {
      log("Arrow Up captured -> Delegating to Cubit selectPrevious");
      searchCubit.selectPrevious();
    } else if (event.logicalKey == LogicalKeyboardKey.tab) {
      log("Tab captured -> Delegating to Cubit selectNext");
      searchCubit.selectNext();
    }

    // --- Escape Key ---
    else if (event.logicalKey == LogicalKeyboardKey.escape) {
      log("Escape captured");
      if (globalController.searchController.text.isNotEmpty) {
        log("Clearing text field and resetting search.");
        globalController.searchController.clear();
        searchCubit.resetSearch();
        globalController.searchFocusNode.requestFocus();
      } else {
        log("Hiding window.");
        windowManager.hide();
      }
      return true;
    }
  } else if (event is KeyUpEvent) {
    if (event.logicalKey == LogicalKeyboardKey.enter ||
        event.logicalKey == LogicalKeyboardKey.numpadEnter) {
      log("Enter captured -> Executing action, selecting text, hiding window");

      searchCubit.executeSelectedAction();
      return true;
    }
  }
  // Indicate that the event was not handled
  return false;
}

Future<void> main() async {
  WidgetsFlutterBinding.ensureInitialized();
  Get.put(GlobalController()); // Initialize and register the controller
  log('GlobalController Initialized');

  HardwareKeyboard.instance.addHandler(_handleRawKeyEvent);
  log('Global Key Listener Added');

  await hotKeyManager.unregisterAll();
  log('Hotkey Manager Initialized');

  await setupWindow();
  log('Window Setup Complete');

  await registerHotkeys();
  log('Hotkeys Registered');

  runApp(BlocProvider(
    create: (context) => searchCubit,
    child: const MyApp(),
  ));
  log('App Running');
}
