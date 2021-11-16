#include <iostream>
#include "Definitions.hpp"
#include "Log.hpp"
#include "Time.hpp"

inline float64 ProgramStartTime;
inline float64 ProgramEndTime;

CONSTRUCTOR(1) inline void OnProgramStart()
{
    ProgramStartTime = Time::Now();
    LOG_UNGUARDED(LogProgram, "starting at {} seconds", ProgramStartTime);
}

DESTRUCTOR(100) inline void OnProgramEnd()
{
    ProgramEndTime = Time::Now();
    LOG_UNGUARDED(LogProgram, "ending at {} second", ProgramEndTime);
    LOG_UNGUARDED(LogProgram, "total time in execution: {} seconds", ProgramEndTime - ProgramStartTime);
}

void InitializeVariables()
{
    Thread::MainThreadID = pthread_self();
}

class FMovie final
{
public:

    FMovie() = delete;

    inline FMovie(FString<SS60> InTitle, uint32 InReleaseDate, uint32 InScreenTime)
        : Title{Move(InTitle)}
        , ReleaseDate{InReleaseDate}
        , ScreenTime{InScreenTime}
    {
    }

    FString<SS60> Information() const;

private:

    FString<SS60> Title; //movie title
    uint32 ReleaseDate; //release date in year
    uint32 ScreenTime; //move length in minutes
};

FString<SS60> FMovie::Information() const
{
    FString<SS60> ResultString{Title};
    fmt::format_to(ResultString.End(), " ({})", ReleaseDate);
    return ResultString;
}

void PrintUserAlternatives()
{
    fmt::print("type \"0\" to exit the program\n type \"1\" to see information about a movie\n");
}

void MainLoop()
{
    int32 UserChoice{-1};

    while(UserChoice != 0)
    {
        PrintUserAlternatives();
        std::cin >> UserChoice;

        switch(UserChoice)
        {
            case 1:
            {

            }
            case 2:
            {

            }
            case 3:
            {

            }
            case 4:
            {

            }
        }
    }
}

int32 main()
{
    FMovie Movie{"princess pringles", 2019, 180};
    fmt::print("{}\n", Movie.Information());

    MainLoop();

    return 0;
}
