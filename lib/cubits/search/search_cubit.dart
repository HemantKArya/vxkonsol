// lib/cubits/search/search_cubit.dart
import 'dart:async';
import 'dart:developer';
import 'package:bloc/bloc.dart';
import 'package:fluentui_system_icons/fluentui_system_icons.dart';
import 'package:flutter/foundation.dart'; // For kDebugMode, listEquals, compute
import 'package:flutter/material.dart'; // For IconData etc.
import 'package:flutter/services.dart'; // For MethodChannel, PlatformException, RootIsolateToken
import 'package:get/get.dart';
import 'package:vxkonsol/core/global_controller.dart';
import 'package:vxkonsol/core/search_result_sorter.dart';
import 'package:vxkonsol/core/window_setup.dart'; // Assuming window resize logic is here
import 'package:vxkonsol/models/search_result.dart';
import 'package:vxkonsol/native_apis/program_fetcher.dart';
import 'package:vxkonsol/native_apis/program_info.dart';
import 'package:vxkonsol/native_apis/window_search.dart';
import 'package:path/path.dart' as p;
import 'package:window_manager/window_manager.dart'; // Make sure this import is present

part 'search_state.dart'; // Assuming search_state.dart is in the same directory

class SearchCubit extends Cubit<SearchState> {
  // Platform channel for opening items
  static const MethodChannel _platformChannel =
      MethodChannel('windows_native_channel');

  final GlobalController _globalController = Get.find<GlobalController>();

  Timer? _debounce;
  // Keep track of the current search operation ID to ignore stale results
  int _currentSearchId = 0;

  // --- Keywords to Filter Out (Case-insensitive) ---
  // This list defines keywords that, if found in the program's name or path,
  // will cause the item to be excluded from the search results.
  static const List<String> _bannedKeywords = [
    'uninstall', // Common uninstaller keyword
    'unins', // Common prefix for uninstaller executables
    'remove', // Another common term for removal tools
    'setup',
    'installer',
  ];

  SearchCubit() : super(const SearchState()) {
    // Start loading installed programs immediately when the cubit is created.
    _loadInstalledPrograms();
  }

  // --- Initialization ---
  /// Fetches the list of all installed programs in the background using an isolate.
  /// Updates the state with the loaded programs or an error.
  Future<void> _loadInstalledPrograms() async {
    log("[SearchCubit] Initializing: Fetching installed programs...");
    // Ensure state reflects loading, only if not already loading
    if (!state.isLoadingInstalledPrograms) {
      emit(state.copyWith(
          isLoadingInstalledPrograms: true, clearInstalledProgramsError: true));
    }

    final RootIsolateToken? rootIsolateToken = RootIsolateToken.instance;
    if (rootIsolateToken == null) {
      const errorMsg =
          "Fatal Error: RootIsolateToken is null. Cannot fetch programs.";
      log(errorMsg, level: 1000); // Log severe errors
      if (!isClosed) {
        emit(state.copyWith(
          isLoadingInstalledPrograms: false,
          installedProgramsError: errorMsg,
        ));
      }
      return;
    }

    try {
      // Use compute to run the fetching logic in a separate isolate.
      final List<ProgramInfo> programs =
          await compute(fetchAllProgramsIsolateEntry, rootIsolateToken);

      log("[SearchCubit] Initial program fetch completed. Received ${programs.length} unique programs.");
      if (!isClosed) {
        // Check if cubit is closed before emitting
        emit(state.copyWith(
          allInstalledPrograms: programs,
          isLoadingInstalledPrograms: false,
          clearInstalledProgramsError: true, // Clear any previous error
        ));
      }
    } catch (e, s) {
      final errorMsg = "Error fetching installed programs: $e";
      log(errorMsg, stackTrace: s, level: 1000); // Log errors with stack trace
      if (!isClosed) {
        emit(state.copyWith(
          isLoadingInstalledPrograms: false,
          installedProgramsError: errorMsg,
          allInstalledPrograms: [], // Clear any potentially partial data
        ));
      }
    }
  }

