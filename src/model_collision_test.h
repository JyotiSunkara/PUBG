#pragma once

#include "base_collision_test.h"

namespace Claudette {

class CollisionModel3D;

/*! \brief Collision test between two meshes
 */
class CLAUDETTE_LIB_EXPORT ModelCollisionTest : public BaseCollisionTest
{
public:
    ModelCollisionTest();
    ~ModelCollisionTest();

    const CollisionModel3D* otherModel() const;
    void setOtherModel(const CollisionModel3D* other);

    /*! \brief Allows overriding the other model's transform, by supplying an alternative one.
     *
     *  This can be useful when testing a model against itself with different orientations.
     */
    const float* otherModelTransform() const;
    void setOtherModelTransform(const float trsf[16]);

    int accuracyDepth() const;
    void setAccuracyDepth(int depth);

    bool maxProcessingTimedOut() const;

    //! The triangle that collided (in other model)
    const float* otherModelTriangle() const;

    //! The index of the triangle that collided (belongs to other model)
    int otherTriangleId() const;

    void computePoint();

private:
    friend class CollisionModel3D;

    const CollisionModel3D* m_otherModel;
    const float* m_otherModelTransform;
    int m_accuracyDepth;

    float m_otherColTri[9];
    int m_iOtherColTri;

    //! Does the collision point need to be recomputed ?
    bool m_colPointIsDirty;

    bool m_maxProcessingTimedOut;
};

} // namespace Claudette
