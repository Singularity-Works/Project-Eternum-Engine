/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: SystemRegistry
* Description:
*     Manages the registration and retrieval of all systems in the Eternum Engine.
*
* Author:     Jax Clayton
* Created:    8/2/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/

#ifndef SYSTEMREGISTRY_H
#define SYSTEMREGISTRY_H

class SystemRegistry
{
    public:
        static SystemRegistry& Instance()
        {
            static SystemRegistry instance;
            return instance;
        }

        void Register(System* sys)
        {
            for (const auto& existingSys : systems) {
                if (existingSys->GetName() == sys->GetName()) {
                    return; // System already registered
                }
            }

            std::cout << "Registering system: " << sys->GetName() << std::endl;
            systems.push_back(sys);
        }

        const std::vector<System*>& GetSystems() const
        {
            return systems;
        }

    private:
        SystemRegistry() = default;
        std::vector<System*> systems;
};

static SystemRegistry* Registry()
{
    return &SystemRegistry::Instance();
}

/// @brief Registers a system type automatically.
/// This macro should be used in the system's implementation file to ensure that the system
// AutoRegisterSystem.h

#define REGISTER_SYSTEM(SystemType)                      \
namespace {                                                \
struct SystemType##AutoRegister {                      \
SystemType##AutoRegister() {                       \
SystemRegistry::Instance().Register(           \
SystemType::GetInstance().get());               \
}                                                   \
};                                                      \
static SystemType##AutoRegister s_##SystemType##AutoRegisterInstance; \
}

#define REGISTER_COMPONENT_SYSTEM(ComponentType)                          \
namespace {                                                               \
    struct ComponentSystemAutoRegister_##ComponentType {                 \
        ComponentSystemAutoRegister_##ComponentType() {                  \
            SystemRegistry::Instance().Register(                         \
                Components<ComponentType>());          \
        }                                                                \
    };                                                                   \
    static ComponentSystemAutoRegister_##ComponentType                   \
        instance_ComponentSystemAutoRegister_##ComponentType;            \
}


#endif //SYSTEMREGISTRY_H
