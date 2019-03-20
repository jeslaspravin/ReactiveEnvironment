#pragma once



#define LOG_WARN(FMT, ...) UE_LOG(LogReactiveEnvEditor, Warning, (FMT), ##__VA_ARGS__)
#define LOG_ERROR(FMT, ...) UE_LOG(LogReactiveEnvEditor, Error, (FMT), ##__VA_ARGS__)
#define LOGGER(FMT, ...) UE_LOG(LogReactiveEnvEditor, Log, (FMT), ##__VA_ARGS__)