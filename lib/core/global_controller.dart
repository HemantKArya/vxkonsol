import 'package:flutter/material.dart';
import 'package:get/get.dart';

class GlobalController extends GetxController {
  // Make FocusNode reactive if needed, otherwise keep it simple
  final FocusNode searchFocusNode = FocusNode();
  // Add the TextEditingController
  final TextEditingController searchController = TextEditingController();

  @override
  void onClose() {
    searchFocusNode
        .dispose(); // Dispose the FocusNode when the controller is closed
    searchController
        .dispose(); // Dispose the TextEditingController when the controller is closed
    super.onClose();
  }
}
