/*******************************************************************************************
* Project Eternum Engine
* -----------------------------------------------------------------------------------------
* File: SaveSystem
* Description:
*     Writes the whole game out and reads it back.
*
*     The snapshot is built once as a json object and then written twice, as readable text
*     and as MessagePack. They hold exactly the same data, which is the point: one is for
*     a person to open, the other is for the game to load, and the binary is a good deal
*     smaller. Loading prefers the binary and falls back to the text.
*
* Author:     Jax Clayton
* Created:    9/19/2025
* License:    MIT License (see LICENSE file in project root)
*******************************************************************************************/
#ifndef SAVESYSTEM_H
#define SAVESYSTEM_H

#include <pch.h>
#include <Systems/system.h>
#include <nlohmann/json_fwd.hpp>

class SaveSystem final : public System
{

public:

    /// @brief  what happened the last time a save or load was asked for
    struct Result
    {
        bool m_Succeeded = false;
        std::string m_Message;

        /// @brief  how many bytes each format took, zero when it was not written
        std::size_t m_JsonBytes = 0;
        std::size_t m_BinaryBytes = 0;
    };

    // ----------------------------------------------------------------
    // Lifecycle
    // ----------------------------------------------------------------

    void Init() override;
    void Update( double deltaTime ) override;
    void FixedUpdate() override;
    void Render() override;
    void Shutdown() override;

    // ----------------------------------------------------------------
    // Saving and Loading
    // ----------------------------------------------------------------

    /// @brief  writes the game to both save files
    /// @return what happened, for the panel to report
    Result Save();

    /// @brief  reads the game back, binary first and text as a fallback
    /// @return what happened, for the panel to report
    Result Load();

    /// @brief  builds the snapshot without writing it anywhere
    /// @return the whole game as json
    nlohmann::json BuildSnapshot() const;

    /// @brief  puts a snapshot back into the running game
    /// @param  snapshot    the snapshot to apply
    /// @return whether it could be applied
    bool ApplySnapshot( nlohmann::json const& snapshot );

    // ----------------------------------------------------------------
    // Accessors
    // ----------------------------------------------------------------

    /// @brief  what happened last time, for the panel
    /// @return the last result
    Result const& GetLastResult() const { return m_LastResult; }

    /// @brief  where the readable save is written
    static constexpr const char* JSON_PATH = "eternum-save.json";

    /// @brief  where the binary save is written
    static constexpr const char* BINARY_PATH = "eternum-save.msgpack";

    /// @brief  the snapshot format, bumped whenever the shape changes
    static constexpr int FORMAT_VERSION = 1;

    // ----------------------------------------------------------------
    // Singleton Pattern
    // ----------------------------------------------------------------

    static std::shared_ptr< SaveSystem > GetInstance()
    {
        static std::shared_ptr< SaveSystem > instance( new SaveSystem() );
        return instance;
    }

private:

    SaveSystem(); // Private constructor

    /// @brief  what happened the last time a save or load was asked for
    Result m_LastResult;

};

// Static SaveSystem instance call
inline SaveSystem* Saves()
{
    return SaveSystem::GetInstance().get();
}

// Register the SaveSystem with the SystemRegistry
REGISTER_SYSTEM(SaveSystem)

#endif //SAVESYSTEM_H
