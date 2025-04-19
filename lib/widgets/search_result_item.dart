// lib/widgets/search_result_item.dart
import 'package:flutter/material.dart';
import 'package:vxkonsol/models/search_result.dart'; // Use the adapted SearchResult

class SearchResultItem extends StatelessWidget {
  final SearchResult result;
  final bool isSelected;
  final VoidCallback? onTap;
  // Shortcut text removed for now, add back if needed based on index

  const SearchResultItem({
    super.key,
    required this.result,
    required this.isSelected,
    this.onTap,
  });

  Widget _buildIcon(BuildContext context, SearchResult result, Color color) {
    final bytes = result.iconBytes;
    if (bytes != null && bytes.isNotEmpty) {
      // Use FadeInImage for smoother loading if icons take time
      return Image.memory(
        bytes,
        width: 22, // Adjust size
        height: 22,
        fit: BoxFit.contain,
        gaplessPlayback: true, // Avoid flicker when icon data changes
        filterQuality: FilterQuality.medium, // Balance quality and performance
        errorBuilder: (context, error, stackTrace) {
          // Log the error only in debug mode to avoid console spam
          // Consider more robust error logging if needed
          // dev.log('Error loading image memory for ${result.title}: $error');
          if (result.icon != null) {
            return Icon(result.icon,
                size: 20, color: color); // Fallback icon data
          } else {
            return Icon(Icons.apps_outlined,
                size: 20, color: color); // Generic fallback
          }
        },
        // Optional: Add a placeholder while loading, though local icons should be fast
        // placeholder: (context, url) => SizedBox(width: 20, height: 20, child: CircularProgressIndicator(strokeWidth: 1)),
      );
    } else if (result.icon != null) {
      // Use the fallback IconData if bytes are not available
      return Icon(result.icon, size: 20, color: color);
    } else {
      // Default fallback if no icon data or bytes provided
      return Icon(Icons.apps_outlined, size: 20, color: color);
    }
  }

  @override
  Widget build(BuildContext context) {
    final theme = Theme.of(context);
    final colorScheme = theme.colorScheme;
    final bool isDark = theme.brightness == Brightness.dark;

    // --- Define Colors (Keep existing logic, seems reasonable) ---
    final Color selectedBackgroundColor = isDark
        ? Colors.white.withOpacity(0.08) // Slightly more visible selection
        : colorScheme.primary.withOpacity(0.1);

    final Color currentBackgroundColor =
        isSelected ? selectedBackgroundColor : Colors.transparent;

    // Content Colors should contrast with both background and selected background
    final Color contentColor = isSelected
        ? (isDark
            ? Colors.white
            : Colors.grey.shade800) // Higher contrast when selected
        : (isDark ? Colors.grey.shade300 : colorScheme.onSurface);

    final Color descriptionColor = isSelected
        ? (isDark ? Colors.grey.shade400 : Colors.grey.shade800)
        : (isDark
            ? Colors.grey.shade500
            : colorScheme.onSurface.withOpacity(0.7));

    // Interaction Colors
    final Color hoverColor = isDark
        ? Colors.white.withOpacity(0.04)
        : colorScheme.onSurface.withOpacity(0.04);
    final Color splashColor = isDark
        ? Colors.white.withOpacity(0.08)
        : colorScheme.primary.withOpacity(0.08);

    // --- Dimensions ---
    final BorderRadius itemBorderRadius = BorderRadius.circular(8.0);

    return Material(
      color: currentBackgroundColor,
      borderRadius: itemBorderRadius,
      child: InkWell(
        // Use the onTap passed from the list, which triggers cubit's execute
        onTap: onTap,
        borderRadius: itemBorderRadius,
        hoverColor: hoverColor,
        splashColor: splashColor,
        highlightColor: splashColor,
        child: Padding(
          padding: const EdgeInsets.symmetric(horizontal: 12, vertical: 8),
          child: Row(
            crossAxisAlignment: CrossAxisAlignment.center,
            children: [
              // Icon - Use the helper function
              _buildIcon(context, result, contentColor),
              const SizedBox(width: 12),

              // Title and Description
              Expanded(
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  mainAxisAlignment: MainAxisAlignment.center,
                  children: [
                    Text(
                      result.title, // Use title from SearchResult
                      style: theme.textTheme.bodyMedium?.copyWith(
                        color: contentColor,
                        fontWeight: FontWeight.w500,
                        fontSize: 15, // Slightly larger?
                      ),
                      maxLines: 1,
                      overflow: TextOverflow.ellipsis,
                    ),
                    if (result.description != null &&
                        result.description!.isNotEmpty &&
                        result.description != result.title)
                      Text(
                        result.description!, // Use description (path)
                        style: theme.textTheme.bodySmall?.copyWith(
                          color: descriptionColor,
                          fontSize: 12.5, // Slightly larger?
                          fontWeight: FontWeight.w500,
                        ),
                        maxLines: 1,
                        overflow: TextOverflow.ellipsis,
                      )
                    else
                      Text(
                        result.path, // Use description (path)
                        style: theme.textTheme.bodySmall?.copyWith(
                          color: descriptionColor,
                          fontSize: 12.5, // Slightly larger?
                          fontWeight: FontWeight.w500,
                        ),
                        maxLines: 1,
                        overflow: TextOverflow.ellipsis,
                      ), // Avoids empty space if no description
                  ],
                ),
              ),
              // Shortcut badge removed for now
            ],
          ),
        ),
      ),
    );
  }
}
