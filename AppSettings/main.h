#pragma once

#include <kernel.h>
#include <common_dialog.h>

#include "app_settings_internal.h"

int _appSettingsInit(AppSettingsInitParam *pInitParam);
int _appSettingsTerm();
int _appSettingsGetInt(const char *key, int *value, int defaultValue);
int _appSettingsSetInt(const char *key, int value);
int _appSettingsOpen();
int _appSettingsReset();
int _appSettingsWaitEnd();
int _appSettingsUpdate(SceCommonDialogUpdateParam *updateParam);
int _appSettingsIsOpened();