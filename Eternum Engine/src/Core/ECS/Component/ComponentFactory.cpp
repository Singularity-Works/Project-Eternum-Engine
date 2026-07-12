/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: ComponentFactory
* Description:
*     Builds a Component from the name a save file calls it by.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/

#include <pch.h>
#include "ComponentFactory.h"

void ComponentFactory::Register( std::string const& name, Creator creator )
{
    if ( name.empty() || creator == nullptr )
        return;

    if ( m_Creators.find( name ) != m_Creators.end() )
    {
        std::cout << "WARNING: component type \"" << name << "\" is registered twice" << std::endl;
        return;
    }

    m_Creators[ name ] = std::move( creator );
}

Component* ComponentFactory::Create( std::string const& name ) const
{
    const auto it = m_Creators.find( name );
    if ( it == m_Creators.end() )
        return nullptr;

    return it->second();
}

bool ComponentFactory::Knows( std::string const& name ) const
{
    return m_Creators.find( name ) != m_Creators.end();
}

std::vector< std::string > ComponentFactory::GetKnownNames() const
{
    std::vector< std::string > names;
    names.reserve( m_Creators.size() );

    for ( auto const& [ name, creator ] : m_Creators )
        names.push_back( name );

    return names;
}
