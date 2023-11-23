// Copyright Acinonyx Ltd. 2023. All Rights Reserved.
#include "DateTimeTypes.h"

DEFINE_LOG_CATEGORY(LogDateTimeSystem);
DEFINE_LOG_CATEGORY(LogClimateSystem);

FDateTimeCommonCoreInitializer::FDateTimeCommonCoreInitializer()
    : LengthOfDay(0)
    , DaysInOrbitalYear(0)
    , YearbookTable(nullptr)
    , DateOverridesTable(nullptr)
    , UseDayIndexForOverride(false)
    , PlanetRadius(0)
    , ReferenceLatitude(0)
    , ReferenceLongitude(0)
    , DaysInWeek(0)
    , OverridedDatesSetDate(false)
{

}
