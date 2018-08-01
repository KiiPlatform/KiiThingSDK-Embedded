#ifndef _KII_FILE_IMPL
#define _KII_FILE_IMPL

#include "kii_file_callback.h"

#ifdef __cplusplus
extern "C" {
#endif


kii_file_code_t kii_file_open();

kii_file_code_t kii_file_write(const char* buffer, const size_t length);

kii_file_code_t kii_file_close();

#ifdef __cplusplus
}
#endif

#endif /* _KII_FILE_IMPL */
/* vim:set ts=4 sts=4 sw=4 et fenc=UTF-8 ff=unix: */
