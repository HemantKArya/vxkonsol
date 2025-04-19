#include "UwpFinder.h"
#include "common_utils.h" // Include for WideToUtf8, PathToUtf8, Base64Encode etc.

// Define NOMINMAX before including Windows.h to prevent min/max macro definitions
#define NOMINMAX
#include <Windows.h>
#include <thread>
#include <future>
#include <ShlObj.h> // For SHLoadIndirectString
#include <propkey.h> // For PKEYs
#include <propvarutil.h> // For PropVariantToString
#include <wrl/client.h> // For ComPtr
#include <combaseapi.h> // For CoInitializeEx, CoUninitialize
#include <limits> // Include for std::numeric_limits
#include <vector> // Include for std::vector

// WinRT headers
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.Management.Deployment.h>
#include <winrt/Windows.Storage.h> // Required for InstalledLocation
#include <winrt/Windows.ApplicationModel.Core.h> // Add this header for AppListEntry definitions
#include <winrt/Windows.Storage.Streams.h> // Required for IBuffer and DataReader

// Link necessary libraries for WinRT
#pragma comment(lib, "windowsapp")


std::vector<utils::Program> GetInstalledUWPPrograms()
{
    std::vector<utils::Program> programs;
    // winrt::init_apartment(winrt::apartment_type::single_threaded); // Initialize WinRT/COM

    try
    {
        winrt::Windows::Management::Deployment::PackageManager packageManager;
        winrt::Windows::Foundation::Collections::IIterable<winrt::Windows::ApplicationModel::Package> packages = packageManager.FindPackagesForUser(L"");

        for (const auto& package : packages)
        {
            // Skip framework packages as they usually don't have launchable entries/icons
            if (package.IsFramework()) {
                continue;
            }

            try
            {
                // Asynchronously get the list of app entries for the package
                auto appListEntries = package.GetAppListEntriesAsync().get();

                if (appListEntries && appListEntries.Size() > 0)
                {
                    // Often, the first entry is the main application
                    auto entry = appListEntries.GetAt(0); // Using the first entry as representative
                    auto displayInfo = entry.DisplayInfo();
                    std::string name = utils::WideToUtf8(displayInfo.DisplayName().c_str());
                    std::string aumid = utils::WideToUtf8(entry.AppUserModelId().c_str());

                    if (name.empty() || aumid.empty()) {
                        continue; // Skip if essential info is missing
                    }

                    utils::Program prog;
                    prog.name = std::move(name);
                    // UWP apps are launched via AUMID, not a direct path
                    prog.executablePath = "shell:AppsFolder\\" + aumid;
                    prog.source = "UWP";
                    prog.kind = "program"; // From PKEY_Kind
                    prog.description = utils::WideToUtf8(displayInfo.Description().c_str());
                    prog.iconPath = ""; // Reset icon path, as we'll use embedded data
                    prog.iconDataBase64 = ""; // Ensure it's empty initially

                    // --- New Icon Handling using GetLogo() and Base64 encoding ---
                    try
                    {
                        // Request a specific common logo size instead of default {0, 0}
                        winrt::Windows::Foundation::Size desiredSize{ 48, 48 }; // Changed from {0, 0}
                        auto logoStreamRef = entry.DisplayInfo().GetLogo(desiredSize);
                        // fprintf(stderr, "[%s] LogoStreamRef valid: %s\n", prog.name.c_str(), logoStreamRef ? "true" : "false");

                        if (logoStreamRef)
                        {
                            auto logoStream = logoStreamRef.OpenReadAsync().get(); // Open stream async
                            // fprintf(stderr, "[%s] LogoStream valid: %s\n", prog.name.c_str(), logoStream ? "true" : "false");

                            if (logoStream && logoStream.Size() > 0)
                            {
                                uint64_t streamSize64 = logoStream.Size();
                                // fprintf(stderr, "[%s] LogoStream size: %llu\n", prog.name.c_str(), streamSize64);

                                // Read the stream into a buffer
                                winrt::Windows::Storage::Streams::DataReader reader(logoStream);
                                // Check if stream size exceeds uint32_t max before casting
                                if (streamSize64 > std::numeric_limits<uint32_t>::max()) {
                                     fprintf(stderr, "[%s] Warning: Logo stream size (%llu) exceeds uint32_t max. Skipping icon.\n", prog.name.c_str(), streamSize64);
                                     // Handle error appropriately, maybe skip this icon
                                } else {
                                    uint32_t streamSize = static_cast<uint32_t>(streamSize64);
                                    reader.LoadAsync(streamSize).get(); // Load the data async
                                    // fprintf(stderr, "[%s] LoadAsync completed.\n", prog.name.c_str());

                                    // Prepare vector to hold the data
                                    std::vector<uint8_t> iconData(streamSize);
                                    // Read bytes directly into the vector
                                    reader.ReadBytes(iconData);
                                    // fprintf(stderr, "[%s] ReadBytes completed. Vector size: %zu\n", prog.name.c_str(), iconData.size());


                                    // Encode the raw image data (likely PNG) to Base64
                                    if (!iconData.empty()) {
                                        prog.iconDataBase64 = utils::Base64Encode(iconData);
                                        // fprintf(stderr, "[%s] Base64 encoding attempted. Result empty: %s\n", prog.name.c_str(), prog.iconDataBase64.empty() ? "true" : "false");
                                    } else {
                                        // fprintf(stderr, "[%s] Icon data vector was empty before Base64 encoding.\n", prog.name.c_str());
                                    }
                                    // Optionally store content type if needed later
                                    // std::string contentType = winrt::to_string(logoStream.ContentType());
                                }
                            }
                            // else {
                            //     fprintf(stderr, "[%s] LogoStream is null or size is 0.\n", prog.name.c_str());
                            // }
                        }
                        // else {
                        //     fprintf(stderr, "[%s] LogoStreamRef was null.\n", prog.name.c_str());
                        // }
                    }
                    catch (const winrt::hresult_error& e)
                    {
                        // Log error getting icon stream for this specific app entry
                        fprintf(stderr, "WinRT error getting logo for %s: %ls\n", prog.name.c_str(), e.message().c_str());
                        prog.iconDataBase64 = ""; // Ensure it's empty on failure
                    }
                    catch (const std::exception& e) {
                         // Log standard exception during icon processing
                        fprintf(stderr, "Standard exception getting logo for %s: %s\n", prog.name.c_str(), e.what());
                        prog.iconDataBase64 = ""; // Ensure it's empty on failure
                    }
                    // --- End of New Icon Handling ---


                    programs.push_back(std::move(prog));
                }
            }
            catch (const winrt::hresult_error& e)
            {
                // Log error processing a specific package/app entry
                fprintf(stderr, "WinRT error processing package %ls: %ls\n", package.Id().FullName().c_str(), e.message().c_str());
                continue; // Continue with the next package
            }
            catch (const std::exception& e) {
                // Log standard exception
                fprintf(stderr, "Standard exception processing package %ls: %s\n", package.Id().FullName().c_str(), e.what());
                continue;
            }
        }
    }
    catch (const winrt::hresult_error& e)
    {
        // Log error initializing or finding packages
        fprintf(stderr, "WinRT error during init/find: %ls\n", e.message().c_str());
    }
     catch (const std::exception& e) {
        // Log standard exception during initialization or package finding
        fprintf(stderr, "Standard exception during init/find: %s\n", e.what());
    }


    // winrt::uninit_apartment(); // Uninitialize WinRT/COM
    return programs;
}



std::vector<utils::Program> GetUwpProgramsOnStaThread() {
    // Use std::async to launch the task potentially on a new thread.
    // The lambda running inside async will execute on that thread.
    auto future = std::async(std::launch::async, []() {
        // Initialize COM/WinRT *specifically for this new thread* as STA.
        // This is crucial.
        winrt::init_apartment(winrt::apartment_type::single_threaded);

        // Now call the original function. It will run entirely within
        // this STA thread context. It might call init_apartment again
        // internally, which is safe (it will likely return S_FALSE).
        std::vector<utils::Program> programs = GetInstalledUWPPrograms();

        // Uninitialize COM/WinRT for this thread before it exits.
        winrt::uninit_apartment();

        return programs; // Return the result from the thread
    });

    // Wait for the asynchronous operation to complete and return its result.
    // This blocks the *calling* thread (the method channel thread) until
    // the STA thread finishes its work.
    try {
        return future.get();
    } catch (...) {
        // Rethrow the exception caught by future.get() if the async task failed
        // Or handle it more gracefully here if needed
         fprintf(stderr, "Exception caught while getting result from STA thread future.\n");
         throw; // Rethrow to be caught by the method channel handler's catch block
    }
}