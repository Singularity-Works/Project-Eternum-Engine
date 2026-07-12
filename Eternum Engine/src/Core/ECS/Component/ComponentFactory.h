/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: ComponentFactory
* Description:
*     Builds a Component from the name a save file calls it by.
*
*     Saving a Component is easy, it knows its own type. Loading one is not, because all the
*     file has is a string. This is the lookup that turns that string back into an object.
*     Each Component type registers itself with REGISTER_COMPONENT in its own cpp file.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#ifndef COMPONENTFACTORY_H
#define COMPONENTFACTORY_H

#include <pch.h>
#include <Core/ECS/Component/Component.h>

class ComponentFactory
{

public:

    /// @brief  makes one Component of a particular type
    using Creator = std::function< Component*() >;

    //-----------------------------------------------------------------------------
    // Public Methods
    //-----------------------------------------------------------------------------

    /// @brief  teaches the factory how to build one type
    /// @param  name    the name the type is saved under
    /// @param  creator what to call to build one
    void Register( std::string const& name, Creator creator );

    /// @brief  builds a Component by name
    /// @param  name    the name to build
    /// @return a new Component, or nullptr when the name is not known
    Component* Create( std::string const& name ) const;

    /// @brief  whether the factory knows how to build a name
    /// @param  name    the name to look for
    /// @return whether it can be built
    bool Knows( std::string const& name ) const;

    /// @brief  every name the factory can build
    /// @return the known names, in order
    std::vector< std::string > GetKnownNames() const;

    //-----------------------------------------------------------------------------
    // Singleton Pattern
    //-----------------------------------------------------------------------------

    static ComponentFactory& Instance()
    {
        static ComponentFactory instance;
        return instance;
    }

    ComponentFactory( ComponentFactory const& ) = delete;
    ComponentFactory& operator=( ComponentFactory const& ) = delete;

private:

    ComponentFactory() = default;

    /// @brief  what the factory knows how to build
    std::map< std::string, Creator > m_Creators;

};

/// @brief  Teaches the ComponentFactory how to build one Component type.
/// @param  Type    the Component type, which must be default constructible
/// @note   put this in the type's cpp file, next to its other definitions. Without it a
///         Component saves fine and then cannot be loaded back.
#define REGISTER_COMPONENT(Type)                                                    \
namespace {                                                                         \
    struct Type##FactoryRegistrar                                                   \
    {                                                                               \
        Type##FactoryRegistrar()                                                    \
        {                                                                           \
            ComponentFactory::Instance().Register(                                  \
                #Type, []() -> Component* { return new Type(); } );                 \
        }                                                                           \
    };                                                                              \
    static Type##FactoryRegistrar s_##Type##FactoryRegistrar;                       \
}

#endif //COMPONENTFACTORY_H
