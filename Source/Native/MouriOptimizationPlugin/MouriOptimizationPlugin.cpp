﻿/*
 * PROJECT:   Mouri Optimization Plugin
 * FILE:      MouriOptimizationPlugin.cpp
 * PURPOSE:   Private implementation for Mouri Optimization Plugin
 *
 * LICENSE:   The MIT License
 *
 * DEVELOPER: Mouri_Naruto (Mouri_Naruto AT Outlook.com)
 */

#include "MouriOptimizationPlugin.h"

void MoPrivateWriteLine(
    _In_ PNSUDO_CONTEXT Context,
    _In_z_ _Printf_format_string_ wchar_t const* const Format,
    ...)
{
    if (Context)
    {
        va_list ArgList;
        va_start(ArgList, Format);
        Context->WriteLine(
            Context,
            Mile::VFormatUtf16String(Format, ArgList).c_str());
        va_end(ArgList);
    }
}

void MoPrivateWriteErrorMessage(
    _In_ PNSUDO_CONTEXT Context,
    _In_ Mile::HResult const& hr,
    _In_z_ _Printf_format_string_ wchar_t const* const Format,
    ...)
{
    va_list ArgList;
    va_start(ArgList, Format);
    ::MoPrivateWriteLine(
        Context,
        L"%s: %s",
        Mile::VFormatUtf16String(Format, ArgList).c_str(),
        Mile::GetHResultMessage(hr).c_str());
    va_end(ArgList);
}

void MoPrivateWriteFinalResult(
    _In_ PNSUDO_CONTEXT Context,
    _In_ Mile::HResult const& hr)
{
    ::MoPrivateWriteLine(
        Context,
        L"%s\r\n",
        Mile::GetHResultMessage(hr).c_str());
}

void MoPrivatePrintPurgeScanResult(
    _In_ PNSUDO_CONTEXT Context,
    _In_ std::uint64_t ByteSize)
{
    if (Context)
    {
        //L"Total amount of disk space you may gain: ";
        //L"或许可获得的磁盘空间总量: ";

        ::MoPrivateWriteLine(
            Context,
            L"Total amount of disk space you may gain: %s.",
            Mile::ConvertByteSizeToUtf16String(ByteSize).c_str());
    }
}

DWORD MoPrivateParsePurgeMode(
    _In_ PNSUDO_CONTEXT Context)
{
    DWORD Result = 0;

    std::vector<std::wstring> Arguments = Mile::SpiltCommandArguments(
        Context->GetContextPluginCommandArguments(Context));
    for (auto& Argument : Arguments)
    {
        if (0 == ::_wcsicmp(Argument.c_str(), L"/Scan"))
        {
            Result = MO_PRIVATE_PURGE_MODE_SCAN;
            break;
        }
        else if (0 == ::_wcsicmp(Argument.c_str(), L"/Purge"))
        {
            Result = MO_PRIVATE_PURGE_MODE_PURGE;
            break;
        }
    }

    if (Result == 0)
    {
        // TODO: Maybe we should ask for user to choice
    }

    LPCWSTR ModeText = nullptr;

    if (Result == MO_PRIVATE_PURGE_MODE_SCAN)
    {
        ModeText = L"Scan";
    }
    else if(Result == MO_PRIVATE_PURGE_MODE_PURGE)
    {
        ModeText = L"Purge";
    }
    else
    {
        ModeText = L"Canceled";
    }

    ::MoPrivateWriteLine(
        Context,
        L"Purge Mode: %s.",
        ModeText);

    return Result;
}

