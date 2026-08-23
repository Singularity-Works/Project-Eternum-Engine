/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: CombatTests
* Description:
*     Tests Health, Attacker and StatusEffects on their own, then the two things that only
*     show up once they are wired together: walking into something is how you hit it, and
*     an enemy standing next to you swings instead of shuffling.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#include <gtest/gtest.h>
#include <Core/ECS/Component/Attacker/Attacker.h>
#include <Core/ECS/Component/Chaser/Chaser.h>
#include <Core/ECS/Component/Glyph/Glyph.h>
#include <Core/ECS/Component/Health/Health.h>
#include <Core/ECS/Component/PlayerController/PlayerController.h>
#include <Core/ECS/Component/Pickup/Pickup.h>
#include <Core/ECS/Component/Trap/Trap.h>
#include <Core/ECS/Component/Solid/Solid.h>
#include <Core/ECS/Component/StatusEffects/StatusEffects.h>
#include <Core/ECS/Component/Transform/Transform.h>
#include <Core/ECS/Entity/Entity.h>
#include <Systems/Entity System/EntitySystem.h>
#include <Systems/Grid System/GridSystem.h>
#include <Systems/Pathfinding/PathfindingSystem.h>

class CombatTest : public ::testing::Test {
protected:
    void SetUp() override {
        Entities()->ClearEntities();
        GridSystem::GetInstance()->ClearMaps();
        GridSystem::GetInstance()->ClearAllOverlays();

        // a 9x5 room, open floor inside a solid border
        GridSystem::GetInstance()->CreateMap("Room", GridSystem::Dimension(9, 5), '#');
        GridSystem::GetInstance()->LoadMap("Room");
        for (int y = 1; y <= 3; ++y)
            for (int x = 1; x <= 7; ++x)
                GridSystem::GetInstance()->SetCell(x, y, '.');

        Paths()->InvalidateNavGrid();
    }

    void TearDown() override {
        Entities()->ClearEntities();
        GridSystem::GetInstance()->ClearMaps();
        GridSystem::GetInstance()->ClearAllOverlays();
        Paths()->InvalidateNavGrid();
    }

    // a bare entity standing somewhere, with whatever combat parts are asked for
    static Entity* SpawnFighter(const std::string& name, const Vec2i& cell,
                                const int health, const int damage) {
        Entity* entity = Entities()->CreateEntity(name);
        entity->AddComponent(new Transform(Vec2f{ static_cast<float>(cell.x()),
                                                  static_cast<float>(cell.y()) }));
        entity->AddComponent(new Glyph('X', 1));
        entity->AddComponent(new Solid());
        entity->AddComponent(new Health(health));
        entity->AddComponent(new StatusEffects());
        entity->AddComponent(new Attacker(damage, 0.5));
        return entity;
    }

    static Vec2i CellOf(const Entity* entity) {
        const Vec2f& position = entity->GetComponent<Transform>()->GetTranslation();
        return Vec2i{ static_cast<int>(position.x()), static_cast<int>(position.y()) };
    }
};

// ----------------------------
// Health
// ----------------------------

TEST(HealthTests, StartsFullAndAlive) {
    const Health health(12);

    EXPECT_EQ(health.GetHealth(), 12);
    EXPECT_EQ(health.GetMaxHealth(), 12);
    EXPECT_TRUE(health.IsAlive());
    EXPECT_FALSE(health.IsRecovering());
}

TEST(HealthTests, DamageComesOffAndOpensAMercyWindow) {
    Health health(10);
    health.SetDestroyOnDeath(false);

    EXPECT_EQ(health.TakeDamage(3), 3);
    EXPECT_EQ(health.GetHealth(), 7);
    EXPECT_TRUE(health.IsRecovering());

    // a second hit inside the window is thrown away completely
    EXPECT_EQ(health.TakeDamage(3), 0);
    EXPECT_EQ(health.GetHealth(), 7);
}