  // --- Search Logic ---
  /// Performs a search based on the provided query.
  /// Combines results from the pre-loaded installed programs list and
  /// real-time Windows Search Index results.
  /// Filters out items based on banned keywords.
  /// Sorts the results based on type priority, relevance, and title.
  /// Updates the state with the results, loading status, and errors.
  void search(String query) {
    // Increment search ID to invalidate previous pending operations
    final searchId = ++_currentSearchId;
    log("[SearchCubit] Search requested (ID: $searchId): '$query'");

    // Cancel any existing debounce timer
    if (_debounce?.isActive ?? false) _debounce?.cancel();

    // Immediately update the UI state: show the query, indicate loading (if query is not empty),
    // reset selection, and clear previous search errors.
    emit(state.copyWith(
      query: query,
      isLoading: query.isNotEmpty,
      selectedIndex: -1,
      clearSearchError: true,
    ));

    // If the query is empty, reset the search state immediately without debouncing.
    if (query.isEmpty) {
      log("[SearchCubit] Query empty (ID: $searchId), resetting search.");
      resetSearch();
      return;
    }

    // Debounce the search operation to avoid excessive processing while typing.
    _debounce = Timer(const Duration(milliseconds: 250), () async {
      // Before executing the search, check if this is still the latest request
      // or if the cubit has been closed.
      if (searchId != _currentSearchId || isClosed) {
        log("[SearchCubit] Skipping stale search (ID: $searchId, Current: $_currentSearchId)");
        return;
      }
      log("[SearchCubit] Debounce finished (ID: $searchId). Performing search for '$query'.");

      final lowerCaseQuery = query.toLowerCase();
      // Use a Set to automatically handle duplicates from local and Windows search
      final combinedProgramInfo = <ProgramInfo>{};
      String? searchError; // To store any error encountered during the search

      // == Step 1: Filter local programs (Synchronous) ==
      // Filter the already loaded `allInstalledPrograms` based on the query.
      try {
        if (state.allInstalledPrograms.isNotEmpty) {
          final localResults = state.allInstalledPrograms.where((program) =>
              program.name.toLowerCase().contains(lowerCaseQuery) ||
              program.path.toLowerCase().contains(lowerCaseQuery));
          combinedProgramInfo.addAll(localResults);
          // Log message moved after filtering for clarity
        } else {
          log("[SearchCubit] Local filter (ID: $searchId): No installed programs loaded yet or list is empty.");
        }
      } catch (e, s) {
        log("[SearchCubit] Error during local filtering (ID: $searchId): $e",
            stackTrace: s);
        // Optionally set an error or log more details
      }

      // == Step 2: Windows Search Index (Asynchronous via Isolate) ==
      // Query the Windows Search Index in a separate isolate for performance.
      final RootIsolateToken? rootIsolateToken = RootIsolateToken.instance;
      if (rootIsolateToken != null) {
        try {
          final payload =
              WindowsSearchPayload(token: rootIsolateToken, query: query);
          log("[SearchCubit] Starting Windows Search isolate (ID: $searchId)...");
          // Run the isolate function using compute.
          final List<ProgramInfo> windowsSearchResults =
              await compute(searchWindowsIndexIsolateEntry, payload);
          log("[SearchCubit] Windows Search isolate (ID: $searchId) returned ${windowsSearchResults.length} results.");
          combinedProgramInfo
              .addAll(windowsSearchResults); // Add results to the Set
        } catch (e, s) {
          final errorMsg = "Windows Search failed (ID: $searchId): $e";
          log(errorMsg, stackTrace: s, level: 1000);
          searchError =
              errorMsg; // Store the error to potentially display later
        }
      } else {
        final errorMsg =
            "Cannot perform Windows Search (ID: $searchId): RootIsolateToken is null.";
        log(errorMsg, level: 1000);
        searchError = errorMsg;
      }

      // == Step 3: Filter out banned keywords ==
      // Remove items whose name or path contains any of the defined banned keywords.
      final List<ProgramInfo> filteredProgramInfo =
          combinedProgramInfo.where((program) {
        final lowerName = program.name.toLowerCase();
        final lowerPath = program.path.toLowerCase();

        // Check if name or path contains ANY of the banned keywords
        bool isBanned = _bannedKeywords.any((bannedWord) =>
            lowerName.contains(bannedWord) || lowerPath.contains(bannedWord));

        // Keep the item only if it's NOT banned
        return !isBanned;
      }).toList(); // Convert the filtered Iterable back to a List

      // Log how many items were removed by the filter.
      final int filteredCount =
          combinedProgramInfo.length - filteredProgramInfo.length;
      if (filteredCount > 0) {
        log("[SearchCubit] Filtered out $filteredCount items based on banned keywords (ID: $searchId).");
      }

      // == Step 4: Convert Filtered ProgramInfo to SearchResult ==
      // Map the filtered ProgramInfo objects to SearchResult objects suitable for the UI.
      final List<SearchResult> combinedSearchResults = filteredProgramInfo
          .map((program) => _programInfoToSearchResult(program))
          .toList();

      // == Step 5: Apply Prioritized Sort ==
      // Sort the results using the dedicated SearchResultSorter class.
      final List<SearchResult> sortedResults = SearchResultSorter.sortResults(
          combinedSearchResults, // Pass the filtered and converted list
          query // Pass the original query for relevance scoring
          );

      // == Step 6: Emit Final State ==
      // Before emitting, double-check if this is still the latest search request.
      if (searchId != _currentSearchId || isClosed) {
        log("[SearchCubit] Skipping state update for stale search (ID: $searchId, Current: $_currentSearchId)");
        return;
      }

      final bool hadResultsBefore = state.results.isNotEmpty;
      final bool hasResultsNow = sortedResults.isNotEmpty;
      // Automatically select the first item if results exist, otherwise select none (-1).
      final int newSelectedIndex = hasResultsNow ? 0 : -1;

      log("[SearchCubit] Search complete (ID: $searchId). Combined, filtered, and sorted ${sortedResults.length} results. Selecting index: $newSelectedIndex. Error: $searchError");

      // Emit the final state with the sorted results, updated loading status, etc.
      emit(state.copyWith(
        results: sortedResults, // The final sorted list for the UI
        isLoading: false, // Search is complete
        showHelp:
            false, // Don't show help when results (or no results) are displayed
        selectedIndex: newSelectedIndex,
        searchError:
            searchError, // Include any error message from the search process
      ));

      // == Resize Window ==
      // Adjust window size based on whether results are now shown or hidden.
      if (hadResultsBefore != hasResultsNow) {
        log("[SearchCubit] Resizing window (ID: $searchId) because result visibility changed (had: $hadResultsBefore, has: $hasResultsNow)");
        resizeWindow(hasResultsNow);
      }
      // Also shrink if the search had results before but now has none (and isn't empty query)
      else if (query.isNotEmpty && !hasResultsNow && hadResultsBefore) {
        log("[SearchCubit] Resizing window (ID: $searchId) because query exists but no results found (was showing results before).");
        resizeWindow(false);
      }
    });
  }

