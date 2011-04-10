// Copyright (c) 2010 William Reading
// Available under the terms of the Microsoft Public License (Ms-PL) 

#include "common.h"
#include "WMP-GNTP.h"

void CWMPGNTP::GrowlSong()
{
    CComPtr < IWMPMedia > pIWMPMedia;
    m_spCore->get_currentMedia(&pIWMPMedia);
    if (pIWMPMedia != NULL)
    {
        CComBSTR bstrTitle, bstrArtist, bstrAlbum, sourceURL;
        pIWMPMedia->get_name(&bstrTitle);
        pIWMPMedia->getItemInfo(L"Artist", &bstrArtist);
        pIWMPMedia->getItemInfo(L"Album", &bstrAlbum);
        pIWMPMedia->get_sourceURL(&sourceURL);

        WMPOpenState openstate;
        m_spCore->get_openState(&openstate);

        // Try to avoid spurious displays immediately after startup
        // By seeing if this has already been sent before
        // This has the side-effect of not growling when a song is
        // repeated.
        if (m_lastPlayedSourceURL.Compare(sourceURL) != 0)
        {
            GrowlNotification notification;
            CString growlMessage;

            // Song Metadata
            if (bstrTitle.Length())
            {
                growlMessage = growlMessage + bstrTitle + L"\n";
            }
            if (bstrArtist.Length())
            {
                growlMessage = growlMessage + bstrArtist + L"\n";
            }
            if (bstrAlbum.Length())
            {
                growlMessage = growlMessage + bstrAlbum + L"\n";
            }

            // Check to see if it's an m4a
            CString fileName(sourceURL);
            CString extension = fileName;
            extension.MakeLower();
            extension = extension.Right(4);
            if (extension.Find(L".m4a") > -1)
            {
                // TODO: Refactor this so that less data has to be manipulated directly
                // Try the m4a extraction method
                HRESULT hr = Util::ReadMP4CoverData(fileName,
                        notification.m_icon.m_data);
                if (SUCCEEDED(hr))
                {
                    hr = Util::MD5HashData(notification.m_icon.m_data,
                            notification.m_icon.m_dataHash);
                    notification.m_icon.m_isRemote = false;
                }
                else
                {
                    // Empty to keep state consistent
                    notification.m_icon.m_data.RemoveAll();
                }

            }
            else
            {
                // Song and dance for album art with other media
                // Get a IWMPMedia3 object to query WM/Picture
                CComQIPtr < IWMPMedia3 > pMedia3 = pIWMPMedia;
                if (pMedia3 != NULL)
                {
                    HRESULT hr;

                    // Try to get the WM/Picture Metadata
                    CComVariant itemInfoVariant;
                    hr = pMedia3->getItemInfoByType(L"WM/Picture", NULL, 0,
                            &itemInfoVariant);
                    if (SUCCEEDED(hr))
                    {
                        // QI the IWMPMetadataPicture from the IDispatch value of the variant
                        CComQIPtr < IWMPMetadataPicture > pictureMetadata
                                = itemInfoVariant.pdispVal;
                        if (pictureMetadata != NULL)
                        {
                            // Pull the vnd.ms.wmhtml:// URL that that gives the cover art in Temporary Internet Files
                            CComBSTR metaURLBSTR;
                            hr = pictureMetadata->get_URL(&metaURLBSTR);
                            if (SUCCEEDED(hr))
                            {
                                // Go ask the internet cache for the url.
                                INTERNET_CACHE_ENTRY_INFO *cacheEntryInfo;
                                DWORD cbCacheEntryInfo = 0;

                                // Get structure size for the cache entry
                                BOOL status = GetUrlCacheEntryInfo(metaURLBSTR,
                                        NULL, &cbCacheEntryInfo);

                                // Allocate some memory on the heap for it and retrieve the entry
                                cacheEntryInfo
                                        = (INTERNET_CACHE_ENTRY_INFO *) new BYTE[cbCacheEntryInfo];
                                if (cacheEntryInfo)
                                {
                                    status = GetUrlCacheEntryInfo(metaURLBSTR,
                                            cacheEntryInfo, &cbCacheEntryInfo);
                                    if (status)
                                    {
                                        CString
                                                actualPath =
                                                        cacheEntryInfo->lpszLocalFileName;

                                        // Add the path onto the notification
                                        notification.m_icon.InitWithFilePath(
                                                actualPath);
                                    }
                                    delete[] cacheEntryInfo;
                                }
                            }
                        }
                    }
                }
            }

            // Emit the message
            if (growlMessage.GetLength())
            {
                if (!m_growlClientRegistered)
                {
                    // Set up growl notifications
                    CAtlArray < GrowlNotificationType > notificationTypes;
                    notificationTypes.Add(
                            GrowlNotificationType(L"Item Changed"));

                    // Try to Register for growl
                    GrowlRegistration registration(L"WMP-GNTP",
                            notificationTypes);
                    HRESULT growlHr = m_growlClient.Register(registration);
                    if (SUCCEEDED(growlHr))
                    {
                        m_growlClientRegistered = true;
                    }
                }
                if (m_growlClientRegistered)
                {
                    notification.m_type
                            = GrowlNotificationType(L"Item Changed");
                    notification.m_title = growlMessage;
                    m_growlClient.Notify(notification);
                }
            }
        }
        m_lastPlayedSourceURL = sourceURL;
    }
}

