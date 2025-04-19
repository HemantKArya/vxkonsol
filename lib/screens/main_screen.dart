import 'dart:developer';
import 'package:fluentui_system_icons/fluentui_system_icons.dart';
import 'package:flutter/material.dart';
import 'package:flutter_bloc/flutter_bloc.dart';
import 'package:gradient_borders/gradient_borders.dart';
import 'package:vxkonsol/cubits/search/search_cubit.dart';
import 'package:vxkonsol/cubits/theme/theme_cubit.dart';
import 'package:vxkonsol/widgets/help_section.dart';
import 'package:vxkonsol/widgets/search_box.dart';
import 'package:vxkonsol/widgets/search_results_list.dart';
import 'package:window_manager/window_manager.dart';

class MainScreen extends StatefulWidget {
  const MainScreen({super.key});

  @override
  State<MainScreen> createState() => _MainScreenState();
}

class _MainScreenState extends State<MainScreen> {
  final ValueNotifier<bool> isAlwaysOnTop = ValueNotifier(false);

  Future<void> setIsAlwaysOnTop() async {
    isAlwaysOnTop.value = await windowManager.isAlwaysOnTop();
  }

  @override
  void initState() {
    setIsAlwaysOnTop();
    super.initState();
  }

  @override
  Widget build(BuildContext context) {
    final theme = Theme.of(context);

    const gradientBorder = GradientBoxBorder(
      gradient: RadialGradient(
        colors: [Colors.white, Colors.black],
        stops: [0.0, 1.0],
        radius: 5.5,
        center: Alignment.topCenter,
      ),
      width: 1,
    );

    // Define common icon properties for compactness
    const double iconSize = 20.0; // Smaller icon size (like reference image)
    const EdgeInsets iconPadding = EdgeInsets.zero; // No internal padding
    const VisualDensity iconDensity = VisualDensity.compact; // Compact layout
    const BoxConstraints iconConstraints =
        BoxConstraints(); // Tight constraints
    const double iconSpacing = 8.0; // Space between icons

    return DragToMoveArea(
      child: Scaffold(
        backgroundColor: Colors.transparent,
        body: Center(
          child: Container(
            constraints: const BoxConstraints(
              maxWidth: 700, // Optional: Add a max width if needed
            ),
            decoration: BoxDecoration(
                color: theme.scaffoldBackgroundColor,
                borderRadius: BorderRadius.circular(25),
                border: gradientBorder,
                boxShadow: [
                  BoxShadow(
                    color: Colors.black.withOpacity(0.1),
                    blurRadius: 8,
                    offset: const Offset(0, 2),
                  ),
                ]),
            child: Padding(
              padding: const EdgeInsets.all(10.0),
              child: BlocBuilder<SearchCubit, SearchState>(
                builder: (context, state) {
                  return Column(
                    mainAxisSize: MainAxisSize.max,
                    children: <Widget>[
                      // --- Top Bar ---
                      Padding(
                        padding: const EdgeInsets.only(
                          left: 18,
                          right: 18, // Keep adjusted padding for compactness
                          top: 5, // Keep adjusted padding
                          bottom: 5, // Keep adjusted padding
                        ),
                        child: Row(
                          // mainAxisAlignment: MainAxisAlignment.end,
                          children: [
                            theme.brightness == Brightness.dark
                                ? Image.asset(
                                    'assets/icons/VXKonsolPNG.png',
                                    height: 28,
                                    width: 28,
                                  )
                                : Image.asset(
                                    'assets/icons/VXKonsolPNG2.png',
                                    height: 28,
                                    width: 28,
                                  ),
                            const SizedBox(
                                width: 10), // Space between icon and text

                            const Spacer(),
                            // --- Theme Toggle Button --- (Original Position 1)
                            IconButton(
                              tooltip: "Toggle Theme",
                              iconSize: iconSize, // Apply style
                              padding: iconPadding, // Apply style
                              visualDensity: iconDensity, // Apply style
                              constraints: iconConstraints, // Apply style
                              // splashRadius: 5, // Keep splashRadius small or remove? Removed for compactness
                              icon: Icon(
                                  context.watch<ThemeCubit>().state
                                          is ThemeBright
                                      ? FluentIcons.weather_moon_16_regular
                                      : FluentIcons.weather_sunny_16_regular,
                                  color: theme.focusColor),
                              onPressed: () {
                                context.read<ThemeCubit>().toggleTheme();
                                log('Theme toggled');
                              },
                            ),
                            const SizedBox(width: iconSpacing), // Apply spacing

                            // --- Settings Button --- (Original Position 2)
                            IconButton(
                              tooltip: "Settings",
                              iconSize: iconSize, // Apply style
                              padding: iconPadding, // Apply style
                              visualDensity: iconDensity, // Apply style
                              constraints: iconConstraints, // Apply style
                              icon: Icon(FluentIcons.settings_16_regular,
                                  color: theme.primaryColor),
                              onPressed: () {
                                log("Settings pressed");
                              },
                            ),
                            const SizedBox(width: iconSpacing), // Apply spacing

                            // --- Pin Button --- (Original Position 3)
                            IconButton(
                              tooltip: "Pin",
                              iconSize: iconSize, // Apply style
                              padding: iconPadding, // Apply style
                              visualDensity: iconDensity, // Apply style
                              constraints: iconConstraints, // Apply style
                              icon: ValueListenableBuilder(
                                builder: (context, value, child) {
                                  return Icon(
                                    value
                                        ? FluentIcons.pin_16_filled
                                        : FluentIcons.pin_16_regular,
                                    color: theme.primaryColor,
                                  );
                                },
                                valueListenable: isAlwaysOnTop,
                              ),
                              onPressed: () async {
                                final onTop =
                                    await windowManager.isAlwaysOnTop();
                                isAlwaysOnTop.value = !onTop;
                                windowManager.setAlwaysOnTop(!onTop);
                                log("Pin pressed");
                              },
                            ),
                            const SizedBox(width: iconSpacing), // Apply spacing

                            // --- Hide Button --- (Original Position 5)
                            IconButton(
                              tooltip: "Hide",
                              iconSize: iconSize, // Apply style
                              padding: iconPadding, // Apply style
                              visualDensity: iconDensity, // Apply style
                              constraints: iconConstraints, // Apply style
                              // splashRadius: 5, // Removed for compactness
                              icon: Icon(FluentIcons.dismiss_16_regular,
                                  color:
                                      theme.iconTheme.color?.withOpacity(0.7) ??
                                          theme.primaryColor),
                              onPressed: () {
                                windowManager.hide();
                              },
                            ),
                          ],
                        ),
                      ),
                      // --- Search Box ---
                      const SearchBox(),

                      // --- Conditional Content ---
                      Expanded(
                        // Keep Expanded to constrain the AnimatedSwitcher
                        child: AnimatedSwitcher(
                          duration: const Duration(
                              milliseconds: 150), // Faster fade duration
                          transitionBuilder:
                              (Widget child, Animation<double> animation) {
                            return FadeTransition(
                                opacity: animation, child: child);
                          },
                          child: _buildConditionalContent(
                              state), // Use a helper method
                        ),
                      ),
                    ],
                  );
                },
              ),
            ),
          ),
        ),
      ),
    );
  }

  // Helper method to build the content based on state, needed for AnimatedSwitcher key
  Widget _buildConditionalContent(SearchState state) {
    if (state.isLoading) {
      return const Center(
        // Center the indicator
        key: ValueKey('loading'), // Add key for AnimatedSwitcher
        child: SizedBox(
          height: 24,
          width: 24,
          child: CircularProgressIndicator(strokeWidth: 2.5),
        ),
      );
    } else if (state.showHelp) {
      // Wrap HelpSection in a way that Expanded works correctly if needed,
      // or ensure HelpSection itself handles its layout appropriately.
      // If HelpSection should also fill space, wrap it similarly.
      // For now, assuming it has its own intrinsic size.
      return const Align(
          // Align if it shouldn't expand
          key: ValueKey('help'), // Add key
          alignment: Alignment.topCenter, // Or desired alignment
          child: HelpSection());
    } else {
      return SearchResultsList(
        key: const ValueKey('results'), // Add key
        results: state.results,
        selectedIndex: state.selectedIndex,
      );
    }
  }
}