  // --- Helper Methods ---

  /// Converts a [ProgramInfo] object into a [SearchResult] object.
  SearchResult _programInfoToSearchResult(ProgramInfo program) {
    // Use path and args combined as a somewhat unique ID for equality checks.
    final id = '${program.path}|${program.args}';
    return SearchResult(
      id: id,
      title: program.name,
      description: program.desc, // Use the path as the description string
      path: program.path,
      args: program.args,
      iconBytes:
          program.decodedIconBytes, // Include decoded icon bytes if available
      // Set the callback to execute when the item is selected (e.g., Enter key)
      onSelected: () => _openItem(program.path, program.args),
      // Get a suitable fallback icon based on path/name/extension
      icon: _getFallbackIcon(program),
    );
  }

  /// Determines a fallback [IconData] for a search result based on its path and name.
  /// Used when a specific icon cannot be loaded from the system.
  IconData _getFallbackIcon(ProgramInfo program) {
    final lowerPath = program.path.toLowerCase();
    final lowerName = program.name.toLowerCase();
    final String ext = p.extension(lowerPath).toLowerCase();
    if (program.kind == "setting") {
      return FluentIcons
          .settings_24_regular; // Settings icon for system settings
    }
    if (program.kind == "folder") {
      return FluentIcons.folder_24_regular; // Folder icon for directories
    }
    if (program.kind == "file") {
      return FluentIcons.document_24_regular; // Document icon for files
    }
    if (program.kind == "shortcut") {
      return FluentIcons.link_24_regular; // Link icon for shortcuts
    }
    if (program.kind == "url") {
      return FluentIcons.globe_24_regular; // URL icon for web links
    }
    if (program.kind == "link") {
      return FluentIcons.link_24_regular; // Link icon for links
    }

    // Prioritize specific known system applications/locations
    if (lowerPath.contains('explorer.exe')) {
      return FluentIcons.folder_24_regular;
    }
    if (lowerName == 'settings' || lowerPath.contains('systemsettings.exe')) {
      return FluentIcons.settings_24_regular;
    }
    if (lowerName == 'calculator' || lowerPath.contains('calc.exe')) {
      return FluentIcons.calculator_24_regular;
    }
    if (lowerName == 'command prompt' || lowerPath.contains('cmd.exe')) {
      return Icons.terminal_rounded;
    }
    if (lowerName == 'powershell' || lowerPath.contains('powershell.exe')) {
      return Icons.terminal_rounded;
    }

    // Determine icon based on file extension
    switch (ext) {
      // Executables / Installers / Scripts
      case '.exe':
      case '.msi':
      case '.bat':
      case '.cmd':
      case '.ps1':
        return FluentIcons.apps_24_regular; // Generic application icon
      case '.scr': // Screen saver
        return FluentIcons.desktop_24_regular; // Desktop related

      // Documents
      case '.txt':
      case '.log':
      case '.md':
      case '.rtf':
        return FluentIcons.document_text_24_regular; // Text document
      case '.pdf':
        return FluentIcons.document_pdf_24_regular; // PDF specific
      case '.doc':
      case '.docx':
      case '.odt': // Word / OpenDocument Text
        return FluentIcons.document_24_regular; // Generic document
      case '.xls':
      case '.xlsx':
      case '.csv':
      case '.ods': // Excel / Spreadsheets
        return FluentIcons.document_data_24_regular; // Data/table related
      case '.ppt':
      case '.pptx':
      case '.odp': // PowerPoint / Presentations
        return FluentIcons.document_multiple_24_regular; // Presentation/slides

      // Web related
      case '.html':
      case '.htm':
      case '.xml':
      case '.css':
      case '.js':
        return FluentIcons.code_24_regular; // Code/web related
      case '.url': // Internet Shortcut
        return FluentIcons.globe_24_regular; // Web/link related

      // Links / Shortcuts
      case '.lnk':
        return FluentIcons.link_24_regular; // Link icon

      // Images
      case '.png':
      case '.jpg':
      case '.jpeg':
      case '.gif':
      case '.bmp':
      case '.ico':
      case '.tif':
      case '.tiff':
      case '.webp':
      case '.svg':
        return FluentIcons.image_24_regular;

      // Archives
      case '.zip':
      case '.rar':
      case '.7z':
      case '.tar':
      case '.gz':
      case '.bz2':
        return FluentIcons.folder_zip_24_regular; // Archive specific

      // Audio
      case '.mp3':
      case '.wav':
      case '.ogg':
      case '.flac':
      case '.aac':
      case '.wma':
      case '.m4a':
        return FluentIcons.music_note_1_24_regular; // Music/audio

      // Video
      case '.mp4':
      case '.mkv':
      case '.avi':
      case '.mov':
      case '.wmv':
      case '.flv':
      case '.webm':
        return FluentIcons.video_24_regular;

      // Fonts
      case '.ttf':
      case '.otf':
      case '.woff':
      case '.woff2':
        return FluentIcons.text_font_24_regular;

      // Help files
      case '.chm':
        return FluentIcons.question_24_regular; // Help/question mark

      // Default cases
      default:
        // Basic heuristic check for folders (if extension is empty and ends with separator)
        if (ext.isEmpty &&
            (program.path.endsWith('\\') || program.path.endsWith('/'))) {
          return FluentIcons.folder_24_regular;
        }
        // General fallback for unknown file types
        return FluentIcons.document_24_regular;
    }
  }

