#ifndef TUCANSCRIPT_NETWORKING_H
#define TUCANSCRIPT_NETWORKING_H

#include "../../VirtualMachine.h"
#undef _Exit

#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/fcntl.h>

using namespace TucanScript;

using Socket_t = SInt32;

ExternC {
    TucanAPI Undef Net_SetNonBlocking(ExC_Args);
    TucanAPI Undef Net_CanRead(ExC_Args);
    TucanAPI Undef Net_CanWrite(ExC_Args);
    TucanAPI Undef Net_Listen(ExC_Args);
    TucanAPI Undef Net_Accept(ExC_Args);
    TucanAPI Undef Net_Connect(ExC_Args);
    TucanAPI Undef Net_GetSocketError(ExC_Args);
    TucanAPI Undef Net_IsConnected(ExC_Args);
    TucanAPI Undef Net_Send(ExC_Args);
    TucanAPI Undef Net_Recv(ExC_Args);
    TucanAPI Undef Net_UdpSocket(ExC_Args);
    TucanAPI Undef Net_UdpBind(ExC_Args);
    TucanAPI Undef Net_UdpSendTo(ExC_Args);
    TucanAPI Undef Net_UdpRecvFrom(ExC_Args);
    TucanAPI Undef Net_Close(ExC_Args);
}

#endif
