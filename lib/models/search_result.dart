// lib/models/search_result.dart
import 'dart:typed_data'; // For Uint8List
import 'package:flutter/material.dart'; // For IconData

// Keep SearchResult but adapt it to hold ProgramInfo data + action
@immutable
class SearchResult {
  final String id; // Can use path+args or generate UUID
  final String title;
  final String? description; // Often the path
  final IconData? icon; // Fallback icon
  final Uint8List? iconBytes; // Decoded icon from base64
  final String path; // Execution path
  final String args; // Execution arguments
  final VoidCallback? onSelected; // Action to execute

  const SearchResult({
    required this.id,
    required this.title,
    this.description,
    this.icon, // Keep fallback icon
    this.iconBytes, // Add bytes
    required this.path, // Add path
    required this.args, // Add args
    this.onSelected,
  });

  // Override equals and hashCode based on unique identifier (path + args)
  // This is crucial for deduplication using Sets.
  @override
  bool operator ==(Object other) =>
      identical(this, other) ||
      other is SearchResult &&
          runtimeType == other.runtimeType &&
          path == other.path &&
          args == other.args;

  @override
  int get hashCode => path.hashCode ^ args.hashCode;

  @override
  String toString() {
    return 'SearchResult{id: $id, title: $title, path: $path, args: $args, hasIconBytes: ${iconBytes != null}}';
  }
}
