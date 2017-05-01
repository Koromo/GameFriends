#ifndef GAMEFRIENDS_FRUSTUM_H
#define GAMEFRIENDS_FRUSTUM_H

#include "plane.h"
#include "vector3.h"
#include "prerequest.h"

GF_NAMESPACE_BEGIN

class Matrix44;

struct Frustum
{
    static const size_t LEFT   = 0 | 0 | 0;
    static const size_t RIGHT  = 1 | 0 | 0;
    static const size_t BOTTOM = 0 | 2 | 0;
    static const size_t TOP    = 1 | 2 | 0;
    static const size_t ZNEAR   = 0 | 0 | 4;
    static const size_t ZFAR    = 1 | 0 | 4;

    Plane planes[6];

    static const size_t RTF = 0 | 0 | 0;
    static const size_t LTF = 1 | 0 | 0;
    static const size_t RBF = 0 | 2 | 0;
    static const size_t LBF = 1 | 2 | 0;
    static const size_t RTN = 0 | 0 | 4;
    static const size_t LTN = 1 | 0 | 4;
    static const size_t RBN = 0 | 2 | 4;
    static const size_t LBN = 1 | 2 | 4;

    Vector3 corners[8];
};

Frustum makeFrustumFromMatrix(const Matrix44& m);

GF_NAMESPACE_END

#endif