TEST(HealthTests, TheMercyWindowRunsOut) {
    Health health(10);
    health.SetDestroyOnDeath(false);

    ASSERT_EQ(health.TakeDamage(3), 3);

    for (int frame = 0; frame < 40; ++frame)
        health.OnUpdate(0.016);

    EXPECT_FALSE(health.IsRecovering());
    EXPECT_EQ(health.TakeDamage(3), 3);
    EXPECT_EQ(health.GetHealth(), 4);
}

TEST(HealthTests, PoisonStyleDamageIgnoresTheMercyWindow) {
    Health health(10);
    health.SetDestroyOnDeath(false);

    ASSERT_EQ(health.TakeDamage(3), 3);
    ASSERT_TRUE(health.IsRecovering());

    // an effect ticking should still bite, otherwise poison never lands in a fight
    EXPECT_EQ(health.TakeDamageIgnoringMercy(2), 2);
    EXPECT_EQ(health.GetHealth(), 5);
}

TEST(HealthTests, DamageNeverGoesBelowZero) {
    Health health(5);
    health.SetDestroyOnDeath(false);

    EXPECT_EQ(health.TakeDamage(99), 5);
    EXPECT_EQ(health.GetHealth(), 0);
    EXPECT_FALSE(health.IsAlive());

    // and nothing more can be taken off a corpse
    EXPECT_EQ(health.TakeDamage(5), 0);
}

TEST(HealthTests, HealingStopsAtFull) {
    Health health(10);
    health.SetDestroyOnDeath(false);
    health.TakeDamage(6);

    EXPECT_EQ(health.Heal(2), 2);
    EXPECT_EQ(health.GetHealth(), 6);

    EXPECT_EQ(health.Heal(99), 4);
    EXPECT_EQ(health.GetHealth(), 10);
}

TEST(HealthTests, TheDeadCannotBeHealed) {
    Health health(5);
    health.SetDestroyOnDeath(false);
    health.Kill();

    ASSERT_FALSE(health.IsAlive());
    EXPECT_EQ(health.Heal(5), 0);
}

TEST(HealthTests, ResetPutsItBackToFull) {
    Health health(10);
    health.SetDestroyOnDeath(false);
    health.TakeDamage(7);

    health.Reset();

    EXPECT_EQ(health.GetHealth(), 10);
    EXPECT_TRUE(health.IsAlive());
    EXPECT_FALSE(health.IsRecovering());
}

TEST(HealthTests, LoweringTheMaximumClampsWhatIsLeft) {
    Health health(20);
    health.SetDestroyOnDeath(false);

    health.SetMaxHealth(8);

    EXPECT_EQ(health.GetMaxHealth(), 8);
    EXPECT_EQ(health.GetHealth(), 8);
}

TEST_F(CombatTest, DyingRemovesTheEntityWhenItIsMeantTo) {
    Entity* enemy = SpawnFighter("Enemy", Vec2i{ 1, 1 }, 5, 1);
    ASSERT_EQ(Entities()->GetEntityCount(), 1);

    enemy->GetComponent<Health>()->Kill();
    Entities()->Update(0.016);

    EXPECT_EQ(Entities()->GetEntityCount(), 0);
}

TEST_F(CombatTest, ThePlayerStaysInTheSceneAfterDying) {
    Entity* player = SpawnFighter("Player", Vec2i{ 1, 1 }, 5, 1);
    player->GetComponent<Health>()->SetDestroyOnDeath(false);

    player->GetComponent<Health>()->Kill();
    Entities()->Update(0.016);

    EXPECT_EQ(Entities()->GetEntityCount(), 1);
    EXPECT_FALSE(player->GetComponent<Health>()->IsAlive());
}

// ----------------------------
// Status Effects
// ----------------------------

TEST(StatusEffectTests, AnEffectRunsOutOnItsOwn) {
    StatusEffects effects;
    effects.Apply(EffectType::Stun, 0.2);

    EXPECT_TRUE(effects.Has(EffectType::Stun));
    EXPECT_TRUE(effects.IsStunned());

    for (int frame = 0; frame < 20; ++frame)
        effects.OnUpdate(0.016);

    EXPECT_FALSE(effects.Has(EffectType::Stun));
    EXPECT_FALSE(effects.IsStunned());
}

