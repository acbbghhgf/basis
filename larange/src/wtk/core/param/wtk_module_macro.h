#ifndef WTK_CORE_PARAM_WTK_MODULE_MACRO_H_
#define WTK_CORE_PARAM_WTK_MODULE_MACRO_H_
#ifdef __cplusplus
extern "C" {
#endif
#define InitModuleFuncName "init"
#define ReleaseModuleFuncName "release"
#define CreateEngineFuncName "create_engine"
#define DisposeEngineFuncName "dispose_engine"
#define CreateHandleFuncName "create_handle"
#define DisposeHandleFuncName "dispose_handle"
#define CalcFuncName "calc"
#define FindFuncName "find"
#define GetCalcArgName "get_arg"
#define PCM_HEADER_BYTES 44

#ifdef WIN32
#ifndef DLLEXPORT
#define DLLEXPORT __declspec( dllexport )
#endif
#else
#define DLLEXPORT __attribute__ ((visibility("default")))
#endif
#ifdef __cplusplus
#define DEF_FUNC_MAP() \
static wtk_func_map_t maps[]=  \
{ \
	{InitModuleFuncName,(Pointer)WTK_MODULE_FUNC(init)}, \
	{ReleaseModuleFuncName,(Pointer)WTK_MODULE_FUNC(release)}, \
	{CreateEngineFuncName,(Pointer)WTK_MODULE_FUNC(create_engine)}, \
	{DisposeEngineFuncName,(Pointer)WTK_MODULE_FUNC(dispose_engine)}, \
    {CreateHandleFuncName,(Pointer)WTK_MODULE_FUNC(create_handle)}, \
    {DisposeHandleFuncName,(Pointer)WTK_MODULE_FUNC(dispose_handle)}, \
    {CalcFuncName,(Pointer)WTK_MODULE_FUNC(calc)}, \
	{GetCalcArgName,(Pointer)WTK_MODULE_FUNC(get_type)} \
};  \
extern "C" DLLEXPORT Pointer  find(char* name) \
{ \
        Pointer func; \
        int i,count; \
        func=0; \
        if(name) \
        { \
                count=sizeof(maps)/sizeof(wtk_func_map_t);\
                for(i=0;i<count;++i) \
                { \
                        if(strcmp(maps[i].name,name)==0) \
                        { \
                                func=maps[i].func; \
                                break; \
                        } \
                } \
        }\
        return func;\
}
#else
#define DEF_FUNC_MAP() \
static wtk_func_map_t maps[]=  \
{ \
	{InitModuleFuncName,(Pointer)WTK_MODULE_FUNC(init)}, \
	{ReleaseModuleFuncName,(Pointer)WTK_MODULE_FUNC(release)}, \
	{CreateEngineFuncName,(Pointer)WTK_MODULE_FUNC(create_engine)}, \
	{DisposeEngineFuncName,(Pointer)WTK_MODULE_FUNC(dispose_engine)}, \
    {CreateHandleFuncName,(Pointer)WTK_MODULE_FUNC(create_handle)}, \
    {DisposeHandleFuncName,(Pointer)WTK_MODULE_FUNC(dispose_handle)}, \
    {CalcFuncName,(Pointer)WTK_MODULE_FUNC(calc)}, \
	{GetCalcArgName,(Pointer)WTK_MODULE_FUNC(get_type)} \
};  \
DLLEXPORT Pointer  find(char* name) \
{ \
        Pointer func; \
        int i,count; \
        func=0; \
        if(name) \
        { \
               count=sizeof(maps)/sizeof(wtk_func_map_t); \
                for(i=0;i<count;++i) \
                { \
                        if(strcmp(maps[i].name,name)==0) \
                        { \
                                func=maps[i].func; \
                                break; \
                        } \
                } \
        }\
        return func;\
}
#endif
#ifdef __cplusplus
};
#endif
#endif
