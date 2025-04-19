#include "SettingsPages.h"


std::vector<utils::Program> getAllSettingsPages()
{
    std::vector<utils::Program> settingsPages;
    utils::Program p; 

    // --- Settings home page ---
    p = utils::Program{};
    p.name = "Settings";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:";
    p.description = "Windows Settings";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    // --- System ---
    p = utils::Program{};
    p.name = "Display";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:display";
    p.description = "Settings > System > Display";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Night light settings";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:nightlight";
    p.description = "Settings > System > Display > Night light settings";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Advanced scaling settings";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:display-advanced"; // URI for advanced display settings
    p.description = "Settings > System > Display > Advanced scaling settings";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    // Note: ms-settings-connectabledevices:devicediscovery is not an ms-settings: URI, skipping.

    p = utils::Program{};
    p.name = "Graphics settings";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:display-advancedgraphics";
    p.description = "Settings > System > Display > Graphics settings";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Display orientation";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:screenrotation";
    p.description = "Settings > System > Display > Display orientation";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Sound";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:sound";
    p.description = "Settings > System > Sound";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Manage sound devices";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:sound-devices";
    p.description = "Settings > System > Sound > Manage sound devices";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "App volume and device preferences";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:apps-volume";
    p.description = "Settings > System > Sound > App volume and device preferences";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Notifications & actions";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:notifications";
    p.description = "Settings > System > Notifications & actions";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Focus assist";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:quietmomentshome"; // Preferred URI
    p.description = "Settings > System > Focus assist";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Focus assist - During these hours";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:quietmomentsscheduled";
    p.description = "Settings > System > Focus assist > During these hours";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Focus assist - Duplicating my display";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:quietmomentspresentation";
    p.description = "Settings > System > Focus assist > When I'm duplicating my display";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Focus assist - Playing a game full screen";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:quietmomentsgame";
    p.description = "Settings > System > Focus assist > When I'm playing a game";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Power & sleep";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:powersleep";
    p.description = "Settings > System > Power & sleep";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Battery";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:batterysaver";
    p.description = "Settings > System > Battery";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Battery - App usage details";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:batterysaver-usagedetails";
    p.description = "Settings > System > Battery > See which apps are affecting your battery life";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Battery Saver settings";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:batterysaver-settings";
    p.description = "Settings > System > Battery > Battery Saver settings";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Storage";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:storagesense";
    p.description = "Settings > System > Storage";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Configure Storage Sense";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:storagepolicies";
    p.description = "Settings > System > Storage > Configure Storage Sense or run it now";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Change where new content is saved";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:savelocations";
    p.description = "Settings > System > Storage > Change where new content is saved";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Tablet mode";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:tabletmode";
    p.description = "Settings > System > Tablet mode";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Multitasking";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:multitasking";
    p.description = "Settings > System > Multitasking";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Projecting to this PC";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:project";
    p.description = "Settings > System > Projecting to this PC";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Shared experiences";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:crossdevice";
    p.description = "Settings > System > Shared experiences";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Clipboard";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:clipboard";
    p.description = "Settings > System > Clipboard";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Remote Desktop";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:remotedesktop";
    p.description = "Settings > System > Remote Desktop";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Device Encryption";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:deviceencryption";
    p.description = "Settings > System > Device Encryption (where available)";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "About";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:about";
    p.description = "Settings > System > About";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    // --- Devices ---
    p = utils::Program{};
    p.name = "Bluetooth & other devices";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:bluetooth"; // Preferred URI
    p.description = "Settings > Devices > Bluetooth & other devices";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Printers & scanners";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:printers";
    p.description = "Settings > Devices > Printers & scanners";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Mouse";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:mousetouchpad"; // URI covers both
    p.description = "Settings > Devices > Mouse";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Touchpad";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:devices-touchpad";
    p.description = "Settings > Devices > Touchpad";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Typing";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:typing";
    p.description = "Settings > Devices > Typing";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Hardware keyboard - Text suggestions";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:devicestyping-hwkbtextsuggestions";
    p.description = "Settings > Devices > Typing > Hardware keyboard text suggestions";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Wheel";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:wheel";
    p.description = "Settings > Devices > Wheel (where available)";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Pen & Windows Ink";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:pen";
    p.description = "Settings > Devices > Pen & Windows Ink";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "AutoPlay";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:autoplay";
    p.description = "Settings > Devices > AutoPlay";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "USB";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:usb";
    p.description = "Settings > Devices > USB";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    // --- Phone ---
    p = utils::Program{};
    p.name = "Phone";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:mobile-devices";
    p.description = "Settings > Phone";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Add a phone";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:mobile-devices-addphone";
    p.description = "Settings > Phone > Add a phone";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Your Phone (opens app)";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:mobile-devices-addphone-direct";
    p.description = "Settings > Phone > Your Phone (opens app)";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    // --- Network & Internet ---
    p = utils::Program{};
    p.name = "Network & Internet";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:network";
    p.description = "Settings > Network & Internet";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Status";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:network-status";
    p.description = "Settings > Network & Internet > Status";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    // Note: ms-availablenetworks: is not an ms-settings: URI, skipping.

    p = utils::Program{};
    p.name = "Cellular & SIM";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:network-cellular";
    p.description = "Settings > Network & Internet > Cellular & SIM";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Wi-Fi";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:network-wifi";
    p.description = "Settings > Network & Internet > Wi-Fi";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Manage known networks";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:network-wifisettings";
    p.description = "Settings > Network & Internet > Wi-Fi > Manage known networks";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Wi-Fi Calling";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:network-wificalling";
    p.description = "Settings > Network & Internet > Wi-Fi Calling";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Ethernet";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:network-ethernet";
    p.description = "Settings > Network & Internet > Ethernet";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Dial-up";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:network-dialup";
    p.description = "Settings > Network & Internet > Dial-up";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "DirectAccess";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:network-directaccess";
    p.description = "Settings > Network & Internet > DirectAccess (where available)";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "VPN";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:network-vpn";
    p.description = "Settings > Network & Internet > VPN";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Airplane mode";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:network-airplanemode"; // Preferred URI
    p.description = "Settings > Network & Internet > Airplane mode";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Mobile hotspot";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:network-mobilehotspot";
    p.description = "Settings > Network & Internet > Mobile hotspot";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "NFC";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:nfctransactions";
    p.description = "Settings > Network & Internet > NFC";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Data usage";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:datausage";
    p.description = "Settings > Network & Internet > Data usage";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Proxy";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:network-proxy";
    p.description = "Settings > Network & Internet > Proxy";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    // --- Personalization ---
    p = utils::Program{};
    p.name = "Personalization";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:personalization";
    p.description = "Settings > Personalization";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Background";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:personalization-background";
    p.description = "Settings > Personalization > Background";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Colors";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:colors"; // Preferred URI
    p.description = "Settings > Personalization > Colors";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Lock screen";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:lockscreen";
    p.description = "Settings > Personalization > Lock screen";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Themes";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:themes";
    p.description = "Settings > Personalization > Themes";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Fonts";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:fonts";
    p.description = "Settings > Personalization > Fonts";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Start";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:personalization-start";
    p.description = "Settings > Personalization > Start";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Choose which folders appear on Start";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:personalization-start-places";
    p.description = "Settings > Personalization > Start > Choose which folders appear on Start";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Taskbar";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:taskbar";
    p.description = "Settings > Personalization > Taskbar";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    // --- Apps ---
    p = utils::Program{};
    p.name = "Apps & features";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:appsfeatures"; // Preferred URI
    p.description = "Settings > Apps > Apps & features";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Manage optional features";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:optionalfeatures";
    p.description = "Settings > Apps > Manage optional features";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Default apps";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:defaultapps";
    p.description = "Settings > Apps > Default apps";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Offline maps";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:maps";
    p.description = "Settings > Apps > Offline maps";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Download maps";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:maps-downloadmaps";
    p.description = "Settings > Apps > Offline maps > Download maps";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Apps for websites";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:appsforwebsites";
    p.description = "Settings > Apps > Apps for websites";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Video playback";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:videoplayback";
    p.description = "Settings > Apps > Video playback";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Startup";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:startupapps";
    p.description = "Settings > Apps > Startup";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    // --- Accounts ---
    p = utils::Program{};
    p.name = "Your info";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:yourinfo";
    p.description = "Settings > Accounts > Your info";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Email & accounts";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:emailandaccounts";
    p.description = "Settings > Accounts > Email & accounts";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Sign-in options";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:signinoptions";
    p.description = "Settings > Accounts > Sign-in options";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Windows Hello face setup";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:signinoptions-launchfaceenrollment";
    p.description = "Settings > Accounts > Sign-in options > Windows Hello face setup";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Windows Hello fingerprint setup";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:signinoptions-launchfingerprintenrollment";
    p.description = "Settings > Accounts > Sign-in options > Windows Hello fingerprint setup";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Security Key setup";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:signinoptions-launchsecuritykeyenrollment";
    p.description = "Settings > Accounts > Sign-in options > Security Key setup";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Dynamic Lock";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:signinoptions-dynamiclock";
    p.description = "Settings > Accounts > Sign-in options > Dynamic Lock";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Access work or school";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:workplace";
    p.description = "Settings > Accounts > Access work or school";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Family & other people";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:otherusers";
    p.description = "Settings > Accounts > Family & other people";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Set up a kiosk";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:assignedaccess";
    p.description = "Settings > Accounts > Set up a kiosk";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Sync your settings";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:sync";
    p.description = "Settings > Accounts > Sync your settings";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    // --- Time & language ---
    p = utils::Program{};
    p.name = "Date & time";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:dateandtime";
    p.description = "Settings > Time & language > Date & time";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Region";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:regionformatting";
    p.description = "Settings > Time & language > Region";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Language";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:regionlanguage"; // Preferred URI
    p.description = "Settings > Time & language > Language";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Windows Display language";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:regionlanguage-setdisplaylanguage";
    p.description = "Settings > Time & language > Language > Windows Display language";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Add Display language";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:regionlanguage-adddisplaylanguage";
    p.description = "Settings > Time & language > Language > Add Display language";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Speech";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:speech";
    p.description = "Settings > Time & language > Speech";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    // --- Gaming ---
    p = utils::Program{};
    p.name = "Game bar";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:gaming-gamebar";
    p.description = "Settings > Gaming > Game bar";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Captures";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:gaming-gamedvr";
    p.description = "Settings > Gaming > Captures";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Broadcasting";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:gaming-broadcasting";
    p.description = "Settings > Gaming > Broadcasting";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Game Mode";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:gaming-gamemode";
    p.description = "Settings > Gaming > Game Mode";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "TruePlay";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:gaming-trueplay";
    p.description = "Settings > Gaming > TruePlay (removed in version 1809+)";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Xbox Networking";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:gaming-xboxnetworking";
    p.description = "Settings > Gaming > Xbox Networking";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    // --- Ease of Access ---
    p = utils::Program{};
    p.name = "Display (Ease of Access)";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:easeofaccess-display";
    p.description = "Settings > Ease of Access > Display";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Mouse Pointer";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:easeofaccess-cursorandpointersize";
    p.description = "Settings > Ease of Access > Mouse Pointer";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Text Cursor";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:easeofaccess-cursor";
    p.description = "Settings > Ease of Access > Text Cursor";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Magnifier";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:easeofaccess-magnifier";
    p.description = "Settings > Ease of Access > Magnifier";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Color Filters";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:easeofaccess-colorfilter";
    p.description = "Settings > Ease of Access > Color Filters";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Adaptive Color Filters Link";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:easeofaccess-colorfilter-adaptivecolorlink";
    p.description = "Settings > Ease of Access > Color Filters > Adaptive Color Filters Link";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Night Light Link";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:easeofaccess-colorfilter-bluelightlink";
    p.description = "Settings > Ease of Access > Color Filters > Night Light Link";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "High Contrast";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:easeofaccess-highcontrast";
    p.description = "Settings > Ease of Access > High Contrast";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Narrator";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:easeofaccess-narrator";
    p.description = "Settings > Ease of Access > Narrator";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Narrator - Start after sign-in";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:easeofaccess-narrator-isautostartenabled";
    p.description = "Settings > Ease of Access > Narrator > Start Narrator after sign-in for me";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Audio (Ease of Access)";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:easeofaccess-audio";
    p.description = "Settings > Ease of Access > Audio";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Closed captions";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:easeofaccess-closedcaptioning";
    p.description = "Settings > Ease of Access > Closed captions";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Speech (Ease of Access)";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:easeofaccess-speechrecognition";
    p.description = "Settings > Ease of Access > Speech";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Keyboard (Ease of Access)";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:easeofaccess-keyboard";
    p.description = "Settings > Ease of Access > Keyboard";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Mouse (Ease of Access)";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:easeofaccess-mouse";
    p.description = "Settings > Ease of Access > Mouse";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Eye Control";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:easeofaccess-eyecontrol";
    p.description = "Settings > Ease of Access > Eye Control";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Other options (Ease of Access)";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:easeofaccess-otheroptions";
    p.description = "Settings > Ease of Access > Other options (removed in version 1809+)";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    // --- Search ---
    p = utils::Program{};
    p.name = "Permissions & history";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:search-permissions";
    p.description = "Settings > Search > Permissions & history";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Searching Windows";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:cortana-windowssearch"; // URI from list
    p.description = "Settings > Search > Searching Windows";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Search - More details";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:search-moredetails";
    p.description = "Settings > Search > More details";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    // --- Cortana ---
    p = utils::Program{};
    p.name = "Cortana";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:cortana";
    p.description = "Settings > Cortana";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Talk to Cortana";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:cortana-talktocortana";
    p.description = "Settings > Cortana > Talk to Cortana";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Cortana - Permissions";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:cortana-permissions";
    p.description = "Settings > Cortana > Permissions";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Cortana - More details";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:cortana-moredetails";
    p.description = "Settings > Cortana > More details";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    // --- Privacy ---
    p = utils::Program{};
    p.name = "General (Privacy)";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:privacy"; // URI for General privacy settings
    p.description = "Settings > Privacy > General";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Speech (Privacy)";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:privacy-speech";
    p.description = "Settings > Privacy > Speech";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Inking & typing personalization";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:privacy-speechtyping"; // URI from list
    p.description = "Settings > Privacy > Inking & typing personalization";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Diagnostics & feedback";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:privacy-feedback";
    p.description = "Settings > Privacy > Diagnostics & feedback";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "View Diagnostic Data";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:privacy-feedback-telemetryviewergroup"; // URI from list
    p.description = "Settings > Privacy > Diagnostics & feedback > View Diagnostic Data";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Activity history";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:privacy-activityhistory";
    p.description = "Settings > Privacy > Activity history";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Location";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:privacy-location";
    p.description = "Settings > Privacy > Location";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Camera";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:privacy-webcam";
    p.description = "Settings > Privacy > Camera";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Microphone";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:privacy-microphone";
    p.description = "Settings > Privacy > Microphone";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Voice activation";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:privacy-voiceactivation";
    p.description = "Settings > Privacy > Voice activation";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Notifications (Privacy)";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:privacy-notifications";
    p.description = "Settings > Privacy > Notifications";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Account info";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:privacy-accountinfo";
    p.description = "Settings > Privacy > Account info";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Contacts";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:privacy-contacts";
    p.description = "Settings > Privacy > Contacts";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Calendar";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:privacy-calendar";
    p.description = "Settings > Privacy > Calendar";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Phone calls";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:privacy-phonecalls";
    p.description = "Settings > Privacy > Phone calls (removed in version 1809+)";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Call history";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:privacy-callhistory";
    p.description = "Settings > Privacy > Call history";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Email";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:privacy-email";
    p.description = "Settings > Privacy > Email";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Eye tracker";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:privacy-eyetracker";
    p.description = "Settings > Privacy > Eye tracker (requires hardware)";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Tasks";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:privacy-tasks";
    p.description = "Settings > Privacy > Tasks";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Messaging";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:privacy-messaging";
    p.description = "Settings > Privacy > Messaging";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Radios";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:privacy-radios";
    p.description = "Settings > Privacy > Radios";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Other devices";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:privacy-customdevices";
    p.description = "Settings > Privacy > Other devices";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Background apps";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:privacy-backgroundapps";
    p.description = "Settings > Privacy > Background apps";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "App diagnostics";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:privacy-appdiagnostics";
    p.description = "Settings > Privacy > App diagnostics";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Automatic file downloads";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:privacy-automaticfiledownloads";
    p.description = "Settings > Privacy > Automatic file downloads";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Documents";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:privacy-documents";
    p.description = "Settings > Privacy > Documents";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Pictures";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:privacy-pictures";
    p.description = "Settings > Privacy > Pictures";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Videos";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:privacy-videos"; // URI from list (was privacy-documents)
    p.description = "Settings > Privacy > Videos";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "File system";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:privacy-broadfilesystemaccess";
    p.description = "Settings > Privacy > File system";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    // --- Update & security ---
    p = utils::Program{};
    p.name = "Windows Update";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:windowsupdate";
    p.description = "Settings > Update & security > Windows Update";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Check for updates";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:windowsupdate-action";
    p.description = "Settings > Update & security > Windows Update > Check for updates";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "View update history";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:windowsupdate-history";
    p.description = "Settings > Update & security > Windows Update > View update history";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Restart options";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:windowsupdate-restartoptions";
    p.description = "Settings > Update & security > Windows Update > Restart options";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Advanced options";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:windowsupdate-options";
    p.description = "Settings > Update & security > Windows Update > Advanced options";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Change active hours";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:windowsupdate-activehours";
    p.description = "Settings > Update & security > Windows Update > Change active hours";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Optional updates";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:windowsupdate-optionalupdates"; // Preferred URI
    p.description = "Settings > Update & security > Windows Update > Optional updates";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Delivery Optimization";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:delivery-optimization";
    p.description = "Settings > Update & security > Delivery Optimization";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Windows Security / Windows Defender";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:windowsdefender";
    p.description = "Settings > Update & security > Windows Security";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    // Note: windowsdefender: is not an ms-settings: URI, skipping "Open Windows Security".

    p = utils::Program{};
    p.name = "Backup";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:backup";
    p.description = "Settings > Update & security > Backup";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Troubleshoot";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:troubleshoot";
    p.description = "Settings > Update & security > Troubleshoot";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Recovery";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:recovery";
    p.description = "Settings > Update & security > Recovery";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Activation";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:activation";
    p.description = "Settings > Update & security > Activation";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Find My Device";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:findmydevice";
    p.description = "Settings > Update & security > Find My Device";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "For developers";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:developers";
    p.description = "Settings > Update & security > For developers";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Windows Insider Program";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:windowsinsider"; // Preferred URI
    p.description = "Settings > Update & security > Windows Insider Program";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    // --- Mixed reality ---
    p = utils::Program{};
    p.name = "Mixed reality";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:holographic";
    p.description = "Settings > Mixed reality";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Audio and speech";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:holographic-audio";
    p.description = "Settings > Mixed reality > Audio and speech";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Environment";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:privacy-holographic-environment";
    p.description = "Settings > Mixed reality > Environment";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Headset display";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:holographic-headset";
    p.description = "Settings > Mixed reality > Headset display";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    p = utils::Program{};
    p.name = "Uninstall (Mixed Reality)";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:holographic-management";
    p.description = "Settings > Mixed reality > Uninstall";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));

    // --- Extras ---
    p = utils::Program{};
    p.name = "Extras";
    p.executablePath = "explorer.exe";
    p.arguments = "ms-settings:extras";
    p.description = "Settings > Extras (available only when Settings app extensions installed)";
    p.kind = "setting";
    p.source = "Microsoft";
    settingsPages.push_back(std::move(p));


    return settingsPages;
}