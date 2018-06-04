#ifndef _KII_FILE_CALLBACK
#define _KII_FILE_CALLBACK

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum kii_file_code_t {
    KII_FILE_OK,
    KII_FILE_FAIL
} kii_file_code_t;

typedef kii_file_code_t
    (*KII_FILE_OPEN_CB)(void);

typedef kii_file_code_t
    (*KII_FILE_WRITE_CB)(const char* buffer, const size_t length);

typedef kii_file_code_t
    (*KII_FILE_CLOSE_CB)(void);

#ifdef __cplusplus
}
#endif

#endif /* _KII_TASK_CALLBACK  */
/* vim:set ts=4 sts=4 sw=4 et fenc=UTF-8 ff=unix: */