TEST(StatusEffectTests, ApplyingTheSameEffectRefreshesInsteadOfStacking) {
    StatusEffects effects;

    effects.Apply(EffectType::Poison, 1.0, 1);
    effects.Apply(EffectType::Poison, 3.0, 2);

    ASSERT_EQ(effects.GetActive().size(), 1u);

    // the longer duration and the stronger magnitude both win
    EXPECT_DOUBLE_EQ(effects.GetRemaining(EffectType::Poison), 3.0);
    EXPECT_EQ(effects.GetActive().front().m_Magnitude, 2);
}

TEST(StatusEffectTests, ARefreshNeverShortensWhatIsAlreadyRunning) {
    StatusEffects effects;

    effects.Apply(EffectType::Stun, 5.0);
    effects.Apply(EffectType::Stun, 0.1);

    EXPECT_DOUBLE_EQ(effects.GetRemaining(EffectType::Stun), 5.0);
}

TEST(StatusEffectTests, DifferentEffectsRunSideBySide) {
    StatusEffects effects;

    effects.Apply(EffectType::Poison, 2.0, 1);
    effects.Apply(EffectType::Slow, 2.0);

    EXPECT_EQ(effects.GetActive().size(), 2u);
    EXPECT_TRUE(effects.Has(EffectType::Poison));
    EXPECT_TRUE(effects.Has(EffectType::Slow));
}

TEST(StatusEffectTests, SlowMakesActionsTakeLonger) {
    StatusEffects effects;

    EXPECT_DOUBLE_EQ(effects.GetCooldownMultiplier(), 1.0);

    effects.Apply(EffectType::Slow, 1.0);
    EXPECT_DOUBLE_EQ(effects.GetCooldownMultiplier(), StatusEffects::SLOW_MULTIPLIER);
    EXPECT_GT(StatusEffects::SLOW_MULTIPLIER, 1.0);
}

TEST(StatusEffectTests, ClearingWorksOneAtATimeAndAllAtOnce) {
    StatusEffects effects;
    effects.Apply(EffectType::Poison, 5.0, 1);
    effects.Apply(EffectType::Slow, 5.0);

    effects.Clear(EffectType::Poison);
    EXPECT_FALSE(effects.Has(EffectType::Poison));
    EXPECT_TRUE(effects.Has(EffectType::Slow));

    effects.ClearAll();
    EXPECT_TRUE(effects.GetActive().empty());
}

TEST(StatusEffectTests, NonsenseApplicationsAreIgnored) {
    StatusEffects effects;

    effects.Apply(EffectType::None, 5.0);
    effects.Apply(EffectType::Stun, 0.0);
    effects.Apply(EffectType::Stun, -1.0);

    EXPECT_TRUE(effects.GetActive().empty());
}

TEST(StatusEffectTests, EveryEffectHasAName) {
    EXPECT_EQ(StatusEffects::GetEffectName(EffectType::Poison), "poisoned");
    EXPECT_EQ(StatusEffects::GetEffectName(EffectType::Slow), "slowed");
    EXPECT_EQ(StatusEffects::GetEffectName(EffectType::Stun), "stunned");
}

TEST_F(CombatTest, PoisonTakesHealthOverTime) {
    Entity* victim = SpawnFighter("Victim", Vec2i{ 1, 1 }, 20, 1);
    victim->GetComponent<Health>()->SetDestroyOnDeath(false);

    victim->GetComponent<StatusEffects>()->Apply(EffectType::Poison, 3.0, 2);

    StatusEffects* effects = victim->GetComponent<StatusEffects>();
    const Health* health = victim->GetComponent<Health>();

    // three seconds of frames, ticking every POISON_TICK seconds
    for (int frame = 0; frame < 190; ++frame)
        effects->OnUpdate(0.016);

    EXPECT_LT(health->GetHealth(), 20);
    EXPECT_FALSE(effects->Has(EffectType::Poison));
}

