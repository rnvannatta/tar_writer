#define _XOPEN_SOURCE 500
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "tar_writer.h"

tar_writer make_tar_writer(char const * path) {
  size_t path_length = strlen(path);
  char * path_copy = malloc(path_length + 1);
  strcpy(path_copy, path);
  char * tmpfile = malloc(path_length + sizeof(".XXXXXX"));
  strcpy(tmpfile, path);
  strcat(tmpfile + path_length, ".XXXXXX");
  mkstemp(tmpfile);
  FILE * f = fopen(tmpfile, "wb");
  return (tar_writer) {
    .path = path_copy,
    .tmpfile = tmpfile,
    .f = f,
  };
}

static char zeros[512];
void write_tar_file(tar_writer * writer, tar_file file) {
  raw_tar_file raw = { .checksum = { [0 ... 7] = ' ' } };
  strncpy(raw.name, file.name, sizeof(raw.name));
  if(raw.name[99] != '\0')
    exit(1);

  if(file.mode > 0777)
    exit(1);
  sprintf(raw.mode, "%07o", file.mode);

  snprintf(raw.length, sizeof(raw.length), "%011zo", file.length);
  if(raw.length[11] != '\0')
    exit(1);

  size_t checksum = 0;
  char const * raw_chars = (char const*) &raw;
  for(int i = 0; i < 512; i++)
    checksum += (unsigned char) raw_chars[i];
  sprintf(raw.checksum, "%06zo", checksum);

  FILE * f = writer->f;
  fwrite(&raw, sizeof(raw), 1, f);
  fwrite(file.data, 1, file.length, f);
  
  int rem = 512 - (file.length % 512);
  if(rem == 512)
    rem = 0;
  fwrite(zeros, 1, rem, f);
}

void close_tar_writer(tar_writer * writer) {
  fwrite(zeros, 1, sizeof(zeros), writer->f);
  fwrite(zeros, 1, sizeof(zeros), writer->f);
  fclose(writer->f);
  rename(writer->tmpfile, writer->path);
  free(writer->path);
  free(writer->tmpfile);
  
}

raw_tar_file * next_tar_file(raw_tar_file * raw_tar) {
  size_t length;
  sscanf(raw_tar->length, "%zo", &length);
  char * ret = raw_tar->data[(length+511)/512];
  bool end = !memcmp(zeros, ret, sizeof(zeros));
  if(end)
    return NULL;
  else
    return (raw_tar_file*) ret;
}

tar_file decode_tar_file(raw_tar_file * raw)  {
  size_t length;
  sscanf(raw->length, "%zo", &length);
  return (tar_file) {
    .name  = raw->name,
    .length = length,
    .data = raw->data[0]
  };
}

// TODO proper unit testing
#if 0
int main(int argc, char ** argv) {
  if(argc < 2)
    exit(1);
  int fd = open(argv[1], O_RDONLY);
  if(fd < 0)
    exit(1);
  struct stat st;
  if(fstat(fd, &st) < 0)
    exit(1);

  void * mem = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if(mem == MAP_FAILED)
    exit(1);
  raw_tar_file * read_file = mem;
  while(read_file) {
    printf("%s\n", read_file->name);
    read_file = next_tar_file(read_file);
  }

  tar_writer writer = make_tar_writer("foo.tar");
  char name[100] = { 0 };
  char data[1024] = { 0 };
  while(true) {
    printf("enter filename:");
    scanf("%99s", name);
    if(name[0] == 'q')
      break;
    printf("enter data:");
    int len;
    scanf("%1023s%n", data, &len);
    tar_file file = {
      .name = name,
      .mode = 0644,
      .length = len,
      .data = data
    };
    write_tar_file(&writer, file);
  }
  close_tar_writer(&writer);
}
#endif
