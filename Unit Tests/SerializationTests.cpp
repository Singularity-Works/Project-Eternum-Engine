/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: SerializationTests
* Description:
*     Tests saving and loading. The property that matters is the round trip: write the game
*     out, throw it away, read it back, and end up somewhere indistinguishable. The other
*     half is that a damaged or foreign save is refused rather than half applied.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include <Core/ECS/Component/Attacker/Attacker.h>
#include <Core/ECS/Component/Chaser/Chaser.h>
#include <Core/ECS/Component/ComponentFactory.h>
#include <Core/ECS/Component/Glyph/Glyph.h>
#include <Core/ECS/Component/Health/Health.h>
#include <Core/ECS/Component/PlayerController/PlayerController.h>
#include <Core/ECS/Component/StatusEffects/StatusEffects.h>
#include <Core/ECS/Component/Transform/Transform.h>
#include <Core/ECS/Component/Trap/Trap.h>
#include <Core/ECS/Component/Pickup/Pickup.h>
#include <Core/ECS/Entity/Entity.h>
#include <Systems/Dungeon System/DungeonSystem.h>
#include <Systems/Entity System/EntitySystem.h>
#include <Systems/Grid System/GridSystem.h>
#include <Systems/Pathfinding/PathfindingSystem.h>
#include <Systems/Save/SaveSystem.h>

class SerializationTest : public ::testing::Test {
protected:
    void SetUp() override {
        Entities()->ClearEntities();
        GridSystem::GetInstance()->ClearMaps();
        GridSystem::GetInstance()->ClearAllOverlays();
        Paths()->InvalidateNavGrid();
    }

    void TearDown() override {
        Entities()->ClearEntities();
        GridSystem::GetInstance()->ClearMaps();
        GridSystem::GetInstance()->ClearAllOverlays();
        Paths()->InvalidateNavGrid();
    }

    // writes one component out and reads it into a fresh one of the same type
    template <typename T>
    static void RoundTrip(const T& from, T& into) {
        nlohmann::json data = nlohmann::json::object();
        from.Write(data);
        into.Read(data);
    }
};

// ----------------------------
// The Factory
// ----------------------------

TEST(ComponentFactoryTests, EveryComponentTypeCanBeBuiltByName) {
    for (const std::string& name : { "Transform", "Glyph", "Health",
                                     "Attacker", "StatusEffects",
                                     "PlayerController", "Chaser",
                                     "Trap", "Pickup" }) {
        EXPECT_TRUE(ComponentFactory::Instance().Knows(name)) << name;

        Component* built = ComponentFactory::Instance().Create(name);
        ASSERT_NE(built, nullptr) << name;

        // what it says it is has to match what the save file will call it
        EXPECT_EQ(built->GetTypeName(), name);
        delete built;
    }
}

TEST(ComponentFactoryTests, AnUnknownNameBuildsNothingRatherThanGuessing) {
    EXPECT_FALSE(ComponentFactory::Instance().Knows("NotAComponent"));
    EXPECT_EQ(ComponentFactory::Instance().Create("NotAComponent"), nullptr);
    EXPECT_EQ(ComponentFactory::Instance().Create(""), nullptr);
}

// ----------------------------
// Components
// ----------------------------

TEST_F(SerializationTest, ATransformSurvivesTheRoundTrip) {
    const Transform original(Vec2f{ 4.0f, 9.0f }, 90.0f, Vec2f{ 2.0f, 3.0f });
    Transform loaded;

    RoundTrip(original, loaded);

    EXPECT_TRUE(loaded.GetTranslation().AlmostEqual(original.GetTranslation()));
    EXPECT_FLOAT_EQ(loaded.GetRotation(), original.GetRotation());
    EXPECT_TRUE(loaded.GetScale().AlmostEqual(original.GetScale()));
}

TEST_F(SerializationTest, AGlyphSurvivesTheRoundTrip) {
    Glyph original('g', 7);
    original.SetVisible(false);
    Glyph loaded;

    RoundTrip(original, loaded);

    EXPECT_EQ(loaded.GetSymbol(), 'g');
    EXPECT_EQ(loaded.GetDrawOrder(), 7);
    EXPECT_FALSE(loaded.IsVisible());
}

