#include "collision_model_3d.h"

#include "box.h"
#include "math3d.h"
#include "sysdep.h"
#include "mytritri.h"

#include "model_collision_test.h"
#include "ray_collision_test.h"
#include "sphere_collision_test.h"

#include <algorithm>
#include <cassert>
#include <vector>

namespace Claudette {

using namespace Internal;

namespace {

class Check
{
public:
    Check()
        : m_first(NULL),
          m_second(NULL),
          m_depth(0)
    { }

    Check(const BoxTreeNode* f, const BoxTreeNode* s, int d)
        : m_first(f),
          m_second(s),
          m_depth(d)
    { }

    const BoxTreeNode* m_first;
    const BoxTreeNode* m_second;
    int m_depth;
};

} // Anonymous namespace

class CollisionModel3D::Private
{
public:
    Private()
        : m_root(Vector3D::Zero, Vector3D::Zero, 0),
          m_transform(Matrix3D::Identity),
          m_invTransform(Matrix3D::Identity),
          m_isFinalized(false),
          m_isStatic(false)
    { }

    static const Matrix3D& toMatrix3D(const float m[16])
    {
        return *reinterpret_cast<const Matrix3D*>(m);
    }

    void addTriangle(const Vector3D& v1, const Vector3D& v2, const Vector3D& v3)
    {
        if (m_isFinalized)
            throw Inconsistency();
        m_triangles.push_back(BoxedTriangle(v1, v2, v3));
    }

    void setTransform(const Matrix3D& m)
    {
        m_transform = m;
        if (m_isStatic)
            m_invTransform = m_transform.Inverse();
    }

    int getTriangleIndex(const BoxedTriangle* bt) const
    {
        return int(bt - &(*m_triangles.begin()));
    }

    /** Stores all the actual triangles.  Other objects will use pointers into this array.
     */
    std::vector<BoxedTriangle> m_triangles;

    /** Root of the hierarchy tree */
    BoxTreeInnerNode m_root;

    /** The current transform and its inverse */
    Matrix3D m_transform;
    Matrix3D m_invTransform;

    /** Flag for indicating the model is finalized. */
    bool m_isFinalized;

