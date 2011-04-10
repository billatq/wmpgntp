// Copyright (c) 2010 William Reading
// Available under the terms of the Microsoft Public License (Ms-PL) 

#include "GrowlClient.h"

// Instantiate from a local file path
HRESULT GrowlResource::InitWithFilePath(const CString &Path)
{
    m_isRemote = false;

    // Read in the data
    HRESULT hr = Util::ReadFileData(Path, m_data);
    if (FAILED(hr))
    {
        return hr;
    }

    // Hash the data
    hr = Util::MD5HashData(m_data, m_dataHash);
    return hr;
}

const int GrowlClient::DefaultGrowlPort = 23053;

HRESULT GrowlClient::Register(const GrowlRegistration &Registration)
{
    m_growlRegistration = Registration;

    // Set up connection to growl server
    CString portNumber;
    portNumber.Format(L"%d", DefaultGrowlPort);

    m_connSocket = GetSocket(L"localhost", portNumber);

    if (m_connSocket == INVALID_SOCKET)
    {
        // TODO: Better error codes?
        //_ASSERT(false);
        return E_FAIL;
    }
    bool bCouldSendRegistration = SendRegistration(Registration);
    if (!bCouldSendRegistration)
    {
        return E_FAIL;
    }
    ParseResponse();
    CloseSocket();

    return S_OK;
}

HRESULT GrowlClient::Notify(const GrowlNotification &Notification)
{
    // Set up connection to growl server
    CString portNumber;
    portNumber.Format(L"%d", DefaultGrowlPort);

    m_connSocket = GetSocket(L"localhost", portNumber);
    //_ASSERT(m_connSocket);

    if (m_connSocket == INVALID_SOCKET)
    {
        // TODO: Better error codes?
        //_ASSERT(false);
        return E_FAIL;
    }
    SendNotification(Notification);
    ParseResponse();
    CloseSocket();

    return S_OK;
}

// Adapted from http://msdn.microsoft.com/en-us/library/ms738566(VS.85).aspx
HRESULT GrowlClient::InitWinsock()
{
    // Initialize Winsock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        printf("WSAStartup failed: %d\n", iResult);
        return E_FAIL;
    }
    return S_OK;
}

// Adapted from http://msdn.microsoft.com/en-us/library/ms738630(VS.85).aspx
SOCKET GrowlClient::GetSocketXP(const CString &NodeName,
        const CString &PortName)
{
    //_ASSERT(false);
    CStringA nodeName = Util::GetUTF8String(NodeName);
    CStringA portName = Util::GetUTF8String(PortName);

    ADDRINFO Hints, *AddrInfo, *AI;
    SOCKET ConnSocket;

    memset(&Hints, 0, sizeof(Hints));
    Hints.ai_family = PF_UNSPEC;
    Hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(nodeName.GetBuffer(), portName.GetBuffer(), &Hints,
            &AddrInfo))
    {
        WSACleanup();
        return INVALID_SOCKET;
    }

    for (AI = AddrInfo; AI != NULL; AI = AI->ai_next)
    {
        // Open a socket with the correct address family for this address.
        ConnSocket = socket(AI->ai_family, AI->ai_socktype, AI->ai_protocol);

        // This one didn't pan out; try another
        if (ConnSocket == INVALID_SOCKET)
        {
            continue;
        }

        if (connect(ConnSocket, AI->ai_addr, AI->ai_addrlen) != SOCKET_ERROR)
        {
            // Success!
            return ConnSocket;
        }

        // Failed, clean up
        closesocket(ConnSocket);
    }
    return INVALID_SOCKET;
}

// Adapted from http://msdn.microsoft.com/en-us/library/ms737937(VS.85).aspx
SOCKET GrowlClient::GetSocket(const CString &NodeName, const CString &PortName)
{
    return GetSocketXP(NodeName, PortName);
}

// TODO: Actually parse instead of just reading
HRESULT GrowlClient::ParseResponse()
{
    // TODO: Don't use a static buffer
    char recvbuf[1024];

    int iResult;
    // Receive data until the server closes the connection
    do
    {
        iResult = recv(m_connSocket, recvbuf, 1024, 0);
        if (iResult > 0)
        {
            printf("Bytes received: %d\n", iResult);
            recvbuf[iResult] = '\0';
            printf("%s\n", recvbuf);
            return S_OK;
        }
        else if (iResult == 0)
        {
            printf("Connection closed\n");
            return E_FAIL;
        }
        else
        {
            printf("recv failed: %d\n", WSAGetLastError());
            return E_FAIL;
        }
    } while (iResult > 0);

    return S_OK;
}

HRESULT GrowlClient::CloseSocket()
{
    int iResult = shutdown(m_connSocket, SD_SEND);
    if (iResult == SOCKET_ERROR)
    {
        closesocket(m_connSocket);
        m_connSocket = NULL;
        //_ASSERT(false);
        return E_FAIL;
    }
    return S_OK;
}

