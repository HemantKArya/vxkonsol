part of 'theme_cubit.dart';

@immutable
sealed class ThemeState {}

final class ThemeBright extends ThemeState {}

final class ThemeDark extends ThemeState {}

final class ThemeSystem extends ThemeState {} // If you need system theme option