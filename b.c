#define PCB_LIB_PATH "lib"
#define PCB_IMPLEMENTATION
#include <PCB.h>

static bool alwaysBuild = false;
static uint8_t parallel = 0;

PCB_Enum(Target, uint8_t) {
    TARGET_CORE,
    TARGET_MAINUI,
    TARGET_RENDERER,

    TARGET_PCB,
    TARGET_RGFW,
    TARGET_CLAY,
    TARGET_MINIAUDIO,
    TARGET_STB_IMAGE_WRITE,
    TARGET_COUNT
};

static bool targets[TARGET_COUNT] = {
    [TARGET_CORE]               = true,
    [TARGET_MAINUI]             = true,
    [TARGET_RENDERER]           = true,

    [TARGET_PCB]                = true,
    [TARGET_RGFW]               = true,
    [TARGET_CLAY]               = true,
    [TARGET_MINIAUDIO]          = false,
    [TARGET_STB_IMAGE_WRITE]    = true,
};

static bool RENDERER_DYNAMIC = true;
static bool PCB_DYNAMIC = true;
static bool RGFW_DYNAMIC = false;
static bool STB_IMAGE_WRITE_DYNAMIC = false;
static bool MINIAUDIO_DYNAMIC = false;

static void common_flags(PCB_BuildContext *context) {
    PCB_BuildContext_flags(context).alwaysBuild = alwaysBuild;
    PCB_BuildContext_flags(context).parallel    = parallel;
}

static void common_options(PCB_BuildContext *context) {
#if 0
    PCB_CStrings_append(&context->otherCompilerFlags, "-ffunction-sections");
    PCB_CStrings_append(&context->otherCompilerFlags, "-fdata-sections");
    PCB_CStrings_append(&context->otherLinkerFlags, "-Wl,--gc-sections");

    PCB_CStrings_append(&context->otherCompilerFlags, "-flto");
    PCB_CStrings_append(&context->otherLinkerFlags,   "-flto");
#else
    (void)context;
#endif
}

static void common_hol_options(PCB_BuildContext *context, bool dynamic) {
    PCB_BuildContext_flags(context).alwaysBuild = alwaysBuild;
    PCB_BuildContext_flags(context).buildType = dynamic ? PCB_BUILDTYPE_DYNAMICLIB : PCB_BUILDTYPE_STATICLIB;
    context->buildPath = "build";
    PCB_CStrings_append_many(&context->otherCompilerFlags, "-x", "c");
}

static int build_PCB(PCB_BuildContext *context, bool dynamic) {
    if(!targets[TARGET_PCB]) return 0;
    PCB_BuildContext_init(
        context,
        PCB_BUILDOPTION_DEFAULT_COMPILER |
        PCB_BUILDOPTION_DEFAULT_WARNINGS
        // | PCB_BUILDOPTION_OPTIMIZE
    );
    common_options(context);
    common_hol_options(context, dynamic);
    PCB_CStrings_append(&context->sources, "lib/PCB.h");
    PCB_Vec_append(
        &context->preprocessorFlags.defines,
        (PCB_CLITERAL(PCB_CStringPair){"PCB_IMPLEMENTATION", NULL})
    );
    context->outputPath = "bin/PCB";

    int result = PCB_build_fromContext(context);
    PCB_BuildContext_reset(context);
    targets[TARGET_PCB] = false;
    return result;
}

static int build_RGFW(PCB_BuildContext *context, bool dynamic) {
    if(!targets[TARGET_RGFW]) return 0;
    PCB_BuildContext_init(
        context,
        PCB_BUILDOPTION_DEFAULT_COMPILER |
        PCB_BUILDOPTION_DEFAULT_WARNINGS |
        PCB_BUILDOPTION_OPTIMIZE
    );
    common_options(context);
    common_hol_options(context, dynamic);
    PCB_CStrings_append(&context->sources, "lib/RGFW.h");
    PCB_CStringPair macros[] = {
        { "RGFW_IMPLEMENTATION", NULL },
        { "RGFW_EXPORT", NULL },
        { "RGFW_OPENGL", NULL },
    };
    PCB_Vec_append_multiple(
        &context->preprocessorFlags.defines,
        macros,
        PCB_ARRAY_LEN(macros)
    );
    context->outputPath = "bin/RGFW";

    int result = PCB_build_fromContext(context);
    PCB_BuildContext_reset(context);
    targets[TARGET_RGFW] = false;
    return result;
}

