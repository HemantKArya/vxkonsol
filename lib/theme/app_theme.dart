// lib/theme/app_theme.dart
import 'package:flutter/material.dart';

// --- Light Theme ---
// (Keeping light theme mostly as is, but ensure consistency where possible)
final ThemeData lightTheme = ThemeData(
  fontFamily: 'Inconsolata', // Add this line
  brightness: Brightness.light,
  primaryColor: Colors.blueGrey[700], // A main accent color
  focusColor:
      Colors.blueGrey[900], // For specific focused elements like top icons
  scaffoldBackgroundColor: Colors.grey[100], // Slightly off-white background
  cardColor: Colors.white, // Background for cards, search bar, list items
  hintColor: Colors.grey[500],
  iconTheme: IconThemeData(
    color: Colors.grey[700], // Default icon color matches text
  ),
  textTheme: TextTheme(
    // Search text, List Title
    bodyLarge: const TextStyle(
        color: Colors.black87, fontWeight: FontWeight.w500, fontSize: 14),
    // Description text
    bodyMedium: const TextStyle(
        color: Colors.black54,
        fontSize: 13,
        fontWeight: FontWeight.w500), // Make subtitle medium weight
    // Badges text, Help section description
    bodySmall: TextStyle(
        color: Colors.grey[600],
        fontWeight: FontWeight.normal, // Normal weight for badges
        fontSize: 11),
  ),
  textSelectionTheme: TextSelectionThemeData(
    cursorColor: Colors.blueGrey[800],
    selectionColor: Colors.blueGrey[100],
    selectionHandleColor: Colors.blueGrey[800],
  ),
  dividerColor: Colors.grey[300],
  colorScheme: ColorScheme.light(
    primary: Colors.blueGrey[700]!, // Main accent
    secondary: Colors.blueAccent, // Secondary accent
    surface: Colors.white, // Main window background
    error: Colors.red[600]!,
    onPrimary: Colors.white, // Text on primary bg
    onSecondary: Colors.white, // Text on secondary bg
    onSurface: Colors.grey[800]!, // Main text on window background
    onError: Colors.white,
    brightness: Brightness.light,
  ),
);

// --- Dark Theme ---
// (Revised significantly based on screenshots)
final ThemeData darkTheme = ThemeData(
  fontFamily: 'Inconsolata', // Add this line
  brightness: Brightness.dark,
  // Primary isn't visually dominant; use content color or a subtle accent.
  primaryColor: Colors.grey[400],
  // Color for top-right icons (slightly lighter gray)
  focusColor: Colors.grey[500],
  // Very dark background (near black)
  scaffoldBackgroundColor: const Color(0xFF1A1A1A),
  // Dark gray for Search bar, selected items, badges background
  cardColor: const Color(0xFF2C2C2C),
  // Hint text color inside the search bar
  hintColor: Colors.grey[600],
  // Default Icon color (matches main text)
  iconTheme: IconThemeData(
    color: Colors.grey[300], // Light gray for icons
  ),
  textTheme: TextTheme(
    // Search text, List Title
    bodyLarge: TextStyle(
        color: Colors.grey[300], // Light gray
        fontWeight: FontWeight.w500, // Slightly bolder for titles/search
        fontSize: 14),
    // Description text in list items
    bodyMedium: TextStyle(
        color: Colors.grey[400], // Slightly dimmer gray for descriptions
        fontSize: 13,
        fontWeight: FontWeight.w500), // Make subtitle medium weight
    // Badges text, Help section description text
    bodySmall: TextStyle(
        color: Colors.grey[500], // Even dimmer gray for tertiary info/badges
        fontWeight: FontWeight.normal, // Normal weight for badges
        fontSize: 11),
  ),
  // Text selection style within the search bar
  textSelectionTheme: TextSelectionThemeData(
    cursorColor: Colors.grey[400], // Light gray cursor
    selectionColor: Colors.grey[700]?.withOpacity(0.6), // Darker gray selection
    selectionHandleColor: Colors.grey[400],
  ),
  dividerColor: Colors.grey[800], // For any dividers used
  colorScheme: ColorScheme.dark(
    // Main light gray content color (text/icons)
    primary: Colors.grey[300]!,
    // A secondary content color if needed (e.g., descriptions)
    secondary: Colors.grey[400]!,
    // Background for search bar, selected items, cards, badges
    surface: const Color(0xFF2C2C2C),
    error: Colors.redAccent[100]!,
    // Color of text/icons *ON* a primary-colored background (if used)
    onPrimary: Colors.black, // Likely black if primary were a light color
    // Color of text/icons *ON* a secondary-colored background
    onSecondary: Colors.black,
    // Color of text/icons *ON* the surface color (e.g., text in search bar)
    onSurface: Colors.grey[300]!, // Light gray text on near-black background
    onError: Colors.black,
    brightness: Brightness.dark,
  ),
);

// --- Shaders ---
// Shader for Search Hint Text (Dark Theme) - Needs to be visible on cardColor (0xFF2C2C2C)
final Shader linearGradientDark = LinearGradient(
  // Subtle gradient from a slightly visible gray to a more faded one
  colors: <Color>[Colors.grey[600]!, Colors.grey[700]!.withOpacity(0.8)],
).createShader(const Rect.fromLTWH(0.0, 0.0, 200.0, 70.0));

// Shader for Search Hint Text (Light Theme) - Needs to be visible on surface (white)
final Shader linearGradientLight = LinearGradient(
  colors: <Color>[
    Colors.grey[600]!, // Start with a visible gray
    Colors.grey[400]!.withOpacity(0.8) // Fade to a lighter gray
  ],
).createShader(const Rect.fromLTWH(0.0, 0.0, 200.0, 70.0));
