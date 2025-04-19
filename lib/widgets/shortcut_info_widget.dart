import 'package:flutter/material.dart';

class ShortcutInfoWidget extends StatelessWidget {
  final String keyPair;
  final String use;
  final bool small; // Added flag for potentially smaller version

  const ShortcutInfoWidget({
    super.key,
    required this.keyPair,
    required this.use,
    this.small = false, // Default to normal size
  });

  @override
  Widget build(BuildContext context) {
    final textStyle = TextStyle(
      // Use bodySmall color by default, maybe slightly different if needed
      color: Theme.of(context).textTheme.bodySmall!.color?.withOpacity(0.8),
      fontWeight: FontWeight.w700,
      fontSize: small ? 10 : 12, // Adjust size based on flag
    );

    final containerPadding = small
        ? const EdgeInsets.symmetric(horizontal: 4, vertical: 1)
        : const EdgeInsets.symmetric(horizontal: 8, vertical: 2);
    final containerMargin = small
        ? EdgeInsets.zero
        : const EdgeInsets.only(
            right: 4); // Add space before 'use' text only if not small

    return Row(
      mainAxisSize: MainAxisSize.min,
      crossAxisAlignment: CrossAxisAlignment.center,
      children: [
        Container(
          padding: containerPadding,
          margin: containerMargin,
          decoration: BoxDecoration(
              // Use a subtle background, adjust opacity as needed
              color: Theme.of(context).colorScheme.onSurface.withOpacity(0.08),
              borderRadius: BorderRadius.circular(
                  small ? 4 : 6), // Smaller radius if small
              border: Border.all(
                color:
                    Theme.of(context).colorScheme.onSurface.withOpacity(0.15),
                width: 0.5,
              )),
          child: Text(
            keyPair,
            style: textStyle.copyWith(
              fontWeight: FontWeight.w600, // Make key slightly bolder
              // Slightly larger font for key itself if not small
              fontSize: small ? 10 : 12,
            ),
          ),
        ),
        // Only show the 'use' text if it's provided and not in small mode
        if (use.isNotEmpty && !small)
          Flexible(
            // Use Flexible to prevent overflow if use text is long
            child: Text(
              ' $use', // Add space manually
              style: textStyle,
              overflow: TextOverflow.ellipsis, // Prevent long text overflow
            ),
          ),
      ],
    );
  }
}
