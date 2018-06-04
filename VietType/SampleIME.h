// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "Globals.h"
#include "stdafx.h"

#include <iostream>
#include <fstream>

class CSampleIME : public ITfTextInputProcessorEx,
                   public ITfThreadMgrEventSink,
                   public ITfTextEditSink,
                   public ITfKeyEventSink,
                   public ITfThreadFocusSink,
                   public ITfFunctionProvider {
  public:
    CSampleIME();
    ~CSampleIME();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, _Outptr_ void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    // ITfTextInputProcessor
    STDMETHODIMP Activate(ITfThreadMgr *pThreadMgr, TfClientId tfClientId) {
        return ActivateEx(pThreadMgr, tfClientId, 0);
    }
    // ITfTextInputProcessorEx
    STDMETHODIMP ActivateEx(ITfThreadMgr *pThreadMgr, TfClientId tfClientId, DWORD dwFlags);
    STDMETHODIMP Deactivate();

    // ITfThreadMgrEventSink
    STDMETHODIMP OnInitDocumentMgr(_In_ ITfDocumentMgr *pDocMgr);
    STDMETHODIMP OnUninitDocumentMgr(_In_ ITfDocumentMgr *pDocMgr);
    STDMETHODIMP
    OnSetFocus(_In_ ITfDocumentMgr *pDocMgrFocus, _In_ ITfDocumentMgr *pDocMgrPrevFocus);
    STDMETHODIMP OnPushContext(_In_ ITfContext *pContext);
    STDMETHODIMP OnPopContext(_In_ ITfContext *pContext);

    // ITfTextEditSink
    STDMETHODIMP OnEndEdit(
        __RPC__in_opt ITfContext *pContext,
        TfEditCookie ecReadOnly,
        __RPC__in_opt ITfEditRecord *pEditRecord);

    // ITfKeyEventSink
    STDMETHODIMP OnSetFocus(BOOL fForeground);
    STDMETHODIMP OnTestKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten);
    STDMETHODIMP OnKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten);
    STDMETHODIMP OnTestKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten);
    STDMETHODIMP OnKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten);
    STDMETHODIMP OnPreservedKey(ITfContext *pContext, REFGUID rguid, BOOL *pIsEaten);

    // ITfThreadFocusSink
    STDMETHODIMP OnSetThreadFocus();
    STDMETHODIMP OnKillThreadFocus();

    // ITfFunctionProvider
    STDMETHODIMP GetType(__RPC__out GUID *pguid);
    STDMETHODIMP GetDescription(__RPC__deref_out_opt BSTR *pbstrDesc);
    STDMETHODIMP GetFunction(
        __RPC__in REFGUID rguid, __RPC__in REFIID riid, __RPC__deref_out_opt IUnknown **ppunk);

    // ITfFunction
    STDMETHODIMP GetDisplayName(_Out_ BSTR *pbstrDisplayName);

    // ITfFnGetPreferredTouchKeyboardLayout, it is the Optimized layout feature.
    STDMETHODIMP GetLayout(_Out_ TKBLayoutType *ptkblayoutType, _Out_ WORD *pwPreferredLayoutId);

    // CClassFactory factory callback
    static HRESULT CreateInstance(_In_ IUnknown *pUnkOuter, REFIID riid, _Outptr_ void **ppvObj);

    // utility function for thread manager.
    ITfThreadMgr *_GetThreadMgr() {
        return _pThreadMgr;
    }
    TfClientId _GetClientId() {
        return _tfClientId;
    }

    BOOL _IsSecureMode(void) {
        return (_dwActivateFlags & TF_TMAE_SECUREMODE) ? TRUE : FALSE;
    }
    BOOL _IsComLess(void) {
        return (_dwActivateFlags & TF_TMAE_COMLESS) ? TRUE : FALSE;
    }
    BOOL _IsStoreAppMode(void) {
        return (_dwActivateFlags & TF_TMF_IMMERSIVEMODE) ? TRUE : FALSE;
    };

    // comless helpers
    /*
    static HRESULT CSampleIME::CreateInstance(
        REFCLSID rclsid,
        REFIID riid,
        _Outptr_result_maybenull_ LPVOID *ppv,
        _Out_opt_ HINSTANCE *phInst,
        BOOL isComLessMode);
    static HRESULT CSampleIME::ComLessCreateInstance(
        REFGUID rclsid,
        REFIID riid,
        _Outptr_result_maybenull_ void **ppv,
        _Out_opt_ HINSTANCE *phInst);
    static HRESULT CSampleIME::GetComModuleName(
        REFGUID rclsid, _Out_writes_(cchPath) WCHAR *wchPath, DWORD cchPath);
        */

  private:
    BOOL _InitThreadMgrEventSink();
    void _UninitThreadMgrEventSink();

    BOOL _InitTextEditSink(_In_ ITfDocumentMgr *pDocMgr);

    BOOL _InitKeyEventSink();
    void _UninitKeyEventSink();

    BOOL _InitThreadFocusSink();
    void _UninitThreadFocusSink();

    BOOL _InitFunctionProviderSink();
    void _UninitFunctionProviderSink();

  private:
    ITfThreadMgr *_pThreadMgr;
    TfClientId _tfClientId;
    DWORD _dwActivateFlags;

    // The cookie of ThreadMgrEventSink
    DWORD _threadMgrEventSinkCookie;

    ITfContext *_pTextEditSinkContext;
    DWORD _textEditSinkCookie;

    // The cookie of ThreadFocusSink
    DWORD _dwThreadFocusSinkCookie;

    ITfDocumentMgr *_pDocMgrLastFocused;

    ITfContext *_pContext;

    LONG _refCount;

    std::ofstream logfile;
};
