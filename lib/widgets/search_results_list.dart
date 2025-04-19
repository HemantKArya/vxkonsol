// lib/widgets/search_results_list.dart
import 'package:flutter/material.dart';
import 'package:flutter_bloc/flutter_bloc.dart';
import 'package:vxkonsol/cubits/search/search_cubit.dart';
// Use the adapted SearchResult model
import 'package:vxkonsol/models/search_result.dart';
import 'package:vxkonsol/widgets/search_result_item.dart';
import 'dart:developer';
import 'package:flutter_staggered_animations/flutter_staggered_animations.dart';

class SearchResultsList extends StatefulWidget {
  // Expect List<SearchResult> now
  final List<SearchResult> results;
  final int selectedIndex;

  const SearchResultsList({
    super.key,
    required this.results,
    required this.selectedIndex,
  });

  @override
  State<SearchResultsList> createState() => _SearchResultsListState();
}

class _SearchResultsListState extends State<SearchResultsList> {
  final ScrollController _scrollController = ScrollController();
  // Keep item height, adjust if needed after testing visuals
  final double _itemHeight = 58.0;

  @override
  void didUpdateWidget(covariant SearchResultsList oldWidget) {
    super.didUpdateWidget(oldWidget);
    // Scroll to selected is important
    if (widget.selectedIndex != oldWidget.selectedIndex &&
        widget.selectedIndex >= 0 && // Ensure index is valid
        widget.selectedIndex < widget.results.length) {
      _scrollToSelected();
    }
    // Scroll to top if results change and nothing is selected (e.g., new search)
    // Check if the list *content* changed, not just the instance
    if (!listEquals(widget.results, oldWidget.results) &&
        widget.selectedIndex == -1) {
      _scrollToTop();
    }
    // Handle case where results shrink and selectedIndex becomes invalid
    else if (widget.selectedIndex >= widget.results.length &&
        widget.results.isNotEmpty) {
      // Maybe select the last item or reset selection? Resetting seems safer.
      // This scenario should ideally be handled by the cubit ensuring selectedIndex is valid.
      // For robustness here, we could scroll to top or end. Let's scroll to top.
      log("Warning: selectedIndex ${widget.selectedIndex} is out of bounds for results length ${widget.results.length}. Scrolling to top.");
      _scrollToTop();
    }
  }

  void _scrollToTop() {
    // Add check for mounted widget
    if (mounted && _scrollController.hasClients) {
      _scrollController.animateTo(
        0,
        duration: const Duration(milliseconds: 100),
        curve: Curves.easeOut,
      );
    }
  }

  void _scrollToSelected() {
    if (!mounted ||
        !_scrollController.hasClients ||
        widget.selectedIndex < 0 ||
        widget.selectedIndex >= widget.results.length) {
      log("ScrollToSelected: Aborted (mounted=$mounted, hasClients=${_scrollController.hasClients}, index=${widget.selectedIndex}, length=${widget.results.length})");
      return;
    }

    final double targetOffset = widget.selectedIndex * _itemHeight;
    final double currentScroll = _scrollController.offset;
    final double maxScroll = _scrollController.position.maxScrollExtent;
    // Ensure viewportDimension is positive, handle edge case during layout
    final double viewportHeight =
        _scrollController.position.hasViewportDimension
            ? _scrollController.position.viewportDimension
            : 0.0;

    if (viewportHeight <= 0) {
      log("ScrollToSelected: Aborted (viewportHeight <= 0)");
      return; // Cannot calculate visibility without viewport height
    }

    // Check if the item is already fully visible
    final bool isTopVisible = targetOffset >= currentScroll;
    final bool isBottomVisible =
        (targetOffset + _itemHeight) <= (currentScroll + viewportHeight);

    if (isTopVisible && isBottomVisible) {
      log("ScrollToSelected: Item $widget.selectedIndex already visible.");
      return; // Already visible, no need to scroll
    }

    // Calculate desired scroll position to center the item (approximately)
    double desiredScrollPosition =
        targetOffset - (viewportHeight / 2) + (_itemHeight / 2);

    // Clamp the desired position to valid scroll extents
    desiredScrollPosition = desiredScrollPosition.clamp(0.0, maxScroll);

    log("ScrollToSelected: Scrolling to position $desiredScrollPosition for index $widget.selectedIndex");

    _scrollController.animateTo(
      desiredScrollPosition,
      duration: const Duration(milliseconds: 100),
      curve: Curves.easeOut,
    );
  }

  @override
  void dispose() {
    _scrollController.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    // Handle loading state from cubit if needed, or show message based on results
    final searchState = context.watch<SearchCubit>().state;

    if (searchState.isLoading && widget.results.isEmpty) {
      // Show loading indicator only if there are no previous results displayed
      return const Padding(
        padding: EdgeInsets.symmetric(vertical: 20.0),
        child: Center(
          child: SizedBox(
            height: 24,
            width: 24,
            child: CircularProgressIndicator(strokeWidth: 2.5),
          ),
        ),
      );
    }

    if (widget.results.isEmpty &&
        !searchState.isLoading &&
        !searchState.showHelp) {
      // Show "No results" or error message
      return Padding(
        padding: const EdgeInsets.only(
          top: 20,
          bottom: 20,
          left: 28,
          right: 28,
        ),
        child:
            Center(child: Text(searchState.searchError ?? "No results found.")),
      );
    }

    // If results are not empty, build the list
    return AnimationLimiter(
      child: ListView.builder(
        controller: _scrollController,
        padding: const EdgeInsets.symmetric(vertical: 5, horizontal: 18),
        itemCount: widget.results.length,
        itemExtent: _itemHeight, // Use the defined item height
        itemBuilder: (context, index) {
          final result = widget.results[index];
          return AnimationConfiguration.staggeredList(
            position: index,
            duration: const Duration(
                milliseconds: 300), // Faster total duration for list items
            child: SlideAnimation(
              verticalOffset: 30.0, // Slightly smaller offset for quicker feel
              child: FadeInAnimation(
                child: SearchResultItem(
                  result: result,
                  isSelected: index == widget.selectedIndex,
                  onTap: () {
                    log("Item tapped: ${result.title} at index $index");

                    // context.read<SearchCubit>().executeSelectedAction();
                    context.read<SearchCubit>().selectAndExecuteAction(index);
                  },
                ),
              ),
            ),
          );
        },
      ),
    );
  }
}

// Helper function for list comparison (if not already available)
// Can be placed in a utility file or directly here if only used once.
bool listEquals<T>(List<T>? a, List<T>? b) {
  if (a == null) return b == null;
  if (b == null || a.length != b.length) return false;
  if (identical(a, b)) return true;
  for (int index = 0; index < a.length; index += 1) {
    if (a[index] != b[index]) return false;
  }
  return true;
}