// Portions adapted from http://msdn.microsoft.com/en-us/library/bb530747(VS.85).aspx
bool GrowlClient::SendRegistration(const GrowlRegistration &Registration)
{
    //_ASSERT(m_connSocket != NULL);

    // Register statement
    WriteStringToSocket(CString(L"GNTP/1.0 REGISTER NONE"));

    // Application-Name
    WriteStringToSocket(
            CString(L"Application-Name: ") + Registration.m_applicationName);

    // Application Icon
    if (!Registration.m_applicationIcon.m_dataHash.IsEmpty())
    {
        // We don't currently support remote resources
        if (!Registration.m_applicationIcon.m_isRemote)
        {
            WriteStringToSocket(
                    CString(L"Application-Icon: x-growl-resource://")
                            + Registration.m_applicationIcon.m_dataHash);
        }
    }

    // Notifications-Count
    CAtlArray<GrowlNotificationType> notificationTypes;
    CString notificationCount;
    notificationCount.Format(L"%d", Registration.m_notificationTypes.GetCount());
    WriteStringToSocket(CString(L"Notifications-Count: ") + notificationCount);

    // Write a blank line indicating the end of the register header
    WriteStringToSocket(CString(L""));

    for (unsigned int i = 0; i < Registration.m_notificationTypes.GetCount(); ++i)
    {
        // Notification-Name
        WriteStringToSocket(
                CString(L"Notification-Name: ")
                        + Registration.m_notificationTypes.GetAt(i).m_name);

        // Todo: write helper to make boolean string formatting cleaner
        if (Registration.m_notificationTypes.GetAt(i).m_enabled)
        {
            WriteStringToSocket(CString(L"Notification-Enabled: ") + L"True");
        }
        else
        {
            WriteStringToSocket(CString(L"Notification-Enabled: ") + L"False");
        }

        // Write out a blank line indicating the end of the notification
        WriteStringToSocket(CString(L""));
    }

    // Write out resources
    for (DWORD i = 0; i < m_growlResources.GetCount(); ++i)
    {
        WriteStringToSocket(
                CString(L"Identifier: ") + m_growlResources.GetAt(i).m_dataHash);

        CString countData;
        countData.Format(L"%d", m_growlResources.GetAt(i).m_data.GetCount());
        WriteStringToSocket(CString(L"Length: ") + countData);

        WriteStringToSocket(CString(L""));
        WriteDataToSocket(m_growlResources.GetAt(i).m_data);
        WriteStringToSocket(CString(L""));
    }

    return true;
}

bool GrowlClient::WriteDataToSocket(const CAtlArray<BYTE> &Data)
{
    int iResult = send(m_connSocket, (const char *) (Data.GetData()),
            Data.GetCount() * sizeof(BYTE), 0);
    if (iResult == SOCKET_ERROR)
    {
        int wsa_err = WSAGetLastError();
        printf("send failed: %d\n", wsa_err);
        closesocket(m_connSocket);
        WSACleanup();
        return false;
    }
    return true;
}

bool GrowlClient::WriteStringToSocket(const CString &Text)
{
    CString writeString;
    writeString.Format(L"%s\r\n", Text);

    CStringA strUTF8 = Util::GetUTF8String(writeString);

    // We don't want to send null characters after each write, so we truncate the buffer at one minus the actual contents
    int iResult = send(m_connSocket,
            (const char *) strUTF8.GetBuffer(strUTF8.GetLength()),
            strUTF8.GetLength() * sizeof(const char), 0);
    if (iResult == SOCKET_ERROR)
    {
        printf("send failed: %d\n", WSAGetLastError());
        closesocket(m_connSocket);
        WSACleanup();
        //_ASSERT(false);
        return false;
    }
    return true;
}

bool GrowlClient::SendNotification(const GrowlNotification &Notification)
{
    //_ASSERT(m_connSocket != NULL);

    // Notify statement
    WriteStringToSocket(CString(L"GNTP/1.0 NOTIFY NONE"));

    // Application-Name
    WriteStringToSocket(
            CString(L"Application-Name: ")
                    + m_growlRegistration.m_applicationName);

    // Notification-Name
    WriteStringToSocket(
            CString(L"Notification-Name: ") + Notification.m_type.m_name);

    // Notification-Text
    WriteStringToSocket(CString(L"Notification-Title: ") + Notification.m_title);

    bool writeNotificationIcon = false;
    // Notification Icon
    if (!Notification.m_icon.m_dataHash.IsEmpty())
    {
        // We don't currently support remote resources
        if (!Notification.m_icon.m_isRemote)
        {
            WriteStringToSocket(
                    CString(L"Notification-Icon: x-growl-resource://")
                            + Notification.m_icon.m_dataHash);
            writeNotificationIcon = true;
        }
    }
    // Write out a blank line indicating the end of the notification
    WriteStringToSocket(CString(L""));

    // Write out icon for local resource
    if (writeNotificationIcon)
    {
        WriteStringToSocket(
                CString(L"Identifier: ") + Notification.m_icon.m_dataHash);

        CString countData;
        countData.Format(L"%d", Notification.m_icon.m_data.GetCount());
        WriteStringToSocket(CString(L"Length: ") + countData);

        WriteStringToSocket(CString(L""));
        WriteDataToSocket(Notification.m_icon.m_data);
        WriteStringToSocket(CString(L""));
    }

    // Write out a blank line indicating the end of the notification
    WriteStringToSocket(CString(L""));
    return true;
}
