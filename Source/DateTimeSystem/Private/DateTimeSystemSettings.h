// [TEMPLATE_COPYRIGHT]

#pragma once

#include "DateTimeCommonCore.h"
#include "DateTimeSystem/Public/DateTimeSystemComponent.h"
#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "UObject/SoftObjectPath.h"

#include "DateTimeSystemSettings.generated.h"

/**
 * Settings for a loading screen system.
 */
UCLASS(config = Game, defaultconfig, meta = (DisplayName = "Date Time System Setting"))
class UDateTimeSystemSettings : public UDeveloperSettingsBackedByCVars
{
    GENERATED_BODY()

public:
    UDateTimeSystemSettings();

public:
    UPROPERTY(config, EditAnywhere, Category = "Meta Config")
    TSubclassOf<UDateTimeSystemCore> CoreClass = UDateTimeSystemCore::StaticClass();

    /**
     * Length of a Day in Seconds
     * This refers to both Solar and Calendar days
     */
    UPROPERTY(config, EditAnywhere, Category = "Planetary Config", meta = (ForceUnits = s))
    float LengthOfDay = 86400.f;

    /**
     *
     */
    UPROPERTY(config, EditAnywhere, Category = "Meta Config", meta = (ConsoleVariable = "DateTimeSystem.CanEverTick"))
    bool CanEverTick = false;

    UPROPERTY(config, EditAnywhere, Category = "Planetary Config")
    float DaysInOrbitalYear = 365.25f;

    UPROPERTY(config, EditAnywhere, Category = "Planetary Config", meta = (MetaClass = "/Script/Engine.DataTable"))
    FSoftObjectPath YearBookTable;

    UPROPERTY(config, EditAnywhere, Category = "Planetary Config", meta = (MetaClass = "/Script/Engine.DataTable"))
    FSoftObjectPath DateOverridesTable;

    UPROPERTY(config, EditAnywhere, Category = "Meta Config")
    bool UseDayIndexForOverride = false;

    UPROPERTY(config, EditAnywhere, Category = "Planetary Config", meta = (ForceUnits = km))
    float PlanetRadius = 6360.f;

    UPROPERTY(config, EditAnywhere, Category = "Planetary Config", meta = (ForceUnits = deg))
    float ReferenceLatitude;

    UPROPERTY(config, EditAnywhere, Category = "Planetary Config", meta = (ForceUnits = deg))
    float ReferenceLongitude;

    UPROPERTY(config, EditAnywhere, Category = "Date Config")
    FDateTimeSystemStruct StartDate;

    UPROPERTY(config, EditAnywhere, Category = "Date Config", meta = (ConsoleVariable = "DateTimeSystem.TimeScale"))
    float TimeScale;

    UPROPERTY(config, EditAnywhere, Category = "Meta Config", meta = (ConsoleVariable = "DateTimeSystem.TickStride"))
    int TickStride;

    UPROPERTY(config, EditAnywhere, Category = "Date Config")
    int DaysInWeek = 7;

    UPROPERTY(config, EditAnywhere, Category = "Meta Config")
    bool OverridedDatesSetDate = false;

    //// How long to hold the loading screen up after other loading finishes (in seconds) to
    //// try to give texture streaming a chance to avoid blurriness
    ////
    //// Note: This is not normally applied in the editor for iteration time, but can be
    //// enabled via HoldLoadingScreenAdditionalSecsEvenInEditor
    //	UPROPERTY(config, EditAnywhere, Category=Configuration, meta=(ForceUnits=s,
    //ConsoleVariable="CommonLoadingScreen.HoldLoadingScreenAdditionalSecs")) float HoldLoadingScreenAdditionalSecs
    // = 2.0f;

    //// The interval in seconds beyond which the loading screen is considered permanently hung (if non-zero).
    //	UPROPERTY(config, EditAnywhere, Category=Configuration, meta=(ForceUnits=s))
    // float LoadingScreenHeartbeatHangDuration = 0.0f;

    //// The interval in seconds between each log of what is keeping a loading screen up (if non-zero).
    //	UPROPERTY(config, EditAnywhere, Category=Configuration, meta=(ForceUnits=s))
    // float LogLoadingScreenHeartbeatInterval = 5.0f;

    //// When true, the reason the loading screen is shown or hidden will be printed to the log every frame.
    // UPROPERTY(Transient, EditAnywhere, Category=Debugging,
    // meta=(ConsoleVariable="CommonLoadingScreen.LogLoadingScreenReasonEveryFrame")) bool
    // LogLoadingScreenReasonEveryFrame = 0;

    //// Force the loading screen to be displayed (useful for debugging)
    // UPROPERTY(Transient, EditAnywhere, Category=Debugging, meta=(ConsoleVariable="CommonLoadingScreen.AlwaysShow"))
    // bool ForceLoadingScreenVisible = false;

    //// Should we apply the additional HoldLoadingScreenAdditionalSecs delay even in the editor
    //// (useful when iterating on loading screens)
    // UPROPERTY(Transient, EditAnywhere, Category=Debugging)
    // bool HoldLoadingScreenAdditionalSecsEvenInEditor = false;

    //// Should we apply the additional HoldLoadingScreenAdditionalSecs delay even in the editor
    //// (useful when iterating on loading screens)
    // UPROPERTY(config, EditAnywhere, Category=Configuration)
    // bool ForceTickLoadingScreenEvenInEditor = true;
};