Mile::HResult MoPrivateEnableBackupRestorePrivilege(
    _Out_ PHANDLE PreviousContextTokenHandle)
{
    HANDLE CurrentProcessToken = INVALID_HANDLE_VALUE;
    HANDLE DuplicatedCurrentProcessToken = INVALID_HANDLE_VALUE;
    LUID_AND_ATTRIBUTES RawPrivileges[2];

    auto Handler = Mile::ScopeExitTaskHandler([&]()
    {
        if (DuplicatedCurrentProcessToken != INVALID_HANDLE_VALUE)
        {
            ::CloseHandle(DuplicatedCurrentProcessToken);
        }

        if (CurrentProcessToken != INVALID_HANDLE_VALUE)
        {
            ::CloseHandle(CurrentProcessToken);
        }
    });

    if (!PreviousContextTokenHandle)
    {
        return E_INVALIDARG;
    }

    if (!::OpenProcessToken(
        ::GetCurrentProcess(),
        MAXIMUM_ALLOWED,
        &CurrentProcessToken))
    {
        return Mile::HResultFromLastError(FALSE);
    }

    if (!::DuplicateTokenEx(
        CurrentProcessToken,
        MAXIMUM_ALLOWED,
        nullptr,
        SecurityImpersonation,
        TokenImpersonation,
        &DuplicatedCurrentProcessToken))
    {
        return Mile::HResultFromLastError(FALSE);
    }

    if (!::LookupPrivilegeValueW(
        nullptr,
        SE_BACKUP_NAME,
        &RawPrivileges[0].Luid))
    {
        return Mile::HResultFromLastError(FALSE);
    }
    RawPrivileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!::LookupPrivilegeValueW(
        nullptr,
        SE_RESTORE_NAME,
        &RawPrivileges[1].Luid))
    {
        return Mile::HResultFromLastError(FALSE);
    }
    RawPrivileges[1].Attributes = SE_PRIVILEGE_ENABLED;

    Mile::HResult hr = Mile::AdjustTokenPrivilegesSimple(
        DuplicatedCurrentProcessToken,
        RawPrivileges,
        2);
    if (hr.IsFailed())
    {
        return hr;
    }

    ::OpenThreadToken(
        ::GetCurrentThread(),
        MAXIMUM_ALLOWED,
        FALSE,
        PreviousContextTokenHandle);

    return Mile::HResultFromLastError(::SetThreadToken(
        nullptr, DuplicatedCurrentProcessToken));
}

std::vector<std::wstring> MoPrivateGetProfilePathList()
{
    std::vector<std::wstring> Result;

    Mile::HResult hr = S_OK;
    HKEY ProfileListKeyHandle = nullptr;
    if (ERROR_SUCCESS == ::RegOpenKeyExW(
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList",
        0,
        KEY_READ | KEY_WOW64_64KEY,
        &ProfileListKeyHandle))
    {
        DWORD i = 0;
        for (;;)
        {
            wchar_t Buffer[256];
            HKEY ProfileListItemKeyHandle = nullptr;
            if (ERROR_SUCCESS != ::RegEnumKeyW(
                ProfileListKeyHandle,
                i++,
                Buffer,
                256))
            {
                break;
            }

            if (ERROR_SUCCESS == ::RegOpenKeyExW(
                ProfileListKeyHandle,
                Buffer,
                0,
                KEY_READ | KEY_WOW64_64KEY,
                &ProfileListItemKeyHandle))
            {
                LPWSTR ProfileImagePath = nullptr;
                if (Mile::RegQueryStringValue(
                    ProfileListItemKeyHandle,
                    L"ProfileImagePath",
                    &ProfileImagePath).IsSucceeded())
                {
                    Result.push_back(
                        Mile::ExpandEnvironmentStringsW(ProfileImagePath));

                    Mile::HeapMemory::Free(ProfileImagePath);
                }

                ::RegCloseKey(ProfileListItemKeyHandle);
            }
        }

        ::RegCloseKey(ProfileListKeyHandle);
    }

    return Result;
}

HANDLE MoPrivateCreateFile(
    _In_ LPCWSTR lpFileName,
    _In_ DWORD dwDesiredAccess,
    _In_ DWORD dwShareMode,
    _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    _In_ DWORD dwCreationDisposition,
    _In_ DWORD dwFlagsAndAttributes,
    _In_opt_ HANDLE hTemplateFile)
{
    return ::CreateFileW(
        Mile::FormatUtf16String(L"\\\\?\\%s", lpFileName).c_str(),
        dwDesiredAccess,
        dwShareMode,
        lpSecurityAttributes,
        dwCreationDisposition,
        dwFlagsAndAttributes,
        hTemplateFile);
}

BOOL MoPrivateIsFileExist(
    _In_ LPCWSTR FilePath)
{
    HANDLE FileHandle = ::MoPrivateCreateFile(
        FilePath,
        SYNCHRONIZE | FILE_READ_ATTRIBUTES,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
        nullptr);
    if (FileHandle != INVALID_HANDLE_VALUE)
    {
        ::CloseHandle(FileHandle);
    }
    return (FileHandle != INVALID_HANDLE_VALUE);
}

BOOL WINAPI DllMain(
    _In_ HINSTANCE hinstDLL,
    _In_ DWORD fdwReason,
    _In_ LPVOID lpvReserved)
{
    UNREFERENCED_PARAMETER(hinstDLL);
    UNREFERENCED_PARAMETER(lpvReserved);

    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
