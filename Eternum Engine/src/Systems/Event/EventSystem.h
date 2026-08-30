/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: EventSystem
* Description:
*     Lets one part of the engine tell the rest that something happened without knowing who
*     is listening.
*
*     Events are ordinary types rather than strings, so subscribing to the wrong thing is a
*     compile error instead of a quiet nothing. Listeners are keyed by the event type.
*
*     There are two ways to send. Send delivers straight away, which is what you want when
*     the answer matters now. Post queues it until the end of the frame, which is what you
*     want when the handler might destroy something, for the same reason Entities are only
*     swept up once a frame rather than mid loop.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#ifndef EVENTSYSTEM_H
#define EVENTSYSTEM_H

#include <pch.h>
#include <Systems/system.h>

#include <set>

/// @brief  Base class for anything that can be sent through the EventSystem.
struct Event
{
    virtual ~Event() = default;
};

class EventSystem final : public System
{

public:

    /// @brief  what Subscribe hands back, needed to stop listening again
    using Token = unsigned;

    /// @brief  the token that never refers to a listener
    static constexpr Token NO_TOKEN = 0;

    // ----------------------------------------------------------------
    // Lifecycle
    // ----------------------------------------------------------------

    void Init() override;
    void Update( double deltaTime ) override;
    void FixedUpdate() override;
    void Render() override;
    void Shutdown() override;

    // ----------------------------------------------------------------
    // Listening
    // ----------------------------------------------------------------

    /// @brief  starts listening for one kind of event
    /// @tparam EventType   the event to listen for
    /// @param  handler     what to run when one arrives
    /// @return a token, keep it so you can stop listening later
    template < typename EventType >
    Token Subscribe( std::function< void( EventType const& ) > handler )
    {
        static_assert( std::is_base_of_v< Event, EventType >,
                       "only types derived from Event can be subscribed to" );

        if ( handler == nullptr )
            return NO_TOKEN;

        const Token token = ++m_NextToken;

        m_Listeners[ std::type_index( typeid( EventType ) ) ].push_back(
            Listener{ token, [ handler ]( Event const& base )
            {
                handler( static_cast< EventType const& >( base ) );
            } } );

        m_Active.insert( token );
        return token;
    }

    /// @brief  stops listening
    /// @param  token   the token Subscribe handed back
    void Unsubscribe( Token token );

    /// @brief  whether a token still refers to a live listener
    /// @param  token   the token to check
    /// @return whether it is still subscribed
    bool IsSubscribed( Token token ) const;

    /// @brief  how many listeners are waiting for one kind of event
    /// @tparam EventType   the event to count listeners for
    /// @return the listener count
    template < typename EventType >
    std::size_t GetListenerCount() const
    {
        const auto it = m_Listeners.find( std::type_index( typeid( EventType ) ) );
        return ( it != m_Listeners.end() ) ? it->second.size() : 0;
    }

    // ----------------------------------------------------------------
    // Sending
    // ----------------------------------------------------------------

    /// @brief  delivers an event right now, before this call returns
    /// @tparam EventType   the event being sent
    /// @param  event       the event to deliver
    /// @warning a handler that destroys something will do it mid frame, use Post instead
    template < typename EventType >
    void Send( EventType const& event )
    {
        static_assert( std::is_base_of_v< Event, EventType >,
                       "only types derived from Event can be sent" );

        dispatch( std::type_index( typeid( EventType ) ), event );
    }

    /// @brief  queues an event until the end of the frame
    /// @tparam EventType   the event being sent
    /// @param  event       the event to deliver later
    template < typename EventType >
    void Post( EventType const& event )
    {
        static_assert( std::is_base_of_v< Event, EventType >,
                       "only types derived from Event can be sent" );

        m_Queue.push_back( Queued{ std::type_index( typeid( EventType ) ),
                                   std::make_unique< EventType >( event ) } );
    }

    /// @brief  delivers everything that was posted
    /// @note   called once a frame, exposed so a test can drive it
    void DeliverQueued();

    /// @brief  throws away anything posted but not yet delivered
    void ClearQueue();

    /// @brief  forgets every listener, used when tearing down
    void ClearListeners();

    /// @brief  how many events are waiting to be delivered
    /// @return the queue length
    std::size_t GetQueuedCount() const { return m_Queue.size(); }

    // ----------------------------------------------------------------
    // Singleton Pattern
    // ----------------------------------------------------------------

    static std::shared_ptr< EventSystem > GetInstance()
    {
        static std::shared_ptr< EventSystem > instance( new EventSystem() );
        return instance;
    }

private:

    EventSystem(); // Private constructor

    /// @brief  one thing waiting to hear about an event
    struct Listener
    {
        Token m_Token = NO_TOKEN;
        std::function< void( Event const& ) > m_Handler;
    };

    /// @brief  one event waiting to be delivered
    struct Queued
    {
        std::type_index m_Type;
        std::unique_ptr< Event > m_Event;
    };

    /// @brief  hands an event to everything listening for its type
    /// @param  type    which event type
    /// @param  event   the event itself
    void dispatch( std::type_index type, Event const& event );

    /// @brief  who is listening for what
    std::map< std::type_index, std::vector< Listener > > m_Listeners;

    /// @brief  the tokens still subscribed, so a listener removed mid delivery is skipped
    std::set< Token > m_Active;

    /// @brief  events posted but not yet delivered
    std::vector< Queued > m_Queue;

    /// @brief  the last token handed out
    Token m_NextToken = NO_TOKEN;

};

// Static EventSystem instance call
inline EventSystem* Events()
{
    return EventSystem::GetInstance().get();
}

// Register the EventSystem with the SystemRegistry
REGISTER_SYSTEM(EventSystem)

#endif //EVENTSYSTEM_H