TEST_F(SerializationTest, HealthSurvivesTheRoundTrip) {
    Health original(30);
    original.SetDestroyOnDeath(false);
    original.SetMercyWindow(0.75);
    original.TakeDamage(11);

    Health loaded;
    RoundTrip(original, loaded);

    EXPECT_EQ(loaded.GetHealth(), 19);
    EXPECT_EQ(loaded.GetMaxHealth(), 30);
    EXPECT_DOUBLE_EQ(loaded.GetMercyWindow(), 0.75);
    EXPECT_FALSE(loaded.GetDestroyOnDeath());

    // a load is not a hit, so nothing should be mid recovery
    EXPECT_FALSE(loaded.IsRecovering());
}

TEST_F(SerializationTest, AnAttackerSurvivesTheRoundTripWithItsOnHitEffect) {
    Attacker original(9, 1.25);

    Attacker::OnHit onHit;
    onHit.m_Type = EffectType::Poison;
    onHit.m_Duration = 4.0;
    onHit.m_Magnitude = 3;
    onHit.m_Chance = 40;
    original.SetOnHit(onHit);

    Attacker loaded;
    RoundTrip(original, loaded);

    EXPECT_EQ(loaded.GetDamage(), 9);
    EXPECT_DOUBLE_EQ(loaded.GetAttackInterval(), 1.25);
    EXPECT_EQ(loaded.GetOnHit().m_Type, EffectType::Poison);
    EXPECT_DOUBLE_EQ(loaded.GetOnHit().m_Duration, 4.0);
    EXPECT_EQ(loaded.GetOnHit().m_Magnitude, 3);
    EXPECT_EQ(loaded.GetOnHit().m_Chance, 40);

    // a load is not a swing, the next one is ready
    EXPECT_TRUE(loaded.CanAttack());
}

TEST_F(SerializationTest, StatusEffectsSurviveTheRoundTrip) {
    StatusEffects original;
    original.Apply(EffectType::Poison, 5.0, 2);
    original.Apply(EffectType::Slow, 3.0);

    StatusEffects loaded;
    RoundTrip(original, loaded);

    EXPECT_EQ(loaded.GetActive().size(), 2u);
    EXPECT_TRUE(loaded.Has(EffectType::Poison));
    EXPECT_TRUE(loaded.Has(EffectType::Slow));
    EXPECT_DOUBLE_EQ(loaded.GetRemaining(EffectType::Poison), 5.0);
}

TEST_F(SerializationTest, AnEffectThatHasRunOutIsNotLoadedBack) {
    nlohmann::json data;
    data["effects"] = nlohmann::json::array({
        { { "type", "poisoned" }, { "remaining", 0.0 }, { "magnitude", 2 } },
        { { "type", "stunned" }, { "remaining", -1.0 }, { "magnitude", 0 } }
    });

    StatusEffects loaded;
    loaded.Read(data);

    EXPECT_TRUE(loaded.GetActive().empty());
}

TEST_F(SerializationTest, AnEffectNameThisBuildDoesNotKnowIsDropped) {
    nlohmann::json data;
    data["effects"] = nlohmann::json::array({
        { { "type", "cursed" }, { "remaining", 5.0 }, { "magnitude", 1 } },
        { { "type", "poisoned" }, { "remaining", 5.0 }, { "magnitude", 1 } }
    });

    StatusEffects loaded;
    loaded.Read(data);

    // the one it understands loads, the one it does not is left alone
    EXPECT_EQ(loaded.GetActive().size(), 1u);
    EXPECT_TRUE(loaded.Has(EffectType::Poison));
}

TEST_F(SerializationTest, AChaserSurvivesTheRoundTrip) {
    Chaser original;
    original.SetSightRange(23);
    original.SetTargetName("SomethingElse");

    Chaser loaded;
    RoundTrip(original, loaded);

    EXPECT_EQ(loaded.GetSightRange(), 23);
    EXPECT_EQ(loaded.GetTargetName(), "SomethingElse");
    EXPECT_EQ(loaded.GetState(), ChaserState::Roaming);
}

TEST_F(SerializationTest, EveryChaserStateNameRoundTrips) {
    for (const ChaserState state : { ChaserState::Roaming, ChaserState::Hunting,
                                     ChaserState::Searching }) {
        EXPECT_EQ(Chaser::GetStateFromName(Chaser::GetStateName(state)), state);
    }
}

TEST_F(SerializationTest, EveryEffectNameRoundTrips) {
    for (const EffectType type : { EffectType::Poison, EffectType::Slow, EffectType::Stun }) {
        EXPECT_EQ(StatusEffects::GetEffectFromName(StatusEffects::GetEffectName(type)), type);
    }
}

