#pragma once
#include <assert.h>
/** Serialized file stored inside a tarball, as directly read from disk. To use, simply mmap a tarball,
 *   and cast the pointer to a raw_tar_file. Decode with decode_tar_file() and go to the next file
 *   with next_tar_file().
 */
typedef struct {
  char name[100]; /// name of tarred up file
  char mode[8]; /// perimissions of tarred up file
  char ownerid[8]; /// id of owner of file - unimplemented
  char groupid[8]; /// id of group of owner of file - unimplemented
  char length[12]; /// length of file
  char modify_time[12]; /// last time file was modified - unimplemented
  char checksum[8]; /// checksum of tar header block
  char link[1]; /// whether file is link or regular file - unimplemented
  char link_path[100]; /// path of link - unimplemented
  char padding[255]; /// padding used in ustar extension - unimplemented
  char data[][512];
} raw_tar_file;
static_assert(sizeof(raw_tar_file) == 512, "raw_tar_file is not 512 bytes");

/** Returns next raw_tar_file after the current raw_tar_file. If the current raw_tar_file is
 *   the last one in the tarball, it returns NULL.
 */
raw_tar_file * next_tar_file(raw_tar_file * raw_tar);

/** Deserialized file stored inside a tar. See decode_tar_file() and write_tar_file(). Note that
 *   name and data are non-owning. Do not delete them if you did not malloc them.
 */
typedef struct {
  char * name; /// name of tarred up file. This is a non-owning pointer. Must be 99 chars or less.
  unsigned mode; // permissions of tarred up file. Must be 0777 or less.
  size_t length; // length of tarred up file's data. Must be 8gb or less.
  char * data; // data of tarred up file. This is a non-owning pointer.
} tar_file;

/** Deserializes a raw_tar_file into a usable tar_file.
 */
tar_file decode_tar_file(raw_tar_file * raw);

/** Manages safely writing a tar file to disk. The tar has a temporary name while being written. Methods are
 *   make_tar_writer(), write_tar_file(), and close_tar_writer()
 */
typedef struct {
  char * path; /// Path to final tar.
  char * tmpfile; /// Path to temporary file tar is being written to.
  FILE * f;
} tar_writer;

/** Creates a manager for writing a tarball. This manager writes to a temp file with
 *   the name `path`.XXXXXX where the X's are random digits. The temp file is renamed
 *   to the final path when the tar_writer is closed.
*
 *   \param path - Relative or absolute path with no environment variables. Assumed to exist and have all permissions needed.
 */
tar_writer make_tar_writer(const char * path);

/** Adds a tar file to the tarball being managed by writer. This tarball is written to a temp file.
 */
void write_tar_file(tar_writer * writer, tar_file file);

/** Finalizes and closes the tarball, then renames it to the final path.
 */
void close_tar_writer(tar_writer * writer);
