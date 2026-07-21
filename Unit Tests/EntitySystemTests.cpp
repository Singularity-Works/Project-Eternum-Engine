/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: EntitySystemTests
* Description:
*     Tests that the ECS is actually connected. An Entity that enters the Scene must put
*     its Components into the matching ComponentSystem, and take them back out when it dies.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#include <gtest/gtest.h>
#include <Core/ECS/Entity/Entity.h>
#include <Core/ECS/Component/Component.h>
#include <Systems/ComponentSystem/ComponentSystem.h>
#include <Systems/Entity System/EntitySystem.h>

// A component that records the lifecycle calls it receives
class ProbeComponent final : public ComponentOf<ProbeComponent> {
public:
    int initCount = 0;
    int exitCount = 0;

    ProbeComponent() = default;

    // a clone starts its own lifecycle, it does not inherit the counts
    ProbeComponent(ProbeComponent const& other)
        : ComponentOf(other)
    {}

    void OnInit() override { ++initCount; }
    void OnExit() override { ++exitCount; }
};

// convenience, the number of ProbeComponents currently registered
static size_t ProbeCount()
{
    return Components<ProbeComponent>()->GetComponents().size();
}

class EntitySystemTest : public ::testing::Test {
protected:
    // the Scene is a singleton so each test starts and leaves it empty
    void SetUp() override { Entities()->ClearEntities(); }
    void TearDown() override { Entities()->ClearEntities(); }
};

// ----------------------------
// Registration
// ----------------------------

TEST_F(EntitySystemTest, ComponentsRegisterWhenTheEntityEntersTheScene) {
    auto* entity = new Entity();
    entity->SetName("Built");
    auto* probe = new ProbeComponent();
    entity->AddComponent(probe);

    // built but not in the Scene, so the system has not seen it
    EXPECT_EQ(ProbeCount(), 0u);
    EXPECT_EQ(probe->initCount, 0);

    Entities()->AddEntity(entity);

    EXPECT_EQ(ProbeCount(), 1u);
    EXPECT_EQ(probe->initCount, 1);
}

TEST_F(EntitySystemTest, ComponentAddedToALiveEntityRegistersImmediately) {
    Entity* entity = Entities()->CreateEntity("Live");
    EXPECT_EQ(ProbeCount(), 0u);

    auto* probe = new ProbeComponent();
    entity->AddComponent(probe);

    EXPECT_EQ(ProbeCount(), 1u);
    EXPECT_EQ(probe->initCount, 1);
}

TEST_F(EntitySystemTest, RemoveComponentTakesItOutOfTheSystem) {
    Entity* entity = Entities()->CreateEntity("Holder");
    auto* probe = new ProbeComponent();
    entity->AddComponent(probe);
    ASSERT_EQ(ProbeCount(), 1u);

    entity->RemoveComponent(probe);

    EXPECT_EQ(ProbeCount(), 0u);
    EXPECT_EQ(entity->getComponents().size(), 0u);
}

TEST_F(EntitySystemTest, ComponentSystemRegistersItselfWithTheEngine) {
    System* system = Components<ProbeComponent>();
    auto const& systems = Registry()->GetSystems();

    EXPECT_NE(std::find(systems.begin(), systems.end(), system), systems.end());
}

TEST_F(EntitySystemTest, SceneIsRegisteredWithTheEngine) {
    auto const& systems = Registry()->GetSystems();
    const bool found = std::any_of(systems.begin(), systems.end(),
        [](System const* s) { return s->GetName() == "Entity System"; });

    EXPECT_TRUE(found);
}

// ----------------------------
// Destruction
// ----------------------------

TEST_F(EntitySystemTest, DestructionIsDeferredToTheNextUpdate) {
    Entity* entity = Entities()->CreateEntity("Doomed");
    entity->AddComponent(new ProbeComponent());
    ASSERT_EQ(ProbeCount(), 1u);

    entity->Destroy();

    // flagged but still alive, nothing dies mid frame
    EXPECT_EQ(ProbeCount(), 1u);
    EXPECT_EQ(Entities()->GetEntityCount(), 1);

    Entities()->Update(0.016);

    EXPECT_EQ(ProbeCount(), 0u);
    EXPECT_EQ(Entities()->GetEntityCount(), 0);
}

TEST_F(EntitySystemTest, DestroyingAChildLeavesTheParentAlone) {
    Entity* parent = Entities()->CreateEntity("Parent");

    auto* child = new Entity();
    child->SetName("Child");
    child->AddComponent(new ProbeComponent());
    child->SetParent(parent);
    ASSERT_EQ(ProbeCount(), 1u);

    child->Destroy();
    Entities()->Update(0.016);

    EXPECT_EQ(ProbeCount(), 0u);
    EXPECT_EQ(Entities()->GetEntityCount(), 1);
    EXPECT_EQ(parent->GetChildren().size(), 0u);
    EXPECT_EQ(parent->GetNumDescendants(), 0);
}

TEST_F(EntitySystemTest, DestroyingAParentTakesTheWholeBranch) {
    Entity* parent = Entities()->CreateEntity("Parent");
    parent->AddComponent(new ProbeComponent());

    auto* child = new Entity();
    child->SetName("Child");
    child->AddComponent(new ProbeComponent());
    child->SetParent(parent);
    ASSERT_EQ(ProbeCount(), 2u);

    parent->Destroy();
    Entities()->Update(0.016);

    EXPECT_EQ(ProbeCount(), 0u);
    EXPECT_EQ(Entities()->GetEntityCount(), 0);
}

