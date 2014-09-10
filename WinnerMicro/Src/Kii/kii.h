#ifndef KII_H
#define KII_H


extern int kii_init(char *site, char *appID, char *appKey);

extern int kiiDev_checkRegistered(void);
extern int kiiDev_getToken(char *deviceVendorID, char *password);
extern int kiiDev_register(char *vendorDeviceID, char *deviceType, unsigned char *password);

extern int kiiObj_create(char *bucketName, char *jsonObject, char *objectID);
extern int kiiObj_createWithID(char *bucketName, char *jsonObject, char *objectID);
extern int kiiObj_fullyUpdate(char *bucketName, char *jsonObject, char *objectID);
extern int kiiObj_partiallyUpdate(char *bucketName, char *jsonObject, char *objectID);

#endif

