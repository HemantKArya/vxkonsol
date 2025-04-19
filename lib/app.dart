// app.dart
import 'package:flutter/material.dart';
import 'package:flutter_bloc/flutter_bloc.dart';
import 'package:vxkonsol/cubits/theme/theme_cubit.dart';
import 'package:vxkonsol/screens/main_screen.dart';
import 'package:vxkonsol/theme/app_theme.dart';

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    // Provide Cubits at the top level
    return MultiBlocProvider(
      providers: [
        BlocProvider<ThemeCubit>(
          create: (context) => ThemeCubit(),
        ),
      ],
      child: BlocBuilder<ThemeCubit, ThemeState>(
        builder: (context, themeState) {
          final ThemeMode themeMode;
          if (themeState is ThemeBright) {
            themeMode = ThemeMode.light;
          } else if (themeState is ThemeDark) {
            themeMode = ThemeMode.dark;
          } else {
            themeMode = ThemeMode.system;
          }

          return MaterialApp(
            title: 'VXKonsol', // Changed app title
            debugShowCheckedModeBanner: false,
            theme: lightTheme,
            darkTheme: darkTheme,
            themeMode: themeMode,
            home: const MainScreen(), // Use const if possible
          );
        },
      ),
    );
  }
}