    /** Static models will maintain the same transform for a while
        so the inverse transform is calculated each set instead
        of in the collision test. */
    bool m_isStatic;
};

CollisionModel3D::CollisionModel3D(ModelType type)
    : d(new Private)
{
    d->m_isFinalized = false;
    d->m_isStatic = type == CollisionModel3D::StaticModel;
}

CollisionModel3D::~CollisionModel3D()
{
    delete d;
}

void CollisionModel3D::setTriangleCount(std::size_t count)
{
    if (!d->m_isFinalized)
        d->m_triangles.reserve(count);
}

void CollisionModel3D::addTriangle(float x1, float y1, float z1,
                                   float x2, float y2, float z2,
                                   float x3, float y3, float z3)
{
    d->addTriangle(Vector3D(x1, y1, z1),
                   Vector3D(x2, y2, z2),
                   Vector3D(x3, y3, z3));
}

void CollisionModel3D::addTriangle(const float v1[3], const float v2[3], const float v3[3])
{
    d->addTriangle(Vector3D(v1[0], v1[1], v1[2]),
                   Vector3D(v2[0], v2[1], v2[2]),
                   Vector3D(v3[0], v3[1], v3[2]));
}

void CollisionModel3D::finalize()
{
    if (d->m_isFinalized)
        throw Inconsistency();
    // Prepare initial triangle list
    d->m_isFinalized = true;
    for (unsigned i = 0; i < d->m_triangles.size(); ++i) {
        BoxedTriangle& bt = d->m_triangles[i];
        d->m_root.m_Boxes.push_back(&bt);
    }

    int logdepth = 0;
    for (std::size_t num = d->m_triangles.size(); num > 0; num >>= 1)
        ++logdepth;
    d->m_root.m_logdepth = int(logdepth*1.5f);
    d->m_root.divide(0);
}

void CollisionModel3D::setTransform(const float m[16])
{
    d->setTransform(Private::toMatrix3D(m));
}

bool CollisionModel3D::modelCollision(ModelCollisionTest *test, int maxProcessingTime) const
{
    const CollisionModel3D* other = test->otherModel();
    test->m_collides = false;
    test->m_iColTri = -1;
    test->m_iOtherColTri = -1;
    test->m_colPointIsDirty = true;
    test->m_maxProcessingTimedOut = false;

    if (!d->m_isFinalized)
        throw Inconsistency();
    if (!other->d->m_isFinalized)
        throw Inconsistency();
    Matrix3D t = test->otherModelTransform() == NULL ? other->d->m_transform :
                                                       Private::toMatrix3D(test->otherModelTransform());
    if (d->m_isStatic)
        t *= d->m_invTransform;
    else
        t *= d->m_transform.Inverse();
    RotationState rs(t);

    int accuracyDepth = test->accuracyDepth();
    if (accuracyDepth < 0)
        accuracyDepth = 0xFFFFFF;
    if (maxProcessingTime == 0)
        maxProcessingTime = 0xFFFFFF;

    const DWORD beginTime = GetTickCount();
    DWORD endTime = 0;

    const std::size_t num = std::max(d->m_triangles.size(), other->d->m_triangles.size());
    std::size_t allocated = std::max(std::size_t(64), (num>>4));
    std::vector<Check> checks(allocated);

    int queue_idx = 1;

    { // Initialize first Check object
        checks.front().m_first = &d->m_root;
        checks.front().m_depth = 0;
        checks.front().m_second = &other->d->m_root;
    }

    while (queue_idx > 0) {
        if (queue_idx > (int)(allocated / 2)) { // Enlarge the queue
            Check c;
            checks.insert(checks.end(), allocated, c);
            allocated *= 2;
        }
        endTime = GetTickCount();
        if (endTime >= (beginTime + maxProcessingTime)) {
            test->m_maxProcessingTimedOut = true;
            return false;
        }

        // @@@ add depth check
        //Check c=checks.back();
        Check& c = checks[--queue_idx];
        const BoxTreeNode* first = c.m_first;
        const BoxTreeNode* second = c.m_second;
        assert(first != NULL);
        assert(second != NULL);
        if (first->intersect(*second, rs)) {
            std::size_t tnum1 = first->getTrianglesNumber();
            std::size_t tnum2 = second->getTrianglesNumber();
            if (tnum1 > 0 && tnum2 > 0) {
                for (unsigned int i = 0; i < tnum2; i++) {
                    const BoxedTriangle* bt2 = second->getTriangle(i);
                    const Triangle tt(Transform(bt2->v1, rs.t),
                                      Transform(bt2->v2, rs.t),
                                      Transform(bt2->v3, rs.t));
                    for (unsigned int j = 0; j < tnum1; j++) {
                        const BoxedTriangle* bt1 = first->getTriangle(j);
                        if (tt.intersect(*bt1)) {
                            bt1->copyCoords(test->m_colTri);
                            test->m_iColTri = d->getTriangleIndex(bt1);
                            tt.copyCoords(test->m_otherColTri);
                            test->m_iOtherColTri = other->d->getTriangleIndex(bt2);
                            test->m_collides = true;
                            return true;
                        }
                    }
                }
            }
            else if (first->getSonsNumber() == 0) {
                const BoxTreeNode* s1 = second->getSon(0);
                const BoxTreeNode* s2 = second->getSon(1);
                assert(s1 != NULL);
                assert(s2 != NULL);

                Check& c1 = checks[queue_idx++];
                c1.m_first = first;
                c1.m_second = s1;

                Check& c2 = checks[queue_idx++];
                c2.m_first = first;
                c2.m_second = s2;
            }
            else if (second->getSonsNumber() == 0) {
                const BoxTreeNode* f1 = first->getSon(0);
                const BoxTreeNode* f2 = first->getSon(1);
                assert(f1 != NULL);
                assert(f2 != NULL);

                Check& c1 = checks[queue_idx++];
                c1.m_first = f1;
                c1.m_second = second;

                Check& c2 = checks[queue_idx++];
                c2.m_first = f2;
                c2.m_second = second;
            }
            else {
                float v1 = first->getVolume();
                float v2 = second->getVolume();
                if (v1 > v2) {
                    const BoxTreeNode* f1=first->getSon(0);
                    const BoxTreeNode* f2=first->getSon(1);
                    assert(f1 != NULL);
                    assert(f2 != NULL);

                    Check& c1 = checks[queue_idx++];
                    c1.m_first = f1;
                    c1.m_second = second;

                    Check& c2 = checks[queue_idx++];
                    c2.m_first = f2;
                    c2.m_second = second;
                }
                else {
                    const BoxTreeNode* s1 = second->getSon(0);
                    const BoxTreeNode* s2 = second->getSon(1);
                    assert(s1 != NULL);
                    assert(s2 != NULL);

                    Check& c1 = checks[queue_idx++];
                    c1.m_first = first;
                    c1.m_second = s1;

                    Check& c2 = checks[queue_idx++];
                    c2.m_first = first;
                    c2.m_second = s2;
                }
            }
        }
    }
    return false;
}

bool CollisionModel3D::rayCollision(RayCollisionTest *test) const
{
    if (test == NULL)
        return false;

    float mintparm = 9e9f, tparm;
    Vector3D col_point;
    Vector3D O;
    Vector3D D;
    float segmin = test->raySegmentMin();
    float segmax = test->raySegmentMax();

    test->m_collides = false;
    test->m_iColTri = -1;

    if (d->m_isStatic) {
        O = Transform(Vector3D::asConstRef(test->rayOrigin()), d->m_invTransform);
        D = rotateVector(Vector3D::asConstRef(test->rayDirection()), d->m_invTransform);
    }
    else {
        const Matrix3D inv = d->m_transform.Inverse();
        O = Transform(Vector3D::asConstRef(test->rayOrigin()), inv);
        D = rotateVector(Vector3D::asConstRef(test->rayDirection()), inv);
    }

    if (!fuzzyIsNull(segmin)) { // Normalize ray
        O += segmin * D;
        segmax -= segmin;
        segmin = 0.0f;
    }
    if (segmax < segmin)
    {
        D = -D;
        segmax = -segmax;
    }
    std::vector<const BoxTreeNode*> checks;
    checks.push_back(&d->m_root);
    while (!checks.empty()) {
        const BoxTreeNode* b = checks.back();
        checks.pop_back();
        if (b->intersect(O, D, segmax)) {
            int sons = b->getSonsNumber();
            if (sons) {
                while (sons--)
                    checks.push_back(b->getSon(sons));
            }
            else {
                std::size_t tri = b->getTrianglesNumber();
                while (tri--) {
                    const BoxedTriangle* bt = b->getTriangle(tri);
                    const Triangle* t = static_cast<const Triangle*>(bt);
                    if (t->intersect(O, D, col_point, tparm, segmax))  {
                        if (test->raySearch() == RayCollisionTest::SearchClosestTriangle) {
                            if (tparm < mintparm) {
                                mintparm = tparm;
                                bt->copyCoords(test->m_colTri);
                                test->m_iColTri = d->getTriangleIndex(bt);
                                Vector3D::asRef(test->m_colPnt) = col_point;
                            }
                        }
                        else {
                            bt->copyCoords(test->m_colTri);
                            test->m_iColTri = d->getTriangleIndex(bt);
                            Vector3D::asRef(test->m_colPnt) = col_point;
                            test->m_collides = true;
                            return true;
                        }
                    }
                }
            }
        }
    }
    if (test->raySearch() == RayCollisionTest::SearchClosestTriangle && mintparm < 9e9f) {
        test->m_collides = true;
        return true;
    }
    return false;
}

bool CollisionModel3D::sphereCollision(SphereCollisionTest *test) const
{
    if (test == NULL)
        return false;

    test->m_collides = false;
    test->m_iColTri = -1;

    Vector3D O;
    if (d->m_isStatic) {
        O = Transform(Vector3D::asConstRef(test->sphereCenter()), d->m_invTransform);
    }
    else {
        const Matrix3D inv = d->m_transform.Inverse();
        O = Transform(Vector3D::asConstRef(test->sphereCenter()), inv);
    }
    std::vector<const BoxTreeNode*> checks;
    checks.push_back(&d->m_root);
    while (!checks.empty()) {
        const BoxTreeNode* b = checks.back();
        checks.pop_back();
        if (b->intersect(O, test->sphereRadius())) {
            int sons = b->getSonsNumber();
            if (sons) {
                while (sons--)
                    checks.push_back(b->getSon(sons));
            }
            else {
                std::size_t tri = b->getTrianglesNumber();
                while (tri--) {
                    const BoxedTriangle* bt = b->getTriangle(tri);
                    const Triangle* t = static_cast<const Triangle*>(bt);
                    if (t->intersect(O, test->sphereRadius(), Vector3D::asPointer(test->m_colPnt))) {
                        bt->copyCoords(test->m_colTri);
                        test->m_iColTri = d->getTriangleIndex(bt);
                        test->m_collides = true;
                        return true;
                    }
                }
            }
        }
    }
    return false;
}


bool SphereRayCollision(const float sphereCenter[3],
                        float sphereRadius,
                        const float rayOrigin[3],
                        const float rayDirection[3],
                        float point[3])
{
    const Vector3D& C=  *((const Vector3D*)sphereCenter);
    const Vector3D& O = *((const Vector3D*)rayOrigin);
    const Vector3D  D = ((const Vector3D*)rayDirection)->Normalized();
    Vector3D& P = *((Vector3D*)point);
    const Vector3D EO = C - O;
    const float v = EO*D;
    const float disc = sphereRadius*sphereRadius - (EO*EO - v*v);
    if (disc<0.0f)
        return false;
    const float d = std::sqrt(disc);
    P = O + (v - d) * D;
    return true;
}

bool SphereSphereCollision(const float c1[], float r1,
                           const float c2[], float r2,
                           float point[3])
{
    const Vector3D& C1 = *((const Vector3D*)c1);
    const Vector3D& C2 = *((const Vector3D*)c2);
    const float dist = (C2-C1).SquareMagnitude();
    const float sum = r1+r2;
    if (dist < sum*sum) {
        Vector3D& P =*((Vector3D*)point);

        P = C1 - C2;
        P.Normalized();
        P *= r1;
        P += C1;
        return true;
    }
    return false;
}

} // namespace Claudette
