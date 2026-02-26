#include "ast.h"
#include "codegen.h"
#include "ir.h"
#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "update.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ANEMO_VERSION "0.2.0"

static void print_ascii_art(void) {
    fputs(
        "                                                                               \n"
        "                                                                              \n"
        "                                                                              \n"
        "                                                                              \n"
        "                                                                              \n"
        "                                =========--::     -    ----                   \n"
        "                           +++++=    =---      ---  -----        :       --   \n"
        "                       +++++========                          ---: -  ----    \n"
        "                    ***+===+===     ------++++====---     :-                  \n"
        "                  ***++++++=    ====+*******++++=======----   ---             \n"
        "            --  ***+++***    ++++**#####* **+++++++=======-      ---          \n"
        "          --   ***++**** -  =++****** ******#######****++===---     -:        \n"
        "      -----   ****++*** =- ===+++++ ******##########*+-----==----     :       \n"
        "    = ====-   ********  = ---++===+++**************######+---:-- --           \n"
        "   =======-   +*******+ == --=+== +++++++****####*+++*#####*---- :   -        \n"
        "   = ======    +++**+**+ =- --=== =+++=        **###=++*##***---- -  --       \n"
        "  ====+++===     +++*++*++    - ==  ==        + ++#* ==+*#*** ------ --       \n"
        "   ++ ++++====     =++++++++=               ++ ==+** +++***++ ===--- =-       \n"
        "   ++++****++++==       ==============--      =+*** +++*#*+++ === ===== -     \n"
        "     ****###***++++++++==++=        +=====+++**** ****#**+++ ======+++=-:     \n"
        "       ****######*****++++++++++++++++++******  ****##**++ =++++++++++ -      \n"
        "         **########################*******  *****##****+  ++++++++++  =       \n"
        "      **+      ###%%%%%%%%%%%%%###*     ******##****+  ++++++++***+  =        \n"
        "       *****                     *********###****+ ******+*******  +=         \n"
        "              ***     =+++++*****+*****###***  ****#******####*  +=           \n"
        "                    =  =   +******#####**#######*****#######  +++             \n"
        "               ++++  = ****###########%%##*****####%%%##    -                 \n"
        "            **+    ***####%%%%%%%#******##########                            \n"
        "          ++    ***###%%%###******#########+=     **+     ++                  \n"
        "              *****###******###**********      +***+   ++++                   \n"
        "             *** ***************          +++++    ++                         \n"
        "            ** +**+******                                                     \n"
        "            +* *= +**+                                                        \n"
        "               =  ++                                                          \n"
        "                  ++                                                          \n"
        "                                                                              \n"
        "                                                                              \n"
        "                                                                              \n"
        "                                                                              \n"
        "                                                                              \n",
        stdout);
}

static void usage(void) {
    printf(
            "Available commands:\n"
            "anemo build <file.anm>\n"
            "anemo run <file.anm>\n"
            "anemo vortex\n"
            "anemo update\n"
            "anemo version\n");
}

static int write_file_all_text(const char *path, const char *text) {
    FILE *f = fopen(path, "wb");
    if (!f) {
        fprintf(stderr, "error: cannot open '%s' for writing\n", path);
        return 0;
    }
    size_t n = strlen(text);
    if (fwrite(text, 1, n, f) != n) {
        fclose(f);
        fprintf(stderr, "error: failed writing '%s'\n", path);
        return 0;
    }
    fclose(f);
    return 1;
}

static int load_file_all_text(const char *path, char **out_text) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        return 0;
    }
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return 0;
    }
    long size = ftell(f);
    if (size < 0) {
        fclose(f);
        return 0;
    }
    if (fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return 0;
    }
    char *buf = xmalloc((size_t)size + 1);
    size_t n = fread(buf, 1, (size_t)size, f);
    fclose(f);
    if (n != (size_t)size) {
        free(buf);
        return 0;
    }
    buf[n] = '\0';
    *out_text = buf;
    return 1;
}

static void vortex_help(void) {
    printf(
        "Vortex commands:\n"
        ":help                 Show this help\n"
        ":new                  Clear current buffer\n"
        ":edit                 Replace buffer (finish with single '.' line)\n"
        ":append               Append to buffer (finish with single '.' line)\n"
        ":show                 Print current buffer\n"
        ":load <file.anm>      Load file into buffer\n"
        ":save [file.anm]      Save buffer\n"
        ":build [file.anm]     Save and run 'anemo build'\n"
        ":run [file.anm]       Save and run 'anemo run'\n"
        ":quit                 Exit Vortex\n");
}