void CWMPGNTP::OpenStateChange(long NewState)
{
    switch (NewState)
    {
    case wmposUndefined:
        break;
    case wmposPlaylistChanging:
        break;
    case wmposPlaylistLocating:
        break;
    case wmposPlaylistConnecting:
        break;
    case wmposPlaylistLoading:
        break;
    case wmposPlaylistOpening:
        break;
    case wmposPlaylistOpenNoMedia:
        break;
    case wmposPlaylistChanged:
        break;
    case wmposMediaChanging:
        break;
    case wmposMediaLocating:
        break;
    case wmposMediaConnecting:
        break;
    case wmposMediaLoading:
        break;
    case wmposMediaOpening:
        break;
    case wmposMediaOpen:
        break;
    case wmposBeginCodecAcquisition:
        break;
    case wmposEndCodecAcquisition:
        break;
    case wmposBeginLicenseAcquisition:
        break;
    case wmposEndLicenseAcquisition:
        break;
    case wmposBeginIndividualization:
        break;
    case wmposEndIndividualization:
        break;
    case wmposMediaWaiting:
        break;
    case wmposOpeningUnknownURL:
        break;
    default:
        break;
    }
}

void CWMPGNTP::PlayStateChange(long NewState)
{
    switch (NewState)
    {
    case wmppsUndefined:
        break;
    case wmppsStopped:
        break;
    case wmppsPaused:
        break;
    case wmppsPlaying:
        GrowlSong();
        break;
    case wmppsScanForward:
        break;
    case wmppsScanReverse:
        break;
    case wmppsBuffering:
        break;
    case wmppsWaiting:
        break;
    case wmppsMediaEnded:
        break;
    case wmppsTransitioning:
        break;
    case wmppsReady:
        break;
    case wmppsReconnecting:
        break;
    case wmppsLast:
        break;
    default:
        break;
    }
}

void CWMPGNTP::AudioLanguageChange(long LangID)
{
}

void CWMPGNTP::StatusChange()
{
}

void CWMPGNTP::ScriptCommand(BSTR scType, BSTR Param)
{
}

void CWMPGNTP::NewStream()
{
}

void CWMPGNTP::Disconnect(long Result)
{
}

void CWMPGNTP::Buffering(VARIANT_BOOL Start)
{
}

void CWMPGNTP::Error()
{
    CComPtr < IWMPError > spError;
    CComPtr < IWMPErrorItem > spErrorItem;
    HRESULT dwError = S_OK;
    HRESULT hr = S_OK;

    if (m_spCore)
    {
        hr = m_spCore->get_error(&spError);

        if (SUCCEEDED(hr))
        {
            hr = spError->get_item(0, &spErrorItem);
        }

        if (SUCCEEDED(hr))
        {
            hr = spErrorItem->get_errorCode((long *) &dwError);
        }
    }
}

void CWMPGNTP::Warning(long WarningType, long Param, BSTR Description)
{
}

void CWMPGNTP::EndOfStream(long Result)
{
}

void CWMPGNTP::PositionChange(double oldPosition, double newPosition)
{
}

void CWMPGNTP::MarkerHit(long MarkerNum)
{
}

