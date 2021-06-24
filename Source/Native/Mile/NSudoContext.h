﻿/*
 * PROJECT:   NSudo Software Development Kit
 * FILE:      NSudoContext.h
 * PURPOSE:   Definition for NSudo context plugin
 *
 * LICENSE:   The MIT License
 *
 * DEVELOPER: Mouri_Naruto (Mouri_Naruto AT Outlook.com)
 */

#ifndef _NSUDO_CONTEXT_H_
#define _NSUDO_CONTEXT_H_

#include <Windows.h>

/**
 * @brief Definition for NSudo version information.
*/
typedef struct _NSUDO_VERSION
{
    UINT16 Major;
    UINT16 Minor;
    UINT16 Patch;
    UINT16 Revision;
    LPCWSTR Tag;
} NSUDO_VERSION, *PNSUDO_VERSION;

/**
 * @brief Forward definition for NSudo context.
*/
typedef struct _NSUDO_CONTEXT NSUDO_CONTEXT, *PNSUDO_CONTEXT;

/**
 * @brief Definition for NSudo context.
*/
struct _NSUDO_CONTEXT
{
    /**
     * @brief Gets NSudo version information.
     * @param Version The NSudo version information.
    */
    VOID(WINAPI* GetNSudoVersion)(
        _Out_ PNSUDO_VERSION Version);

    /**
     * @brief Gets the module handle of the context plugin.
     * @param Context The NSudo context.
     * @return The module handle of the context plugin.
    */
    HMODULE(WINAPI* GetContextPluginModuleHandle)(
        _In_ PNSUDO_CONTEXT Context);

    /**
     * @brief Gets the command line arguments for the context plugin.
     * @param Context The NSudo context.
     * @return The command line arguments for the context plugin.
    */
    LPCWSTR(WINAPI* GetContextPluginCommandLine)(
        _In_ PNSUDO_CONTEXT Context);

    /**
     * @brief Frees a memory block allocated from methods in PNSUDO_CONTEXT.
     * @param Context The NSudo context.
     * @param Block A pointer to the memory block to be freed.
    */
    VOID(WINAPI* Free)(
        _In_ PNSUDO_CONTEXT Context,
        _In_ LPVOID Block);

    /**
     * @brief Writes the specified string value to the NSudo user interface.
     * @param Context The NSudo context.
     * @param Value The value to write.
    */
    VOID(WINAPI* Write)(
        _In_ PNSUDO_CONTEXT Context,
        _In_ LPCWSTR Value);

    /**
     * @brief Writes the specified string value, followed by the current line
     *        terminator, to NSudo user interface.
     * @param Context The NSudo context.
     * @param Value The value to write.
    */
    VOID(WINAPI* WriteLine)(
        _In_ PNSUDO_CONTEXT Context,
        _In_ LPCWSTR Value);

    /**
     * @brief Reads the next line of characters from the user input.
     * @param Context The NSudo context.
     * @param InputPrompt The prompt you want to notice to the user.
     * @return The next line of characters from the user input. If the return
     *         value is not nullptr, you should use PNSUDO_CONTEXT::Free method
     *         to release.
    */
    LPCWSTR(WINAPI* ReadLine)(
        _In_ PNSUDO_CONTEXT Context,
        _In_ LPCWSTR InputPrompt);
};

/**
 * @brief The entry point type of NSudo context plugin.
 * @param Context The NSudo context.
 * @return HRESULT. If the function succeeds, the return value is S_OK.
*/
typedef HRESULT(WINAPI* NSudoContextPluginEntryPointType)(
    _In_ PNSUDO_CONTEXT Context);

#endif // !_NSUDO_CONTEXT_H_
