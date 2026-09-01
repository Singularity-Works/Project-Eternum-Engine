/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: EventTests
* Description:
*     Tests the dispatcher, then the two things built on it. The dispatcher cases that
*     matter are the awkward ones: a handler that unsubscribes while it is running, a
*     handler that posts another event, and posting something whose subject is destroyed
*     before delivery.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#include <gtest/gtest.h>

#include <Core/ECS/Component/Glyph/Glyph.h>
#include <Core/ECS/Component/Health/Health.h>
#include <Core/ECS/Component/Pickup/Pickup.h>
#include <Core/ECS/Component/StatusEffects/StatusEffects.h>
#include <Core/ECS/Component/Transform/Transform.h>
#include <Core/ECS/Component/Trap/Trap.h>
#include <Core/ECS/Entity/Entity.h>
#include <Systems/Entity System/EntitySystem.h>
#include <Systems/Event/GameEvents.h>
#include <Systems/Grid System/GridSystem.h>
#include <Systems/Pathfinding/PathfindingSystem.h>

namespace {
    // a couple of events that exist only for these tests
    struct PingEvent final : Event { int m_Value = 0; };
    struct PongEvent final : Event { int m_Value = 0; };
}

class EventTest : public ::testing::Test {
protected:
    void SetUp() override {
        Events()->ClearQueue();
        Events()->ClearListeners();

        Entities()->ClearEntities();
        GridSystem::GetInstance()->ClearMaps();

        // a 9x5 room, open floor inside a solid border
        GridSystem::GetInstance()->CreateMap("Room", GridSystem::Dimension(9, 5), '#');
        GridSystem::GetInstance()->LoadMap("Room");
        for (int y = 1; y <= 3; ++y)
            for (int x = 1; x <= 7; ++x)
                GridSystem::GetInstance()->SetCell(x, y, '.');

        Paths()->InvalidateNavGrid();
    }

    void TearDown() override {
        Events()->ClearQueue();
        Events()->ClearListeners();

        Entities()->ClearEntities();
        GridSystem::GetInstance()->ClearMaps();
        Paths()->InvalidateNavGrid();
    }

    static Entity* SpawnVictim(const std::string& name, const Vec2i& cell, const int health) {
        Entity* entity = Entities()->CreateEntity(name);
        entity->AddComponent(new Transform(Vec2f{ static_cast<float>(cell.x()),
                                                  static_cast<float>(cell.y()) }));
        entity->AddComponent(new Glyph('@', 10));
        entity->AddComponent(new StatusEffects());

        auto* hp = new Health(health);
        hp->SetDestroyOnDeath(false);
        entity->AddComponent(hp);

        return entity;
    }
};

// ----------------------------
// The Dispatcher
// ----------------------------

TEST_F(EventTest, ASubscriberHearsWhatItAskedFor) {
    int heard = 0;

    Events()->Subscribe<PingEvent>([&heard](PingEvent const& event) { heard = event.m_Value; });

    PingEvent ping;
    ping.m_Value = 42;
    Events()->Send(ping);

    EXPECT_EQ(heard, 42);
}

TEST_F(EventTest, ASubscriberHearsNothingElse) {
    int pings = 0;

    Events()->Subscribe<PingEvent>([&pings](PingEvent const&) { ++pings; });

    Events()->Send(PongEvent{});

    EXPECT_EQ(pings, 0);
}

TEST_F(EventTest, EverySubscriberHearsIt) {
    int first = 0;
    int second = 0;

    Events()->Subscribe<PingEvent>([&first](PingEvent const&) { ++first; });
    Events()->Subscribe<PingEvent>([&second](PingEvent const&) { ++second; });

    Events()->Send(PingEvent{});

    EXPECT_EQ(first, 1);
    EXPECT_EQ(second, 1);
}

TEST_F(EventTest, SendingWithNobodyListeningIsHarmless) {
    Events()->Send(PingEvent{});
    SUCCEED();
}

