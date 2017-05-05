#include "frustum.h"
#include "matrix44.h"
#include "vector4.h"

GF_NAMESPACE_BEGIN

Frustum makeFrustum(const Matrix44& m)
{
    const auto col0 = m.column(0);
    const auto col1 = m.column(1);
    const auto col2 = m.column(2);
    const auto col3 = m.column(3);

    Vector4 vecPlanes[6];
    vecPlanes[0] = col3 + col0;
    vecPlanes[1] = col3 - col0;
    vecPlanes[2] = col3 + col1;
    vecPlanes[3] = col3 - col1;
    vecPlanes[4] = col2;
    vecPlanes[5] = col3 - col2;

    Frustum frustum;

    for (int i = 0; i < 6; ++i)
    {
        frustum.planes[i].n = vecPlanes[i].xyz();
        frustum.planes[i].d = vecPlanes[i].w;
        frustum.planes[i].normalize();
    }

    for (int i = 0; i<8; i++)
    {
        const auto& p0 = (i & 1) ? frustum.planes[0] : frustum.planes[1];
        const auto& p1 = (i & 2) ? frustum.planes[2] : frustum.planes[3];
        const auto& p2 = (i & 4) ? frustum.planes[4] : frustum.planes[5];
        intersects(p0, p1, p2, frustum.corners + i);
    }

    return frustum;
}

GF_NAMESPACE_END