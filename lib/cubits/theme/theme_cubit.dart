import 'package:bloc/bloc.dart';
import 'package:flutter/material.dart'; // Keep for @immutable

part 'theme_state.dart';

class ThemeCubit extends Cubit<ThemeState> {
  ThemeCubit() : super(ThemeDark()); // Default theme

  void toggleTheme() {
    if (state is ThemeBright) {
      emit(ThemeDark());
    } else {
      emit(ThemeBright());
    }
    // If you add ThemeSystem, add logic here
  }

  void setTheme(ThemeMode mode) {
    switch (mode) {
      case ThemeMode.light:
        emit(ThemeBright());
        break;
      case ThemeMode.dark:
        emit(ThemeDark());
        break;
      case ThemeMode.system:
        emit(ThemeSystem());
        break; // Handle if needed
    }
  }
}
