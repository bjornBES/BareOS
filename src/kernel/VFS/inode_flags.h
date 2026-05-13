/*
 * File: inode_flags.h
 * File Created: 13 May 2026
 * Author: BjornBEs
 * -----
 * Last Modified: 13 May 2026
 * Modified By: BjornBEs
 * -----
 */

#pragma once

// Regular File
#define DT_REG 0x0001

// Directory
#define DT_DIR 0x0002

// Symbolic Link
#define DT_LNK 0x0004

// Block Special File
#define DT_BLK 0x0008

// Character Special File
#define DT_CHR 0x0010
