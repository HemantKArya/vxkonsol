import 'package:flutter/material.dart';
import 'package:vxkonsol/widgets/shortcut_info_widget.dart';

class HelpSection extends StatelessWidget {
  const HelpSection({super.key});

  @override
  Widget build(BuildContext context) {
    // Added vertical padding for spacing
    return const Padding(
      padding: EdgeInsets.symmetric(horizontal: 8.0, vertical: 15.0),
      child: Wrap(
        spacing: 16, // Horizontal space between items
        runSpacing: 10, // Vertical space between lines
        alignment: WrapAlignment.center, // Center items horizontally
        children: [
          ShortcutInfoWidget(
            keyPair: "Shift + Enter",
            use: "Get web results",
          ),
          ShortcutInfoWidget(
            keyPair: "Alt + Q",
            use: "Toggle VXKonsol", // Updated use text
          ),
          ShortcutInfoWidget(
            keyPair: "↑ / ↓", // Indicate arrow keys
            use: "Navigate Results",
          ),
          ShortcutInfoWidget(
            keyPair: "Enter",
            use: "Execute Action",
          ),
          ShortcutInfoWidget(
            keyPair: "Esc",
            use: "Clear Search / Hide", // Added Esc action
          ),
          // Add more shortcuts as needed
          // ShortcutInfoWidget(
          //   keyPair: "Alt + Enter",
          //   use: "Open in browser", // Example
          // ),
        ],
      ),
    );
  }
}
