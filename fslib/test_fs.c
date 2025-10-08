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
    test_create_inode();

    // test deleting files
    test_delete_inode();

    // test reading files
    test_read_inode();

    // test writing and reading files
    test_write_and__read_inode();

    // test filepath parsing
    test_filepath();
    
    // Unload the filesystem
    printf("Unloading filesystem...\n");
    unload_fs();
    printf("Filesystem unloaded successfully\n");
    
    printf("\n=== All tests passed! ===\n");
    return 0;
}

void test_create_inode() {
    printf("=== Testing creating files ===\n\n");

    printf("Formatting disk...\n");
    assert(format_disk("disk", BLOCK_SIZE * 100, 1000) == 0);

    int temp = 0;

    inode_t* root = get_inode_by_index(0);
    assert(root != NULL);

    printf("Creating file 'test1.txt'\n");
    // inode_t** inodes = split_path_inodes("root/test1.txt", NULL, NULL);
    temp = create_file("root/test1.txt", false);
    assert(temp == 0);
    assert(_inode_exists(root, "test1.txt") == 0);
    printf("Existing files:\n");
    assert(print_all_files() == 2);
    
    printf("Creating file 'test2.txt'\n");
    _create_inode(root, "test2.txt", false);
    assert(_inode_exists(root, "test2.txt") == 0);
    printf("Existing files:\n");
    assert(print_all_files() == 3);
    
    printf("Creating 997 files\n");
    char temp_filename[MAX_FILENAME_LEN];
    for (int i = 3; i <= 999; i++) {
        sprintf(temp_filename, "test%d.txt", i);
        temp_filename[MAX_FILENAME_LEN-1] = '\0';
        _create_inode(root, temp_filename, false);
        assert(_inode_exists(root, temp_filename) == 0);
    }
    printf("Existing files (should be 1000, including root):\n");
    printf("Creating 1001th file (should fail)\n");
    assert(_create_inode(root, "test1001.txt", false) < 0);

    printf("Ensure root directory has 999 links...");
    inode_t* root_inode = (inode_t*)(inode_table);
    assert(root_inode->nlinks == 999);
    printf("passed\n");

    printf("Deleting all files...");
    for (int i = 1; i <= 999; i++) {
        sprintf(temp_filename, "test%d.txt", i);
        temp_filename[MAX_FILENAME_LEN-1] = '\0';
        assert(_inode_exists(root, temp_filename) == 0);
        _delete_inode(root, temp_filename);
        assert(_inode_exists(root, temp_filename) == -1);
    }
    printf("done\n");
    // print_all_files();
    // printf("%d\n", temp);
    // assert(temp == 1);


    printf("Testing directory structure\n");
    temp = _create_inode(root, "dirA", true);
    assert(temp == 0);
    temp = _create_inode(root, "dirB", true);
    assert(temp == 0);
    temp = _create_inode(root, "dirC", true);
    assert(temp == 0);
    temp = _create_inode(root, "dirD", true);
    assert(temp == 0);

    inode_t* dirA = get_subfile_by_name(root, "dirA", NULL);
    assert(dirA != NULL);
    assert(dirA->is_directory == true);
    assert(dirA->nlinks == 1);
    
    temp = _create_inode(dirA, "fileA", false);
    assert(temp == 0);

    inode_t** root_subfiles = get_files_in_dir(root);
    assert(root_subfiles != NULL);
    assert(strcmp(root_subfiles[0]->name, dirA->name) == 0);
    assert(strcmp(root_subfiles[1]->name, "dirB") == 0);
    assert(strcmp(root_subfiles[2]->name, "dirC") == 0);
    assert(strcmp(root_subfiles[3]->name, "dirD") == 0);
    assert(root_subfiles[4] == NULL);




    printf("Creating files test completed\n\n");
}

void test_delete_inode() {
    printf("=== Testing deleting files ===\n\n");

    printf("Formatting disk...\n");
    assert(format_disk("disk", BLOCK_SIZE * 100, 1000) == 0);

    inode_t* root = get_inode_by_index(0);
    int temp;

    printf("Creating 999 files\n");
    char temp_filename[MAX_FILENAME_LEN];
    for (int i = 1; i <= 999; i++) {
        sprintf(temp_filename, "test%d.txt", i);
        temp_filename[MAX_FILENAME_LEN-1] = '\0';
        _create_inode(root, temp_filename, false);
    }

    printf("Deleting files test114.txt to test999.txt\n");
    for (int i = 114; i <= 999; i++) {
        sprintf(temp_filename, "test%d.txt", i);
        temp_filename[MAX_FILENAME_LEN-1] = '\0';
        _delete_inode(root, temp_filename);
    }
    printf("Existing files (should end at test113.txt):\n");
    temp = print_all_files();
    assert(temp == 114);

    printf("Ensure root directory has 113 links:\n");
    inode_t* root_inode = (inode_t*)(inode_table);
    assert(root_inode->nlinks == 113);

    printf("Deleting files test1.txt to test113.txt\n");
    for (int i = 1; i <= 113; i++) {
        sprintf(temp_filename, "test%d.txt", i);
        temp_filename[MAX_FILENAME_LEN-1] = '\0';
        _delete_inode(root, temp_filename);
    }
    printf("Existing files (should be root):\n");
    assert(print_all_files() == 1);

    printf("Ensure root directory has 1 link:\n");
    assert(root_inode->nlinks == 0);

    printf("Deleting file test1.txt (should fail)\n");
    assert(_delete_inode(root, "test1.txt") < 0);

    printf("Deleting root directory (should fail)\n");
    assert(_delete_inode(root, "root") < 0);
    
    printf("Deleting files test completed\n\n");
}


