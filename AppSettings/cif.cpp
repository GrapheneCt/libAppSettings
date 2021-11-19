#include <kernel.h>
#include <paf.h>

#include "main.h"
#include "app_settings_internal.h"

extern "C" {

	int __module_stop(SceSize argc, const void *args) {
		return SCE_KERNEL_STOP_SUCCESS;
	}

	int __module_exit() {
		return SCE_KERNEL_STOP_SUCCESS;
	}

	int __module_start(SceSize argc, void *args) {
		return SCE_KERNEL_START_SUCCESS;
	}

	__declspec (dllexport) int appSettingsInit(AppSettingsInitParam *pInitParam)
	{
		return _appSettingsInit(pInitParam);
	}

	__declspec (dllexport) int appSettingsTerm()
	{
		return _appSettingsTerm();
	}

	__declspec (dllexport) int appSettingsGetInt(const char *key, int *value, int defaultValue)
	{
		return _appSettingsGetInt(key, value, defaultValue);
	}

	__declspec (dllexport) int appSettingsSetInt(const char *key, int value)
	{
		return _appSettingsSetInt(key, value);
	}

	__declspec (dllexport) int appSettingsOpen()
	{
		return _appSettingsOpen();
	}

	__declspec (dllexport) int appSettingsReset()
	{
		return _appSettingsReset();
	}

	__declspec (dllexport) int appSettingsWaitEnd()
	{
		return _appSettingsWaitEnd();
	}
}