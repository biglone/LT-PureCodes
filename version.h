#ifndef _VERSION_H_
#define _VERSION_H_

// verion information
#define VER_MAJOR    1
#define VER_MINOR    3
#define VER_REVISION 0
#define VER_BUILD    2310

#define TOSTRING(x) #x
#define VER_TOSTRING(x,y,z,b) TOSTRING(x)##"."##TOSTRING(y)##"."##TOSTRING(z)##"."##TOSTRING(b)

#define FILEVER(x,y,z,b) x,y,z,b
#define _STRPRODUCTVER(x) TOSTRING(x)
#define STRPRODUCTVER(x,y,z,b) _STRPRODUCTVER(FILEVER(x,y,z,b))

// file information
#define COMPANY_NAME             "Suzhou Grand Lynn Information Technologies Co., Ltd."
#define DEAMON_FILE_DESCRIPTION  "LingTalk Application"
#define INST_FILE_DESCRIPTION    "LingTalkInst Application"
#define COPYRIGHT                "Copyright (c) 2016 Suzhou Grand Lynn Information Technologies Co., Ltd.  All rights reserved."
#define DEAMON_PRODUCT_NAME      "LingTalk"
#define INST_PRODUCT_NAME        "LingTalkInst"

#endif //!_VERSION_H_
