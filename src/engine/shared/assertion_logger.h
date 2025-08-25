#ifndef ENGINE_SHARED_ASSERTION_LOGGER_H
#define ENGINE_SHARED_ASSERTION_LOGGER_H

#include <memory>

class ILogger;
class IGameStorage;

std::unique_ptr<ILogger> CreateAssertionLogger(IGameStorage *pStorage, const char *pGameName);

#endif
