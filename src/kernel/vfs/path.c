/*
 * File: path.c
 * File Created: 17 Oct 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 17 Oct 2026
 * Modified By: BjornBEs
 * -----
 */

#include "vfs/path.h"

#include "stdio.h"

#include "memory.h"
#include "string.h"

#include <config.h>

#define MODULE "vfs-path"

status_t path_has_volume(char *path)
{
#if CONFIG_VOLUME_PREFIX != 0

    char *ret = strchr(path, CONFIG_VOLUME_PREFIX);
    if (!ret)
    {
        return KERRNO_UNSUCCESS;
    }
#endif
#if CONFIG_VOLUME_POSTFIX != 0
// TODO
#endif
    return KERRNO_SUCCESSES;
}

status_t path_is_rooted(char *path)
{
    return *path == CONFIG_PATH_DIR_SEPARATOR;
}

status_t path_combind_volume_id(char *volume, char *path)
{
    FUNC_NOT_IMPLEMENTED();
    return KERRNO_UNSUCCESS;
}

status_t path_combind(char *path, char *path2)
{
    FUNC_NOT_IMPLEMENTED();
    return KERRNO_UNSUCCESS;
}

status_t path_split_prefix(const char *path, char *volume_id_out, const char **rel_out)
{
    char *cpath = (char *)path;
    if (path_has_volume(cpath) != KERRNO_SUCCESSES)
    {
        return KERRNO_SUCCESSES;
    }

#if CONFIG_VOLUME_POSTFIX != 0
// TODO
#else
    const char *colon = strchr(cpath, CONFIG_VOLUME_PREFIX) + 1; // strchr uses cpath to find it
    colon = strchr(colon, CONFIG_PATH_DIR_SEPARATOR);
#endif
    cpath = (char *)path; // and so restore cpath from strchr
    if (colon == NULL)
    {
        if (rel_out != NULL)
        {
            *rel_out = NULL;
        }
        return KERRNO_UNSUCCESS;
    }

    if (path_is_rooted(cpath) == KERRNO_SUCCESSES)
    {
        cpath++;
    }

    if (rel_out != NULL)
    {
#if CONFIG_VOLUME_POSTFIX != 0
        *rel_out = colon + 1;
#else
        *rel_out = colon;
#endif
    }

    if (volume_id_out != NULL)
    {
        cpath++;
#if CONFIG_VOLUME_PREFIX != 0
        cpath++;
#endif
        size_t volume_length = (size_t)(colon - cpath);
        trace_debug(MODULE, "colon = %p (%s), cpath = %p (%s), colon - cpath = 0x%x", colon, colon, cpath, cpath, volume_length);
        if (volume_length > CONFIG_MAX_VOLUME_LENGTH)
        {
            KERRNO_RETURN(ENAMETOOLONG, "Path volume (%s) is too long.", path);
        }
        volume_id_out = memcpy(volume_id_out, cpath, volume_length);
        volume_id_out[volume_length] = '\0';
    }
    return KERRNO_SUCCESSES;
}

status_t path_next_segment(const char *path, char *seg_out, const char **rest_out)
{
    FUNC_NOT_IMPLEMENTED();
    return KERRNO_UNSUCCESS;
}
