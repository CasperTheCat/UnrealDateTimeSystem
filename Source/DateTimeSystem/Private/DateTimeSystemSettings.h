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
    UPROPERTY(config, EditAnywhere, Category = "Meta Config")
    bool CanEverTick = true;

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
};