TEST_F(CombatTest, PoisonWithoutHealthDoesNotCrash) {
    Entity* rock = Entities()->CreateEntity("Rock");
    auto* effects = new StatusEffects();
    rock->AddComponent(effects);

    effects->Apply(EffectType::Poison, 2.0, 5);

    for (int frame = 0; frame < 130; ++frame)
        effects->OnUpdate(0.016);

    SUCCEED();
}

// ----------------------------
// Attacker
// ----------------------------

TEST_F(CombatTest, AnAttackTakesHealthOffTheTarget) {
    Entity* attacker = SpawnFighter("Attacker", Vec2i{ 1, 1 }, 10, 4);
    Entity* target = SpawnFighter("Target", Vec2i{ 2, 1 }, 10, 1);
    target->GetComponent<Health>()->SetDestroyOnDeath(false);

    EXPECT_EQ(attacker->GetComponent<Attacker>()->Attack(target), 4);
    EXPECT_EQ(target->GetComponent<Health>()->GetHealth(), 6);
    EXPECT_EQ(attacker->GetComponent<Attacker>()->GetTotalDamageDealt(), 4);
}

TEST_F(CombatTest, TheSwingHasToRecharge) {
    Entity* attacker = SpawnFighter("Attacker", Vec2i{ 1, 1 }, 10, 4);
    Entity* target = SpawnFighter("Target", Vec2i{ 2, 1 }, 40, 1);
    target->GetComponent<Health>()->SetDestroyOnDeath(false);
    target->GetComponent<Health>()->SetMercyWindow(0.0);

    Attacker* weapon = attacker->GetComponent<Attacker>();

    ASSERT_EQ(weapon->Attack(target), 4);
    EXPECT_FALSE(weapon->CanAttack());
    EXPECT_EQ(weapon->Attack(target), 0);

    for (int frame = 0; frame < 40; ++frame)
        weapon->OnUpdate(0.016);

    EXPECT_TRUE(weapon->CanAttack());
    EXPECT_EQ(weapon->Attack(target), 4);
}

TEST_F(CombatTest, AttackingNothingIsHarmless) {
    Entity* attacker = SpawnFighter("Attacker", Vec2i{ 1, 1 }, 10, 4);
    Entity* rock = Entities()->CreateEntity("Rock");

    EXPECT_EQ(attacker->GetComponent<Attacker>()->Attack(nullptr), 0);
    EXPECT_EQ(attacker->GetComponent<Attacker>()->Attack(rock), 0);
}

TEST_F(CombatTest, AHitCanLeaveAnEffectBehind) {
    Entity* attacker = SpawnFighter("Attacker", Vec2i{ 1, 1 }, 10, 2);
    Entity* target = SpawnFighter("Target", Vec2i{ 2, 1 }, 20, 1);
    target->GetComponent<Health>()->SetDestroyOnDeath(false);

    Attacker::OnHit onHit;
    onHit.m_Type = EffectType::Poison;
    onHit.m_Duration = 2.0;
    onHit.m_Magnitude = 1;
    onHit.m_Chance = 100;
    attacker->GetComponent<Attacker>()->SetOnHit(onHit);

    ASSERT_GT(attacker->GetComponent<Attacker>()->Attack(target), 0);

    EXPECT_TRUE(target->GetComponent<StatusEffects>()->Has(EffectType::Poison));
}

TEST_F(CombatTest, AnEffectNeverLandsWhenTheHitDidNot) {
    Entity* attacker = SpawnFighter("Attacker", Vec2i{ 1, 1 }, 10, 2);
    Entity* target = SpawnFighter("Target", Vec2i{ 2, 1 }, 20, 1);
    target->GetComponent<Health>()->SetDestroyOnDeath(false);

    Attacker::OnHit onHit;
    onHit.m_Type = EffectType::Stun;
    onHit.m_Duration = 2.0;
    onHit.m_Chance = 100;

    Attacker* weapon = attacker->GetComponent<Attacker>();
    weapon->SetOnHit(onHit);

    // land one hit so the target is inside its mercy window
    ASSERT_GT(weapon->Attack(target), 0);
    target->GetComponent<StatusEffects>()->ClearAll();

    for (int frame = 0; frame < 40; ++frame)
        weapon->OnUpdate(0.016);

    // the swing is ready again but the target shrugs it off, so no effect either
    ASSERT_TRUE(weapon->CanAttack());
    EXPECT_EQ(weapon->Attack(target), 0);
    EXPECT_FALSE(target->GetComponent<StatusEffects>()->Has(EffectType::Stun));
}

