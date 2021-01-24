#include <stdlib.h>
#include <stdio.h>
#include <string.h>

FILE *rom_open(const char *pathname) {
  return fopen(pathname, "rb");
}

int rom_close(FILE *stream) {
  return fclose(stream);
}

void get_rom_field(char *field, const unsigned char *object, int position, int size) {

  int index;

  memset(field, 0, size);
  for (index = 0; index < (size - 1); index++)
    field[index] = object[index + position];

}

void do_interleave(unsigned char *object, int size) {

  unsigned char buf[16384];
  unsigned char *src;
  int rom_size = size;

  int iblock, idx, nb_blocks, ptr;

  src = &object[0x200];
  rom_size -= 512;

  nb_blocks = rom_size / 16384;

  for(ptr = 0, iblock = 0; iblock < nb_blocks; iblock++, ptr += 16384) {
    memcpy(buf, &src[ptr], 16384);
    for(idx = 0; idx < 8192; idx++) {
      object[ptr + (idx << 1) + 1] = buf[idx];
      object[ptr + (idx << 1)] = buf[idx + 8192];
    }
  }

}

int main(int argc, char **argv) {

  unsigned char rom[6*1024*1024];

  char rom_filename[2048];
  FILE *rom_file;
  int rom_size = 0;

  char rom_system[17];
  char rom_copyright[17];
  char rom_name_domestic[49];
  char rom_name_overseas[49];
  char rom_serial_number[15];
  unsigned int rom_checksum;

  int index;

  memset(rom, 0, 6*1024*1024);
  strncpy(rom_filename, argv[1], 2048);

  if ((rom_file = rom_open(rom_filename)) == 0) {
    printf("Cannot open file %s\n", rom_filename);
    return -1;
  }

  fseek(rom_file, 0, SEEK_END);
  rom_size = ftell(rom_file);
  fseek(rom_file, 0, SEEK_SET);

  if ((rom_size) > ((6*1024*1024)+512)) {
    rom_close(rom_file);
    return -1;
  }

  fread(rom, rom_size, 1, rom_file);
  rom_close(rom_file);

  get_rom_field(rom_system, rom, 256, 17);

  if (strcmp(rom_system, "") == 0)
    do_interleave(rom, rom_size);

  get_rom_field(rom_system, rom, 256, 17);
  get_rom_field(rom_copyright, rom, 272, 17);
  get_rom_field(rom_name_domestic, rom, 288, 49);
  get_rom_field(rom_name_overseas, rom, 336, 49);
  get_rom_field(rom_serial_number, rom, 384, 15);
  rom_checksum = (rom[398] << 8) | rom[399];

  printf("%s;%d;%s;%s;%s;%s;%s;%d\n", rom_filename, rom_size, rom_name_domestic, rom_name_overseas, rom_system, rom_copyright, rom_serial_number, rom_checksum);

  return 0;
}
