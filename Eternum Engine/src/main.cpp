#include <pch.h>
#include "Core/Runtime/Runtime.h"
#include <Core/Log/Log.h>
#include <Systems/Dungeon System/DungeonSystem.h>
#include <Systems/Grid System/GridSystem.h>

// cmake passes the real one, this only covers a build put together by hand
#ifndef ETERNUM_VERSION
    #define ETERNUM_VERSION "0.0.0-dev"
#endif

namespace
{
    void PrintUsage()
    {
        std::cout << "Eternum Engine " << ETERNUM_VERSION << "\n"
                  << "  --seed <number>   build the first dungeon from a known seed\n"
                  << "  --gen <name>      bsp, rooms or cave\n"
                  << "  --shake           start with the camera shake held on\n"
                  << "  --verbose         say what the engine is doing while it starts\n"
                  << "  --version         print the version and exit\n"
                  << "  --help            show this\n"
                  << std::endl;
    }

    /// @brief  reads the command line and sets up the first dungeon
    /// @param  argc    how many arguments were given
    /// @param  argv    the arguments
    /// @return whether the engine should run, false means we only printed something
    bool ApplyArguments( const int argc, char** argv )
    {
        for ( int i = 1; i < argc; ++i )
        {
            const std::string argument = argv[ i ];
            const bool hasValue = ( i + 1 < argc );

            if ( argument == "--help" || argument == "-h" )
            {
                PrintUsage();
                return false;
            }

            if ( argument == "--seed" && hasValue )
            {
                try
                {
                    DungeonSystem::GetInstance()->SetStartupSeed(
                        static_cast< unsigned >( std::stoul( argv[ ++i ] ) ) );
                }
                catch ( std::exception const& )
                {
                    std::cout << "Ignoring --seed, \"" << argv[ i ] << "\" is not a number" << std::endl;
                }
                continue;
            }

            if ( argument == "--version" )
            {
                std::cout << "Eternum Engine " << ETERNUM_VERSION << std::endl;
                return false;
            }

            if ( argument == "--verbose" || argument == "-v" )
            {
                SetVerboseLogging( true );
                continue;
            }

            if ( argument == "--shake" )
            {
                GridSystem::GetInstance()->SetContinuousShake( true );
                continue;
            }

            if ( argument == "--gen" && hasValue )
            {
                const std::string name = argv[ ++i ];
                const std::size_t index = DungeonSystem::GetInstance()->FindGenerator( name );

                if ( index < DungeonSystem::GetInstance()->GetGeneratorCount() )
                    DungeonSystem::GetInstance()->SetGenerator( index );
                else
                    std::cout << "Ignoring --gen, no generator called \"" << name << "\"" << std::endl;

                continue;
            }

            std::cout << "Ignoring unknown argument \"" << argument << "\"" << std::endl;
        }

        return true;
    }
}

int main( int argc, char** argv )
{
    if ( !ApplyArguments( argc, argv ) )
        return 0;

    RuntimeSystem()->Run();

    // Leave the console open until the user presses a key
    std::cout << "Press any key to exit..." << std::endl;
    std::cin.get();
    return 0;
}
