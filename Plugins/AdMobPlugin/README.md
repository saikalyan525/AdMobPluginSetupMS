# AdMob Plugin for Unreal Engine 5.4.4 — Billboard v5

This version keeps the verified Banner/Interstitial/Rewarded implementation and adds a compile-safe World-Space WidgetComponent banner tracker.

## Blueprint API
- Initialize Ads
- Load Banner
- Show Banner
- Hide Banner
- Is Banner Loaded
- Show Banner At Widget (WidgetComponent)
- Show Banner At Screen Position
- Load/Show/Is Ready for Interstitial
- Load/Show/Is Ready for Rewarded

## Billboard usage
1. Create a normal UMG widget used by the WidgetComponent.
2. Put the WidgetComponent on your billboard Actor and set Space = World.
3. Set Widget Class to your billboard widget.
4. At BeginPlay, call **Show Banner At Widget** and pass the WidgetComponent.
5. The plugin projects the WidgetComponent world location to the game viewport and moves the native Android AdMob view to that screen position.
6. Move/rotate the billboard anywhere in the world; the tracker updates the native ad position.

## Important limitation
AdMob Banner is a native Android view. It is not literally rendered into the WidgetComponent render target. The plugin keeps the native view visually aligned with the billboard's projected screen position.

## Clean build
Close Unreal/Visual Studio before replacing the plugin. Delete the project's `Binaries` and `Intermediate` folders, regenerate project files, rebuild the Editor, then package Android.

## Android permissions
The UPL explicitly removes legacy READ/WRITE_EXTERNAL_STORAGE and UE StartupPermissions metadata because this plugin does not need legacy public storage.
