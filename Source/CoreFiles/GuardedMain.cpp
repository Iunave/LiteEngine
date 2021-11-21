/*
#include "Definitions.hpp"
#include "Log.hpp"
#include "Time.hpp"
#include "Rendering/VulkanRenderer.hpp"

inline float64 ProgramStartTime;
inline float64 ProgramEndTime;

CONSTRUCTOR(1) void OnProgramStart()
{
    ProgramStartTime = Time::Now();
    LOG_UNGUARDED(LogProgram, "starting at {}", ProgramStartTime);

    ::chdir("../.."); //change directory to be the Program source directory
    Thread::MainThreadID = pthread_self();
}

DESTRUCTOR(100) void OnProgramEnd()
{
    ProgramEndTime = Time::Now();
    LOG_UNGUARDED(LogProgram, "ending at {}", ProgramEndTime);
    LOG_UNGUARDED(LogProgram, "total time in execution: {} seconds", ProgramEndTime - ProgramStartTime);
}

OBJECT_CLASS(OFoo)
class OFoo : public OObject
{
    OBJECT_BASES(OObject)
public:

};

int32 main()
{
    TSharedPtr<OFoo> A{MakeShared<OFoo>()};
    TSharedPtr<OFoo> B{MakeShared<OFoo>()};
    TSharedPtr<OFoo> C{MakeShared<OFoo>()};
    TSharedPtr<OFoo> D{MakeShared<OFoo>()};

    for(TObjectIterator<OFoo> Iterator{}; Iterator; ++Iterator)
    {
        LOG(LogProgram, "{}", Iterator->GetClassName());
    }

    Render::Initialize();
    Render::Loop();
    Render::ShutDown();

    return 0;
}
*/

#include <iostream>
#include "Definitions.hpp"
#include "Array.hpp"
#include "Log.hpp"
#include "Time.hpp"

inline float64 ProgramStartTime;
inline float64 ProgramEndTime;

CONSTRUCTOR(1) void OnProgramStart()
{
    ProgramStartTime = Time::Now();
    LOG_UNGUARDED(LogProgram, "starting at {}", ProgramStartTime);
}

DESTRUCTOR(100) void OnProgramEnd()
{
    ProgramEndTime = Time::Now();
    LOG_UNGUARDED(LogProgram, "ending at {}", ProgramEndTime);
    LOG_UNGUARDED(LogProgram, "total time in execution: {} seconds", ProgramEndTime - ProgramStartTime);
}

CONSTRUCTOR(2) void InitializeVariables()
{
    Thread::MainThreadID = pthread_self();
}

enum class EMovieGenre : uint32
{
    Comedy = 0,
    Horror = 1,
    Fantasy = 2
};

void PrintMovieGenres()
{
    fmt::print("Comedy: {}\nHorror: {}\nFantasy: {}\n", EMovieGenre::Comedy, EMovieGenre::Horror, EMovieGenre::Fantasy);
}

namespace StrUtl
{
    const char8* ToString(EMovieGenre Genre)
    {
        static constexpr TStaticArray<const char8*, 3> Table{"Comedy", "Horror", "Fantasy"};
        return Table[static_cast<uint32>(Genre)];
    }
}

class FMovie final
{
public:

    FMovie() = delete;

    inline constexpr FMovie(FString<SS60> InTitle, FString<SS124> InSummary, EMovieGenre InGenre, uint32 InReleaseDate, uint32 InScreenTime, uint32 InScore)
            : Title{Move(InTitle)}
            , Summary{Move(InSummary)}
            , ReleaseDate{InReleaseDate}
            , ScreenTime{InScreenTime}
            , Genre{InGenre}
            , Score{InScore}
    {
    }

    inline constexpr bool operator==(const FString<SS60>& OtherTitle) const
    {
        return Title == OtherTitle;
    }

    FString<SS60> Information() const;
    FString<SS508> DetailedInformation() const;

    inline constexpr const FString<SS60>& GetTitle() const {return Title;}
    inline constexpr const FString<SS124>& GetSummary() const {return Summary;}
    inline constexpr uint32 GetReleaseDate() const {return ReleaseDate;}
    inline constexpr uint32 GetScreenTime() const {return ScreenTime;}
    inline constexpr EMovieGenre GetGenre() const {return Genre;}
    inline constexpr uint32 GetScore() const {return Score;}

private:

    FString<SS60> Title; //movie title
    FString<SS124> Summary; //summary of the movie plot
    uint32 ReleaseDate; //release date in year
    uint32 ScreenTime; //move length in minutes
    EMovieGenre Genre; //the genre of the movie
    uint32 Score; //how much people liked this movie (1 to 10)
};

class FUser
{
public:

    FUser(FString<SS60> InName)
            : Name{Move(InName)}
    {
    }

    void PrintSavedMovies() const;

    bool operator==(const FString<SS60>& OtherName) const
    {
        return Name == OtherName;
    }

    void AddNewMovie(const FMovie* NewMovie);

private:

    const FString<SS60> Name;
    TDynamicArray<const FMovie*> SavedMovies;
};

void FUser::PrintSavedMovies() const
{
    for(const FMovie* Movie : SavedMovies)
    {
        fmt::print(fmt::fg(fmt::color::cyan), "{}\n", Movie->Information());
    }
}

void FUser::AddNewMovie(const FMovie* NewMovie)
{
    for(const FMovie* Movie : SavedMovies)
    {
        if(Movie == NewMovie)
        {
            return;
        }
    }

    SavedMovies.Append(NewMovie);
}

