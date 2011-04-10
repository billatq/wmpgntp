// Copyright (c) 2010 William Reading
// Available under the terms of the Microsoft Public License (Ms-PL) 

#include "common.h"
#include "WMP-GNTP.h"

CWMPGNTP::CWMPGNTP()
{
    m_dwAdviseCookie = 0;
    m_growlClientRegistered = false;
}

CWMPGNTP::~CWMPGNTP()
{
}

HRESULT CWMPGNTP::FinalConstruct()
{
    return S_OK;
}

void CWMPGNTP::FinalRelease()
{
    ReleaseCore();
}

HRESULT CWMPGNTP::SetCore(IWMPCore *pCore)
{
    HRESULT hr = S_OK;

    // release any existing WMP core interfaces
    ReleaseCore();

    // If we get passed a NULL core, this  means
    // that the plugin is being shutdown.
    if (pCore == NULL)
    {
        return S_OK;
    }

    m_spCore = pCore;

    // connect up the event interface
    CComPtr < IConnectionPointContainer > spConnectionContainer;

    hr = m_spCore->QueryInterface(&spConnectionContainer);

    if (SUCCEEDED(hr))
    {
        hr = spConnectionContainer->FindConnectionPoint(__uuidof(IWMPEvents),
                &m_spConnectionPoint);
    }

    if (SUCCEEDED(hr))
    {
        hr = m_spConnectionPoint->Advise(GetUnknown(), &m_dwAdviseCookie);

        if ((FAILED(hr)) || (0 == m_dwAdviseCookie))
        {
            m_spConnectionPoint = NULL;
        }
    }

    // Set up growl notifications
    // TODO: Set this up as a member variable
    CAtlArray < GrowlNotificationType > notificationTypes;
    notificationTypes.Add(GrowlNotificationType(L"Item Changed"));

    // Create Icon Resource and compute hash
    GrowlResource applicationIcon;
    applicationIcon.m_isRemote = false;

    // Try to read a fancy resource from WMP. If that fails, fallback.
    HRESULT growlHr = Util::ReadWMPResourceData(L"GERERIC_DRAG_OTHER.PNG",
            MAKEINTRESOURCE(257), applicationIcon.m_data);
    if (FAILED(growlHr))
    {
        Util::ReadResourceData(IDR_GROWLICON, applicationIcon.m_data);
    }
    Util::MD5HashData(applicationIcon.m_data, applicationIcon.m_dataHash);

    // Add to the global list of resources
    m_growlClient.m_growlResources.Add(applicationIcon);

    // Try to Register for growl
    GrowlRegistration registration(L"WMP-GNTP", notificationTypes);
    registration.m_applicationIcon = applicationIcon;
    growlHr = m_growlClient.Register(registration);
    if (SUCCEEDED(growlHr))
    {
        m_growlClientRegistered = true;
    }

    return hr;
}

void CWMPGNTP::ReleaseCore()
{
    if (m_spConnectionPoint)
    {
        if (0 != m_dwAdviseCookie)
        {
            m_spConnectionPoint->Unadvise(m_dwAdviseCookie);
            m_dwAdviseCookie = 0;
        }
        m_spConnectionPoint = NULL;
    }

    if (m_spCore)
    {
        m_spCore = NULL;
    }
}

HRESULT CWMPGNTP::GetProperty(const WCHAR *pwszName, VARIANT *pvarProperty)
{
    if (NULL == pvarProperty)
    {
        return E_POINTER;
    }

    return E_NOTIMPL;
}

HRESULT CWMPGNTP::SetProperty(const WCHAR *pwszName,
        const VARIANT *pvarProperty)
{
    return E_NOTIMPL;
}
