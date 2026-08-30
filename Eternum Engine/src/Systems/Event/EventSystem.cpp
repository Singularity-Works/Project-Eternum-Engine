/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: EventSystem
* Description:
*     Lets one part of the engine tell the rest that something happened.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/

#include <pch.h>
#include "EventSystem.h"

EventSystem::EventSystem()
    : System( "Event System" )
{}

// ----------------------------------------------------------------
// Lifecycle
// ----------------------------------------------------------------

void EventSystem::Init()
{
    System::Init();
}

void EventSystem::Update( double deltaTime )
{
    DeliverQueued();
}

void EventSystem::FixedUpdate()
{
}

void EventSystem::Render()
{
}

void EventSystem::Shutdown()
{
    ClearQueue();
    ClearListeners();

    System::Shutdown();
}

// ----------------------------------------------------------------
// Listening
// ----------------------------------------------------------------

void EventSystem::Unsubscribe( const Token token )
{
    if ( token == NO_TOKEN )
        return;

    m_Active.erase( token );

    for ( auto& [ type, listeners ] : m_Listeners )
    {
        listeners.erase(
            std::remove_if( listeners.begin(), listeners.end(),
                [ token ]( Listener const& listener ) { return listener.m_Token == token; } ),
            listeners.end() );
    }
}

bool EventSystem::IsSubscribed( const Token token ) const
{
    return m_Active.find( token ) != m_Active.end();
}

void EventSystem::ClearListeners()
{
    m_Listeners.clear();
    m_Active.clear();
}

// ----------------------------------------------------------------
// Sending
// ----------------------------------------------------------------

void EventSystem::DeliverQueued()
{
    // take the queue, a handler is allowed to post more and those wait for the next pass
    std::vector< Queued > delivering;
    delivering.swap( m_Queue );

    for ( Queued const& queued : delivering )
    {
        if ( queued.m_Event != nullptr )
            dispatch( queued.m_Type, *queued.m_Event );
    }
}

void EventSystem::ClearQueue()
{
    m_Queue.clear();
}

void EventSystem::dispatch( const std::type_index type, Event const& event )
{
    const auto it = m_Listeners.find( type );
    if ( it == m_Listeners.end() )
        return;

    // walk a copy, a handler is allowed to subscribe or unsubscribe while it runs
    const std::vector< Listener > listeners = it->second;

    for ( Listener const& listener : listeners )
    {
        // something earlier in this same delivery may have removed this listener, and
        // calling it then would run a handler belonging to an object that is already gone
        if ( !IsSubscribed( listener.m_Token ) )
            continue;

        if ( listener.m_Handler )
            listener.m_Handler( event );
    }
}
