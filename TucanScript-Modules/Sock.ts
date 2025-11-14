hNetLib = LoadLibrary("libNetworking.dylib");

Net_SetNonBlocking = GetProcAddr(hNetLib, "Net_SetNonBlocking");
Net_CanRead = GetProcAddr(hNetLib, "Net_CanRead");
Net_CanWrite = GetProcAddr(hNetLib, "Net_CanWrite");
Net_Listen = GetProcAddr(hNetLib, "Net_Listen");
Net_Accept = GetProcAddr(hNetLib, "Net_Accept");
Net_Connect = GetProcAddr(hNetLib, "Net_Connect");
Net_GetSocketError = GetProcAddr(hNetLib, "Net_GetSocketError");
Net_IsConnected = GetProcAddr(hNetLib, "Net_IsConnected");
Net_Send = GetProcAddr(hNetLib, "Net_Send");
Net_Recv = GetProcAddr(hNetLib, "Net_Recv");
Net_UdpSocket = GetProcAddr(hNetLib, "Net_UdpSocket");
Net_UdpBind = GetProcAddr(hNetLib, "Net_UdpBind");
Net_UdpSendTo = GetProcAddr(hNetLib, "Net_UdpSendTo");
Net_UdpRecvFrom = GetProcAddr(hNetLib, "Net_UdpRecvFrom");
Net_Close = GetProcAddr(hNetLib, "Net_Close");

EINPROGRESS = 36;