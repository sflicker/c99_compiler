//
// Created by scott on 8/9/25.
//

#ifndef _INTERNAL_H
#define _INTERNAL_H

#ifdef UNIT_TEST
  #define INTERNAL
#else
  #define INTERNAL static
#endif

#endif //MIMIC99_STAGE13_INTERNAL_H
