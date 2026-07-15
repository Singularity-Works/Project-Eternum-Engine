/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: ComponentTests
* Description:
*     Tests for the base Component class in the ECS architecture.
*
* Author:     Jax Clayton
* Created:    8/6/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#include <gtest/gtest.h>
#include <Core/ECS/Component/Component.h>
#include <Core/ECS/Entity/Entity.h>
#include <Systems/ComponentSystem/ComponentSystem.h>


class MockEntity : public Entity {

public:
    MockEntity() : Entity()
    {
        this->SetName("MockEntity");
    }

};

// Concrete Test Component
// ComponentOf supplies the type tag, the Clone and the System registration
class TestComponent final : public ComponentOf<TestComponent> {
};

// ----------------------------
// Component Tests
// ----------------------------

TEST(ComponentTests, UniqueIdGeneration) {
    const TestComponent c1;
    const TestComponent c2;
    EXPECT_NE(c1.GetId(), c2.GetId()) << "Component IDs should be unique";
}

TEST(ComponentTests, TypeMatchesExpected) {
    const TestComponent comp;
    EXPECT_EQ(comp.GetType(), typeid(TestComponent));
}

TEST(ComponentTests, CloneCreatesDistinctObject) {
    TestComponent original;
    Component* clone = original.Clone();

    ASSERT_NE(clone, nullptr);
    EXPECT_NE(clone, &original);
    EXPECT_EQ(clone->GetType(), original.GetType());

    delete clone;
}

TEST(ComponentTests, SetAndGetEntityPointer) {
    TestComponent comp;
    MockEntity mockEntity;
    comp.SetEntity(reinterpret_cast<Entity*>(&mockEntity));

    EXPECT_EQ(comp.GetEntity(), reinterpret_cast<Entity*>(&mockEntity));
}

TEST(ComponentTests, NameFormatting) {
    TestComponent comp;
    MockEntity mockEntity;
    comp.SetEntity(reinterpret_cast<Entity*>(&mockEntity));
    const std::string expectedPrefix = "MockEntity->";
    EXPECT_TRUE(StartsWith(comp.GetName(), expectedPrefix));
}
