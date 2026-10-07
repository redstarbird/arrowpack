#ifndef EXPORT_H
#define EXPORT_H

#ifdef __EMSCRIPTEN__
#    include <emscripten.h>
#else
#    ifndef EMSCRIPTEN_KEEPALIVE
#        define EMSCRIPTEN_KEEPALIVE
#    endif
#endif


#endif  // EXPORT_H