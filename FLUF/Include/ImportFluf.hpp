#pragma once

#ifndef FLUF_API
    #ifdef FLUF
        #define FLUF_API __declspec(dllexport)
    #else
        #define FLUF_API __declspec(dllimport)
    #endif
#endif