TEST_F(SerializationTest, ATrapSurvivesTheRoundTripIncludingWhetherItHasGoneOff) {
    Trap original(9);
    original.SetEffect(EffectType::Slow, 2.5, 0);
    original.Trigger(nullptr);

    Trap loaded;
    RoundTrip(original, loaded);

    EXPECT_EQ(loaded.GetDamage(), 9);
    EXPECT_EQ(loaded.GetEffect(), EffectType::Slow);

    // a spent trap must not come back armed, or a save would rearm the whole dungeon
    EXPECT_EQ(loaded.IsArmed(), original.IsArmed());
    EXPECT_EQ(loaded.IsHidden(), original.IsHidden());
}

TEST_F(SerializationTest, APickupSurvivesTheRoundTrip) {
    const Pickup original(11);
    Pickup loaded;

    RoundTrip(original, loaded);

    EXPECT_EQ(loaded.GetRestores(), 11);
    EXPECT_FALSE(loaded.IsCollected());
}

// ----------------------------
// Entities
// ----------------------------

TEST_F(SerializationTest, AnEntityRebuildsItsComponentsFromNothing) {
    Entity* original = Entities()->CreateEntity("Hero");
    original->AddComponent(new Transform(Vec2f{ 5.0f, 6.0f }));
    original->AddComponent(new Glyph('@', 10));
    original->AddComponent(new Health(15));

    nlohmann::json data;
    original->Write(data);

    Entity rebuilt;
    rebuilt.Read(data);

    EXPECT_EQ(rebuilt.GetName(), "Hero");
    EXPECT_EQ(rebuilt.getComponents().size(), 3u);

    ASSERT_NE(rebuilt.GetComponent<Transform>(), nullptr);
    EXPECT_TRUE(rebuilt.GetComponent<Transform>()->GetTranslation().AlmostEqual(Vec2f{ 5.0f, 6.0f }));

    ASSERT_NE(rebuilt.GetComponent<Glyph>(), nullptr);
    EXPECT_EQ(rebuilt.GetComponent<Glyph>()->GetSymbol(), '@');

    ASSERT_NE(rebuilt.GetComponent<Health>(), nullptr);
    EXPECT_EQ(rebuilt.GetComponent<Health>()->GetMaxHealth(), 15);
}

TEST_F(SerializationTest, ChildrenComeBackToo) {
    Entity* parent = Entities()->CreateEntity("Parent");
    parent->AddComponent(new Transform(Vec2f{ 1.0f, 1.0f }));

    auto* child = new Entity();
    child->SetName("Child");
    child->AddComponent(new Transform(Vec2f{ 2.0f, 2.0f }));
    child->SetParent(parent);

    nlohmann::json data;
    parent->Write(data);

    Entity rebuilt;
    rebuilt.Read(data);

    ASSERT_EQ(rebuilt.GetChildren().size(), 1u);
    EXPECT_EQ(rebuilt.GetChildren()[0]->GetName(), "Child");
    EXPECT_EQ(rebuilt.GetNumDescendants(), 1);
}

TEST_F(SerializationTest, ReadingOverAnEntityReplacesWhatWasThere) {
    Entity entity;
    entity.SetName("Before");
    entity.AddComponent(new Health(99));
    entity.AddComponent(new Glyph('x', 0));

    nlohmann::json data;
    data["name"] = "After";
    data["components"] = { { "Transform", nlohmann::json::object() } };

    entity.Read(data);

    EXPECT_EQ(entity.GetName(), "After");
    EXPECT_EQ(entity.getComponents().size(), 1u);
    EXPECT_NE(entity.GetComponent<Transform>(), nullptr);
    EXPECT_EQ(entity.GetComponent<Health>(), nullptr);
}

// ----------------------------
// The Whole Game
// ----------------------------

