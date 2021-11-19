#include <libsysmodule.h>
#include <kernel.h>
#include <appmgr.h>
#include <paf.h>
#include <shellsvc.h>
#include <taihen.h>

#ifdef _DEBUG
#include <libdbg.h>
#endif

#include <bxce.h>
#include <app_settings.h>
#include <ini_file_processor.h>

#include "main.h"
#include "app_settings.h"

extern "C" {

	int sceKernelIsGameBudget(void);

}

typedef struct SceSysmoduleOpt {
	int flags;
	int *result;
	int unused[2];
} SceSysmoduleOpt;

typedef struct ScePafInit {
	SceSize global_heap_size;
	int a2;
	int a3;
	int cdlg_mode;
	int heap_opt_param1;
	int heap_opt_param2;
} ScePafInit; // size is 0x18

using namespace paf;
using namespace sce;

static Framework *s_fw = SCE_NULL;
static SceUID s_appSetThrd = SCE_UID_INVALID_UID;
static SceUID s_appSetThrdOpEvf = SCE_UID_INVALID_UID;
static int s_error = SCE_OK;
static int s_isOpened = 0;
static AppSettings *s_appSet = SCE_NULL;
static AppSettingsInitParam s_params;

static int loadPafPrx()
{
	int ret = -1, load_res;
	ScePafInit init_param;
	SceSysmoduleOpt sysmodule_opt;

	init_param.global_heap_size = 3 * 1024 * 1024;
	init_param.a2 = 0x0000EA60;
	init_param.a3 = 0x00040000;
	init_param.cdlg_mode = 0;
	init_param.heap_opt_param1 = 0;
	init_param.heap_opt_param2 = 0;

	sysmodule_opt.flags = 0; // with arg
	sysmodule_opt.result = &load_res;

	ret = sceSysmoduleLoadModuleInternalWithArg(SCE_SYSMODULE_INTERNAL_PAF, sizeof(init_param), &init_param, &sysmodule_opt);

#ifdef _DEBUG
	if (ret < 0) {
		SCE_DBG_LOG_ERROR("[PAF PRX] Loader: 0x%x\n", ret);
		SCE_DBG_LOG_ERROR("[PAF PRX] Loader result: 0x%x\n", load_res);
	}
#endif

	return ret;
}