// ----------------------------
// Bump Attacks
// ----------------------------

TEST_F(CombatTest, WalkingIntoSomethingHitsItInsteadOfMoving) {
    Entity* player = Entities()->CreateEntity("Player");
    player->AddComponent(new Transform(Vec2f{ 2.0f, 2.0f }));
    player->AddComponent(new Glyph('@', 10));
    auto* controller = new PlayerController();
    player->AddComponent(controller);
    player->AddComponent(new Health(20));
    player->AddComponent(new StatusEffects());
    player->AddComponent(new Attacker(4, 0.35));

    Entity* enemy = SpawnFighter("Enemy", Vec2i{ 3, 2 }, 10, 1);
    enemy->GetComponent<Health>()->SetDestroyOnDeath(false);

    // walking east means walking into the enemy
    EXPECT_FALSE(controller->TryMove(Vec2i{ 1, 0 }));

    EXPECT_EQ(CellOf(player), (Vec2i{ 2, 2 }));
    EXPECT_EQ(enemy->GetComponent<Health>()->GetHealth(), 6);
    EXPECT_EQ(controller->GetStepsTaken(), 0);
}

TEST_F(CombatTest, ALandedHitStaggersWhatTookIt) {
    Entity* player = Entities()->CreateEntity("Player");
    player->AddComponent(new Transform(Vec2f{ 2.0f, 2.0f }));
    auto* controller = new PlayerController();
    player->AddComponent(controller);
    player->AddComponent(new Attacker(4, 0.35));

    Entity* enemy = SpawnFighter("Enemy", Vec2i{ 3, 2 }, 10, 1);
    enemy->GetComponent<Health>()->SetDestroyOnDeath(false);

    ASSERT_GT(controller->TryAttack(Vec2i{ 3, 2 }), 0);

    EXPECT_TRUE(enemy->GetComponent<StatusEffects>()->Has(EffectType::Stun));
}

TEST_F(CombatTest, YouCannotWalkThroughSomethingSolidYouCannotHit) {
    Entity* player = Entities()->CreateEntity("Player");
    player->AddComponent(new Transform(Vec2f{ 2.0f, 2.0f }));
    auto* controller = new PlayerController();
    player->AddComponent(controller);

    // a barrel, no health and nothing to attack with, but it takes up space
    Entity* barrel = Entities()->CreateEntity("Barrel");
    barrel->AddComponent(new Transform(Vec2f{ 3.0f, 2.0f }));
    barrel->AddComponent(new Solid());

    EXPECT_FALSE(controller->TryMove(Vec2i{ 1, 0 }));
    EXPECT_EQ(CellOf(player), (Vec2i{ 2, 2 }));
}

TEST_F(CombatTest, YouWalkStraightOverThingsLyingOnTheFloor) {
    Entity* player = Entities()->CreateEntity("Player");
    player->AddComponent(new Transform(Vec2f{ 2.0f, 2.0f }));
    auto* controller = new PlayerController();
    player->AddComponent(controller);
    player->AddComponent(new Solid());

    // loot has no Solid, so it must not behave like a wall
    Entity* loot = Entities()->CreateEntity("Loot");
    loot->AddComponent(new Transform(Vec2f{ 3.0f, 2.0f }));
    loot->AddComponent(new Glyph(Pickup::SYMBOL, 2));
    loot->AddComponent(new Pickup(6));

    EXPECT_TRUE(controller->TryMove(Vec2i{ 1, 0 }));
    EXPECT_EQ(CellOf(player), (Vec2i{ 3, 2 }));
}