inline TDynamicArray<FUser> UserArray{};

FString<SS60> FMovie::Information() const
{
    FString<SS60> ResultString{Title};
    fmt::format_to(ResultString.End(), " ({})", ReleaseDate);
    return ResultString;
}

FString<SS508> FMovie::DetailedInformation() const
{
    FString<SS508> ResultString{Title.Data(), Title.Num()};
    fmt::format_to(ResultString.End(), "\nreleased {}, screen time {} minutes, genre: {}, score {}/10\n{}", ReleaseDate, ScreenTime, StrUtl::ToString(Genre), Score, Summary.Data());
    return ResultString;
}

constexpr TStaticArray<FMovie, 3> MovieArray
        {
                FMovie{"princess pringles", "the princess eats too much pringles and has to go to the castle nurse", EMovieGenre::Comedy, 2019, 180, 6},
                FMovie{"princess jingles", "an old and ugly hag of a princess goes to a magical land to get some well needed beauty surgery", EMovieGenre::Fantasy, 2018, 134, 9},
                FMovie{"princess mingles", "the princess attends a balley and suffers from social anxiety, will she last the night or fall prey to her fears?", EMovieGenre::Horror, 2017, 210, 10},
        };

void PrintAllMovies()
{
    #pragma unroll
    for(const FMovie& Movie : MovieArray)
    {
        fmt::print(fmt::fg(fmt::color::cyan), "{}\n", Movie.Information());
    }
}

void PrintAllMoviesWithIndex()
{
    #pragma unroll
    for(int64 Index{0}; Index < MovieArray.Num(); ++Index)
    {
        fmt::print(fmt::fg(fmt::color::cyan), "{}: {}\n", Index, MovieArray[Index].Information());
    }
}

void PrintAllMovies(EMovieGenre MovieGenre)
{
    #pragma unroll
    for(const FMovie& Movie : MovieArray)
    {
        if(Movie.GetGenre() == MovieGenre)
        {
            fmt::print(fmt::fg(fmt::color::cyan), "{}\n", Movie.Information());
        }
    }
}

void PrintAllMovies(uint32 MovieScore)
{
    #pragma unroll
    for(const FMovie& Movie : MovieArray)
    {
        if(Movie.GetScore() >= MovieScore)
        {
            fmt::print(fmt::fg(fmt::color::cyan), "{}\n", Movie.Information());
        }
    }
}

FUser* FindExistingUser(FString<SS60> UserName)
{
    for(FUser& User : UserArray)
    {
        if(User == UserName)
        {
            return &User;
        }
    }
    return nullptr;
}

void PrintMainMenu()
{
    fmt::print("type \"0\" to exit the program\n"
               "type \"1\" to see all available movies\n"
               "type \"2\" to see all movies of a genre\n"
               "type \"3\" to see movies with a score equal or more than x\n"
               "type \"4\" to see detailed information about a movie\n"
               "type \"5\" to switch to or create an account\n"
               "type \"6\" to see the saved movies of your account\n"
               "type \"7\" to add a movie to your account\n");
}

void MainLoop()
{
    static FUser* ActiveUser{nullptr};
    static int32 UserChoice;

    auto CaseOne = []() -> void
    {
        PrintAllMovies();
    };

    auto CaseTwo = []() -> void
    {
        PrintMovieGenres();

        uint32 GenreChoice;
        std::cin >> GenreChoice;

        PrintAllMovies(static_cast<EMovieGenre>(GenreChoice));
    };

    auto CaseThree = []() -> void
    {
        fmt::print(fmt::fg(fmt::color::cyan), "choose a score between 1 and 10 (inclusive)\n");

        uint32 ScoreChoice;
        std::cin >> ScoreChoice;

        PrintAllMovies(ScoreChoice);
    };

    auto CaseFour = []() -> void
    {
        PrintAllMoviesWithIndex();

        uint32 MovieChoice;
        std::cin >> MovieChoice;

        if(MovieArray.IsIndexValid(MovieChoice))
        {
            fmt::print(fmt::fg(fmt::color::cyan), "{}\n", MovieArray[MovieChoice].DetailedInformation());
        }
    };

    auto CaseFive = []() -> void
    {
        fmt::print(fmt::fg(fmt::color::cyan), "type your username\n");

        std::string UserName{};
        std::cin >> UserName;

        FString<SS60> NameConverted{UserName.data(), static_cast<uint32>(StrUtl::Length(UserName.data()))};

        ActiveUser = FindExistingUser(NameConverted);

        if(ActiveUser == nullptr)
        {
            ActiveUser = UserArray + UserArray.Append(NameConverted);
        }
    };

    auto CaseSix = []() -> void
    {
        if(ActiveUser != nullptr)
        {
            ActiveUser->PrintSavedMovies();
        }
    };

    auto CaseSeven = []() -> void
    {
        if(ActiveUser != nullptr)
        {
            PrintAllMoviesWithIndex();

            uint32 MovieChoice;
            std::cin >> MovieChoice;

            ActiveUser->AddNewMovie(MovieArray + MovieChoice);
        }
    };

    constexpr TStaticArray<void(*)(), 7> Functions{CaseOne, CaseTwo, CaseThree, CaseFour, CaseFive, CaseSix, CaseSeven};

    do
    {
        PrintMainMenu();
        std::cin >> UserChoice;

        if(Functions.IsIndexValid(UserChoice - 1))
        {
            (*Functions[UserChoice - 1])();
        }
    }
    while(UserChoice != 0);
}

int32 main()
{
    MainLoop();
    return 0;
}
