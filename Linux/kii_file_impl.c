#include "kii_file_impl.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "example.h"

int fd = -1;

kii_file_code_t kii_file_open() {
    if ( access(EX_OBJECT_BODY_FILE_NAME, F_OK) != -1) {
        if (unlink(EX_OBJECT_BODY_FILE_NAME) != 0) {
            printf("faild to remove existing body file\n");
            return KII_FILE_FAIL;
        }
    }
    fd = open(EX_OBJECT_BODY_FILE_NAME, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        return KII_FILE_FAIL;
    }
    return KII_FILE_OK;
}

kii_file_code_t kii_file_write(const char* buffer, const size_t length) {
    if (fd == -1) {
        return KII_FILE_FAIL;
    }
    if (write(fd, buffer, length) == -1) {
        return KII_FILE_FAIL;
    }
    return KII_FILE_OK;
}

kii_file_code_t kii_file_close() {
    if (close(fd) == -1) {
        return KII_FILE_FAIL;
    }
    return KII_FILE_OK;
}