static char *trim_left(char *s) {
    while (*s == ' ' || *s == '\t') {
        s++;
    }
    return s;
}

static void strip_newline(char *s) {
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) {
        s[n - 1] = '\0';
        n--;
    }
}

static void read_multiline_into(char **buffer, size_t *len, int append) {
    char line[2048];
    if (!append) {
        (*buffer)[0] = '\0';
        *len = 0;
    }

    for (;;) {
        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }
        strip_newline(line);
        if (strcmp(line, ".") == 0) {
            break;
        }

        size_t add = strlen(line) + 1;
        *buffer = xrealloc(*buffer, *len + add + 1);
        memcpy(*buffer + *len, line, strlen(line));
        *len += strlen(line);
        (*buffer)[(*len)++] = '\n';
        (*buffer)[*len] = '\0';
    }
}

static int run_subcommand(const char *self_path, const char *verb, const char *file) {
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "\"%s\" %s \"%s\"", self_path, verb, file);
    return system(cmd);
}

static void run_vortex(const char *self_path) {
    char *buffer = xmalloc(1);
    size_t len = 0;
    int dirty = 0;
    int quit_armed = 0;
    char current_file[512];
    strcpy(current_file, "vortex.anm");
    buffer[0] = '\0';

    print_ascii_art();
    printf("Welcome to Vortex (Anemo IDLE)\n");
    vortex_help();

    for (;;) {
        char line[2048];
        printf("vortex> ");
        fflush(stdout);
        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }

        strip_newline(line);
        char *cmd = trim_left(line);
        if (*cmd == '\0') {
            continue;
        }
        if (*cmd != ':') {
            printf("Use Vortex commands starting with ':' (try :help)\n");
            continue;
        }

        if (strcmp(cmd, ":help") == 0) {
            vortex_help();
            quit_armed = 0;
            continue;
        }

        if (strcmp(cmd, ":new") == 0) {
            buffer[0] = '\0';
            len = 0;
            dirty = 1;
            quit_armed = 0;
            printf("Buffer cleared.\n");
            continue;
        }

        if (strcmp(cmd, ":edit") == 0) {
            printf("Enter code. End with '.' on a single line.\n");
            read_multiline_into(&buffer, &len, 0);
            dirty = 1;
            quit_armed = 0;
            continue;
        }

        if (strcmp(cmd, ":append") == 0) {
            printf("Append code. End with '.' on a single line.\n");
            read_multiline_into(&buffer, &len, 1);
            dirty = 1;
            quit_armed = 0;
            continue;
        }

        if (strcmp(cmd, ":show") == 0) {
            printf("----- %s -----\n%s----- end -----\n", current_file, buffer);
            quit_armed = 0;
            continue;
        }

        if (strncmp(cmd, ":load", 5) == 0) {
            char *arg = trim_left(cmd + 5);
            if (*arg == '\0') {
                printf("usage: :load <file.anm>\n");
                continue;
            }
            char *loaded = NULL;
            if (!load_file_all_text(arg, &loaded)) {
                printf("error: cannot load %s\n", arg);
                continue;
            }
            free(buffer);
            buffer = loaded;
            len = strlen(buffer);
            strncpy(current_file, arg, sizeof(current_file) - 1);
            current_file[sizeof(current_file) - 1] = '\0';
            dirty = 0;
            quit_armed = 0;
            printf("Loaded %s\n", current_file);
            continue;
        }

        if (strncmp(cmd, ":save", 5) == 0) {
            char *arg = trim_left(cmd + 5);
            const char *out = (*arg == '\0') ? current_file : arg;
            if (!has_extension(out, ".anm")) {
                printf("error: output file must end with .anm\n");
                continue;
            }
            if (!write_file_all_text(out, buffer)) {
                continue;
            }
            strncpy(current_file, out, sizeof(current_file) - 1);
            current_file[sizeof(current_file) - 1] = '\0';
            dirty = 0;
            quit_armed = 0;
            printf("Saved %s\n", current_file);
            continue;
        }

        if (strncmp(cmd, ":build", 6) == 0 || strncmp(cmd, ":run", 4) == 0) {
            const char *verb = (cmd[1] == 'b') ? "build" : "run";
            char *arg = trim_left(cmd + (cmd[1] == 'b' ? 6 : 4));
            const char *target = (*arg == '\0') ? current_file : arg;
            if (!has_extension(target, ".anm")) {
                printf("error: file must end with .anm\n");
                continue;
            }
            if (!write_file_all_text(target, buffer)) {
                continue;
            }
            strncpy(current_file, target, sizeof(current_file) - 1);
            current_file[sizeof(current_file) - 1] = '\0';
            dirty = 0;
            quit_armed = 0;
            int rc = run_subcommand(self_path, verb, current_file);
            if (rc != 0) {
                printf("command failed (exit code %d)\n", rc);
            }
            continue;
        }

        if (strcmp(cmd, ":quit") == 0) {
            if (dirty) {
                if (!quit_armed) {
                    printf("Unsaved changes in %s. Use :save or :quit again to exit.\n", current_file);
                    quit_armed = 1;
                    continue;
                }
                printf("Exiting without saving.\n");
                break;
            }
            break;
        }

        printf("unknown command: %s (try :help)\n", cmd);
    }

    free(buffer);
}

