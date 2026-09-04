AdMob Plugin - Unreal Engine 5.4.4

Features:
- Banner ads
- Show/Hide Banner
- Show Banner At Screen Position (X,Y)
- Interstitial ads
- Rewarded ads

Install:
1. Close Unreal Editor.
2. Extract the AdMobPlugin folder into Project/Plugins/.
3. Regenerate project files if required.
4. Open the project in Unreal Engine 5.4.4.
5. Enable AdMob Plugin.
6. Restart the editor.

Android:
The plugin uses UPL to add Google Mobile Ads and remove the legacy StartupPermissions storage metadata.

Important:
- The current native ad unit IDs in the C++ test implementation are Google test IDs.
- Show Banner At Screen Position places a native Android overlay at screen pixel coordinates; it is not a texture/material on a 3D billboard.