static int build_stb_image_write(PCB_BuildContext *context, bool dynamic, bool pic) {
    if(!targets[TARGET_STB_IMAGE_WRITE]) return 0;
    PCB_BuildContext_init(
        context,
        PCB_BUILDOPTION_DEFAULT_COMPILER | //gives lots of warnings
        PCB_BUILDOPTION_DEFAULT_WARNINGS |
        PCB_BUILDOPTION_OPTIMIZE
    );
    PCB_CStrings_append(&context->warningFlags, "-Wno-sign-conversion");
    common_options(context);
    common_hol_options(context, dynamic);
    PCB_CStrings_append(&context->sources, "lib/stb_image_write.h");
    PCB_Vec_append(
        &context->preprocessorFlags.defines,
        (PCB_CLITERAL(PCB_CStringPair){"STB_IMAGE_WRITE_IMPLEMENTATION", NULL})
    );
    if(pic && !dynamic) PCB_CStrings_append(&context->otherCompilerFlags, "-fPIC");
    context->outputPath = "bin/stb_image_write";

    int result = PCB_build_fromContext(context);
    PCB_BuildContext_reset(context);
    targets[TARGET_STB_IMAGE_WRITE] = false;
    return result;
}

static int build_miniaudio(PCB_BuildContext *context, bool dynamic) {
    if(!targets[TARGET_MINIAUDIO]) return 0;
    PCB_BuildContext_init(
        context,
        PCB_BUILDOPTION_DEFAULT_COMPILER |
        PCB_BUILDOPTION_DEFAULT_WARNINGS | //gives lots of warnings
        PCB_BUILDOPTION_OPTIMIZE
    );
    PCB_CStrings_append_many(&context->warningFlags, "-Wno-sign-conversion");
    common_options(context);
    common_hol_options(context, dynamic);
    PCB_CStrings_append(&context->sources, "lib/miniaudio.h");
    PCB_Vec_append(
        &context->preprocessorFlags.defines,
        (PCB_CLITERAL(PCB_CStringPair){"MINIAUDIO_IMPLEMENTATION", NULL})
    );
    context->outputPath = "bin/miniaudio";

    int result = PCB_build_fromContext(context);
    PCB_BuildContext_reset(context);
    targets[TARGET_MINIAUDIO] = false;
    return result;
}

static int build_clay(PCB_BuildContext *context, bool pic) {
    if(!targets[TARGET_CLAY]) return 0;
    PCB_BuildContext_init(
        context,
        PCB_BUILDOPTION_DEFAULT_COMPILER |
        PCB_BUILDOPTION_OPTIMIZE
    );
    PCB_CStrings_append_many(&context->warningFlags, "-Wall", "-Wextra");
    PCB_BuildContext_flags(context).alwaysBuild = alwaysBuild;
    PCB_BuildContext_flags(context).buildType = PCB_BUILDTYPE_STATICLIB;
    context->buildPath = "build";
    PCB_CStrings_append(&context->sources, "lib/clay.h");
    PCB_Vec_append(
        &context->preprocessorFlags.defines,
        (PCB_CLITERAL(PCB_CStringPair){"CLAY_IMPLEMENTATION", NULL})
    );
    PCB_CStrings_append_many(&context->otherCompilerFlags, "-x", "c",);
    if(pic) PCB_CStrings_append(&context->otherCompilerFlags, "-fPIC");
    context->outputPath = "bin/clay";

    int result = PCB_build_fromContext(context);
    PCB_BuildContext_reset(context);
    targets[TARGET_CLAY] = false;
    return result;
}

static int build_Renderer(PCB_BuildContext *context, bool dynamic) {
    if(!targets[TARGET_RENDERER]) return 0;

    int result;
    if((result = build_PCB(context, PCB_DYNAMIC)) != 0) return result;
    PCB_BuildContext_init(
        context,
        PCB_BUILDOPTION_DEFAULT_WARNINGS | PCB_BUILDOPTION_DEFAULT_COMPILER
        // | PCB_BUILDOPTION_ASAN
        // | PCB_BUILDOPTION_OPTIMIZE
    );
    PCB_BuildContext_flags(context).alwaysBuild = alwaysBuild;
    PCB_BuildContext_flags(context).parallel    = parallel;
    PCB_CStrings_append_many(
        &context->includes,
        "include",
        "lib",
        "/usr/include/freetype2",
    );
    PCB_BuildContext_flags(context).buildType = dynamic ? PCB_BUILDTYPE_DYNAMICLIB : PCB_BUILDTYPE_STATICLIB;
    context->buildPath = "build/Renderer";
    PCB_CStrings_append(&context->sources, "src/Renderer");

    PCB_CStrings_append(&context->librarySearchPaths, "bin");
    if(PCB_DYNAMIC)
        PCB_CStrings_append(&context->libs, "PCB");
    else
        PCB_CStrings_append(&context->staticLibs, "PCB");

    if(STB_IMAGE_WRITE_DYNAMIC)
        PCB_CStrings_append(&context->libs, "stb_image_write");
    else
        PCB_CStrings_append(&context->staticLibs, "stb_image_write");

    if(PCB_DYNAMIC || STB_IMAGE_WRITE_DYNAMIC)
        PCB_CStrings_append(&context->otherLinkerFlags, "-Wl,-rpath,bin");

    PCB_CStrings_append(&context->libs, "freetype");
    context->outputPath = "bin/Renderer";

    result = PCB_build_fromContext(context);
    PCB_BuildContext_reset(context);
    targets[TARGET_RENDERER] = false;
    return result;
}

