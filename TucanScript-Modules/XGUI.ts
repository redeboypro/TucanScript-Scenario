hXGUILib = LoadLibrary("libXGUI.dylib");

XGUI_Frame = GetProcAddr(hXGUILib, "Frame");
XGUI_IsAppRunning = GetProcAddr(hXGUILib, "IsAppRunning");
XGUI_Flush = GetProcAddr(hXGUILib, "Flush");
XGUI_TextField = GetProcAddr(hXGUILib, "TextField");
XGUI_TextField_SetText = GetProcAddr(hXGUILib, "TextField_SetText");
XGUI_TextField_GetText = GetProcAddr(hXGUILib, "TextField_GetText");
XGUI_Button = GetProcAddr(hXGUILib, "Button");