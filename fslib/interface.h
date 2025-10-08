#include "fs.h"

int create_file(const char *filepath, bool is_directory);
int delete_file(const char *filepath);
uint32_t read_file(const char *filepath, char *buffer, uint32_t buffer_size);
uint32_t write_file(const char *filepath, const char *buffer, uint32_t buffer_size);