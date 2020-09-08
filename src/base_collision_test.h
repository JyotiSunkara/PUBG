#pragma once

#include "global.h"

namespace Claudette {

/*! \brief Base class for all collision tests
 *
 *  A collision test consists of some input data (specific to the test) and output data (see
 *  modelTriangle(), triangleId(), ...)
 */
class CLAUDETTE_LIB_EXPORT BaseCollisionTest
{
public:
    BaseCollisionTest();
    ~BaseCollisionTest();

    //! Collision result, returns true if a collision was detected
    bool collides() const;

    /*! \brief The triangle that collided (belongs to _this_ model)
     *
     *  The result is returned as a pointer being an array of 9 float values :
     *    \li x,y,z of 1st vertex : [0..2]
     *    \li          2nd vertex : [3..5]
     *    \li          3rd vertex : [6..8]
     */
    const float* modelTriangle() const;

    //! The index of the triangle that collided (belongs to _this_ model)
    int triangleId() const;

    //! The detected collision point (array of 3 items containing x,y,z coords)
    const float* point() const;

protected:
    float* mutablePoint();

private:
    friend class CollisionModel3D;
    bool m_collides;
    int m_iColTri;
    float m_colTri[9];
    float m_colPnt[3];
};

} // namespace Claudette
