// windows_search.dart
import 'dart:async';
import 'package:flutter/foundation.dart'; // For compute, kDebugMode
import 'package:flutter/services.dart'; // For MethodChannel, PlatformException, RootIsolateToken, BackgroundIsolateBinaryMessenger

// Import the ProgramInfo class
import 'program_info.dart';

// Define the platform channel name as a constant (must match)
const String _platformChannelName = 'windows_native_channel';

// Helper class to pass multiple arguments to the isolate via compute
class WindowsSearchPayload {
  final RootIsolateToken token;
  final String query;

  WindowsSearchPayload({required this.token, required this.query});
}

//--------------------------------------------------------------------------
// Background Isolate Function for Windows Search Index
//--------------------------------------------------------------------------
@pragma('vm:entry-point') // Recommended for AOT compilation
Future<List<ProgramInfo>> searchWindowsIndexIsolateEntry(
    WindowsSearchPayload payload) async {
  // Initialize the background isolate's platform channel mechanism
  BackgroundIsolateBinaryMessenger.ensureInitialized(payload.token);

  // Create a MethodChannel instance specifically for this isolate.
  const platform = MethodChannel(_platformChannelName);
  final String query = payload.query; // Extract the query

  try {
    if (kDebugMode) {
      print(
          "[WindowsSearch Isolate] Calling searchWindowsIndex with query: '$query'");
    }
    // Make the platform call
    final List<dynamic>? result =
        await platform.invokeMethod<List<dynamic>>('searchWindowsIndex', query);

    if (kDebugMode) {
      print(
          "[WindowsSearch Isolate] Received result: ${result?.length ?? 'null'} items for query '$query'");
    }

    // Process the result
    if (result != null) {
      final List<ProgramInfo> programs = [];
      for (var item in result) {
        if (item is Map) {
          try {
            programs.add(ProgramInfo.fromMap(item));
          } catch (e) {
            if (kDebugMode) {
              print(
                  "[WindowsSearch Isolate] Error parsing search item: $item, Error: $e");
            }
            // Decide if you want to skip or add a default entry
          }
        } else {
          if (kDebugMode) {
            print(
                "[WindowsSearch Isolate] Skipping non-map item in search result list: $item");
          }
        }
      }
      // Deduplication can happen here or in the main isolate after combining results.
      // Let's keep deduplication in the main isolate for combined results.
      if (kDebugMode) {
        print(
            "[WindowsSearch Isolate] Parsed ${programs.length} search results successfully for query '$query'.");
      }
      return programs;
    } else {
      if (kDebugMode) {
        print(
            "[WindowsSearch Isolate] searchWindowsIndex returned null for query '$query'.");
      }
      return []; // Return empty list on null result
    }
  } on PlatformException catch (e) {
    if (kDebugMode) {
      print(
          "[WindowsSearch Isolate] PlatformException for query '$query': ${e.message} ${e.details}");
    }
    // Propagate the error
    return Future.error("Windows Search failed (Platform Error): ${e.message}");
  } catch (e, s) {
    if (kDebugMode) {
      print(
          "[WindowsSearch Isolate] Unexpected error for query '$query': $e\n$s");
    }
    // Propagate the error
    return Future.error(
        "An unexpected error occurred during Windows Search: $e");
  }
}