TEST_F(EventTest, UnsubscribingStopsTheHandler) {
    int heard = 0;

    const EventSystem::Token token =
        Events()->Subscribe<PingEvent>([&heard](PingEvent const&) { ++heard; });

    EXPECT_TRUE(Events()->IsSubscribed(token));
    Events()->Send(PingEvent{});
    ASSERT_EQ(heard, 1);

    Events()->Unsubscribe(token);

    EXPECT_FALSE(Events()->IsSubscribed(token));
    Events()->Send(PingEvent{});
    EXPECT_EQ(heard, 1);
}

TEST_F(EventTest, UnsubscribingMidDeliveryStopsTheOneBeingRemoved) {
    int secondHeard = 0;
    EventSystem::Token secondToken = EventSystem::NO_TOKEN;

    // the first handler pulls the second one out before it gets its turn
    Events()->Subscribe<PingEvent>([&secondToken](PingEvent const&) {
        Events()->Unsubscribe(secondToken);
    });

    secondToken = Events()->Subscribe<PingEvent>(
        [&secondHeard](PingEvent const&) { ++secondHeard; });

    Events()->Send(PingEvent{});

    // calling it anyway is how a handler belonging to a destroyed object gets run
    EXPECT_EQ(secondHeard, 0);
}

TEST_F(EventTest, AHandlerMaySubscribeWhileItRuns) {
    int later = 0;

    Events()->Subscribe<PingEvent>([&later](PingEvent const&) {
        Events()->Subscribe<PingEvent>([&later](PingEvent const&) { ++later; });
    });

    // the new listener joins after this delivery has already been decided
    Events()->Send(PingEvent{});
    EXPECT_EQ(later, 0);

    Events()->Send(PingEvent{});
    EXPECT_GT(later, 0);
}

// ----------------------------
// Queued Delivery
// ----------------------------

TEST_F(EventTest, APostedEventWaitsForDelivery) {
    int heard = 0;
    Events()->Subscribe<PingEvent>([&heard](PingEvent const&) { ++heard; });

    Events()->Post(PingEvent{});

    EXPECT_EQ(heard, 0);
    EXPECT_EQ(Events()->GetQueuedCount(), 1u);

    Events()->DeliverQueued();

    EXPECT_EQ(heard, 1);
    EXPECT_EQ(Events()->GetQueuedCount(), 0u);
}

TEST_F(EventTest, PostedEventsArriveInTheOrderTheyWereSent) {
    std::vector<int> order;
    Events()->Subscribe<PingEvent>([&order](PingEvent const& event) {
        order.push_back(event.m_Value);
    });

    for (int value = 1; value <= 3; ++value) {
        PingEvent ping;
        ping.m_Value = value;
        Events()->Post(ping);
    }

    Events()->DeliverQueued();

    ASSERT_EQ(order.size(), 3u);
    EXPECT_EQ(order[0], 1);
    EXPECT_EQ(order[1], 2);
    EXPECT_EQ(order[2], 3);
}

TEST_F(EventTest, AHandlerThatPostsDoesNotLoopForever) {
    int deliveries = 0;

    Events()->Subscribe<PingEvent>([&deliveries](PingEvent const&) {
        ++deliveries;

        // posting from inside a delivery has to wait for the next pass, otherwise this
        // would never return
        if (deliveries < 3)
            Events()->Post(PingEvent{});
    });

    Events()->Post(PingEvent{});

    Events()->DeliverQueued();
    EXPECT_EQ(deliveries, 1);

    Events()->DeliverQueued();
    EXPECT_EQ(deliveries, 2);
}

TEST_F(EventTest, ClearingTheQueueThrowsAwayWhatWasWaiting) {
    int heard = 0;
    Events()->Subscribe<PingEvent>([&heard](PingEvent const&) { ++heard; });

    Events()->Post(PingEvent{});
    Events()->ClearQueue();
    Events()->DeliverQueued();

    EXPECT_EQ(heard, 0);
}

// ----------------------------
// Combat Announcements
// ----------------------------

