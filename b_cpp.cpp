#define PCB_LIB_PATH "lib"
#define PCB_IMPLEMENTATION
#include <PCB.h>

static bool alwaysBuild = false;
static uint8_t parallel = 0;
static uint32_t default_board_size = 10;

PCB_Enum(Target, uint8_t) {
    TARGET_SERVER,
    TARGET_COUNT
};

static bool targets[TARGET_COUNT] = {
    true
};

static void common_flags(PCB_BuildContext *context) {
    PCB_BuildContext_flags(context).alwaysBuild = alwaysBuild;
    PCB_BuildContext_flags(context).parallel    = parallel;
}

static int build_server(PCB_BuildContext *context) {
    if(!targets[TARGET_SERVER]) return 0;

    int result;
    PCB_BuildContext_init(
        context,
        PCB_BUILDOPTION_DEFAULT_COMPILER
        | PCB_BUILDOPTION_DEFAULT_WARNINGS
        // | PCB_BUILDOPTION_ASAN
        // | PCB_BUILDOPTION_UBSAN
        // | PCB_BUILDOPTION_OPTIMIZE
    );
    context->buildPath = "build/Server";
    PCB_CStrings_append(&context->sources, "src/Server");
    PCB_CStrings_append(&context->warningFlags, "-Wno-sign-conversion");
    PCB_Vec_append(
        &context->preprocessorFlags.defines,
        (PCB_CLITERAL(PCB_CStringPair){
            "DEFAULT_BOARD_SIZE",
            PCB_Arena_asprintf(context->arena, "%u", default_board_size)
        })
    );

    common_flags(context);
    context->outputPath = "./server";

    result = PCB_build_fromContext(context);
    PCB_BuildContext_reset(context);
    targets[TARGET_SERVER] = false;
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
            if(!PCB_strncasecmp(sv.data, "Server", sv.length))
                targets[TARGET_SERVER] = true;
            else {
                PCB_log(
                    PCB_LOGLEVEL_ERROR,
                    "Unknown target '" PCB_SV_Fmt "'",
                    PCB_SV_Arg(sv)
                ); return false;
            }
            continue;
        }
        if(!PCB_strncmp(sv.data, "--default-board-size=", 21)) {
            sv.data += 21; sv.length -= 21;
            long v = strtol(sv.data, NULL, 10);
            if(v <= 0) {
                PCB_log(PCB_LOGLEVEL_ERROR, "Invalid default board size (%ld)", v);
                return 1;
            }
            default_board_size = (uint32_t)(unsigned long)v;
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

    if((result = build_server(&context)) != 0) goto defer;

defer:
    PCB_BuildContext_destroy(&context);
    return result;
}

