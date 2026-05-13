/*
 * File: dentry_flags.h
 * File Created: 13 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 13 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

#define DENTRY_FLAG_NEGATIVE  (1 << 0)  // known not to exist
#define DENTRY_FLAG_MOUNTED   (1 << 1)  // something mounted here
#define DENTRY_FLAG_ROOT      (1 << 2)  // root of a mountpoint
