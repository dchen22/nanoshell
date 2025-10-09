#include "interface.h"
#include "test_fs.h"
#include <assert.h>

int main() {
    // Format a disk
    printf("=== Testing VSFS load_fs function ===\n\n");
    
    printf("Formatting disk...\n");
    if (format_disk("disk", BLOCK_SIZE * 100, 1000) < 0) {
        printf("Failed to format disk\n");
        return -1;
    }
    printf("Disk formatted successfully\n\n");
    
    // Load the filesystem
    printf("Loading filesystem...\n");
    if (load_fs("disk") < 0) {
        printf("Failed to load filesystem\n");
        return -1;
    }
    printf("Filesystem loaded successfully\n\n");
    
    print_fs_status();

    // No files other than root should be printed here
    printf("Only root should be printed here:\n");
    print_all_files();
    printf("\n");

    // test creating files
    test_create_file();

    // test deleting files
    test_delete_file();

    // test reading files
    test_read_file();

    // test writing and reading files
    test_write_and_read_file();

    // test filepath parsing
    test_filepath();

    // test listing files
    test_list_files();
    
    // Unload the filesystem
    printf("Unloading filesystem...\n");
    unload_fs();
    printf("Filesystem unloaded successfully\n");
    
    printf("\n=== All tests passed! ===\n");
    return 0;
}

void test_create_file() {
    printf("=== Testing creating files ===\n\n");

    printf("Formatting disk...\n");
    assert(format_disk("disk", BLOCK_SIZE * 100, 1000) == 0);

    int temp = 0;


    printf("Creating file 'test1.txt'\n");
    temp = create_file("root/test1.txt");
    assert(temp == 0);
    assert(file_exists("root/test1.txt"));
    printf("Existing files:\n");
    assert(print_all_files() == 2);
    
    printf("Creating file 'test2.txt'\n");
    assert(create_file("root/test2.txt") == 0);
    assert(file_exists("root/test2.txt"));
    printf("Existing files:\n");
    assert(print_all_files() == 3);
    
    printf("Creating 997 files\n");
    char temp_filename[MAX_FILENAME_LEN];
    for (int i = 3; i <= 999; i++) {
        sprintf(temp_filename, "root/test%d.txt", i);
        temp_filename[MAX_FILENAME_LEN-1] = '\0';
        temp = create_file(temp_filename);
        assert(temp == 0);
        assert(file_exists(temp_filename));
    }
    printf("Existing files (should be 1000, including root):\n");
    printf("Creating 1001th file (shoufile_existsld fail)\n");
    assert(create_file("root/test1001.txt") < 0);

    printf("Ensure root directory has 999 links...");
    assert(get_file_metadata("root").nlinks == 999);
    printf("passed\n");

    printf("Deleting all files...");
    for (int i = 1; i <= 999; i++) {
        sprintf(temp_filename, "root/test%d.txt", i);
        temp_filename[MAX_FILENAME_LEN-1] = '\0';
        assert(file_exists(temp_filename));
        assert(delete_file(temp_filename) == 0);
        assert(!file_exists(temp_filename));
    }
    printf("passed\n");
    assert(get_file_metadata("root").nlinks == 0);

    printf("Testing directory structure\n");
    assert(create_file("root/dirA/") == 0);
    assert(create_file("root/dirB/") == 0);
    assert(create_file("root/dirC/") == 0);
    assert(create_file("root/dirD/") == 0);

    assert(create_file("root/dirA/fileA") == 0);

    assert(get_file_metadata("root/dirA/fileA").is_directory == false);
    assert(get_file_metadata("root/dirA/fileA").nlinks == 1);

    inode_t* root = get_inode_by_index(0);
    assert(root != NULL);

    assert(strcmp(get_files_in_dir(root, NULL)[0]->name, "dirA") == 0);
    assert(strcmp(get_files_in_dir(root, NULL)[1]->name, "dirB") == 0);
    assert(strcmp(get_files_in_dir(root, NULL)[2]->name, "dirC") == 0);
    assert(strcmp(get_files_in_dir(root, NULL)[3]->name, "dirD") == 0);
    assert(get_files_in_dir(root, NULL)[4] == NULL);

    inode_t* subfiles = get_subfiles("root");
    assert(subfiles != NULL);
    assert(strcmp(subfiles[0].name, "dirA") == 0);
    assert(strcmp(subfiles[1].name, "dirB") == 0);
    assert(strcmp(subfiles[2].name, "dirC") == 0);
    assert(strcmp(subfiles[3].name, "dirD") == 0);

    free(subfiles);

    printf("Creating files test completed\n\n");
}

void test_delete_file() {
    printf("=== Testing deleting files ===\n\n");

    printf("Formatting disk...\n");
    assert(format_disk("disk", BLOCK_SIZE * 100, 1000) == 0);

    int temp;

    printf("Creating 999 files\n");
    char temp_filename[MAX_FILENAME_LEN];
    for (int i = 1; i <= 999; i++) {
        sprintf(temp_filename, "root/test%d.txt", i);
        temp_filename[MAX_FILENAME_LEN-1] = '\0';
        create_file(temp_filename);
    }

    printf("Deleting files test114.txt to test999.txt\n");
    for (int i = 114; i <= 999; i++) {
        sprintf(temp_filename, "root/test%d.txt", i);
        temp_filename[MAX_FILENAME_LEN-1] = '\0';
        delete_file(temp_filename);
    }
    printf("Existing files (should end at test113.txt):\n");
    temp = print_all_files();
    assert(temp == 114);

    printf("Ensure root directory has 113 links:\n");
    inode_t* root_inode = (inode_t*)(inode_table);
    assert(root_inode->nlinks == 113);

    printf("Deleting files test1.txt to test113.txt\n");
    for (int i = 1; i <= 113; i++) {
        sprintf(temp_filename, "root/test%d.txt", i);
        temp_filename[MAX_FILENAME_LEN-1] = '\0';
        delete_file(temp_filename);
    }
    printf("Existing files (should be root):\n");
    assert(print_all_files() == 1);

    printf("Ensure root directory has 1 link:\n");
    assert(root_inode->nlinks == 0);

    printf("Deleting file test1.txt (should fail)\n");
    assert(delete_file("root/test1.txt") < 0);

    printf("Deleting root directory (should fail)\n");
    assert(delete_file("root") < 0);
    
    printf("Deleting files test completed\n\n");
}


