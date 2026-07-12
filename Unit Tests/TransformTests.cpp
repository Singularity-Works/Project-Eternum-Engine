/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: TransformTests
* Description:
*     Tests for the Transform Component.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#include <gtest/gtest.h>
#include <Core/ECS/Component/Transform/Transform.h>
#include <Core/ECS/Entity/Entity.h>

// ----------------------------
// Defaults
// ----------------------------

TEST(TransformTests, DefaultsToNoMovementAndFullSize) {
    const Transform t;

    EXPECT_TRUE(t.GetTranslation().AlmostEqual(Vec2f::Zero()));
    EXPECT_FLOAT_EQ(t.GetRotation(), 0.0f);

    // scale defaults to one, a zero scale would make everything vanish
    EXPECT_TRUE(t.GetScale().AlmostEqual(Vec2f(1.0f)));
}

TEST(TransformTests, ValueConstructorSetsEverything) {
    const Transform t(Vec2f{ 3.0f, 4.0f }, 90.0f, Vec2f{ 2.0f, 2.0f });

    EXPECT_TRUE(t.GetTranslation().AlmostEqual(Vec2f{ 3.0f, 4.0f }));
    EXPECT_FLOAT_EQ(t.GetRotation(), 90.0f);
    EXPECT_TRUE(t.GetScale().AlmostEqual(Vec2f{ 2.0f, 2.0f }));
}

TEST(TransformTests, TypeIsTransform) {
    const Transform t;
    EXPECT_EQ(t.GetType(), typeid(Transform));
}

// ----------------------------
// Movement
// ----------------------------

TEST(TransformTests, TranslateAccumulates) {
    Transform t;

    t.Translate(Vec2f{ 1.0f, 2.0f });
    t.Translate(Vec2f{ 3.0f, -1.0f });

    EXPECT_TRUE(t.GetTranslation().AlmostEqual(Vec2f{ 4.0f, 1.0f }));
}

TEST(TransformTests, RotationWrapsIntoZeroToThreeSixty) {
    Transform t;

    t.SetRotation(450.0f);
    EXPECT_FLOAT_EQ(t.GetRotation(), 90.0f);

    t.SetRotation(-90.0f);
    EXPECT_FLOAT_EQ(t.GetRotation(), 270.0f);

    t.SetRotation(360.0f);
    EXPECT_FLOAT_EQ(t.GetRotation(), 0.0f);
}

TEST(TransformTests, RotateIsRelative) {
    Transform t;

    t.SetRotation(350.0f);
    t.Rotate(20.0f);

    EXPECT_FLOAT_EQ(t.GetRotation(), 10.0f);
}

// ----------------------------
// Dirty Flag
// ----------------------------

TEST(TransformTests, DirtyFlagTracksChanges) {
    Transform t;

    // a fresh Transform has never been read so it starts dirty
    EXPECT_TRUE(t.IsDirty());

    t.ClearDirty();
    EXPECT_FALSE(t.IsDirty());

    t.SetTranslation(Vec2f{ 1.0f, 1.0f });
    EXPECT_TRUE(t.IsDirty());

    t.ClearDirty();
    t.SetScale(Vec2f{ 2.0f, 2.0f });
    EXPECT_TRUE(t.IsDirty());

    t.ClearDirty();
    t.Rotate(5.0f);
    EXPECT_TRUE(t.IsDirty());
}

// ----------------------------
// Cloning
// ----------------------------

TEST(TransformTests, CloneCopiesValuesButNotIdentity) {
    Transform original(Vec2f{ 5.0f, 6.0f }, 45.0f, Vec2f{ 3.0f, 3.0f });

    Component* clone = original.Clone();
    ASSERT_NE(clone, nullptr);

    auto* cloned = dynamic_cast<Transform*>(clone);
    ASSERT_NE(cloned, nullptr);

    EXPECT_TRUE(cloned->GetTranslation().AlmostEqual(original.GetTranslation()));
    EXPECT_FLOAT_EQ(cloned->GetRotation(), original.GetRotation());
    EXPECT_TRUE(cloned->GetScale().AlmostEqual(original.GetScale()));

    // a clone is its own component, it does not share an id or an owner
    EXPECT_NE(cloned->GetId(), original.GetId());
    EXPECT_EQ(cloned->GetEntity(), nullptr);

    delete clone;
}

TEST(TransformTests, EntityCarriesTheTransformAndHandsItBack) {
    Entity entity;
    entity.SetName("Player");

    auto* transform = new Transform(Vec2f{ 2.0f, 2.0f });
    entity.AddComponent(transform);

    // this is the call that could not link before the templates moved to the header
    Transform* found = entity.GetComponent<Transform>();

    ASSERT_EQ(found, transform);
    EXPECT_TRUE(found->GetTranslation().AlmostEqual(Vec2f{ 2.0f, 2.0f }));
    EXPECT_EQ(found->GetEntity(), &entity);
    EXPECT_EQ(found->GetName(), "Player->" + PrefixlessName(typeid(Transform)));
}
