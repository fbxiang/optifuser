#pragma once
#include <cstdio>
#define printMat(mat)                                                 \
  for (int i = 0; i < 4; i++) {                                       \
    printf("%.2f %.2f %.2f %.2f\n", mat[0][i], mat[1][i], mat[2][i],  \
           mat[3][i]);                                                \
    printf("\n");                                                     \
  }