static int build_MainUI(PCB_BuildContext *context, bool plugin) {
    if(!targets[TARGET_MAINUI]) return 0;

    int result;
    if((result = build_PCB (context, PCB_DYNAMIC || plugin)) != 0) return result;
    if((result = build_Renderer(context, RENDERER_DYNAMIC || plugin)) != 0) return result;
    if((result = build_clay(context, plugin)) != 0) return result;
    PCB_BuildContext_init(
        context,
        PCB_BUILDOPTION_DEFAULT_WARNINGS | PCB_BUILDOPTION_DEFAULT_COMPILER
        | PCB_BUILDOPTION_ASAN
        // | PCB_BUILDOPTION_OPTIMIZE
    );
    PCB_BuildContext_flags(context).alwaysBuild = alwaysBuild;
    PCB_BuildContext_flags(context).parallel    = parallel;
    PCB_CStrings_append_many(
        &context->includes,
        "include",
        "lib",
        "/usr/include/freetype2",
    );
    PCB_BuildContext_flags(context).buildType = plugin ? PCB_BUILDTYPE_DYNAMICLIB : PCB_BUILDTYPE_STATICLIB;
    context->buildPath = "build/MainUI";
    PCB_CStrings_append(&context->sources, "src/MainUI");

    PCB_CStrings_append(&context->librarySearchPaths, "bin");
    PCB_CStrings_append(&context->staticLibs, "clay");
    if(PCB_DYNAMIC) {
        PCB_CStrings_append(&context->libs, "PCB");
    } else {
        PCB_CStrings_append(&context->staticLibs, "PCB");
    }
    if(RENDERER_DYNAMIC) {
        PCB_CStrings_append(&context->libs, "Renderer");
    } else {
        PCB_CStrings_append(&context->staticLibs, "Renderer");
    }
    if(PCB_DYNAMIC || RENDERER_DYNAMIC)
        PCB_CStrings_append(&context->otherLinkerFlags, "-Wl,-rpath,bin");
    context->outputPath = "bin/MainUI";

    result = PCB_build_fromContext(context);
    PCB_BuildContext_reset(context);
    targets[TARGET_MAINUI] = false;
    return result;
}

static int build_core(PCB_BuildContext *context) {
    if(!targets[TARGET_CORE]) return 0;

    int result;
    if((result = build_PCB (context, PCB_DYNAMIC)) != 0) return result;
    if((result = build_RGFW(context, RGFW_DYNAMIC)) != 0) return result;
    if((result = build_stb_image_write(context, STB_IMAGE_WRITE_DYNAMIC, true)) != 0) return result;
    if((result = build_miniaudio(context, MINIAUDIO_DYNAMIC)) != 0) return result;
    PCB_BuildContext_init(
        context,
        PCB_BUILDOPTION_DEFAULT_COMPILER | PCB_BUILDOPTION_DEFAULT_WARNINGS
        | PCB_BUILDOPTION_ASAN
        // | PCB_BUILDOPTION_UBSAN
        | PCB_BUILDOPTION_OPTIMIZE
    );
    context->buildPath = "build/Core";
    PCB_CStrings_append(&context->sources, "src/Core");
    common_options(context);
    common_flags(context);
    context->outputPath = "./t";
    PCB_CStrings_append_many(
        &context->includes,
        "include",
        "lib",
        "/usr/include/freetype2",
    );
    if(PCB_DYNAMIC) {
        PCB_CStrings_append(&context->libs, "PCB");
    } else {
        PCB_CStrings_append(&context->staticLibs, "PCB");
    }

    if(RGFW_DYNAMIC) {
        PCB_CStrings_append(&context->libs, "RGFW");
    } else {
        PCB_CStrings_append(&context->staticLibs, "RGFW");
    }

#if 0
    if(MINIAUDIO_DYNAMIC) {
        PCB_CStrings_append(&context->libs, "miniaudio");
    } else {
        PCB_CStrings_append(&context->staticLibs, "miniaudio");
    }
#endif

    if(RENDERER_DYNAMIC) {
        PCB_CStrings_append(&context->libs, "Renderer");
    } else {
        PCB_CStrings_append(&context->staticLibs, "Renderer");
    }

    PCB_CStrings_append_many(&context->libs,
        "m",
        "X11", "Xrandr",
        "GL", "GLX",
        "freetype"
    );
    PCB_CStrings_append(&context->librarySearchPaths, "bin");

    PCB_CStrings_append(&context->otherLinkerFlags, "-Wl,-rpath,bin");
    //TODO: this shit NEEDS to go, probably causes massive problems
    // PCB_CStrings_append(&context->otherLinkerFlags, "-rdynamic");

    result = PCB_build_fromContext(context);
    PCB_BuildContext_reset(context);
    targets[TARGET_CORE] = false;
    return result;
}

