#pragma once

#define WPP_CONTROL_GUIDS \
    WPP_DEFINE_CONTROL_GUID(TraceGuid,(40D79F49, 83AB, 4E6E, A00C, 0AEA9BE8D6C5),  \
        WPP_DEFINE_BIT(MAIN)                                                       \
        WPP_DEFINE_BIT(CREATE_DISPATCH)                                            \
        WPP_DEFINE_BIT(SECTION_SYNC_DISPATCH)                                      \
        WPP_DEFINE_BIT(STREAM_CONTEXT)                                             \
        WPP_DEFINE_BIT(DRIVER) )

#define WPP_LEVEL_FLAGS_LOGGER(lvl,flags) \
           WPP_LEVEL_LOGGER(flags)

#define WPP_LEVEL_FLAGS_ENABLED(lvl, flags) \
           (WPP_LEVEL_ENABLED(flags) && WPP_CONTROL(WPP_BIT_ ## flags).Level >= lvl)

//begin_wpp config
//USEPREFIX (fm, "%!STDPREFIX! %!FILE! %!FUNC! %!LINE!");
//FUNC fm{LEVEL=TRACE_LEVEL_FATAL}(FLAGS, MSG, ...);
//end_wpp

//begin_wpp config
//USEPREFIX (em, "%!STDPREFIX! %!FILE! %!FUNC! %!LINE!");
//FUNC em{LEVEL=TRACE_LEVEL_ERROR}(FLAGS, MSG, ...);
//end_wpp

//begin_wpp config
//USEPREFIX (wm, "%!STDPREFIX! %!FILE! %!FUNC! %!LINE!");
//FUNC wm{LEVEL=TRACE_LEVEL_WARNING}(FLAGS, MSG, ...);
//end_wpp

//begin_wpp config
//USEPREFIX (im, "%!STDPREFIX! %!FILE! %!FUNC! %!LINE!");
//FUNC im{LEVEL=TRACE_LEVEL_INFORMATION}(FLAGS, MSG, ...);
//end_wpp

//begin_wpp config
//USEPREFIX (vm, "%!STDPREFIX! %!FILE! %!FUNC! %!LINE!");
//FUNC vm{LEVEL=TRACE_LEVEL_VERBOSE}(FLAGS, MSG, ...);
//end_wpp
