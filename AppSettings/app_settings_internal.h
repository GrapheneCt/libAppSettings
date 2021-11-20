#pragma once

typedef struct AppSettingsInitParam AppSettingsInitParam;

extern "C" {

	__declspec (dllexport) int appSettingsInit(AppSettingsInitParam *pInitParam);
	__declspec (dllexport) int appSettingsTerm();
	__declspec (dllexport) int appSettingsGetInt(const char *key, int *value, int defaultValue);
	__declspec (dllexport) int appSettingsSetInt(const char *key, int value);
	__declspec (dllexport) int appSettingsOpen();
	__declspec (dllexport) int appSettingsReset();
	__declspec (dllexport) int appSettingsWaitEnd();
	__declspec (dllexport) int appSettingsUpdate(SceCommonDialogUpdateParam *updateParam);
	__declspec (dllexport) int appSettingsIsOpened();

}