void test_read_file() {
    printf("=== Testing reading files ===\n\n");

    printf("Formatting disk...\n");
    assert(format_disk("disk", BLOCK_SIZE * 100, 1000) == 0);

    inode_t* root = get_inode_by_index(0);
    assert(root != NULL);

    printf("Creating and reading empty test file 'test.txt'\n");
    create_file("root/test.txt");
    char buffer[BLOCK_SIZE];
    assert(read_file("root/test.txt", buffer, BLOCK_SIZE) == 0); // empty file


    printf("Reading files test completed\n\n");
}


void test_write_and_read_file() {
    printf("=== Testing writing and reading files ===\n\n");

    printf("Formatting disk...\n");
    assert(format_disk("disk", BLOCK_SIZE * 100, 1000) == 0);

    inode_t* root = get_inode_by_index(0);
    int temp;

    assert(root != NULL);

    printf("Creating and writing 'Hello, world!' to 'test.txt'\n");
    assert(create_file("root/test.txt") == 0);
    temp = write_file("root/test.txt", "Hello, world!", 13);
    assert(temp == 13);
    
    printf("Reading contents of'test.txt', should be 'Hello, world!':\n");
    char buffer[14];
    temp = read_file("root/test.txt", buffer, 13);
    assert(temp == 13);
    printf("Contents of 'test.txt': %s\n", buffer);
    buffer[13] = '\0';
    assert(strcmp(buffer, "Hello, world!") == 0);

    printf("Running overwrite test... ");
    assert(write_file("root/test.txt", "asdf", 4) == 4);
    assert(read_file("root/test.txt", buffer, 4) == 4);
    buffer[4] = '\0';
    assert(strcmp(buffer, "asdf") == 0);
    printf("passed\n");

    printf("Multiple files writing test... ");
    assert(create_file("root/test2.txt") == 0);
    assert(write_file("root/test2.txt", "Hello, world!", 13) == 13);
    assert(read_file("root/test2.txt", buffer, 13) == 13);
    buffer[13] = '\0';
    assert(strcmp(buffer, "Hello, world!") == 0);
    assert(create_file("root/test3.txt") == 0);
    assert(write_file("root/test3.txt", "asdf", 4) == 4);
    assert(read_file("root/test3.txt", buffer, 4) == 4);
    buffer[4] = '\0';
    assert(strcmp(buffer, "asdf") == 0);
    printf("passed\n");

    printf("Writing test completed\n\n");
}

void test_filepath() {
    printf("=== Testing filepath ===\n\n");

    printf("Formatting disk...\n");
    assert(format_disk("disk", BLOCK_SIZE * 100, 1000) == 0);

    printf("Creating folder 'root/A/'\n");
    int temp = 0;
    temp = create_file("root/A/");
    assert(temp == 0);
    assert(file_exists("root/A/"));
    printf("Creating folder 'root/A/B/'\n");
    temp = create_file("root/A/B/");
    assert(temp == 0);
    assert(file_exists("root/A/B/"));
    printf("Creating folder 'root/A/B/C/'\n");
    temp = create_file("root/A/B/C/");
    assert(temp == 0);
    assert(file_exists("root/A/B/C/"));
    printf("Creating folder 'root/A/B/C/D/'\n");
    temp = create_file("root/A/B/C/D/");
    assert(temp == 0);
    assert(file_exists("root/A/B/C/D/"));

    printf("Creating file 'root/A/B/C/D/test.txt'\n");

    // curr_dir should be folder D
    assert(create_file("root/A/B/C/D/test.txt") == 0);
    assert(file_exists("root/A/B/C/D/test.txt"));

    // test should not be in root
    assert(!file_exists("root/test.txt"));


    printf("filepath test completed\n\n");

}

void test_list_files() {
    printf("=== Testing listing files ===\n\n");

    printf("Formatting disk...\n");
    assert(format_disk("disk", BLOCK_SIZE * 100, 1000) == 0);

    int temp = 0;

    printf("Creating files...\n");
    assert(create_file("root/test1.txt") == 0);
    assert(create_file("root/A/") == 0);
    assert(create_file("root/B/") == 0);
    assert(create_file("root/B/C/") == 0);
    assert(create_file("root/B/C/D/") == 0);
    assert(create_file("root/B/C/test2.txt") == 0);

    printf("Listing files...\n");
    temp = list_files("root");
    assert(temp == 3);
    temp = list_files("root/A");
    assert(temp == 0);
    temp = list_files("root/B");
    assert(temp == 1);
    temp = list_files("root/B/C");
    assert(temp == 2);
    temp = list_files("root/B/C/D");
    assert(temp == 0);
    temp = list_files("root/B/C/D/test2.txt");
    assert(temp < 0);
    temp = list_files("root/test1.txt");
    assert(temp < 0);

    printf("list_files test completed\n\n");
}