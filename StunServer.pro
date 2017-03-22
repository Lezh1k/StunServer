
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += -masm=intel

QMAKE_CXXFLAGS += -pthread
LIBS += -pthread

QMAKE_CFLAGS_WARN_ON -= -W3
QMAKE_CFLAGS_WARN_ON += -W4

SOURCES += src/main.cpp \
    src/Commons/Commons.cpp \
    src/Commons/ApplicationLog.cpp \
    src/EventLoop/IFunctor.cpp \
    src/EventLoop/EventLoop.cpp \
    src/Stun/StunMessage.cpp \
    src/Stun/StunAttributeFactory.cpp \
    src/Stun/StunAttribute.cpp \
    src/Stun/IServer.cpp \
    src/SynchroPrimitives/MRE_Linux.cpp \
    src/SynchroPrimitives/InternalCriticalSection.cpp \
    src/Stun/UdpServer.cpp \
    src/Settings/StunSettings.cpp

HEADERS += \
    include/Commons/Commons.h \
    include/Commons/ApplicationLog.h \
    include/EventLoop/IFunctor.h \
    include/EventLoop/FunctorWithResult.h \
    include/EventLoop/FunctorWithoutResult.h \
    include/EventLoop/EventLoopExceptionInfo.h \
    include/EventLoop/EventLoopException.h \
    include/EventLoop/EventLoop.h \
    include/Stun/StunMessage.h \
    include/Stun/StunErrorCodes.h \
    include/Stun/StunAttributeFactory.h \
    include/Stun/StunAttribute.h \
    include/Stun/ServerErrors.h \
    include/Stun/IServer.h \
    include/SynchroPrimitives/MRE_Wrapper.h \
    include/SynchroPrimitives/MRE_Linux.h \
    include/SynchroPrimitives/Locker.h \
    include/SynchroPrimitives/InternalCriticalSection.h \
    include/Threads/ThreadWrapper.h \
    include/Threads/IRunnable.h \
    include/Commons/FileWrapper.h \
    include/Stun/UdpServer.h \
    include/Settings/StunSettings.h \
    include/Commons/IfstreamWrapper.h

INCLUDEPATH += include/