TEST_F(EventTest, TakingDamageIsAnnounced) {
    Entity* victim = SpawnVictim("Victim", Vec2i{ 2, 2 }, 20);

    int amount = 0;
    Entity* target = nullptr;
    double fraction = 0.0;

    Events()->Subscribe<DamageTakenEvent>([&](DamageTakenEvent const& event) {
        amount = event.m_Amount;
        target = event.m_Target;
        fraction = event.m_Fraction;
    });

    victim->GetComponent<Health>()->TakeDamage(5);

    EXPECT_EQ(amount, 5);
    EXPECT_EQ(target, victim);
    EXPECT_NEAR(fraction, 0.25, 0.001);
}

TEST_F(EventTest, DyingIsAnnouncedOnceDelivered) {
    Entity* victim = SpawnVictim("Victim", Vec2i{ 2, 2 }, 5);

    int deaths = 0;
    Events()->Subscribe<EntityDiedEvent>([&deaths](EntityDiedEvent const&) { ++deaths; });

    victim->GetComponent<Health>()->Kill();

    // death is posted, not sent, because listeners tend to look at an Entity that is
    // about to be swept up
    EXPECT_EQ(deaths, 0);

    Events()->DeliverQueued();
    EXPECT_EQ(deaths, 1);
}

// ----------------------------
// Traps
// ----------------------------

TEST_F(EventTest, ATrapGoesOffWhenSomethingStandsOnIt) {
    Entity* victim = SpawnVictim("Victim", Vec2i{ 3, 2 }, 20);

    Entity* trapEntity = Entities()->CreateEntity("Trap");
    trapEntity->AddComponent(new Transform(Vec2f{ 3.0f, 2.0f }));
    trapEntity->AddComponent(new Glyph(Trap::HIDDEN_SYMBOL, 1));
    auto* trap = new Trap(5);
    trapEntity->AddComponent(trap);

    ASSERT_TRUE(trap->IsArmed());
    ASSERT_TRUE(trap->IsHidden());

    trap->OnUpdate(0.016);

    EXPECT_FALSE(trap->IsArmed());
    EXPECT_FALSE(trap->IsHidden());
    EXPECT_EQ(victim->GetComponent<Health>()->GetHealth(), 15);

    // and it stops looking like floor once it has been found
    EXPECT_EQ(trapEntity->GetComponent<Glyph>()->GetSymbol(), Trap::REVEALED_SYMBOL);
}

TEST_F(EventTest, ATrapOnlyGoesOffOnce) {
    Entity* victim = SpawnVictim("Victim", Vec2i{ 3, 2 }, 40);
    victim->GetComponent<Health>()->SetMercyWindow(0.0);

    Entity* trapEntity = Entities()->CreateEntity("Trap");
    trapEntity->AddComponent(new Transform(Vec2f{ 3.0f, 2.0f }));
    trapEntity->AddComponent(new Glyph(Trap::HIDDEN_SYMBOL, 1));
    auto* trap = new Trap(5);
    trapEntity->AddComponent(trap);

    for (int frame = 0; frame < 10; ++frame)
        trap->OnUpdate(0.016);

    EXPECT_EQ(victim->GetComponent<Health>()->GetHealth(), 35);
}

TEST_F(EventTest, ATrapAnnouncesItselfOnceDelivered) {
    SpawnVictim("Victim", Vec2i{ 3, 2 }, 20);

    Entity* trapEntity = Entities()->CreateEntity("Trap");
    trapEntity->AddComponent(new Transform(Vec2f{ 3.0f, 2.0f }));
    trapEntity->AddComponent(new Glyph(Trap::HIDDEN_SYMBOL, 1));
    auto* trap = new Trap(5);
    trapEntity->AddComponent(trap);

    std::string message;
    Events()->Subscribe<TrapTriggeredEvent>([&message](TrapTriggeredEvent const& event) {
        message = event.m_Message;
    });

    trap->Trigger(Entities()->FindEntity("Victim"));
    Events()->DeliverQueued();

    EXPECT_FALSE(message.empty());
}

