// lib/cubits/search/search_state.dart
part of 'search_cubit.dart';

@immutable
class SearchState {
  final String query;
  final List<SearchResult> results; // These are the *displayed* results
  final int selectedIndex;
  final bool isLoading; // Loading state for *dynamic* search
  final bool showHelp;
  final String? searchError; // Error during dynamic search

  // --- New fields for initial program loading ---
  final List<ProgramInfo> allInstalledPrograms; // Holds all fetched programs
  final bool isLoadingInstalledPrograms; // Loading state for initial fetch
  final String? installedProgramsError; // Error during initial fetch

  const SearchState({
    this.query = '',
    this.results = const [],
    this.selectedIndex = -1, // Start with no selection when results appear
    this.isLoading = false,
    this.showHelp = true, // Show help initially
    this.searchError,
    // Initial fetch defaults
    this.allInstalledPrograms = const [],
    this.isLoadingInstalledPrograms = true, // Start in loading state
    this.installedProgramsError,
  });

  SearchState copyWith({
    String? query,
    List<SearchResult>? results,
    int? selectedIndex,
    bool? isLoading,
    bool? showHelp,
    String? searchError,
    // New fields
    List<ProgramInfo>? allInstalledPrograms,
    bool? isLoadingInstalledPrograms,
    String? installedProgramsError,
    bool clearSearchError = false, // Helper to clear error easily
    bool clearInstalledProgramsError = false, // Helper to clear error easily
  }) {
    return SearchState(
      query: query ?? this.query,
      results: results ?? this.results,
      selectedIndex: selectedIndex ?? this.selectedIndex,
      isLoading: isLoading ?? this.isLoading,
      showHelp: showHelp ?? this.showHelp,
      searchError: clearSearchError ? null : searchError ?? this.searchError,
      // New fields
      allInstalledPrograms: allInstalledPrograms ?? this.allInstalledPrograms,
      isLoadingInstalledPrograms:
          isLoadingInstalledPrograms ?? this.isLoadingInstalledPrograms,
      installedProgramsError: clearInstalledProgramsError
          ? null
          : installedProgramsError ?? this.installedProgramsError,
    );
  }

  @override
  bool operator ==(Object other) =>
      identical(this, other) ||
      other is SearchState &&
          runtimeType == other.runtimeType &&
          query == other.query &&
          listEquals(results, other.results) && // Use listEquals for lists
          selectedIndex == other.selectedIndex &&
          isLoading == other.isLoading &&
          showHelp == other.showHelp &&
          searchError == other.searchError &&
          listEquals(allInstalledPrograms,
              other.allInstalledPrograms) && // Use listEquals
          isLoadingInstalledPrograms == other.isLoadingInstalledPrograms &&
          installedProgramsError == other.installedProgramsError;

  @override
  int get hashCode =>
      query.hashCode ^
      results.hashCode ^ // Consider deep hash if listEquals is slow
      selectedIndex.hashCode ^
      isLoading.hashCode ^
      showHelp.hashCode ^
      searchError.hashCode ^
      allInstalledPrograms.hashCode ^ // Consider deep hash
      isLoadingInstalledPrograms.hashCode ^
      installedProgramsError.hashCode;
}