int _appSettingsThread(unsigned int argSize, void *pArgBlock)
{
	int	res;
	int paf_sysmoduleLoadedLocally = 0;
	int bxce_sysmoduleLoadedLocally = 0;
	int ini_sysmoduleLoadedLocally = 0;
	int cdlg_sysmoduleLoadedLocally = 0;
	Framework::InitParam *fwParam;
	Framework::PluginInitParam *pluginParam;
	AppSettings::InitParam sparam;

	if (sceSysmoduleIsLoadedInternal(SCE_SYSMODULE_INTERNAL_PAF)) {
		res = loadPafPrx();
		if (res != SCE_OK) {
#ifdef _DEBUG
			SCE_DBG_LOG_ERROR("loadPafPrx(): 0x%X\n", res);
#endif
			goto return_with_result;
		}
		paf_sysmoduleLoadedLocally = 1;
	}

	if (sceSysmoduleIsLoadedInternal(SCE_SYSMODULE_INTERNAL_BXCE)) {
		sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_BXCE);
		bxce_sysmoduleLoadedLocally = 1;
	}
	if (sceSysmoduleIsLoadedInternal(SCE_SYSMODULE_INTERNAL_INI_FILE_PROCESSOR)) {
		sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_INI_FILE_PROCESSOR);
		ini_sysmoduleLoadedLocally = 1;
	}
	if (sceSysmoduleIsLoadedInternal(SCE_SYSMODULE_INTERNAL_COMMON_GUI_DIALOG)) {
		sceSysmoduleLoadModuleInternal(SCE_SYSMODULE_INTERNAL_COMMON_GUI_DIALOG);
		cdlg_sysmoduleLoadedLocally = 1;
	}

	fwParam = new Framework::InitParam();

	fwParam->LoadDefaultParams();

	if (sceKernelIsGameBudget())
		fwParam->applicationMode = Framework::Mode_ApplicationDefault;
	else
		fwParam->applicationMode = Framework::Mode_ApplicationA;

	fwParam->defaultSurfacePoolSize = 4 * 1024 * 1024;
	fwParam->textSurfaceCacheSize = 1 * 1024 * 1024;
	fwParam->graphMemSystemHeapSize = 512 * 1024;

	s_fw = new Framework(fwParam);
	s_fw->LoadCommonResource();

	pluginParam = new Framework::PluginInitParam();

	pluginParam->pluginName.Set("app_settings_plugin");
	pluginParam->resourcePath.Set("vs0:vsh/common/app_settings_plugin.rco");
	pluginParam->scopeName.Set("__main__");

	pluginParam->pluginCreateCB = sce::AppSettings::PluginCreateCB;
	pluginParam->pluginInitCB = sce::AppSettings::PluginInitCB;
	pluginParam->pluginStartCB = sce::AppSettings::PluginStartCB;
	pluginParam->pluginStopCB = sce::AppSettings::PluginStopCB;
	pluginParam->pluginExitCB = sce::AppSettings::PluginExitCB;
	pluginParam->pluginPath.Set("vs0:vsh/common/app_settings.suprx");
	pluginParam->unk_58 = 0x96;

	s_fw->LoadPlugin(pluginParam);

	Misc::OpenFile(&sparam.xmlFile, s_params.configFilePath, SCE_O_RDONLY, 0, &res);
	if (res != SCE_OK) {
#ifdef _DEBUG
		SCE_DBG_LOG_ERROR("Misc::OpenFile(): 0x%X\n", res);
#endif
		goto return_with_result;
	}

	sparam.allocCB = sce_paf_malloc;
	sparam.freeCB = sce_paf_free;
	sparam.reallocCB = sce_paf_realloc;
	sparam.safeMemoryOffset = s_params.iniSafememOffset;
	sparam.safeMemorySize = s_params.iniSafememMaxSize;

	res = AppSettings::GetInstance(&sparam, &s_appSet);
	if (res != SCE_OK) {
#ifdef _DEBUG
		SCE_DBG_LOG_ERROR("AppSettings::GetInstance(): 0x%X\n", res);
#endif
		goto return_with_result;
	}

	delete fwParam;
	delete pluginParam;

	sceKernelSetEventFlag(s_appSetThrdOpEvf, 1);

	s_fw->EnterRenderingLoop();

	Framework::UnloadPluginAsync("app_settings_plugin");
	delete s_fw;

	if (cdlg_sysmoduleLoadedLocally)
		sceSysmoduleUnloadModuleInternal(SCE_SYSMODULE_INTERNAL_COMMON_GUI_DIALOG);
	if (ini_sysmoduleLoadedLocally)
		sceSysmoduleUnloadModuleInternal(SCE_SYSMODULE_INTERNAL_INI_FILE_PROCESSOR);
	if (bxce_sysmoduleLoadedLocally)
		sceSysmoduleUnloadModuleInternal(SCE_SYSMODULE_INTERNAL_BXCE);
	if (paf_sysmoduleLoadedLocally)
		sceSysmoduleUnloadModuleInternal(SCE_SYSMODULE_INTERNAL_PAF);

	return sceKernelExitDeleteThread(0);

return_with_result:
	s_error = res;
	sceKernelSetEventFlag(s_appSetThrdOpEvf, 1);
	return sceKernelExitDeleteThread(0);
}

int _appSettingsInit(AppSettingsInitParam *pInitParam)
{
	int res;

#ifdef _DEBUG
	sceDbgSetMinimumLogLevel(SCE_DBG_LOG_LEVEL_TRACE);
#endif

	if (!pInitParam) {
#ifdef _DEBUG
		SCE_DBG_LOG_ERROR("pInitParam is NULL\n");
#endif
		return -1;
	}

	sceClibMemcpy(&s_params, pInitParam, sizeof(AppSettingsInitParam));

	s_appSetThrd = sceKernelCreateThread("AppSettingsMainThread", _appSettingsThread, SCE_KERNEL_DEFAULT_PRIORITY_USER, SCE_KERNEL_256KiB, 0, 0, SCE_NULL);
	if (s_appSetThrd <= 0) {
#ifdef _DEBUG
		SCE_DBG_LOG_ERROR("sceKernelCreateThread(): 0x%X\n", res);
#endif
		return s_appSetThrd;
	}
	sceKernelStartThread(s_appSetThrd, 0, SCE_NULL);
	s_appSetThrdOpEvf = sceKernelCreateEventFlag("AppSettingsMainThreadEvf", SCE_KERNEL_EVF_ATTR_MULTI, 0, SCE_NULL);
	if (s_appSetThrdOpEvf <= 0) {
#ifdef _DEBUG
		SCE_DBG_LOG_ERROR("sceKernelCreateEventFlag(): 0x%X\n", res);
#endif
		return s_appSetThrdOpEvf;
	}

	sceKernelWaitEventFlag(s_appSetThrdOpEvf, 1, SCE_KERNEL_EVF_WAITMODE_AND | SCE_KERNEL_EVF_WAITMODE_CLEAR_PAT, SCE_NULL, SCE_NULL);

	return s_error;
}

