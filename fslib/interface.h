#include "fs.h"

int create_file(const char *filepath);
int delete_file(const char *filepath);
uint32_t read_file(const char *filepath, char *buffer, uint32_t buffer_size);
uint32_t write_file(const char *filepath, const char *buffer, uint32_t buffer_size);
inode_t get_file_metadata(const char *filepath);
bool file_exists(const char *filepath);