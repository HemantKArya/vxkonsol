#include "flutter_window.h"
#include <optional>
#include "native_utils/ShellExecution.h"
#include "native_utils/common_utils.h"
#include "native_utils/ProgramFinder.h"
#include "native_utils/winsearch.h"
#include <flutter/event_channel.h>
#include <flutter/event_sink.h>
#include <flutter/event_stream_handler_functions.h>
#include <flutter/method_channel.h>
#include <flutter/standard_method_codec.h>
#include <windows.h>
#include <memory>
#include "flutter/generated_plugin_registrant.h"


FlutterWindow::FlutterWindow(const flutter::DartProject& project)
    : project_(project) {}

FlutterWindow::~FlutterWindow() {}

bool FlutterWindow::OnCreate() {
  if (!Win32Window::OnCreate()) {
    return false;
  }

  RECT frame = GetClientArea();

  // The size here must match the window dimensions to avoid unnecessary surface
  // creation / destruction in the startup path.
  flutter_controller_ = std::make_unique<flutter::FlutterViewController>(
      frame.right - frame.left, frame.bottom - frame.top, project_);
  // Ensure that basic setup of the controller was successful.
  if (!flutter_controller_->engine() || !flutter_controller_->view()) {
    return false;
  }
  RegisterPlugins(flutter_controller_->engine());

  //Method channel for native windows apis
  flutter::MethodChannel<> channel(
    flutter_controller_->engine()->messenger(), "windows_native_channel",
    &flutter::StandardMethodCodec::GetInstance());
channel.SetMethodCallHandler(
    [](const flutter::MethodCall<>& call,
       std::unique_ptr<flutter::MethodResult<>> result) {
        // Handle method calls on this channel.
        if (call.method_name() == "searchWindowsIndex") {
          // Extract the string argument from the method call
          const flutter::EncodableValue* args = call.arguments();
          std::string query;
          if (args && std::holds_alternative<std::string>(*args)) {
            query = std::get<std::string>(*args);
          } else {
            return result->Error("INVALID_ARGUMENT", "Invalid argument");
          }
          std::vector<utils::Program> items = SearchWindowsIndex(query);
          flutter::EncodableList flutter_list;
          for (const auto& item : items) {
            flutter::EncodableMap flutter_item;
            flutter_item[flutter::EncodableValue("name")] = flutter::EncodableValue(item.name);
            flutter_item[flutter::EncodableValue("path")] = flutter::EncodableValue(item.executablePath);
            flutter_item[flutter::EncodableValue("args")] = flutter::EncodableValue(item.arguments);
            flutter_item[flutter::EncodableValue("kind")] = flutter::EncodableValue(item.kind);
            flutter_item[flutter::EncodableValue("desc")] = flutter::EncodableValue(item.description);
            if (item.iconDataBase64.empty()) {
              flutter_item[flutter::EncodableValue("icon")] = flutter::EncodableValue("");
            } else {
              flutter_item[flutter::EncodableValue("icon")] = flutter::EncodableValue(item.iconDataBase64);
            }
            flutter_list.push_back(flutter::EncodableValue(flutter_item));
          }
          result->Success(flutter::EncodableValue(flutter_list));
        }
        else if(call.method_name() == "getAllPrograms") {
          std::vector<utils::Program> items = ProgramFinder::GetAllPrograms();
          flutter::EncodableList flutter_list;
          for (const auto& item : items) {
            flutter::EncodableMap flutter_item;
            flutter_item[flutter::EncodableValue("name")] = flutter::EncodableValue(item.name);
            flutter_item[flutter::EncodableValue("path")] = flutter::EncodableValue(item.executablePath);
            flutter_item[flutter::EncodableValue("args")] = flutter::EncodableValue(item.arguments);
            flutter_item[flutter::EncodableValue("kind")] = flutter::EncodableValue(item.kind);
            flutter_item[flutter::EncodableValue("desc")] = flutter::EncodableValue(item.description);
            if (item.iconDataBase64.empty()) {
              flutter_item[flutter::EncodableValue("icon")] = flutter::EncodableValue("");
            } else {
              flutter_item[flutter::EncodableValue("icon")] = flutter::EncodableValue(item.iconDataBase64);
            }
            flutter_list.push_back(flutter::EncodableValue(flutter_item));
          }
          result->Success(flutter::EncodableValue(flutter_list));
        }
        else if(call.method_name() == "OpenItem"){
          const flutter::EncodableValue* args = call.arguments();
          // Expected two arguments: path and arguments
          if (args && std::holds_alternative<flutter::EncodableList>(*args)) {
            const auto& arg_list = std::get<flutter::EncodableList>(*args);
            if (arg_list.size() == 2 &&
                std::holds_alternative<std::string>(arg_list[0]) &&
                std::holds_alternative<std::string>(arg_list[1])) {
              std::string path = std::get<std::string>(arg_list[0]);
              std::string arguments = std::get<std::string>(arg_list[1]);
              // Call the OpenItem function with the extracted arguments
              ShellExecution::OpenItem(path, arguments);
              result->Success();
            } else {
              result->Error("INVALID_ARGUMENT", "Invalid argument");
            }
          } else {
            result->Error("INVALID_ARGUMENT", "Invalid argument");
          }
        }
        else {
          result->NotImplemented();
        }
    });

  SetChildContent(flutter_controller_->view()->GetNativeWindow());

  flutter_controller_->engine()->SetNextFrameCallback([&]() {
    this->Show();
  });

  // Flutter can complete the first frame before the "show window" callback is
  // registered. The following call ensures a frame is pending to ensure the
  // window is shown. It is a no-op if the first frame hasn't completed yet.
  flutter_controller_->ForceRedraw();

  return true;
}

void FlutterWindow::OnDestroy() {
  if (flutter_controller_) {
    flutter_controller_ = nullptr;
  }

  Win32Window::OnDestroy();
}

LRESULT
FlutterWindow::MessageHandler(HWND hwnd, UINT const message,
                              WPARAM const wparam,
                              LPARAM const lparam) noexcept {
  // Give Flutter, including plugins, an opportunity to handle window messages.
  if (flutter_controller_) {
    std::optional<LRESULT> result =
        flutter_controller_->HandleTopLevelWindowProc(hwnd, message, wparam,
                                                      lparam);
    if (result) {
      return *result;
    }
  }

  switch (message) {
    case WM_FONTCHANGE:
      flutter_controller_->engine()->ReloadSystemFonts();
      break;
  }

  return Win32Window::MessageHandler(hwnd, message, wparam, lparam);
}
