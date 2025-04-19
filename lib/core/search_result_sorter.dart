// lib/core/search_result_sorter.dart
import 'package:path/path.dart' as p;
import 'package:vxkonsol/models/search_result.dart'; // Adjust path if needed

class SearchResultSorter {
  // Private constructor to prevent instantiation
  SearchResultSorter._();

  /// Sorts a list of SearchResult based on type priority, relevance, and title.
  static List<SearchResult> sortResults(
      List<SearchResult> results, String query) {
    if (results.isEmpty) return results; // No need to sort empty list

    final lowerCaseQuery = query.toLowerCase();

    results.sort((a, b) {
      // *** CHANGE: Priority Level 1: Type Priority (Lower priority number comes first) ***
      final priorityA = _getTypePriority(a);
      final priorityB = _getTypePriority(b);
      if (priorityA != priorityB) {
        return priorityA.compareTo(priorityB); // Ascending order for priority
      }

      // *** CHANGE: Priority Level 2: Relevance Score (Higher score comes first) ***
      final scoreA = _getRelevanceScore(a, lowerCaseQuery);
      final scoreB = _getRelevanceScore(b, lowerCaseQuery);
      if (scoreA != scoreB) {
        return scoreB.compareTo(scoreA); // Descending order for score
      }

      // Level 3: Alphabetical by Title (Tie-breaker)
      return a.title.toLowerCase().compareTo(b.title.toLowerCase());
    });

    return results; // Return the sorted list
  }

  // --- Relevance Score Helper ---
  // (This method remains unchanged)
  static int _getRelevanceScore(SearchResult result, String lowerCaseQuery) {
    if (lowerCaseQuery.isEmpty) return 0;

    final lowerTitle = result.title.toLowerCase();
    final lowerDescription = result.description?.toLowerCase() ?? '';

    if (lowerTitle.startsWith(lowerCaseQuery)) {
      return 3;
    } else if (lowerTitle.contains(lowerCaseQuery)) {
      return 2;
    } else if (lowerDescription.contains(lowerCaseQuery)) {
      return 1;
    } else {
      return 0;
    }
  }

  // --- Type Priority Helper ---
  // (This method remains unchanged)
  static int _getTypePriority(SearchResult result) {
    final lowerPath = result.path.toLowerCase();
    final lowerName = result.title.toLowerCase();
    final String ext = p.extension(lowerPath).toLowerCase();

    // Priority 0: Specific System Items
    if (lowerName == 'settings' || lowerPath.contains('systemsettings.exe')) {
      return 0;
    }
    if (lowerPath.contains('explorer.exe')) return 0;

    // Priority 1: Executables
    if (ext == '.exe' || ext == '.msi') return 1;

    // Priority 2: Shortcuts
    if (ext == '.lnk') return 2;

    // Priority 3: Scripts/Commands
    if (ext == '.bat' || ext == '.cmd' || ext == '.ps1') return 3;
    if (lowerName == 'command prompt' || lowerName == 'powershell') return 3;

    // Priority 4: URLs/Web Links
    if (ext == '.url') return 4;

    // Priority 5: Folders (heuristic)
    if (ext.isEmpty &&
        (result.path.endsWith('\\') || result.path.endsWith('/'))) {
      return 5;
    }

    // Priority 6: Common Documents
    const commonDocExts = {
      '.doc',
      '.docx',
      '.odt',
      '.pdf',
      '.xls',
      '.xlsx',
      '.ods',
      '.csv',
      '.ppt',
      '.pptx',
      '.odp',
      '.rtf'
    };
    if (commonDocExts.contains(ext)) return 6;

    // Priority 7: Images & Media
    const mediaExts = {
      '.png',
      '.jpg',
      '.jpeg',
      '.gif',
      '.bmp',
      '.ico',
      '.tif',
      '.tiff',
      '.webp',
      '.svg',
      '.mp3',
      '.wav',
      '.ogg',
      '.flac',
      '.aac',
      '.wma',
      '.m4a',
      '.mp4',
      '.mkv',
      '.avi',
      '.mov',
      '.wmv',
      '.flv',
      '.webm'
    };
    if (mediaExts.contains(ext)) return 7;

    // Priority 8: Text, Misc, Less Common
    const miscExts = {
      '.txt',
      '.log',
      '.md',
      '.chm',
      '.scr',
      '.html',
      '.htm',
      '.xml',
      '.css',
      '.js',
      '.zip',
      '.rar',
      '.7z',
      '.tar',
      '.gz',
      '.bz2',
      '.ttf',
      '.otf',
      '.woff',
      '.woff2'
    };
    if (miscExts.contains(ext)) return 8;

    // Default Priority
    return 99;
  }
}
