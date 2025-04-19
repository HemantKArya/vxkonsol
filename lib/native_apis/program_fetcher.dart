// program_fetcher.dart
import 'dart:async';
import 'package:flutter/foundation.dart'; // For compute, kDebugMode
import 'package:flutter/services.dart'; // For MethodChannel, PlatformException, RootIsolateToken, BackgroundIsolateBinaryMessenger

// Import the ProgramInfo class
import 'program_info.dart';

// Define the platform channel name as a constant
const String _platformChannelName = 'windows_native_channel';

//--------------------------------------------------------------------------
// Background Isolate Function for Fetching All Programs
//--------------------------------------------------------------------------
// Renamed for clarity
@pragma('vm:entry-point') // Recommended for AOT compilation
Future<List<ProgramInfo>> fetchAllProgramsIsolateEntry(
    RootIsolateToken rootIsolateToken) async {
  // Initialize the background isolate's platform channel mechanism
  BackgroundIsolateBinaryMessenger.ensureInitialized(rootIsolateToken);

  // Create a MethodChannel instance specifically for this isolate.
  const platform = MethodChannel(_platformChannelName);

  try {
    if (kDebugMode) {
      print("[ProgramFetcher Isolate] Calling getAllPrograms...");
    }
    // Make the platform call
    final List<dynamic>? result =
        await platform.invokeMethod<List<dynamic>>('getAllPrograms');

    if (kDebugMode) {
      print(
          "[ProgramFetcher Isolate] Received result: ${result?.length ?? 'null'} items");
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
                  "[ProgramFetcher Isolate] Error parsing program item: $item, Error: $e");
            }
            // Decide if you want to skip or add a default entry
          }
        } else {
          if (kDebugMode) {
            print(
                "[ProgramFetcher Isolate] Skipping non-map item in result list: $item");
          }
        }
      }
      // Deduplicate *within* the isolate before returning (optional but efficient)
      final uniquePrograms = Set<ProgramInfo>.from(programs).toList();
      if (kDebugMode) {
        print(
            "[ProgramFetcher Isolate] Parsed and deduplicated ${uniquePrograms.length} programs successfully.");
      }
      return uniquePrograms;
    } else {
      if (kDebugMode) {
        print("[ProgramFetcher Isolate] getAllPrograms returned null.");
      }
      return []; // Return empty list on null result
    }
  } on PlatformException catch (e) {
    if (kDebugMode) {
      print(
          "[ProgramFetcher Isolate] PlatformException: ${e.message} ${e.details}");
    }
    // Propagate the error so 'compute' catches it
    return Future.error(
        "Failed to fetch programs (Platform Error): ${e.message}");
  } catch (e, s) {
    if (kDebugMode) {
      print("[ProgramFetcher Isolate] Unexpected error: $e\n$s");
    }
    // Propagate the error
    return Future.error(
        "An unexpected error occurred while fetching programs: $e");
  }
}