int _appSettingsTerm()
{
	if (!s_fw || s_isOpened)
		return -1;

	s_fw->ExitRenderingLoop();

	sceKernelDeleteEventFlag(s_appSetThrdOpEvf);
	sceKernelWaitThreadEnd(s_appSetThrd, SCE_NULL, SCE_NULL);

	return SCE_OK;
}

static SceUInt32 getHash(const char *name)
{
	Resource::Element searchRequest;
	Resource::Element searchResult;

	searchRequest.id.Set(name);
	searchResult.hash = searchResult.GetHashById(&searchRequest);

	return searchResult.hash;
}

void fe1(const char *key)
{
}

void fe2(const char *key)
{
}

void fe3(const char *key)
{
}

SceInt32 fe5(const char *key)
{
	return 0;
}

SceInt32 fe6(const char* key, ui::Widget *widg)
{
	return 0;
}

SceInt32 fe8(const char *key, const char* val)
{
	return 0;
}

void fe9()
{
	s_isOpened = 0;
	sceKernelSetEventFlag(s_appSetThrdOpEvf, 2);
}

SceInt32 fe11(graphics::Texture *tex, const char *key)
{
	int res;
	Plugin *appSetPlug = Plugin::Find("app_settings_plugin");
	Resource::Element textureSearchParam;
	Misc::OpenResult fres;
	String fullPath;

	fullPath.Set(key);
	Misc::OpenFile(&fres, fullPath.data, SCE_O_RDONLY, 0, &res);
	fullPath.Clear();

	if (!appSetPlug || res != SCE_OK) {
		textureSearchParam.hash = getHash(key);
		Plugin::LoadTexture(tex, s_fw->crPlugin, &textureSearchParam);
		if (!tex->texSurface) {
			textureSearchParam.hash = getHash("_common_texture_transparent");
			Plugin::LoadTexture(tex, s_fw->crPlugin, &textureSearchParam);
		}
		return 0;
	}

	graphics::Texture::CreateFromFile(tex, appSetPlug->memoryPool, &fres);

	return 0;
}

int _appSettingsOpen()
{
	if (s_isOpened)
		return SCE_OK;

	Plugin *appSetPlug = Plugin::Find("app_settings_plugin");
	if (!appSetPlug) {
#ifdef _DEBUG
		SCE_DBG_LOG_ERROR("Plugin::Find() failed\n");
#endif
		return -1;
	}

	AppSettings::Interface *appSetIf = (AppSettings::Interface *)appSetPlug->GetInterface(1);
	if (!appSetIf) {
#ifdef _DEBUG
		SCE_DBG_LOG_ERROR("appSetPlug->GetInterface() failed\n");
#endif
		return -1;
	}

	AppSettings::InterfaceCallbacks ifCb;

	ifCb.listChangeCb = fe1;
	ifCb.listForwardChangeCb = fe2;
	ifCb.listBackChangeCb = fe3;
	ifCb.isVisibleCb = s_params.callbacks.isElemVisible;
	ifCb.elemInitCb = fe5;
	ifCb.elemAddCb = fe6;
	ifCb.valueChangeCb = s_params.callbacks.onValueChange;
	ifCb.valueChangeCb2 = fe8;
	ifCb.termCb = fe9;
	ifCb.getStringCb = (AppSettings::InterfaceCallbacks::GetStringElement)s_params.callbacks.getString;
	ifCb.getTexCb = fe11;

	s_isOpened = 1;

	return appSetIf->Show(&ifCb);
}

int _appSettingsGetInt(const char *key, int *value, int defaultValue)
{
	if (!s_appSet)
		return -1;

	return s_appSet->GetInt(key, value, defaultValue);
}

int _appSettingsSetInt(const char *key, int value)
{
	if (!s_appSet)
		return -1;

	return s_appSet->SetInt(key, value);
}

int _appSettingsReset()
{
	if (!s_appSet)
		return -1;

	return s_appSet->Initialize();
}

int _appSettingsWaitEnd()
{
	if (!s_isOpened)
		return -1;

	sceKernelWaitEventFlag(s_appSetThrdOpEvf, 2, SCE_KERNEL_EVF_WAITMODE_AND | SCE_KERNEL_EVF_WAITMODE_CLEAR_PAT, SCE_NULL, SCE_NULL);

	return SCE_OK;
}