// Copyright Acinonyx Ltd. 2023. All Rights Reserved.

#include "DateTimeSystemSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(DateTimeSystemSettings)

UDateTimeSystemSettings::UDateTimeSystemSettings()
    : ReferenceLatitude(0)
    , ReferenceLongitude(0)
    , StartDate()
    , TimeScale(0)
    , TickStride(0)
{
    CategoryName = TEXT("Game");
}
