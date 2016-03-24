#ifndef _kii_hidden_
#define _kii_hidden_

#ifdef __cplusplus
extern "C" {
#endif

/** Initializes Kii SDK
 *
 *  This method is for SDK developer. Application developers does not
 *  need to use this method.
 *
 *  \param [inout] kii core sdk instance.
 *  \param [in] site the input of site name,
 *  should be one of "CN", "CN3", "JP", "US", "SG"
 *  \param [in] app_id the input of Application ID
 *  \param [in] app_key the input of Application Key
 *  \param [in] version the input of SDK version.
 *  \return  KIIE_OK:success, KIIE_FAIL: failure
 */
kii_error_code_t kii_core_init_with_version(
        kii_core_t* kii,
        const char* site,
        const char* app_id,
        const char* app_key,
        const char* version);

#ifdef __cplusplus
}
#endif

#endif
