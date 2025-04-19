import 'dart:developer';
import 'package:fluentui_system_icons/fluentui_system_icons.dart';
import 'package:flutter/material.dart';
import 'package:flutter_bloc/flutter_bloc.dart';
import 'package:vxkonsol/cubits/theme/theme_cubit.dart';
import 'package:window_manager/window_manager.dart'; // For closing

class WindowControls extends StatelessWidget {
  const WindowControls({super.key});

  @override
  Widget build(BuildContext context) {
    // Use iconTheme color as default, allow overrides if needed
    final iconColor = Theme.of(context).iconTheme.color?.withOpacity(0.8);

    return Padding(
      // Reduced padding slightly
      padding: const EdgeInsets.only(left: 10, right: 15, top: 5, bottom: 0),
      child: Row(
        // Changed from OverflowBar to Row for simplicity
        mainAxisAlignment: MainAxisAlignment.end, // Align to the right
        children: [
          IconButton(
            tooltip: "Toggle Theme",
            iconSize: 16,
            splashRadius: 18,
            icon: Icon(
                context.watch<ThemeCubit>().state
                        is ThemeBright // Watch for changes
                    ? FluentIcons.weather_moon_16_regular
                    : FluentIcons.weather_sunny_16_regular,
                color: iconColor),
            onPressed: () {
              context.read<ThemeCubit>().toggleTheme();
              log('Theme toggled');
            },
          ),
          IconButton(
            tooltip: "Settings (Not Implemented)",
            iconSize: 16,
            splashRadius: 18,
            icon: Icon(FluentIcons.settings_16_regular, color: iconColor),
            onPressed: () {
              log("Settings button pressed");
              // TODO: Implement Settings action
            },
          ),
          // IconButton( // Example: Pin window - Needs state management
          //   tooltip: "Pin Window (Not Implemented)",
          //   iconSize: 16,
          //   splashRadius: 18,
          //   icon: Icon(FluentIcons.pin_16_regular, color: iconColor),
          //   onPressed: () {
          //     // TODO: Implement Pin action (toggle windowManager.setAlwaysOnTop)
          //   },
          // ),
          IconButton(
            // Added Close button
            tooltip: "Close",
            iconSize: 16,
            splashRadius: 18,
            icon: Icon(FluentIcons.dismiss_16_regular, color: iconColor),
            onPressed: () {
              windowManager.hide(); // Or windowManager.close() to exit app
            },
          ),
        ],
      ),
    );
  }
}