TEST_F(SerializationTest, AWholeGameSurvivesASaveAndLoad) {
    DungeonSystem::GetInstance()->StartNewDungeon(1234);

    Entity* player = Entities()->FindEntity("Player");
    ASSERT_NE(player, nullptr);

    // rough the player up so there is something worth restoring
    player->GetComponent<Health>()->TakeDamage(7);
    player->GetComponent<StatusEffects>()->Apply(EffectType::Poison, 4.0, 2);

    const Vec2f position = player->GetComponent<Transform>()->GetTranslation();
    const int health = player->GetComponent<Health>()->GetHealth();
    const int entities = Entities()->GetEntityCount();
    const std::string map(DungeonSystem::GetInstance()->GetCurrentGrid().cells.begin(),
                          DungeonSystem::GetInstance()->GetCurrentGrid().cells.end());

    const nlohmann::json snapshot = Saves()->BuildSnapshot();

    // wipe it out completely, then put it back
    Entities()->ClearEntities();
    ASSERT_EQ(Entities()->GetEntityCount(), 0);

    ASSERT_TRUE(Saves()->ApplySnapshot(snapshot));

    Entity* restored = Entities()->FindEntity("Player");
    ASSERT_NE(restored, nullptr);

    EXPECT_EQ(Entities()->GetEntityCount(), entities);
    EXPECT_TRUE(restored->GetComponent<Transform>()->GetTranslation().AlmostEqual(position));
    EXPECT_EQ(restored->GetComponent<Health>()->GetHealth(), health);
    EXPECT_TRUE(restored->GetComponent<StatusEffects>()->Has(EffectType::Poison));

    const std::string restoredMap(DungeonSystem::GetInstance()->GetCurrentGrid().cells.begin(),
                                  DungeonSystem::GetInstance()->GetCurrentGrid().cells.end());
    EXPECT_EQ(restoredMap, map);
    EXPECT_EQ(DungeonSystem::GetInstance()->GetSeed(), 1234u);
}

TEST_F(SerializationTest, TheBinaryFormCarriesTheSameDataInFewerBytes) {
    DungeonSystem::GetInstance()->StartNewDungeon(99);

    const nlohmann::json snapshot = Saves()->BuildSnapshot();

    const std::string text = snapshot.dump(2);
    const std::vector<std::uint8_t> binary = nlohmann::json::to_msgpack(snapshot);

    // same data back out of the binary
    EXPECT_EQ(nlohmann::json::from_msgpack(binary), snapshot);

    // and it is smaller, which is the reason for having both
    EXPECT_LT(binary.size(), text.size())
        << "json " << text.size() << " bytes, msgpack " << binary.size() << " bytes";
}

TEST_F(SerializationTest, ASaveFromAnotherVersionIsRefused) {
    nlohmann::json snapshot = Saves()->BuildSnapshot();
    snapshot["version"] = SaveSystem::FORMAT_VERSION + 1;

    EXPECT_FALSE(Saves()->ApplySnapshot(snapshot));
}

TEST_F(SerializationTest, AMapThatDoesNotAddUpIsLeftAlone) {
    DungeonSystem::GetInstance()->StartNewDungeon(5);
    const std::string before(DungeonSystem::GetInstance()->GetCurrentGrid().cells.begin(),
                             DungeonSystem::GetInstance()->GetCurrentGrid().cells.end());

    nlohmann::json broken;
    broken["width"] = 40;
    broken["height"] = 20;
    broken["cells"] = "far too short to be a map";

    DungeonSystem::GetInstance()->ReadState(broken);

    const std::string after(DungeonSystem::GetInstance()->GetCurrentGrid().cells.begin(),
                            DungeonSystem::GetInstance()->GetCurrentGrid().cells.end());
    EXPECT_EQ(after, before);
}

TEST_F(SerializationTest, SavingAndLoadingThroughRealFiles) {
    DungeonSystem::GetInstance()->StartNewDungeon(4321);

    Entity* player = Entities()->FindEntity("Player");
    ASSERT_NE(player, nullptr);
    player->GetComponent<Health>()->TakeDamage(5);

    const int health = player->GetComponent<Health>()->GetHealth();

    const SaveSystem::Result saved = Saves()->Save();
    ASSERT_TRUE(saved.m_Succeeded) << saved.m_Message;
    EXPECT_GT(saved.m_JsonBytes, 0u);
    EXPECT_GT(saved.m_BinaryBytes, 0u);

    Entities()->ClearEntities();

    const SaveSystem::Result loaded = Saves()->Load();
    ASSERT_TRUE(loaded.m_Succeeded) << loaded.m_Message;

    Entity* restored = Entities()->FindEntity("Player");
    ASSERT_NE(restored, nullptr);
    EXPECT_EQ(restored->GetComponent<Health>()->GetHealth(), health);

    std::remove(SaveSystem::JSON_PATH);
    std::remove(SaveSystem::BINARY_PATH);
}

TEST_F(SerializationTest, LoadingWithNoSaveFileSaysSoInsteadOfCrashing) {
    std::remove(SaveSystem::JSON_PATH);
    std::remove(SaveSystem::BINARY_PATH);

    const SaveSystem::Result result = Saves()->Load();

    EXPECT_FALSE(result.m_Succeeded);
    EXPECT_FALSE(result.m_Message.empty());
}
