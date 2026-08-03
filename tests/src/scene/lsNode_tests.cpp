// Tests for vortxGE/src/scene/lsNode.{h,cpp}  (Plan.md Phase 5: beyond math)
//
// Testable without a scene/render pass:
//   - ctor defaults, setters (public position/rotation/scale fields)
//   - active/deletion flags
//   - parent-child hierarchy (ctor auto-registration, unique_ptr ownership)
//   - RotateWorld: degrees in, updates local Euler rotation via
//     ToEulerZYX (degrees — pinned in Phase 2); parent-relative path
//     uses transpose() (the alias bug fixed in Phase 2)
//
// NOT tested here: Move/Optimize/Draw and ModelMatrix composition —
// those need lsScene/lsRender (documented scope limit).

#include "doctest/doctest.h"

#include "scene/lsNode.h"

using doctest::Approx;

// lsNode is abstract (pure virtual toString()) — minimal concrete node for tests.
class lsTestNode : public lsNode
{
public:
    lsTestNode( lsNode* parent ) : lsNode( parent ) {}
    const string& toString() override
    {
        static const string s = "lsTestNode";
        return s;
    }
};

TEST_CASE( "lsNode: constructor defaults" )
{
    lsTestNode root( nullptr );

    CHECK( root.getParent() == nullptr );
    CHECK( root.getChildren().empty() );
    CHECK( root.GetActive() );
    CHECK_FALSE( root.IsMarkedForDeletion() );

    CHECK( root.ModelMatrix.isIdentity() );
    CHECK( root.position.x == Approx( 0.0f ) );
    CHECK( root.position.y == Approx( 0.0f ) );
    CHECK( root.position.z == Approx( 0.0f ) );
    CHECK( root.scale.x == Approx( 1.0f ) );
    CHECK( root.scale.y == Approx( 1.0f ) );
    CHECK( root.scale.z == Approx( 1.0f ) );
}

TEST_CASE( "lsNode: setters store local TRS" )
{
    lsTestNode n( nullptr );

    n.Set_Pos( 1.0f, 2.0f, 3.0f );
    CHECK( n.position.x == Approx( 1.0f ) );
    CHECK( n.position.y == Approx( 2.0f ) );
    CHECK( n.position.z == Approx( 3.0f ) );

    n.Set_Rot( 10.0f, 20.0f, 30.0f );
    CHECK( n.rotation.x == Approx( 10.0f ) );
    CHECK( n.rotation.y == Approx( 20.0f ) );
    CHECK( n.rotation.z == Approx( 30.0f ) );

    n.Set_Scale( 2.0f, 3.0f, 4.0f );
    CHECK( n.scale.x == Approx( 2.0f ) );
    CHECK( n.scale.y == Approx( 3.0f ) );
    CHECK( n.scale.z == Approx( 4.0f ) );
}

TEST_CASE( "lsNode: active and deletion flags" )
{
    lsTestNode n( nullptr );

    n.SetActive( false );
    CHECK_FALSE( n.GetActive() );
    n.SetActive( true );
    CHECK( n.GetActive() );

    n.MarkForDeletion();
    CHECK( n.IsMarkedForDeletion() );
}

TEST_CASE( "lsNode: ctor sets parent pointer but does NOT auto-register" )
{
    // FINDING: the lsNode.h doc comment says "If parent is provided, this
    // node is automatically added to the parent's children list" — but
    // the ctor only stores the parent pointer. Registration requires an
    // explicit Add_Node(). Stale documentation.
    lsTestNode root( nullptr );

    lsNode* orphan = new lsTestNode( &root );
    CHECK( orphan->getParent() == &root );
    CHECK( root.getChildren().empty() );          // NOT auto-registered
    delete orphan;                                 // we own it (nobody else does)
}

TEST_CASE( "lsNode: Add_Node registers child and sets parent" )
{
    lsTestNode root( nullptr );

    root.Add_Node( new lsTestNode( nullptr ) );
    CHECK( root.getChildren().size() == 1 );
    CHECK( root.getChildren()[0]->getParent() == &root );

    root.Add_Node( new lsTestNode( nullptr ) );
    CHECK( root.getChildren().size() == 2 );

    SUBCASE( "grandchild hierarchy" )
    {
        lsNode* child1 = root.getChildren()[0].get();
        child1->Add_Node( new lsTestNode( nullptr ) );
        CHECK( child1->getChildren().size() == 1 );
        CHECK( child1->getChildren()[0]->getParent() == child1 );
        CHECK( root.getChildren().size() == 2 );   // root unchanged
    }
    // Add_Node takes unique_ptr ownership — cleaned up with root
}

TEST_CASE( "lsNode: RotateWorld with no parent updates local Euler (degrees)" )
{
    lsTestNode n( nullptr );                      // ModelMatrix = identity

    n.RotateWorld( 90.0f, 0.0f, 0.0f, 1.0f );  // 90° about Z
    CHECK( n.rotation.x == Approx( 0.0f ).epsilon( 1e-3 ) );
    CHECK( n.rotation.y == Approx( 0.0f ).epsilon( 1e-3 ) );
    CHECK( n.rotation.z == Approx( 90.0f ).epsilon( 1e-3 ) );
}

TEST_CASE( "lsNode: RotateWorld is relative to parent world rotation" )
{
    lsTestNode parent( nullptr );
    // Simulate a moved scene pass: parent world rotation = 90° about Z
    parent.ModelMatrix.RotateMatrix( 90.0f, 0.0f, 0.0f, 1.0f );

    lsNode* child = new lsTestNode( &parent );     // child ModelMatrix = identity

    // Rotating the child's world by the SAME 90° about Z should cancel
    // the parent's rotation → local rotation ≈ 0 (world−parent = local).
    // This path uses transpose() on the parent rotation — the exact
    // code fixed for the self-aliasing bug in Phase 2.
    child->RotateWorld( 90.0f, 0.0f, 0.0f, 1.0f );
    CHECK( child->rotation.x == Approx( 0.0f ).epsilon( 1e-2 ) );
    CHECK( child->rotation.y == Approx( 0.0f ).epsilon( 1e-2 ) );
    CHECK( child->rotation.z == Approx( 0.0f ).epsilon( 1e-2 ) );
}