static bool parse_cmdline_args(int argc, char **argv) {
    bool any_specific_target = false;
    for(int i = 1; i < argc; i++) {
        PCB_StringView sv = PCB_StringView_from_cstr(argv[i]);
        if(!PCB_strncmp(sv.data, "-j", 2)) {
            if(sv.length == 2) { parallel = 1; continue; }
            long p = strtol(argv[i] + 2, NULL, 10);
            if(p > 255 || p < 0) {
                PCB_log(
                    PCB_LOGLEVEL_ERROR,
                    "Number of jobs given (%s) not in supported range of 0-255",
                    argv[i]
                ); return false;
            }
            //1 in PCB means "all cores", but it may be confusing for a command-line
            //argument. Therefore, and for Make compatibility, we treat is as
            //"no parallel".
            else if(p == 1) continue;
            parallel = (uint8_t)(unsigned long)p;
        }
        if(!PCB_strcmp(argv[i], "-f")) { alwaysBuild = true; continue; }
        if(!PCB_strncmp(sv.data, "--target=", 9)) {
            sv.data += 9; sv.length -= 9;
            if(!any_specific_target) PCB_memset(targets, 0, sizeof(targets));
            any_specific_target = true;
            if(!PCB_strncasecmp(sv.data, "Core", sv.length))
                targets[TARGET_CORE] = true;
            else if(!PCB_strncasecmp(sv.data, "MainUI", sv.length))
                targets[TARGET_MAINUI] = true;
            else if(!PCB_strncasecmp(sv.data, "Renderer", sv.length))
                targets[TARGET_RENDERER] = true;
            else if(!PCB_strncasecmp(sv.data, "PCB", sv.length))
                targets[TARGET_PCB] = true;
            else if(!PCB_strncasecmp(sv.data, "RGFW", sv.length))
                targets[TARGET_RGFW] = true;
            else if(!PCB_strncasecmp(sv.data, "clay", sv.length))
                targets[TARGET_CLAY] = true;
            else if(!PCB_strncasecmp(sv.data, "stb_image_write", sv.length))
                targets[TARGET_STB_IMAGE_WRITE] = true;
            else if(!PCB_strncasecmp(sv.data, "miniaudio", sv.length))
                targets[TARGET_MINIAUDIO] = true;
            else {
                PCB_log(
                    PCB_LOGLEVEL_ERROR,
                    "Unknown target '"PCB_SV_Fmt"'",
                    PCB_SV_Arg(sv)
                ); return false;
            }
            continue;
        }
    }
    return true;
}

int main(int argc, char **argv) {
    PCB_REBUILD_THIS_SHIT(argc, argv);
    parse_cmdline_args(argc, argv);

    PCB_BuildContext context = PCB_ZEROED;
    int result;

    if((result = build_PCB(&context, PCB_DYNAMIC)) != 0) goto defer;
    if((result = build_RGFW(&context, RGFW_DYNAMIC)) != 0) goto defer;
    if((result = build_stb_image_write(&context, STB_IMAGE_WRITE_DYNAMIC, true)) != 0) goto defer;
    if((result = build_miniaudio(&context, MINIAUDIO_DYNAMIC)) != 0) goto defer;
    if((result = build_clay(&context, true)) != 0) goto defer;
    if((result = build_Renderer(&context, RENDERER_DYNAMIC)) != 0) goto defer;
    if((result = build_MainUI(&context, true)) != 0) goto defer;
    if((result = build_core(&context)) != 0) goto defer;

defer:
    PCB_BuildContext_destroy(&context);
    return result;
}

