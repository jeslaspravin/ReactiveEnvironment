#pragma once

#include "stats/Stats.h"


DECLARE_STATS_GROUP(TEXT("ReactiveEnv"), STATGROUP_ReactiveEnv, STATCAT_Advanced);

#if WITH_EDITOR
#define REACTIVEENV_SCOPE_CYCLE_COUNTER(stat) SCOPE_CYCLE_COUNTER(stat)

#define LOG_WARN(LOGDEF,FMT, ...) UE_LOG(LOGDEF, Warning, (FMT), ##__VA_ARGS__)
#define LOG_ERROR(LOGDEF,FMT, ...) UE_LOG(LOGDEF, Error, (FMT), ##__VA_ARGS__)
#define LOGGER(LOGDEF,FMT, ...) UE_LOG(LOGDEF, Log, (FMT), ##__VA_ARGS__)
#else
#define REACTIVEENV_SCOPE_CYCLE_COUNTER(stat)

#define LOG_WARN(LOGDEF,FMT, ...)
#define LOG_ERROR(LOGDEF,FMT, ...)
#define LOGGER(LOGDEF,FMT, ...)
#endif