TEST_F(EntitySystemTest, ClearEntitiesEmptiesTheComponentSystems) {
    Entities()->CreateEntity("A")->AddComponent(new ProbeComponent());
    Entities()->CreateEntity("B")->AddComponent(new ProbeComponent());
    ASSERT_EQ(ProbeCount(), 2u);

    Entities()->ClearEntities();

    EXPECT_EQ(ProbeCount(), 0u);
    EXPECT_EQ(Entities()->GetEntityCount(), 0);
}

// ----------------------------
// Hierarchy
// ----------------------------

TEST_F(EntitySystemTest, ChildrenEnterTheSceneWithTheirParent) {
    auto* parent = new Entity();
    parent->SetName("Parent");

    auto* child = new Entity();
    child->SetName("Child");
    auto* probe = new ProbeComponent();
    child->AddComponent(probe);
    child->SetParent(parent);

    // the branch is built but nothing is in the Scene yet
    EXPECT_EQ(ProbeCount(), 0u);

    Entities()->AddEntity(parent);

    EXPECT_EQ(ProbeCount(), 1u);
    EXPECT_EQ(probe->initCount, 1);
    EXPECT_EQ(Entities()->GetEntityCount(), 2);
}

TEST_F(EntitySystemTest, JoiningALiveBranchEntersTheScene) {
    Entity* parent = Entities()->CreateEntity("Parent");

    auto* child = new Entity();
    child->SetName("Child");
    auto* probe = new ProbeComponent();
    child->AddComponent(probe);

    EXPECT_EQ(ProbeCount(), 0u);

    child->SetParent(parent);

    EXPECT_TRUE(child->IsInitialized());
    EXPECT_EQ(ProbeCount(), 1u);
    EXPECT_EQ(probe->initCount, 1);
}

TEST_F(EntitySystemTest, SetParentBuildsTheHierarchyBothWays) {
    Entity* root = Entities()->CreateEntity("Root");

    auto* child = new Entity();
    child->SetName("Child");
    child->SetParent(root);

    auto* grandchild = new Entity();
    grandchild->SetName("Grandchild");
    grandchild->SetParent(child);

    EXPECT_EQ(root->GetChildren().size(), 1u);
    EXPECT_EQ(child->GetChildren().size(), 1u);
    EXPECT_EQ(child->GetParent(), root);
    EXPECT_EQ(grandchild->GetParent(), child);

    // the count covers the whole branch, not just direct children
    EXPECT_EQ(root->GetNumDescendants(), 2);
    EXPECT_EQ(child->GetNumDescendants(), 1);
    EXPECT_EQ(Entities()->GetEntityCount(), 3);
}

TEST_F(EntitySystemTest, ReparentingMovesTheDescendantCounts) {
    Entity* first = Entities()->CreateEntity("First");
    Entity* second = Entities()->CreateEntity("Second");

    auto* child = new Entity();
    child->SetName("Child");
    child->SetParent(first);
    ASSERT_EQ(first->GetNumDescendants(), 1);

    child->SetParent(second);

    EXPECT_EQ(first->GetNumDescendants(), 0);
    EXPECT_EQ(first->GetChildren().size(), 0u);
    EXPECT_EQ(second->GetNumDescendants(), 1);
    EXPECT_EQ(child->GetParent(), second);
}

TEST_F(EntitySystemTest, CyclesAreRefused) {
    Entity* root = Entities()->CreateEntity("Root");

    auto* child = new Entity();
    child->SetName("Child");
    child->SetParent(root);

    // parenting the root under its own child would build a loop
    root->SetParent(child);

    EXPECT_EQ(root->GetParent(), nullptr);
    EXPECT_EQ(child->GetParent(), root);
}

TEST_F(EntitySystemTest, AnEntityCannotBeItsOwnParent) {
    Entity* entity = Entities()->CreateEntity("Lonely");

    entity->SetParent(entity);

    EXPECT_EQ(entity->GetParent(), nullptr);
}

// ----------------------------
// Scene Lookup
// ----------------------------

TEST_F(EntitySystemTest, FindEntitySearchesTheWholeTree) {
    Entity* root = Entities()->CreateEntity("Root");

    auto* buried = new Entity();
    buried->SetName("Buried");
    buried->SetParent(root);

    EXPECT_EQ(Entities()->FindEntity("Root"), root);
    EXPECT_EQ(Entities()->FindEntity("Buried"), buried);
    EXPECT_EQ(Entities()->FindEntity("Nothing"), nullptr);
}

TEST_F(EntitySystemTest, AddingTheSameEntityTwiceIsIgnored) {
    auto* entity = new Entity();
    entity->SetName("Once");

    Entities()->AddEntity(entity);
    Entities()->AddEntity(entity);

    EXPECT_EQ(Entities()->GetEntities().size(), 1u);
}

TEST_F(EntitySystemTest, AChildCannotBeAddedAsARoot) {
    Entity* parent = Entities()->CreateEntity("Parent");

    auto* child = new Entity();
    child->SetName("Child");
    child->SetParent(parent);

    // the parent already owns it, adding it again would double delete
    Entities()->AddEntity(child);

    EXPECT_EQ(Entities()->GetEntities().size(), 1u);
    EXPECT_EQ(Entities()->GetEntityCount(), 2);
}