TEST_F(CombatTest, AHiddenTrapIsNotAnInvisibleWall) {
    Entity* player = Entities()->CreateEntity("Player");
    player->AddComponent(new Transform(Vec2f{ 2.0f, 2.0f }));
    auto* controller = new PlayerController();
    player->AddComponent(controller);
    player->AddComponent(new Solid());
    player->AddComponent(new Health(20));

    Entity* trap = Entities()->CreateEntity("Trap");
    trap->AddComponent(new Transform(Vec2f{ 3.0f, 2.0f }));
    trap->AddComponent(new Glyph(Trap::HIDDEN_SYMBOL, 1));
    trap->AddComponent(new Trap(5));

    // you are meant to find a trap by standing on it, not by bouncing off it
    EXPECT_TRUE(controller->TryMove(Vec2i{ 1, 0 }));
    EXPECT_EQ(CellOf(player), (Vec2i{ 3, 2 }));
}

TEST_F(CombatTest, EmptyFloorIsStillJustWalkedOnto) {
    Entity* player = Entities()->CreateEntity("Player");
    player->AddComponent(new Transform(Vec2f{ 2.0f, 2.0f }));
    auto* controller = new PlayerController();
    player->AddComponent(controller);
    player->AddComponent(new Attacker(4, 0.35));

    EXPECT_TRUE(controller->TryMove(Vec2i{ 1, 0 }));
    EXPECT_EQ(CellOf(player), (Vec2i{ 3, 2 }));
}

// ----------------------------
// Acting While Hurt
// ----------------------------

TEST_F(CombatTest, AStunnedPlayerCannotAct) {
    Entity* player = Entities()->CreateEntity("Player");
    player->AddComponent(new Transform(Vec2f{ 2.0f, 2.0f }));
    auto* controller = new PlayerController();
    player->AddComponent(controller);
    auto* effects = new StatusEffects();
    player->AddComponent(effects);
    player->AddComponent(new Health(20));

    EXPECT_TRUE(controller->CanAct());

    effects->Apply(EffectType::Stun, 1.0);
    EXPECT_FALSE(controller->CanAct());

    effects->Clear(EffectType::Stun);
    EXPECT_TRUE(controller->CanAct());
}

TEST_F(CombatTest, ADeadPlayerCannotAct) {
    Entity* player = Entities()->CreateEntity("Player");
    player->AddComponent(new Transform(Vec2f{ 2.0f, 2.0f }));
    auto* controller = new PlayerController();
    player->AddComponent(controller);
    auto* health = new Health(20);
    health->SetDestroyOnDeath(false);
    player->AddComponent(health);

    ASSERT_TRUE(controller->CanAct());

    health->Kill();

    EXPECT_FALSE(controller->CanAct());
}

TEST_F(CombatTest, AnEnemyNextToThePlayerSwingsAtThem) {
    Entity* player = Entities()->CreateEntity("Player");
    player->AddComponent(new Transform(Vec2f{ 4.0f, 2.0f }));
    player->AddComponent(new Glyph('@', 10));
    player->AddComponent(new PlayerController());
    player->AddComponent(new StatusEffects());
    auto* playerHealth = new Health(20);
    playerHealth->SetDestroyOnDeath(false);
    player->AddComponent(playerHealth);

    Entity* enemy = SpawnFighter("Enemy", Vec2i{ 3, 2 }, 10, 3);
    auto* chaser = new Chaser();
    enemy->AddComponent(chaser);

    ASSERT_EQ(chaser->TryAttackTarget(), 3);
    EXPECT_EQ(playerHealth->GetHealth(), 17);
}

TEST_F(CombatTest, AnEnemyAcrossTheRoomCannotReach) {
    Entity* player = Entities()->CreateEntity("Player");
    player->AddComponent(new Transform(Vec2f{ 7.0f, 2.0f }));
    player->AddComponent(new PlayerController());
    player->AddComponent(new StatusEffects());
    auto* playerHealth = new Health(20);
    playerHealth->SetDestroyOnDeath(false);
    player->AddComponent(playerHealth);

    Entity* enemy = SpawnFighter("Enemy", Vec2i{ 1, 2 }, 10, 3);
    auto* chaser = new Chaser();
    enemy->AddComponent(chaser);

    EXPECT_EQ(chaser->TryAttackTarget(), 0);
    EXPECT_EQ(playerHealth->GetHealth(), 20);
}

