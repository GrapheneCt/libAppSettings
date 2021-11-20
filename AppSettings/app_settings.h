#ifndef _APP_SETTINGS_H_
#define _APP_SETTINGS_H_

#ifdef __SNC__
#include <common_dialog.h>
#else
#include <psp2/common_dialog.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef int(*AppSettingsIsElementVisibleCB)(const char *elementId, int *pIsVisible);
typedef int(*AppSettingsOnValueChangeCB)(const char *elementId, const char *newValue);
typedef wchar_t*(*AppSettingsGetStringCB)(const char *elementId);

typedef struct AppSettingsCallbacks {
	AppSettingsIsElementVisibleCB isElemVisible;	// Visibility check for element of app settings. Set IsVisible to 0 to hide element
	AppSettingsOnValueChangeCB onValueChange;		// Called when user selects new value for element of app settings. New value is saved in internal INI config automatically
	AppSettingsGetStringCB getString;				// Called when there is new request for string element value of app settings layout.
} AppSettingsCallbacks;

typedef enum AppSettingsMode {
	APP_SETTINGS_RENDERING_MODE_STANDALONE,		// Use built-in renderer
	APP_SETTINGS_RENDERING_MODE_COMMON_DIALOG,	// Use SceCommonDialog rendering method
} AppSettingsMode;

typedef struct AppSettingsInitParam {
	int renderingMode;					// Dialog rendering mode. If APP_SETTINGS_RENDERING_MODE_COMMON_DIALOG is used, appSettingsUpdate() must be called with the same rules as sceCommonDialogUpdate()
	char *configFilePath;				// Path to XML app settings config/layout file
	unsigned int iniSafememMaxSize;		// Maximum amount of bytes that will be used out of safe memory to store internal INI config
	unsigned int iniSafememOffset;		// Offset in the safe memory where internal INI config will be stored 
	AppSettingsCallbacks callbacks;		// Operation callbacks, see ::AppSettingsCallbacks
} AppSettingsInitParam;

/**
 * Initialize app settings system.
 *
 * @param[in] pInitParam - structure containing app settings init params
 *
 * @return SCE_OK, <0 on error.
 */
int appSettingsInit(AppSettingsInitParam *pInitParam);

/**
 * Terminate app settings system.
 *
 * @return SCE_OK, <0 on error.
 */
int appSettingsTerm();

/**
 * Get integer value corresponding to set key.
 *
 * @param[in] key - key string
 * @param[in] value - buffer to store value
 * @param[in] defaultValue - value to use if key was not found
 *
 * @return SCE_OK, <0 on error.
 */
int appSettingsGetInt(const char *key, int *value, int defaultValue);

/**
 * Set integer value key.
 *
 * @param[in] key - key string
 * @param[in] value - key value
 *
 * @return SCE_OK, <0 on error.
 */
int appSettingsSetInt(const char *key, int value);

/**
 * Open app settings dialog.
 *
 * @return SCE_OK, <0 on error.
 */
int appSettingsOpen();

/**
 * Update app settings dialog. Same as sceCommonDialogUpdate()
 *
 * @param[in] updateParam - render target params
 *
 * @return SCE_OK, <0 on error.
 */
int appSettingsUpdate(SceCommonDialogUpdateParam *updateParam);

/**
 * Reset app settings.
 * Useful if settings version has been changed.
 *
 * @return SCE_OK, <0 on error.
 */
int appSettingsReset();

/**
 * Block caller thread until user closed app settings dialog.
 *
 * @return SCE_OK, <0 on error.
 */
int appSettingsWaitEnd();

/**
 * Checks if app settings dialog is currently opened.
 *
 * @return 1 if opened, 0 otherwise.
 */
int appSettingsIsOpened();

#ifdef __cplusplus
}
#endif

#endif