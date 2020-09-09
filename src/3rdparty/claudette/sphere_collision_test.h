#pragma once

#include "base_collision_test.h"

namespace Claudette {

/*! \brief Collision test with a sphere
 */
class CLAUDETTE_LIB_EXPORT SphereCollisionTest : public BaseCollisionTest
{
public:
    SphereCollisionTest();

    const float* sphereCenter() const;
    void setSphereCenter(float x, float y, float z);

    float sphereRadius() const;
    void setSphereRadius(float radius);

private:
    float m_sphereCenter[3];
    float m_sphereRadius;
};

} // namespace Claudette
