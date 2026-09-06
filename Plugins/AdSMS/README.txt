AdSMS 3.0.0 - Unreal Engine 5.4

PROJECT-AGNOSTIC GAMEINSTANCE ARCHITECTURE
============================================
The plugin does NOT require AdSMSGameInstance. Keep whatever GameInstance class your project already uses (for example AdmobGM).

Runtime ownership:
  Project GameInstance (any class)
      |
      +-- UAdSMSSubsystem (automatically created by Unreal)
              |
              +-- UAdManager

Do NOT cast your GameInstance to AdSMSGameInstance. Do NOT set the project GameInstance to an AdSMS GameInstance.

BLUEPRINT ACCESS
================
The plugin now exposes project-agnostic helper nodes:
  Get AdSMS Subsystem
  Get AdManager

These resolve the subsystem from the current world's GameInstance, regardless of the GameInstance class name.

Recommended Widget flow:
  Event Construct (optional)
      -> Get AdManager
      -> Show Banner / Load Interstitial / Show Rewarded

Recommended GameInstance flow:
  No AdSMS nodes are required in the project's GameInstance.
  If you want to cache a reference, use Get AdSMS Subsystem -> Set your own variable.

IMPORTANT: If an old Blueprint contains a node named "Get Game Instance Subsystem" showing "Invalid Subsystem Type" or pins that no longer exist, delete that old node and recreate it, or use the new Get AdSMS Subsystem helper.

INSTALLATION
============
1. Close Unreal Editor.
2. Remove the old AdSMS plugin folder from the project Plugins folder.
3. Extract this package so the folder structure is:
      <Project>/Plugins/AdSMS/AdSMS.uplugin
      <Project>/Plugins/AdSMS/Source/AdSMS/...
4. Do NOT keep an old AdSMS folder nested inside this folder.
5. Delete the project's Binaries and Intermediate folders. If the project still reports stale Blueprint imports, also delete Saved.
6. Right-click the .uproject -> Generate Visual Studio project files.
7. Open the project and allow the AdSMS module to rebuild.
8. Verify the plugin is enabled in Edit -> Plugins.

PACKAGING ERROR FROM 2026-09-06
==============================
The supplied cook log showed:
  Failed to find script package for import object 'Package /Script/AdSMS'
  Invalid Subsystem Type must have a class specified
  In use pin AdMob Subsystem Ref no longer exists

Those errors occur while loading the AdmobGM Blueprint during cook. The Blueprint still contains a stale AdSMS subsystem node/import while the cook process does not have the AdSMS script package available. The old node must be recreated after the new module is loaded; the helper nodes in this version avoid the custom GameInstance dependency.

ANDROID / ADMOB
================
The plugin includes the AdMob UPL and native billboard implementation. The default native billboard unit is Google's test Native Ad unit. Replace it with your own Native Ad Unit ID for live testing/production.

NOTE
====
The current 3D billboard implementation is a NativeAd -> bitmap -> UE Texture prototype. It is suitable for validating rendering/texture flow, but it should not be treated as a production click/impression implementation without a supported interactive NativeAdView rendering path.
