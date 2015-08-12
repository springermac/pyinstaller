/*
 * ****************************************************************************
 * Copyright (c) 2013, PyInstaller Development Team.
 * Distributed under the terms of the GNU General Public License with exception
 * for distributing bootloader.
 *
 * The full license is in the file COPYING.txt, distributed with this software.
 * ****************************************************************************
 */


/*
 * Bootloader for a packed executable.
 */


#define _CRT_SECURE_NO_WARNINGS 1


#ifdef _WIN32
    #include <windows.h>
    #include <wchar.h>
#else
    #include <limits.h>  // PATH_MAX
#endif
#include <stdio.h>  // FILE
#include <stdlib.h>  // calloc
#include <string.h>  // memset
#include <locale.h>  // setlocale

/* PyInstaller headers. */
#include "pyi_global.h" // PATH_MAX for win32
#include "pyi_path.h"
#include "pyi_archive.h"
#include "pyi_utils.h"
#include "pyi_pythonlib.h"
#include "pyi_launch.h"

#if defined(WIN32) && defined(WINDOWED)
int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,
						LPSTR lpCmdLine, int nCmdShow )
#else
int main(int argc, char* argv[])
#endif
{
    /*  archive_status contain status information of the main process. */
    ARCHIVE_STATUS *archive_status = NULL;
    char executable[PATH_MAX];
    char homepath[PATH_MAX];
    char archivefile[PATH_MAX];
    char MEIPASS2[PATH_MAX];
    int rc = 0;
    char *extractionpath = NULL;
#if defined(WIN32) && defined(WINDOWED)
    int argc = __argc;
    char **argv = __argv;
#endif
    int i = 0;


    VS("PyInstaller Bootloader 3.x\n");

    // TODO create special function to allocate memory for archive status pyi_arch_status_alloc_memory(archive_status);
    archive_status = (ARCHIVE_STATUS *) calloc(1,sizeof(ARCHIVE_STATUS));
    if (archive_status == NULL) {
        FATALERROR("Cannot allocate memory for ARCHIVE_STATUS\n");
        return -1;

    }

    /*
     * LC_CTYPE is "C" initially.
     * We must set LC_CTYPE to the current locale to make `mbstowcs` work correctly
     * We need `mbstowcs` working to convert Linux `char *` filenames to
     * wchar_t so we can pass them to Python 3 APIs.
     *
     * The old locale is saved in the global `saved_locale` and restored immediately
     * before running scripts to preserve this Python behavior:
     *
     * >>> import locale
     * >>> locale.setlocale(locale.LC_CTYPE, None)
     * 'C'
     *
     */
    saved_locale = strdup(setlocale(LC_CTYPE, NULL));
    VS("LOADER: LC_CTYPE is %s\n", saved_locale);
    VS("LOADER: Setting LC_CTYPE to \"\" (using current locale)\n");
    setlocale(LC_CTYPE, "");
    VS("LOADER: LC_CTYPE is now %s\n", setlocale(LC_CTYPE, NULL));

    pyi_path_executable(executable, argv[0]);
    pyi_path_archivefile(archivefile, executable);
    pyi_path_homepath(homepath, executable);

    extractionpath = pyi_getenv("_MEIPASS2");

    VS("LOADER: _MEIPASS2 is %s\n", (extractionpath ? extractionpath : "NULL"));

    if (pyi_arch_setup(archive_status, homepath, &executable[strlen(homepath)])) {
        if (pyi_arch_setup(archive_status, homepath, &archivefile[strlen(homepath)])) {
            FATALERROR("Cannot open self %s or archive %s\n",
                    executable, archivefile);
            return -1;
        }
    }

    /*
     * Cache command-line arguments and convert them to wchar_t.
     * Has to be called after archive_status initialization.
     */
    if (pyi_arch_cache_argv(archive_status, argc, argv)) {
        return -1;
    }

#ifdef _WIN32
    /* On Windows use single-process for --onedir mode. */
    if (!extractionpath && !pyi_launch_need_to_extract_binaries(archive_status)) {
        VS("LOADER: No need to extract files to run; setting extractionpath to homepath\n");
        extractionpath = homepath;
        strcpy(MEIPASS2, homepath);
        pyi_setenv("_MEIPASS2", MEIPASS2); //Bootstrap sets sys._MEIPASS, plugins rely on it
    }
#endif
    if (extractionpath) {
        VS("LOADER: Already in the child - running user's code.\n");
        /*  If binaries were extracted to temppath,
         *  we pass it through status variable
         */
        if (strcmp(homepath, extractionpath) != 0) {
            strcpy(archive_status->temppath, extractionpath);
            /*
             * Temp path exits - set appropriate flag and change
             * status->mainpath to point to temppath.
             */
            archive_status->has_temp_directory = true;
            strcpy(archive_status->mainpath, archive_status->temppath);
        }

        /* Main code to initialize Python and run user's code. */
        pyi_launch_initialize(executable, extractionpath);
        rc = pyi_launch_execute(archive_status);
        pyi_launch_finalize(archive_status);

    } else {

        /* status->temppath is created if necessary. */
        if (pyi_launch_extract_binaries(archive_status)) {
            VS("LOADER: temppath is %s\n", archive_status->temppath);
            VS("LOADER: Error extracting binaries\n");
            return -1;
        }

        /* Run the 'child' process, then clean up. */

        VS("LOADER: Executing self as child\n");
        pyi_setenv("_MEIPASS2", archive_status->temppath[0] != 0 ? archive_status->temppath : homepath);

        VS("LOADER: set _MEIPASS2 to %s\n", pyi_getenv("_MEIPASS2"));

        if (pyi_utils_set_environment(archive_status) == -1)
            return -1;

	/* Transform parent to background process on OSX only. */
	pyi_parent_to_background();

        /* Run user's code in a subprocess and pass command line arguments to it. */
        rc = pyi_utils_create_child(executable, argc, argv);

        VS("LOADER: Back to parent\n");

        VS("LOADER: Doing cleanup\n");
//        if (archive_status->has_temp_directory == true)
//            pyi_remove_temp_path(archive_status->temppath);
//        pyi_arch_status_free_memory(archive_status);
//        if (extractionpath != NULL)
//            free(extractionpath);
    }
    VS("LOADER: RC: %d\n", rc);

    return rc;
}
