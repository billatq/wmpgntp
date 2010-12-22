// Copyright (c) 2010 William Reading
// Available under the terms of the Microsoft Public License (Ms-PL) 

#pragma once
#include "common.h"
#include "Util.h"

class GrowlResource
{
public:
	GrowlResource() {}
	GrowlResource(const GrowlResource & from)
	{
		if (this != &from)
		{
			this->m_isRemote = from.m_isRemote;
			this->m_remoteUrl = from.m_remoteUrl;

			// CAtlArray doesn't seem to have a copy contructor
			this->m_data.RemoveAll();
			this->m_data.Copy(from.m_data);

			this->m_dataHash = from.m_dataHash;
		}
	}
	GrowlResource & operator=(const GrowlResource & from)
	{
		if (this != &from)
		{
			this->m_isRemote = from.m_isRemote;
			this->m_remoteUrl = from.m_remoteUrl;

			// CAtlArray doesn't seem to have a copy contructor
			this->m_data.RemoveAll();
			this->m_data.Copy(from.m_data);

			this->m_dataHash = from.m_dataHash;
		}
		return *this;
	}

	HRESULT InitWithFilePath(const CString &Path);

	bool m_isRemote;
	CString m_remoteUrl;
	CAtlArray<BYTE> m_data;
	CString m_dataHash;
};

class GrowlNotificationType
{
public:
	GrowlNotificationType() {}
	GrowlNotificationType(const CString &NotificationName)
		:m_name(NotificationName),
		m_enabled(true)
	{

	}

//private:
	CString m_name;
	CString m_displayName;
	bool m_enabled;
	GrowlResource m_icon;
};

class GrowlNotification
{
public:
	GrowlNotification(const GrowlNotificationType &Type, const CString &Title)
		:m_type(Type),
		m_title(Title)
	{
	
	}

	GrowlNotification() { }

//private:
	GrowlNotificationType m_type;
	CString m_id;
	CString m_title;
	CString m_text;
	bool m_sticky;
	int m_priority;
	GrowlResource m_icon;
	CString m_coalescingId;
	CString m_callbackContext;
	CString m_callbackContextType;
	CString m_callbackTarget;
};

class GrowlRegistration
{
public:
	GrowlRegistration() { };
	GrowlRegistration(const CString &ApplicationName, const CAtlArray<GrowlNotificationType> &NotificationTypes)
		: m_applicationName(ApplicationName)

	{
		m_notificationTypes.RemoveAll();
		m_notificationTypes.Copy(NotificationTypes);
	}

	GrowlRegistration & operator=(const GrowlRegistration & from)
	{
		if (this != &from)
		{
			this->m_applicationName = from.m_applicationName;
			this->m_applicationIcon = from.m_applicationIcon;

			// CAtlArray doesn't seem to have a copy contructor
			this->m_notificationTypes.RemoveAll();
			this->m_notificationTypes.Copy(from.m_notificationTypes);
		}
		return *this;
	}

	void GetNotificationTypes(CAtlArray<GrowlNotificationType> &NotificationTypes)
	{
		NotificationTypes.RemoveAll();
		NotificationTypes.Copy(m_notificationTypes);
	}

//private:
	CString m_applicationName;
	GrowlResource m_applicationIcon;
	CAtlArray<GrowlNotificationType> m_notificationTypes;
};

class GrowlClient
{
public:
	GrowlClient()
		:m_connSocket(NULL),
		m_growlRegistration(GrowlRegistration())
	{
		InitWinsock();
	}

	~GrowlClient(void)
	{
		if (m_connSocket)
		{
			CloseSocket();
		}
		WSACleanup();
	}

	HRESULT Register(const GrowlRegistration &Registration);
	HRESULT Notify(const GrowlNotification &Notification); 
	//HRESULT Close();

	// TODO: encapsulate
	// These are global resources, which aren't necessarily suitable to write out each time
	CAtlArray<GrowlResource> m_growlResources;
private:
	// Constants
	static const int DefaultGrowlPort;

	// Member Variables
	SOCKET m_connSocket;
	GrowlRegistration m_growlRegistration;
	

	// Helper Functions
	HRESULT InitWinsock();
	SOCKET GetSocket(const CString &NodeName, const CString &PortName);
	SOCKET GetSocketXP(const CString &NodeName, const CString &PortName);
	HRESULT CloseSocket();
	HRESULT ParseResponse();
	bool WriteStringToSocket(const CString &Text);
	bool WriteDataToSocket(const CAtlArray<BYTE> &Data);
	bool SendRegistration(const GrowlRegistration &Registration);
	bool SendNotification(const GrowlNotification &Notification);
};

