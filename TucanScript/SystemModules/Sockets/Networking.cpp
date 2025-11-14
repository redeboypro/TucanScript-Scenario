#include "Networking.h"

using namespace TucanScript;

ExternC {
    TucanAPI Undef Net_SetNonBlocking(ExC_Args) {
        Socket_t sock = ExC_Int32Arg(0);
        Boolean bEn = args->m_Size > 1 ? ExC_Int32Arg(1) : true;

        SInt32 flags = fcntl(sock, F_GETFL, Zero);
        if (flags == -1) {
            stack->Push(false);
            return;
        }

        if (bEn) {
            flags |= O_NONBLOCK;
        }
        else {
            flags &= ~O_NONBLOCK;
        }

        stack->Push(fcntl(sock, F_SETFL, flags) != -1);
    }

    TucanAPI Undef Net_CanRead(ExC_Args) {
        Socket_t sock = ExC_Int32Arg(0);
        if (sock < 0 || sock >= FD_SETSIZE) {
            stack->Push(false);
            return;
        }
        fd_set set;
        FD_ZERO(&set);
        FD_SET(sock, &set);
        struct timeval tv = { 0, 0 };
        SInt32 r = select((SInt32) sock + 1, &set, nullptr, nullptr, &tv);
        stack->Push(r > 0);
    }

    TucanAPI Undef Net_CanWrite(ExC_Args) {
        Socket_t sock = ExC_Int32Arg(0);
        if (sock < 0 || sock >= FD_SETSIZE) {
            stack->Push(false);
            return;
        }
        fd_set set;
        FD_ZERO(&set);
        FD_SET(sock, &set);
        struct timeval tv = { 0, 0 };
        SInt32 r = select((SInt32) sock + 1, nullptr, &set, nullptr, &tv);
        stack->Push(r > 0);
    }

    TucanAPI Undef Net_Listen(ExC_Args) {
        Socket_t sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            stack->Push(InvalidID);
            return;
        }

        int option = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

        struct sockaddr_in addr {};
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(ExC_WordArg(0));

        if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            close(sock);
            stack->Push(InvalidID);
            return;
        }
        if (listen(sock, 8) < 0) {
            close(sock);
            stack->Push(InvalidID);
            return;
        }

        stack->Push(sock);
    }

    TucanAPI Undef Net_Accept(ExC_Args) {
        struct sockaddr_in client_addr {};
        socklen_t len = sizeof(client_addr);
        memset(&client_addr, 0, sizeof(client_addr));

        SInt32 client = accept(ExC_Int32Arg(0), (struct sockaddr*) &client_addr, &len);
        if (client < 0) {
            stack->Push(InvalidID);
            return;
        }

        stack->Push(client);
    }

    TucanAPI Undef Net_Connect(ExC_Args) {
        Socket_t sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            stack->Push(InvalidID);
            return;
        }

        struct sockaddr_in addr {};
        memset(&addr, Zero, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(ExC_WordArg(1));

        int pton_res = inet_pton(AF_INET, ExC_StringArg(0), &addr.sin_addr);
        if (pton_res != 1) {
            close(sock);
            stack->Push(InvalidID);
            return;
        }

        if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            if (errno != EINPROGRESS) {
                close(sock);
                stack->Push(InvalidID);
                return;
            }
        }

        stack->Push(sock);
    }

    TucanAPI Undef Net_GetSocketError(ExC_Args) {
        Socket_t sock = ExC_Int32Arg(0);
        SInt32 error = Zero;
        socklen_t len = sizeof(error);

        if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &len) == 0) {
            stack->Push(error);
        } else {
            stack->Push(errno);
        }
    }

    TucanAPI Undef Net_IsConnected(ExC_Args) {
        Socket_t sock = ExC_Int32Arg(0);

        fd_set write_fds, error_fds;
        FD_ZERO(&write_fds);
        FD_ZERO(&error_fds);
        FD_SET(sock, &write_fds);
        FD_SET(sock, &error_fds);

        struct timeval tv = {Zero, Zero};
        SInt32 result = select(sock + 1, Zero, &write_fds, &error_fds, &tv);

        if (result > 0) {
            if (FD_ISSET(sock, &error_fds)) {
                stack->Push(false);
            } else if (FD_ISSET(sock, &write_fds)) {
                stack->Push(true);
            }
        } else {
            stack->Push(false);
        }
    }

    TucanAPI Undef Net_Send(ExC_Args) {
        stack->Push((SInt32) send(ExC_Int32Arg(0), ExC_StringArg(1), ExC_WordArg(2), Zero));
    }

    TucanAPI Undef Net_Recv(ExC_Args) {
        auto hBuff = ExC_ManagedArg(1);
        ssize_t n = recv(ExC_Int32Arg(0), hBuff->m_Memory.m_hRawBuf, hBuff->m_Size, Zero);
        if (n > 0) {
            hBuff->m_Size = n;
        } else if (n == 0) {
            hBuff->m_Size = 0;
        } else {
            hBuff->m_Size = 0;
            LogErr("Connection error!");
        }
        stack->Push((SInt32)n);
    }

    TucanAPI Undef Net_UdpSocket(ExC_Args) {
        Socket_t sock = socket(AF_INET, SOCK_DGRAM, Zero);
        if (sock < Zero) {
            stack->Push(InvalidID);
            return;
        }
        stack->Push(sock);
    }

    TucanAPI Undef Net_UdpBind(ExC_Args) {
        Socket_t sock = ExC_Int32Arg(0);
        auto sAddr = ExC_StringArg(1);
        auto uPort = ExC_WordArg(2);

        struct sockaddr_in addr {};
        memset(&addr, Zero, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(uPort);

        if (sAddr == nullptr || inet_pton(AF_INET, sAddr, &addr.sin_addr) != 1) {
            stack->Push(false);
            return;
        }

        stack->Push(bind(sock, (struct sockaddr*) &addr, sizeof(addr)) == 0);
    }

    TucanAPI Undef Net_UdpSendTo(ExC_Args) {
        Socket_t sock = ExC_Int32Arg(0);
        auto hBuff = ExC_ManagedArg(1);
        auto sAddr = ExC_StringArg(2);
        auto uPort = ExC_WordArg(3);

        struct sockaddr_in addr {};
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(uPort);
        if (sAddr == nullptr || inet_pton(AF_INET, sAddr, &addr.sin_addr) != 1) {
            stack->Push((SInt32)-1);
            return;
        }

        ssize_t sent = sendto(sock, hBuff->m_Memory.m_hRawBuf, hBuff->m_Size, Zero,
                              (struct sockaddr*)&addr, sizeof(addr));
        stack->Push((SInt32)sent);
    }

    TucanAPI Undef Net_UdpRecvFrom(ExC_Args) {
        auto hBuff = ExC_ManagedArg(1);
        struct sockaddr_in addr {};
        socklen_t addrLength = sizeof(addr);
        memset(&addr, Zero, sizeof(addr));

        ssize_t n = recvfrom(ExC_Int32Arg(0), hBuff->m_Memory.m_hRawBuf, hBuff->m_Size, Zero,
                             (struct sockaddr*)&addr, &addrLength);

        if (n > Zero) {
            hBuff->m_Size = (size_t)n;
        } else {
            hBuff->m_Size = 0;
        }

        stack->Push((SInt32) n);
    }

    TucanAPI Undef Net_Close(ExC_Args) {
        SInt32 fd = ExC_Int32Arg(0);
        if (fd >= Zero) {
            close(fd);
        }
    }
}
