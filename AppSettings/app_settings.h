#ifndef _APP_SETTINGS_H_
#define _APP_SETTINGS_H_

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

typedef struct AppSettingsInitParam {
	char *configFilePath;				// Path to XML app settings config/layout file
	unsigned int iniSafememMaxSize;		// Maximum amount of bytes that will be used out of safe memory to store internal INI config
	unsigned int iniSafememOffset;		// Offset in the safe memory where internal INI config will be stored 
	AppSettingsCallbacks callbacks;		// Operation callbacks, see ::AppSettingsCallbacks
} AppSettingsInitParam;

/**
 * Initialize app settings system.
 * When this function is called, GXM must be uninitialized and all framebuffer operations must be paused.
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

#ifdef __cplusplus
}
#endif

#endif