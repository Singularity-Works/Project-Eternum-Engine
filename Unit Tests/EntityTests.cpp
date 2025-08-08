/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: EntityTests
* Description:
*       Tests for the Entity class in the ECS architecture.
*
* Author:     Jax Clayton
* Created:    8/8/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#include <gtest/gtest.h>
#include <Core/ECS/Entity/Entity.h>
#include <Core/ECS/Component/Component.h>

using namespace std;

// A mock component to test Init and Exit callbacks and cloning
class CountingComponent final : public Component {
public:
    int initCount = 0;
    int exitCount = 0;

    CountingComponent()
        : Component(typeid(CountingComponent))
    {}

    // Copy constructor used in Clone
    CountingComponent(CountingComponent const& other)
        : Component(other)
        , initCount(other.initCount)
        , exitCount(other.exitCount)
    {}

    void OnInit() override {
        ++initCount;
    }

    void OnExit() override {
        ++exitCount;
    }

    Component* Clone() const override {
        return new CountingComponent(*this);
    }
};

// Entity tests covering all public methods and edge cases

TEST(EntityTests, DefaultState) {
    Entity e;
    EXPECT_FALSE(e.IsDestroyed());
    EXPECT_EQ(e.GetName(), "");
    EXPECT_EQ(e.GetParent(), nullptr);
    EXPECT_EQ(e.GetChildren().size(), 0);
    EXPECT_EQ(e.getComponents().size(), 0);
    EXPECT_EQ(e.GetNumDescendants(), 0);
}

TEST(EntityTests, UniqueIdGeneration) {
    Entity e1;
    Entity e2;
    EXPECT_NE(e1.GetId(), e2.GetId());
}

TEST(EntityTests, SetNameAndGetName) {
    Entity e;
    e.SetName("TestEntity");
    EXPECT_EQ(e.GetName(), "TestEntity");
}

TEST(EntityTests, AddAndGetComponent) {
    Entity e;
    // before adding, no component
    EXPECT_EQ(e.GetComponent<Component>(), nullptr);

    // add a counting component
    auto* comp = new CountingComponent();
    e.AddComponent(comp);

    // map should contain the component
    auto& comps = e.getComponents();
    EXPECT_EQ(comps.size(), 1u);
    EXPECT_NE(comps.find(typeid(CountingComponent)), comps.end());

    // retrieval by base type
    Component* basePtr = e.GetComponent<Component>();
    EXPECT_EQ(basePtr, comp);

    // GetComponentsOfType should return a list containing it
    auto list = e.GetComponentsOfType<Component>();
    ASSERT_EQ(list.size(), 1u);
    EXPECT_EQ(list[0], comp);
}

TEST(EntityTests, AddDuplicateComponentIsIgnored) {
    Entity e;
    auto* comp1 = new CountingComponent();
    auto* comp2 = new CountingComponent();
    e.AddComponent(comp1);
    // attempt duplicate type
    e.AddComponent(comp2);
    // only first added
    EXPECT_EQ(e.getComponents().size(), 1u);
    EXPECT_EQ(e.GetComponent<Component>(), comp1);
    delete comp2;  // comp2 not owned by entity
}

TEST(EntityTests, InitAndExitInvokeComponentCallbacks) {
    Entity e;
    auto* comp = new CountingComponent();
    e.AddComponent(comp);

    // Init should call OnInit once
    e.Init();
    EXPECT_EQ(comp->initCount, 1);

    // Exit should call OnExit once
    e.Exit();
    EXPECT_EQ(comp->exitCount, 1);
}

TEST(EntityTests, DestroyFlagsEntity) {
    Entity e;
    EXPECT_FALSE(e.IsDestroyed());
    e.Destroy();
    EXPECT_TRUE(e.IsDestroyed());
}

TEST(EntityTests, ParentChildRelationshipAndDescendance) {
    Entity root;
    Entity child;
    Entity grandchild;

    // set up hierarchy: grandchild -> child -> root
    child.SetParent(&root);
    grandchild.SetParent(&child);

    EXPECT_EQ(child.GetParent(), &root);
    EXPECT_EQ(grandchild.GetParent(), &child);

    EXPECT_TRUE(grandchild.IsDescendedFrom(&root));
    EXPECT_TRUE(grandchild.IsDescendedFrom(&child));
    EXPECT_FALSE(child.IsDescendedFrom(&grandchild));
    EXPECT_FALSE(child.IsDescendedFrom(nullptr));
}

TEST(EntityTests, CopyAssignmentClonesComponentsAndName) {
    Entity original;
    original.SetName("Original");
    auto* comp = new CountingComponent();
    original.AddComponent(comp);

    // make a copy via assignment
    Entity copy;
    copy = original;

    // name and state
    EXPECT_EQ(copy.GetName(), "Original");
    EXPECT_FALSE(copy.IsDestroyed());

    // IDs differ
    EXPECT_NE(copy.GetId(), original.GetId());

    // component cloned
    auto* origComp = original.GetComponent<Component>();
    auto* copyComp = copy.GetComponent<Component>();
    EXPECT_NE(copyComp, nullptr);
    EXPECT_NE(copyComp, origComp);
    EXPECT_EQ(copyComp->GetType(), origComp->GetType());
    EXPECT_NE(copyComp->GetId(), origComp->GetId());
}

TEST(EntityTests, CloneProducesDistinctEntity) {
    Entity original;
    original.SetName("CloneTest");
    auto* comp = new CountingComponent();
    original.AddComponent(comp);

    Entity* cloned = original.Clone();

    // cloned has the same name but different ID
    EXPECT_EQ(cloned->GetName(), "CloneTest");
    EXPECT_NE(cloned->GetId(), original.GetId());

    // component cloned
    auto* clonedComp = cloned->GetComponent<Component>();
    EXPECT_NE(clonedComp, nullptr);
    EXPECT_NE(clonedComp, comp);
    EXPECT_EQ(clonedComp->GetType(), comp->GetType());
    EXPECT_NE(clonedComp->GetId(), comp->GetId());

    delete cloned;
}
