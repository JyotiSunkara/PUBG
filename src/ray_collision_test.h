#pragma once

#include "base_collision_test.h"

namespace Claudette {

/*! \brief Collision test with a ray
 */
class CLAUDETTE_LIB_EXPORT RayCollisionTest : public BaseCollisionTest
{
public:
    //! Search option for the colliding triangle
    enum Search
    {
        //! Search closest triangle on the ray (will slow the test considerably)
        SearchClosestTriangle,
        //! Search stop on the first triangle found that collides with the ray (may not be the closest)
        SearchFirstTriangle
    };

    RayCollisionTest();
    ~RayCollisionTest();

    const float* rayOrigin() const;
    void setRayOrigin(float x, float y, float z);

    const float* rayDirection() const;
    void setRayDirection(float x, float y, float z);

    float raySegmentMin() const;
    float raySegmentMax() const;
    /*! The default ray is a standard infinite ray. However, using segmin and segmax a line segment
     *  along the ray can be defined
     */
    void setRaySegmentBounds(float min = 0.f, float max = 3.4e+38f);

    Search raySearch() const;
    void setRaySearch(Search srh);

private:
    float m_rayOrigin[3];
    float m_rayDirection[3];
    float m_raySegMin;
    float m_raySegMax;
    RayCollisionTest::Search m_raySearch;
};

} // namespace Claudette