TEST_F(CombatTest, AnEnemyThatSeesYouClosesInAndDrawsBlood) {
    Entity* player = Entities()->CreateEntity("Player");
    player->AddComponent(new Transform(Vec2f{ 7.0f, 2.0f }));
    player->AddComponent(new Glyph('@', 10));
    player->AddComponent(new PlayerController());
    player->AddComponent(new StatusEffects());
    auto* playerHealth = new Health(20);
    playerHealth->SetDestroyOnDeath(false);
    player->AddComponent(playerHealth);

    Entity* enemy = SpawnFighter("Enemy", Vec2i{ 1, 2 }, 10, 3);
    auto* chaser = new Chaser();
    enemy->AddComponent(chaser);

    Attacker* weapon = enemy->GetComponent<Attacker>();
    StatusEffects* enemyEffects = enemy->GetComponent<StatusEffects>();

    // six seconds of frames, ticking what the ComponentSystems would tick in the real loop
    for (int frame = 0; frame < 400; ++frame) {
        chaser->OnUpdate(0.016);
        weapon->OnUpdate(0.016);
        enemyEffects->OnUpdate(0.016);
        playerHealth->OnUpdate(0.016);
        player->GetComponent<StatusEffects>()->OnUpdate(0.016);
    }

    EXPECT_EQ(chaser->GetState(), ChaserState::Hunting);
    EXPECT_EQ(Pathfinding::ManhattanDistance(CellOf(enemy), CellOf(player)), 1);
    EXPECT_LT(playerHealth->GetHealth(), 20)
        << "the enemy reached the player but never landed a hit";
}

TEST_F(CombatTest, YouCanKillAnEnemyByWalkingIntoItRepeatedly) {
    Entity* player = Entities()->CreateEntity("Player");
    player->AddComponent(new Transform(Vec2f{ 2.0f, 2.0f }));
    player->AddComponent(new Glyph('@', 10));
    auto* controller = new PlayerController();
    player->AddComponent(controller);
    player->AddComponent(new StatusEffects());
    player->AddComponent(new Health(20));
    auto* weapon = new Attacker(4, 0.35);
    player->AddComponent(weapon);

    Entity* enemy = SpawnFighter("Enemy", Vec2i{ 3, 2 }, 10, 1);
    Health* enemyHealth = enemy->GetComponent<Health>();
    StatusEffects* enemyEffects = enemy->GetComponent<StatusEffects>();

    // 10 health, 4 a hit, so three connections with the cooldowns ticking between them
    for (int swing = 0; swing < 3; ++swing) {
        controller->TryMove(Vec2i{ 1, 0 });

        for (int frame = 0; frame < 40; ++frame) {
            weapon->OnUpdate(0.016);
            enemyHealth->OnUpdate(0.016);
            enemyEffects->OnUpdate(0.016);
        }
    }

    EXPECT_FALSE(enemyHealth->IsAlive());

    // and the Scene sweeps the body up
    Entities()->Update(0.016);
    EXPECT_EQ(Entities()->FindEntity("Enemy"), nullptr);
}

TEST_F(CombatTest, AStaggeredEnemyStandsStill) {
    Entity* player = Entities()->CreateEntity("Player");
    player->AddComponent(new Transform(Vec2f{ 7.0f, 2.0f }));
    player->AddComponent(new PlayerController());
    player->AddComponent(new Health(20));

    Entity* enemy = SpawnFighter("Enemy", Vec2i{ 1, 2 }, 10, 3);
    auto* chaser = new Chaser();
    enemy->AddComponent(chaser);

    ASSERT_TRUE(chaser->CanAct());
    enemy->GetComponent<StatusEffects>()->Apply(EffectType::Stun, 1.0);
    ASSERT_FALSE(chaser->CanAct());

    const Vec2i before = CellOf(enemy);
    for (int frame = 0; frame < 30; ++frame)
        chaser->OnUpdate(0.016);

    EXPECT_EQ(CellOf(enemy), before);
}