  // --- Platform Channel Interaction ---
  /// Opens the specified item using the platform channel.
  /// Sends the path and arguments to the native side.
  Future<void> _openItem(String path, String args) async {
    log("[SearchCubit] Attempting to open item: Path='$path', Args='$args'");
    if (path.isEmpty) {
      log("[SearchCubit] OpenItem skipped: Path is empty.");
      // Optionally update state to inform the user about the issue
      // emit(state.copyWith(searchError: "Cannot open item: Path is missing."));
      return;
    }
    try {
      // Invoke the 'OpenItem' method on the native side, passing path and args.
      // Important: Pass arguments as a Map for clarity and compatibility.
      await _platformChannel.invokeMethod('OpenItem', [path, args]);
      log("[SearchCubit] 'OpenItem' method invoked successfully.");
      // Optional: Consider hiding the window or resetting search after successful execution
      // windowManager.hide();
      // resetSearch();
    } on PlatformException catch (e) {
      // Handle errors specifically from the platform channel communication.
      log("[SearchCubit] PlatformException opening item '$path': ${e.message} ${e.details}",
          level: 1000);
      // Optionally update state with a user-friendly error message:
      // emit(state.copyWith(searchError: "Error opening '${state.results.firstWhere((r)=>r.path==path).title}': ${e.message}"));
    } catch (e, s) {
      // Handle any other unexpected errors during the process.
      log("[SearchCubit] Unexpected error opening item '$path': $e",
          stackTrace: s, level: 1000);
      // emit(state.copyWith(searchError: "An unexpected error occurred while opening the item."));
    }
  }

