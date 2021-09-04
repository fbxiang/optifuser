#pragma once
#include <cstdio>

#define printMat(mat)                                                 \
  for (int i = 0; i < 4; i++) {                                       \
    printf("%.5f %.5f %.5f %.5f\n", mat[0][i], mat[1][i], mat[2][i],  \
           mat[3][i]);                                                \
  }                                                                   \
  printf("\n");

/*
 #define LABEL_TEXTURE(id, label)                                               \
   {                                                                            \
     std::string _l = label;                                                    \
     glObjectLabel(GL_TEXTURE, id, _l.length(), _l.c_str());                    \
   }

 #define LABEL_FRAMEBUFFER(id, label)                                           \
   {                                                                            \
     std::string _l = label;                                                    \
     glObjectLabel(GL_FRAMEBUFFER, id, _l.length(), _l.c_str());                \
   }
*/

#define LABEL_TEXTURE(id, label) {}
#define LABEL_FRAMEBUFFER(id, label) {}
