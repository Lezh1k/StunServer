#ifndef SERVERERRORS_H
#define SERVERERRORS_H

namespace Stun {
  typedef enum ServerErrors {
    SE_SUCCESS = 0,
    SE_ALREADY_LISTENING = 1,
    SE_INVALID_SOCKET_HANDLE = 2,
    SE_BINDING_ERROR = 3,
    SE_NOT_INITIALIZED = 4
  } ServerErrors;
}

#endif // SERVERERRORS_H