void test_read_inode() {
    printf("=== Testing reading files ===\n\n");

    printf("Formatting disk...\n");
    assert(format_disk("disk", BLOCK_SIZE * 100, 1000) == 0);

    inode_t* root = get_inode_by_index(0);
    assert(root != NULL);

    printf("Creating and reading empty test file 'test.txt'\n");
    _create_inode(root, "test.txt", false);
    char buffer[BLOCK_SIZE];
    assert(_read_inode(root, "test.txt", buffer, BLOCK_SIZE) == 0); // empty file


    printf("Reading files test completed\n\n");
}


void test_write_and__read_inode() {
    printf("=== Testing writing and reading files ===\n\n");

    printf("Formatting disk...\n");
    assert(format_disk("disk", BLOCK_SIZE * 100, 1000) == 0);

    inode_t* root = get_inode_by_index(0);
    int temp;

    assert(root != NULL);

    printf("Creating and writing 'Hello, world!' to 'test.txt'\n");
    assert(_create_inode(root, "test.txt", false) == 0);
    temp = _write_inode(root, "test.txt", "Hello, world!", 13);
    assert(temp == 13);
    
    printf("Reading contents of'test.txt', should be 'Hello, world!':\n");
    char buffer[14];
    temp = _read_inode(root, "test.txt", buffer, 13);
    assert(temp == 13);
    printf("Contents of 'test.txt': %s\n", buffer);
    buffer[13] = '\0';
    assert(strcmp(buffer, "Hello, world!") == 0);

    printf("Running overwrite test... ");
    assert(_write_inode(root, "test.txt", "asdf", 4) == 4);
    assert(_read_inode(root, "test.txt", buffer, 4) == 4);
    buffer[4] = '\0';
    assert(strcmp(buffer, "asdf") == 0);
    printf("passed\n");

    printf("Multiple files writing test... ");
    assert(_create_inode(root, "test2.txt", false) == 0);
    assert(_write_inode(root, "test2.txt", "Hello, world!", 13) == 13);
    assert(_read_inode(root, "test2.txt", buffer, 13) == 13);
    buffer[13] = '\0';
    assert(strcmp(buffer, "Hello, world!") == 0);
    assert(_create_inode(root, "test3.txt", false) == 0);
    assert(_write_inode(root, "test3.txt", "asdf", 4) == 4);
    assert(_read_inode(root, "test3.txt", buffer, 4) == 4);
    buffer[4] = '\0';
    assert(strcmp(buffer, "asdf") == 0);
    printf("passed\n");

    printf("Writing test completed\n\n");
}

void test_filepath() {
    printf("=== Testing filepath ===\n\n");

    printf("Formatting disk...\n");
    assert(format_disk("disk", BLOCK_SIZE * 100, 1000) == 0);

    inode_t* root = get_inode_by_index(0);
    assert(root != NULL);

    printf("Creating folder 'root/A/B/C/D/'\n");
    bool is_dir;
    unsigned int path_len;
    char** parts = split_path("A/B/C/D/", &path_len, &is_dir);
    assert(parts != NULL);
    unsigned int i = 0;
    inode_t* curr_dir = root;
    while (parts[i] != NULL) {
        if (i == path_len - 1) {
            assert(_create_inode(curr_dir, parts[i], is_dir) == 0);
        } else {
            assert(_create_inode(curr_dir, parts[i], true) == 0);
        }
        assert(_inode_exists(curr_dir, parts[i]) == 0);
        curr_dir = get_subfile_by_name(curr_dir, parts[i], NULL);
        assert(curr_dir != NULL);
        i++;
    }
    free_split_path(parts);

    printf("Creating file 'root/A/B/C/D/test.txt'\n");

    // curr_dir should be folder D
    assert(_create_inode(curr_dir, "test.txt", false) == 0);
    assert(_inode_exists(curr_dir, "test.txt") == 0);

    // test should not be in root
    assert(_inode_exists(root, "test.txt") == -1);

    path_len = 0;
    is_dir = true;
    inode_t** inodes = split_path_inodes("root/A/B/C/D/test.txt", &path_len, &is_dir);
    assert(inodes != NULL);
    assert(path_len == 6);
    assert(is_dir == false);
    assert(strcmp(inodes[0]->name, "root") == 0);
    assert(strcmp(inodes[1]->name, "A") == 0);
    assert(strcmp(inodes[2]->name, "B") == 0);
    assert(strcmp(inodes[3]->name, "C") == 0);
    assert(strcmp(inodes[4]->name, "D") == 0);
    assert(strcmp(inodes[5]->name, "test.txt") == 0);
    assert(inodes[6] == NULL);
    free_split_path_inodes(inodes);

    printf("filepath test completed\n\n");

}