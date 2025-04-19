// lib/widgets/search_box.dart
import 'dart:developer';
import 'package:fluentui_system_icons/fluentui_system_icons.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:flutter_bloc/flutter_bloc.dart';
import 'package:get/get.dart'; // Import GetX
import 'package:vxkonsol/core/global_controller.dart'; // Import the controller
import 'package:vxkonsol/cubits/search/search_cubit.dart';

class SearchBox extends StatefulWidget {
  const SearchBox({super.key});

  @override
  State<SearchBox> createState() => _SearchBoxState();
}

class _SearchBoxState extends State<SearchBox> {
  // Access the global focus node via GetX
  final FocusNode _searchFocusNode =
      Get.find<GlobalController>().searchFocusNode;
  // Access the global text controller via GetX
  final TextEditingController _controller =
      Get.find<GlobalController>().searchController;

  @override
  void initState() {
    super.initState();
    // Request initial focus when the widget is ready
    WidgetsBinding.instance.addPostFrameCallback((_) {
      if (mounted) {
        // Check if it doesn't already have focus (e.g., from hotkey)
        if (!_searchFocusNode.hasFocus) {
          log("Requesting initial focus for TextField in initState postFrameCallback");
          _searchFocusNode.requestFocus();
        }
      }
    });
  }

  @override
  void dispose() {
    // Do NOT dispose the global searchFocusNode here (it's managed by GlobalController)
    super.dispose();
  }

  KeyEventResult _handleKeyEvent(FocusNode node, KeyEvent event) {
    return KeyEventResult.ignored;
  }

  @override
  Widget build(BuildContext context) {
    final theme = Theme.of(context);

    return Padding(
      padding: const EdgeInsets.symmetric(horizontal: 18, vertical: 5),
      child: Focus(
        onKeyEvent: _handleKeyEvent,
        child: TextField(
          controller: _controller,
          focusNode: _searchFocusNode,
          autofocus: true, // TextField grabs focus initially
          onChanged: (value) {
            context.read<SearchCubit>().search(value);
          },
          cursorWidth: 2,
          cursorRadius: const Radius.circular(2),
          cursorColor: theme.textSelectionTheme.cursorColor,
          style: theme.textTheme.bodyLarge?.copyWith(
            fontSize: 16,
            fontWeight: FontWeight.w500,
          ),
          decoration: InputDecoration(
            hintText: 'Search apps, files, web...',
            hintStyle: TextStyle(
              fontSize: 16,
              fontWeight: FontWeight.w500,
              color: theme.hintColor.withOpacity(0.6),
            ),
            prefixIcon: Icon(
              FluentIcons.search_24_regular,
              color: theme.iconTheme.color?.withOpacity(0.8),
              size: 20,
            ),
            filled: true,
            fillColor: theme.inputDecorationTheme.fillColor,
            contentPadding:
                const EdgeInsets.symmetric(vertical: 12, horizontal: 15),
            border: OutlineInputBorder(
              borderRadius: BorderRadius.circular(15),
              borderSide: BorderSide.none,
            ),
            focusedBorder: OutlineInputBorder(
              borderRadius: BorderRadius.circular(15),
              borderSide: BorderSide(
                color: theme.colorScheme.primary.withOpacity(0.5),
                width: 1.5,
              ),
            ),
          ),
        ),
      ),
    );
  }
}
