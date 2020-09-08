#include "ray_collision_test.h"

#include "math3d.h"

namespace Claudette {

RayCollisionTest::RayCollisionTest()
    : m_raySegMin(0),
      m_raySegMax(3.4e+38f),
      m_raySearch(RayCollisionTest::SearchFirstTriangle)
{
    Internal::Vector3D::fillFloatVec(m_rayOrigin, 0);
    Internal::Vector3D::setFloatVec(m_rayDirection, 0, 0, 1); // Z
}

RayCollisionTest::~RayCollisionTest()
{
}

const float *RayCollisionTest::rayOrigin() const
{
    return m_rayOrigin;
}

void RayCollisionTest::setRayOrigin(float x, float y, float z)
{
    Internal::Vector3D::setFloatVec(m_rayOrigin, x, y, z);
}

const float *RayCollisionTest::rayDirection() const
{
    return m_rayDirection;
}

void RayCollisionTest::setRayDirection(float x, float y, float z)
{
    Internal::Vector3D::setFloatVec(m_rayDirection, x, y, z);
}

float RayCollisionTest::raySegmentMin() const
{
    return m_raySegMin;
}

float RayCollisionTest::raySegmentMax() const
{
    return m_raySegMax;
}

void RayCollisionTest::setRaySegmentBounds(float min, float max)
{
    m_raySegMin = min;
    m_raySegMax = max;
}

RayCollisionTest::Search RayCollisionTest::raySearch() const
{
    return m_raySearch;
}

void RayCollisionTest::setRaySearch(RayCollisionTest::Search srh)
{
    m_raySearch = srh;
}

} // namespace Claudette
