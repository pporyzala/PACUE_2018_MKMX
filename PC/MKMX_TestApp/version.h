#ifndef VERSION_H
#define VERSION_H

#define VER_MAJOR                   0
#define VER_MINOR                   0
#define VER_RELEASE                 0
#define VER_REVISION                1

#define STR(x)                                                   #x
#define FULL_VERSION_STR(MAJOR, MINOR, RELEASE, REVISION)        STR(MAJOR) "." STR(MINOR) "." STR(RELEASE) "." STR(REVISION) "\0"

#define VER_PRODUCTVERSION_BCD      (VER_MAJOR << 12)|(VER_MINOR << 8)|(VER_RELEASE << 4)|(VER_REVISION << 0)

#define VER_FILEVERSION             VER_MAJOR,VER_MINOR,VER_RELEASE,VER_REVISION
#define VER_FILEVERSION_STR         FULL_VERSION_STR(VER_MAJOR,VER_MINOR,VER_RELEASE,VER_REVISION)

#define VER_PRODUCTVERSION          VER_MAJOR,VER_MINOR,VER_RELEASE,VER_REVISION
#define VER_PRODUCTVERSION_STR      FULL_VERSION_STR(VER_MAJOR,VER_MINOR,VER_RELEASE,VER_REVISION)

#define VERSION_STR                 FULL_VERSION_STR(VER_MAJOR,VER_MINOR,VER_RELEASE,VER_REVISION)

#define VER_COMPANYNAME_STR         "Pawel Poryzala"
#define VER_FILEDESCRIPTION_STR     "Mikomax Test App"
#define VER_INTERNALNAME_STR        "Mikomax Test App"
#define VER_LEGALCOPYRIGHT_STR      "Copyright (C) 2018 Pawel Poryzala and/or its subsidiary(-ies)."
#define VER_LEGALTRADEMARKS1_STR    "All Rights Reserved"
#define VER_LEGALTRADEMARKS2_STR    VER_LEGALTRADEMARKS1_STR
#define VER_ORIGINALFILENAME_STR    "MKMX_TestApp.exe"
#define VER_PRODUCTNAME_STR         "Mikomax Test App"

#define VER_COMPANYDOMAIN_STR       "https://blank.org/"

#endif // VERSION_H
