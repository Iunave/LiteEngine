#include <iostream>
#include "Definitions.hpp"
#include "Log.hpp"
#include "Time.hpp"

inline float64 ProgramStartTime;
inline float64 ProgramEndTime;

CONSTRUCTOR(1) inline void OnProgramStart()
{
    ProgramStartTime = Time::Now();
    LOG_UNGUARDED(LogProgram, "starting at {}", ProgramStartTime);
}

DESTRUCTOR(100) inline void OnProgramEnd()
{
    ProgramEndTime = Time::Now();
    LOG_UNGUARDED(LogProgram, "ending at {}", ProgramEndTime);
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

    inline constexpr FMovie(FString<SS60> InTitle, uint32 InReleaseDate, uint32 InScreenTime)
        : Title{Move(InTitle)}
        , ReleaseDate{InReleaseDate}
        , ScreenTime{InScreenTime}
    {
    }

    inline bool operator==(const FString<SS60>& OtherTitle) const
    {
        return Title == OtherTitle;
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

constexpr TStaticArray<FMovie, 3> MovieArray
{
    FMovie{"princess pringles", 2019, 180},
    FMovie{"princess jingles", 2018, 134},
    FMovie{"princess mingles", 2017, 210},
};

void PrintUserAlternatives()
{
    fmt::print("type \"0\" to exit the program\ntype \"1\" to see information about a movie\n");
}

void MainLoop()
{
    int32 UserChoice;

    do
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
    while(UserChoice != 0);
}

int32 main()
{
    FMovie Movie{"princess pringles", 2019, 180};
    fmt::print("{}\n", Movie.Information());

    MainLoop();

    return 0;
}
