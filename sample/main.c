#include <stddef.h>
#include <stdio.h>

#ifdef _SCE_HOST_COMPILER_SNC
#include <kernel.h>
#else
#include <psp2/kernel/modulemgr.h>
#include <psp2/gxt.h> // For SCE_OK. Why is SCE_OK there? Don't ask me...
#endif

#include "../AppSettings/app_settings.h"

static wchar_t teststr[] = L"Test String";

wchar_t *getString(const char *elementId)
{
	return teststr;
}

int isVisible(const char *elementId, int *isVisible)
{
	*isVisible = 1;
	return SCE_OK;
}

int onValueChange(const char *elementId, const char *newValue)
{
	printf("Value of %s changed to %s\n", elementId, newValue);
	return SCE_OK;
}

int main()
{
	int ret = 0;

	printf("AppSettings sample begin\n");

	SceUID modid = sceKernelLoadStartModule("app0:libAppSettings.suprx", 0, 0, 0, 0, 0);
	if (modid <= 0) {
		printf("sceKernelLoadStartModule 0x%X\n", modid);
		return 0;
	}

	AppSettingsInitParam iparam;
	iparam.renderingMode = APP_SETTINGS_RENDERING_MODE_STANDALONE;
	iparam.configFilePath = "app0:settings.xml";
	iparam.iniSafememMaxSize = 0x1000;
	iparam.iniSafememOffset = 0;
	iparam.callbacks.getString = getString;
	iparam.callbacks.isElemVisible = isVisible;
	iparam.callbacks.onValueChange = onValueChange;

	ret = appSettingsInit(&iparam);
	if (ret < 0) {
		printf("appSettingsInit 0x%X\n", ret);
		return 0;
	}

	ret = appSettingsOpen();
	if (ret < 0) {
		printf("appSettingsOpen 0x%X\n", ret);
		return 0;
	}

	printf("App settings dialog opened, waiting for end...\n");

	ret = appSettingsWaitEnd();
	if (ret < 0) {
		printf("appSettingsWaitEnd 0x%X\n", ret);
		return 0;
	}

	printf("App settings dialog closed by user\n");

	ret = appSettingsTerm();
	if (ret < 0) {
		printf("appSettingsTerm 0x%X\n", ret);
		return 0;
	}

	printf("AppSettings sample finished successfully\n");

	return 0;
}