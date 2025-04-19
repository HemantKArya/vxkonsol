// program_info.dart
import 'package:flutter/foundation.dart'; // For @immutable
import 'dart:convert'; // For base64Decode

@immutable // Marking as immutable since its fields are final
class ProgramInfo {
  final String name;
  final String path;
  final String args; // Default empty string for args
  final String kind; // Default empty string for kind
  final String desc; // Default empty string for desc
  final String? iconBase64; // Icon can be null or empty

  const ProgramInfo({
    required this.name,
    required this.path,
    this.kind = "",
    this.desc = "",
    this.args = "",
    this.iconBase64,
  });

  // Factory constructor to parse from the Map received from platform channel
  factory ProgramInfo.fromMap(Map<dynamic, dynamic> map) {
    // Handle potential null values or incorrect types defensively
    final name = map['name'] is String ? map['name'] as String : 'Unknown Name';
    final path = map['path'] is String ? map['path'] as String : '';
    final args = map['args'] is String ? map['args'] as String : '';
    final kind = map['kind'] is String ? map['kind'] as String : '';
    final desc = map['desc'] is String ? map['desc'] as String : '';
    final icon = map['icon'] is String ? map['icon'] as String : null;

    return ProgramInfo(
      name: name,
      path: path,
      args: args,
      kind: kind,
      desc: desc,
      // Store null if the icon string is empty, simplifying checks later
      iconBase64: (icon != null && icon.isNotEmpty) ? icon : null,
    );
  }

  // Override equals and hashCode to allow using Sets for deduplication based on path and args
  @override
  bool operator ==(Object other) =>
      identical(this, other) ||
      other is ProgramInfo &&
          runtimeType == other.runtimeType &&
          args == other.args && // Uniqueness based on args
          path == other.path; // Uniqueness based on path

  @override
  int get hashCode => path.hashCode ^ args.hashCode; // Combine hashCodes

  @override
  String toString() {
    // Useful for debugging
    return 'ProgramInfo{name: $name, path: $path, hasIcon: ${iconBase64 != null}}';
  }

  // Helper to decode icon bytes, returns null on error or if no icon
  Uint8List? get decodedIconBytes {
    if (iconBase64 != null && iconBase64!.isNotEmpty) {
      try {
        return base64Decode(iconBase64!);
      } catch (e) {
        if (kDebugMode) {
          print("Error decoding base64 icon for $name: $e");
        }
        return null; // Return null if decoding fails
      }
    }
    return null; // Return null if no icon string
  }
}