void CWMPGNTP::DurationUnitChange(long NewDurationUnit)
{
}

void CWMPGNTP::CdromMediaChange(long CdromNum)
{
}

void CWMPGNTP::PlaylistChange(IDispatch * Playlist,
        WMPPlaylistChangeEventType change)
{
    switch (change)
    {
    case wmplcUnknown:
        break;
    case wmplcClear:
        break;
    case wmplcInfoChange:
        break;
    case wmplcMove:
        break;
    case wmplcDelete:
        break;
    case wmplcInsert:
        break;
    case wmplcAppend:
        break;
    case wmplcPrivate:
        break;
    case wmplcNameChange:
        break;
    case wmplcMorph:
        break;
    case wmplcSort:
        break;
    case wmplcLast:
        break;
    default:
        break;
    }
}

void CWMPGNTP::CurrentPlaylistChange(WMPPlaylistChangeEventType change)
{
    switch (change)
    {
    case wmplcUnknown:
        break;
    case wmplcClear:
        break;
    case wmplcInfoChange:
        break;
    case wmplcMove:
        break;
    case wmplcDelete:
        break;
    case wmplcInsert:
        break;
    case wmplcAppend:
        break;
    case wmplcPrivate:
        break;
    case wmplcNameChange:
        break;
    case wmplcMorph:
        break;
    case wmplcSort:
        break;
    case wmplcLast:
        break;
    default:
        break;
    }
}

void CWMPGNTP::CurrentPlaylistItemAvailable(BSTR bstrItemName)
{
}

void CWMPGNTP::MediaChange(IDispatch * Item)
{
}

void CWMPGNTP::CurrentMediaItemAvailable(BSTR bstrItemName)
{
}

void CWMPGNTP::CurrentItemChange(IDispatch *pdispMedia)
{
}

void CWMPGNTP::MediaCollectionChange()
{
}

void CWMPGNTP::MediaCollectionAttributeStringAdded(BSTR bstrAttribName,
        BSTR bstrAttribVal)
{
}

void CWMPGNTP::MediaCollectionAttributeStringRemoved(BSTR bstrAttribName,
        BSTR bstrAttribVal)
{
}

void CWMPGNTP::MediaCollectionAttributeStringChanged(BSTR bstrAttribName,
        BSTR bstrOldAttribVal, BSTR bstrNewAttribVal)
{
}

void CWMPGNTP::PlaylistCollectionChange()
{
}

void CWMPGNTP::PlaylistCollectionPlaylistAdded(BSTR bstrPlaylistName)
{
}

void CWMPGNTP::PlaylistCollectionPlaylistRemoved(BSTR bstrPlaylistName)
{
}

void CWMPGNTP::PlaylistCollectionPlaylistSetAsDeleted(BSTR bstrPlaylistName,
        VARIANT_BOOL varfIsDeleted)
{
}

void CWMPGNTP::ModeChange(BSTR ModeName, VARIANT_BOOL NewValue)
{
}

void CWMPGNTP::MediaError(IDispatch * pMediaObject)
{
}

void CWMPGNTP::OpenPlaylistSwitch(IDispatch *pItem)
{
}

void CWMPGNTP::DomainChange(BSTR strDomain)
{
}

void CWMPGNTP::SwitchedToPlayerApplication()
{
}

void CWMPGNTP::SwitchedToControl()
{
}

void CWMPGNTP::PlayerDockedStateChange()
{
}

void CWMPGNTP::PlayerReconnect()
{
}

void CWMPGNTP::Click(short nButton, short nShiftState, long fX, long fY)
{
}

void CWMPGNTP::DoubleClick(short nButton, short nShiftState, long fX, long fY)
{
}

void CWMPGNTP::KeyDown(short nKeyCode, short nShiftState)
{
}

void CWMPGNTP::KeyPress(short nKeyAscii)
{
}

void CWMPGNTP::KeyUp(short nKeyCode, short nShiftState)
{
}

void CWMPGNTP::MouseDown(short nButton, short nShiftState, long fX, long fY)
{
}

void CWMPGNTP::MouseMove(short nButton, short nShiftState, long fX, long fY)
{
}

void CWMPGNTP::MouseUp(short nButton, short nShiftState, long fX, long fY)
{
}
