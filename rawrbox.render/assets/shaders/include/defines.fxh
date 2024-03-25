#ifndef INCLUDED_DEFINES
    #define INCLUDED_DEFINES

    #ifdef VULKAN
        #ifndef NonUniformResourceIndex
            #define NonUniformResourceIndex(x) x
        #endif
        //#ifndef DXCOMPILER
        //#    define NonUniformResourceIndex(x) x
        //#endif
    #endif
#endif
