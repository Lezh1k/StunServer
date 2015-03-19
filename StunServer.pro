
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -masm=intel

QMAKE_CXXFLAGS += -pthread
LIBS += -pthread

QMAKE_CFLAGS_WARN_ON -= -W3
QMAKE_CFLAGS_WARN_ON += -W4

SOURCES += main.cpp \
    Commons/Commons.cpp \
    Commons/ApplicationLog.cpp \
    EventLoop/IFunctor.cpp \
    EventLoop/EventLoop.cpp \
    Stun/StunMessage.cpp \
    Stun/StunAttributeFactory.cpp \
    Stun/StunAttribute.cpp \
    Stun/IServer.cpp \
    SynchroPrimitives/MRE_Linux.cpp \
    SynchroPrimitives/InternalCriticalSection.cpp \
    Stun/UdpServer.cpp \
    Stun/Settings/StunSettings.cpp

HEADERS += \
    Commons/Commons.h \
    Commons/ApplicationLog.h \
    EventLoop/IFunctor.h \
    EventLoop/FunctorWithResult.h \
    EventLoop/FunctorWithoutResult.h \
    EventLoop/EventLoopExceptionInfo.h \
    EventLoop/EventLoopException.h \
    EventLoop/EventLoop.h \
    Stun/StunMessage.h \
    Stun/StunErrorCodes.h \
    Stun/StunAttributeFactory.h \
    Stun/StunAttribute.h \
    Stun/ServerErrors.h \
    Stun/IServer.h \
    SynchroPrimitives/MRE_Wrapper.h \
    SynchroPrimitives/MRE_Linux.h \
    SynchroPrimitives/Locker.h \
    SynchroPrimitives/InternalCriticalSection.h \
    Threads/ThreadWrapper.h \
    Threads/IRunnable.h \
    Commons/FileWrapper.h \
    Stun/UdpServer.h \
    Stun/Settings/StunSettings.h \
    Commons/IfstreamWrapper.h