  // --- UI Interaction Methods ---

  /// Resets the search state to its initial condition (clears query, results, selection).
  /// Shows the help section again.
  void resetSearch() {
    if (_debounce?.isActive ?? false) _debounce?.cancel();
    _currentSearchId++; // Invalidate any pending search operations
    final bool hadResults = state.results.isNotEmpty;
    log("[SearchCubit] Resetting search state.");
    // Emit the initial state, but keep already loaded programs
    emit(state.copyWith(
      query: '',
      results: [],
      selectedIndex: -1,
      isLoading: false,
      showHelp: true, // Show help again
      clearSearchError: true, // Clear any previous errors
    ));
    // Shrink the window if results were previously shown
    if (hadResults) {
      log("[SearchCubit] Resizing window on reset because results were previously shown.");
      resizeWindow(false);
    }
  }

  /// Selects the next item in the search results list.
  /// Stops at the last item (does not wrap around).
  void selectNext() {
    // Don't change selection if list is empty, loading, or results aren't shown
    if (state.results.isEmpty || state.isLoading || state.showHelp) return;

    int nextIndex = state.selectedIndex + 1;
    // Clamp the index to the bounds of the list (stay at the end)
    if (nextIndex >= state.results.length) {
      nextIndex = state.results.length - 1;
    }

    if (nextIndex != state.selectedIndex) {
      log("[SearchCubit] SelectNext: Moving to index $nextIndex");
      emit(state.copyWith(selectedIndex: nextIndex));
    } else {
      log("[SearchCubit] SelectNext: Already at bottom (index $nextIndex)");
    }
  }

  /// Selects the previous item in the search results list.
  /// Stops at the first item (does not wrap around).
  void selectPrevious() {
    // Don't change selection if list is empty, loading, or results aren't shown
    if (state.results.isEmpty || state.isLoading || state.showHelp) return;

    int prevIndex = state.selectedIndex - 1;
    // Clamp the index to the bounds of the list (stay at the beginning)
    if (prevIndex < 0) {
      prevIndex = 0;
    }

    if (prevIndex != state.selectedIndex) {
      log("[SearchCubit] SelectPrevious: Moving to index $prevIndex");
      emit(state.copyWith(selectedIndex: prevIndex));
    } else {
      log("[SearchCubit] SelectPrevious: Already at top (index $prevIndex)");
    }
  }

  /// Executes the action associated with the currently selected search result.
  void executeSelectedAction() {
    if (state.isLoading) {
      log("[SearchCubit] ExecuteAction: Ignored, currently loading.");
      return;
    }
    // Check if an item is actually selected and the index is valid
    if (state.selectedIndex >= 0 &&
        state.selectedIndex < state.results.length) {
      final selectedResult = state.results[state.selectedIndex];
      log("[SearchCubit] Executing action for index ${state.selectedIndex}: ${selectedResult.title}");
      selectedResult.onSelected?.call();
      windowManager.hide();
    } else {
      log("[SearchCubit] ExecuteAction: No valid item selected (index: ${state.selectedIndex}). Query: '${state.query}'");
      // Potential enhancement: Perform a default action if Enter is pressed
      // with no item selected but with text in the query box (e.g., web search).
      // if (state.query.isNotEmpty) { /* Initiate web search */ }
    }
  }

  void selectAndExecuteAction(int index) {
    if (state.isLoading) {
      log("[SearchCubit] ExecuteAction: Ignored, currently loading.");
      return;
    }
    // Check if an item is actually selected and the index is valid
    if (index >= 0 && index < state.results.length) {
      final selectedResult = state.results[index];
      emit(state.copyWith(selectedIndex: index)); // Update selected index
      log("[SearchCubit] Executing action for index $index: ${selectedResult.title}");
      selectedResult.onSelected?.call();
      if (_globalController.searchController.text.isNotEmpty) {
        _globalController.searchController.selection = TextSelection(
          baseOffset: 0,
          extentOffset: _globalController.searchController.text.length,
        );
      }
      windowManager.hide();
    } else {
      log("[SearchCubit] ExecuteAction: No valid item selected (index: $index). Query: '${state.query}'");
    }
  }

  // --- Cleanup ---
  @override
  Future<void> close() {
    log("[SearchCubit] Closing.");
    _debounce?.cancel(); // Cancel any active timer
    _currentSearchId++; // Ensure any final pending operations are invalidated
    return super.close();
  }
} // End of SearchCubit class