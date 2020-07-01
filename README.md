# tar_writer
Simple library for reading and writing pre-ustar tar archives

This library provides functions capable of reading tar archives into memory, and writing tar archives to disk. These tar archives are POSIX compliant and capable of being understood by the builtin GNU tar utility. They are limited in what they are capable of storing or decoding: these functions are purpose built for a level save/load system for my voxel engine. They can only load and store files: no links or empty directories. Owner & Modify time are not supported.

Example usage:

```
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
```