TEST_F(EventTest, ATrapWithNothingOnItStaysArmed) {
    Entity* trapEntity = Entities()->CreateEntity("Trap");
    trapEntity->AddComponent(new Transform(Vec2f{ 6.0f, 2.0f }));
    trapEntity->AddComponent(new Glyph(Trap::HIDDEN_SYMBOL, 1));
    auto* trap = new Trap(5);
    trapEntity->AddComponent(trap);

    for (int frame = 0; frame < 30; ++frame)
        trap->OnUpdate(0.016);

    EXPECT_TRUE(trap->IsArmed());
    EXPECT_TRUE(trap->IsHidden());
}

TEST_F(EventTest, ATrapCatchesAnythingNotJustThePlayer) {
    Entity* enemy = SpawnVictim("Enemy", Vec2i{ 3, 2 }, 20);

    Entity* trapEntity = Entities()->CreateEntity("Trap");
    trapEntity->AddComponent(new Transform(Vec2f{ 3.0f, 2.0f }));
    trapEntity->AddComponent(new Glyph(Trap::HIDDEN_SYMBOL, 1));
    auto* trap = new Trap(5);
    trapEntity->AddComponent(trap);

    trap->OnUpdate(0.016);

    EXPECT_LT(enemy->GetComponent<Health>()->GetHealth(), 20);
}

// ----------------------------
// Pickups
// ----------------------------

TEST_F(EventTest, APickupHealsWhoeverWalksOverIt) {
    Entity* walker = SpawnVictim("Walker", Vec2i{ 4, 2 }, 20);
    walker->GetComponent<Health>()->TakeDamage(10);
    ASSERT_EQ(walker->GetComponent<Health>()->GetHealth(), 10);

    Entity* pickupEntity = Entities()->CreateEntity("Pickup");
    pickupEntity->AddComponent(new Transform(Vec2f{ 4.0f, 2.0f }));
    pickupEntity->AddComponent(new Glyph(Pickup::SYMBOL, 2));
    auto* pickup = new Pickup(6);
    pickupEntity->AddComponent(pickup);

    pickup->OnUpdate(0.016);

    EXPECT_EQ(walker->GetComponent<Health>()->GetHealth(), 16);
    EXPECT_TRUE(pickup->IsCollected());

    // and it leaves the Scene once the sweep runs
    Entities()->Update(0.016);
    EXPECT_EQ(Entities()->FindEntity("Pickup"), nullptr);
}

TEST_F(EventTest, AFullHealthWalkerLeavesThePickupAlone) {
    SpawnVictim("Walker", Vec2i{ 4, 2 }, 20);

    Entity* pickupEntity = Entities()->CreateEntity("Pickup");
    pickupEntity->AddComponent(new Transform(Vec2f{ 4.0f, 2.0f }));
    pickupEntity->AddComponent(new Glyph(Pickup::SYMBOL, 2));
    auto* pickup = new Pickup(6);
    pickupEntity->AddComponent(pickup);

    pickup->OnUpdate(0.016);

    // wasting it on someone who cannot use it would be worse than leaving it there
    EXPECT_FALSE(pickup->IsCollected());
    EXPECT_NE(Entities()->FindEntity("Pickup"), nullptr);
}

TEST_F(EventTest, APickupAnnouncesItselfBeforeItIsSweptUp) {
    Entity* walker = SpawnVictim("Walker", Vec2i{ 4, 2 }, 20);
    walker->GetComponent<Health>()->TakeDamage(10);

    Entity* pickupEntity = Entities()->CreateEntity("Pickup");
    pickupEntity->AddComponent(new Transform(Vec2f{ 4.0f, 2.0f }));
    pickupEntity->AddComponent(new Glyph(Pickup::SYMBOL, 2));
    auto* pickup = new Pickup(6);
    pickupEntity->AddComponent(pickup);

    int restored = 0;
    Events()->Subscribe<PickupCollectedEvent>([&restored](PickupCollectedEvent const& event) {
        restored = event.m_Restored;
    });

    pickup->OnUpdate(0.016);

    // the Entity is destroyed on collection, so the event has to survive that
    Entities()->Update(0.016);
    Events()->DeliverQueued();

    EXPECT_EQ(restored, 6);
}
