import 'package:bloc/bloc.dart';
import 'package:flutter/material.dart';

part 'theme_state.dart';

class ThemeCubit extends Cubit<ThemeState> {
  FocusNode focusNode;
  ThemeCubit({
    required this.focusNode,
  }) : super(ThemeSystem());

  void toggleTheme() {
    if (state is ThemeSystem) {
      emit(ThemeBright());
    } else if (state is ThemeBright) {
      emit(ThemeDark());
    } else {
      emit(ThemeSystem());
    }
  }

  void focus() {
    focusNode.requestFocus();
  }
}
