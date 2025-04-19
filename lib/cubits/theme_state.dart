part of 'theme_cubit.dart';

@immutable
sealed class ThemeState {}

final class ThemeSystem extends ThemeState {}

final class ThemeBright extends ThemeState {}

final class ThemeDark extends ThemeState {}