static void compile_source(const char *input_path, const char *binary_out) {
    if (!has_extension(input_path, ".anm")) {
        fatal("input file must use .anm extension");
    }

    size_t src_size = 0;
    char *src = read_file_all(input_path, &src_size);
    (void)src_size;

    TokenArray tokens;
    lex_source(input_path, src, &tokens);

    Program program;
    parse_program(input_path, &tokens, &program);

    SemanticResult sem;
    semantic_check_program(input_path, &program, &sem);
    if (!sem.ok) {
        fatal("semantic pass failed");
    }

    IRProgram ir;
    ir_generate_program(&program, &ir);

    char *stem = path_stem(input_path);

    char asm_path[512];
    char obj_path[512];
    snprintf(asm_path, sizeof(asm_path), "%s.s", stem);
    snprintf(obj_path, sizeof(obj_path), "%s.o", stem);

    codegen_emit_assembly(&ir, asm_path);

    char cmd_as[1024];
    snprintf(cmd_as, sizeof(cmd_as), "as -o \"%s\" \"%s\"", obj_path, asm_path);
    if (system(cmd_as) != 0) {
        fatal("assembler failed: %s", cmd_as);
    }

    char cmd_link[1024];
    snprintf(cmd_link, sizeof(cmd_link), "gcc -no-pie -o \"%s\" \"%s\"", binary_out, obj_path);
    if (system(cmd_link) != 0) {
        fatal("linker failed: %s", cmd_link);
    }

    free(stem);
    free_ir_program(&ir);
    free_program(&program);
    free_tokens(&tokens);
    free(src);
}

int main(int argc, char **argv) {
    anemo_auto_check_for_updates(ANEMO_VERSION);

    if (argc < 2) {
        print_ascii_art();
        usage();
        return 0;
    }

    if (strcmp(argv[1], "version") == 0) {
        printf("anemo %s\n", ANEMO_VERSION);
        return 0;
    }

    if (strcmp(argv[1], "vortex") == 0) {
        run_vortex(argv[0]);
        return 0;
    }

    if (strcmp(argv[1], "update") == 0) {
        return anemo_run_update(ANEMO_VERSION);
    }

    if ((strcmp(argv[1], "build") == 0 || strcmp(argv[1], "run") == 0) && argc == 3) {
        const char *src = argv[2];
        char *stem = path_stem(src);

        compile_source(src, stem);

        if (strcmp(argv[1], "build") == 0) {
            printf("built: %s\n", stem);
        } else {
            char cmd_run[1024];
#ifdef _WIN32
            snprintf(cmd_run, sizeof(cmd_run), ".\\%s", stem);
#else
            snprintf(cmd_run, sizeof(cmd_run), "./%s", stem);
#endif
            int rc = system(cmd_run);
            if (rc != 0) {
                free(stem);
                fatal("program exited with code %d", rc);
            }
        }

        free(stem);
        return 0;
    }

    usage();
    return 1;
}
