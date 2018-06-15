// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "Globals.h"
#include "stdafx.h"
#include "Telex.h"

class IMECore :
    public ITfTextInputProcessorEx,
    public ITfThreadMgrEventSink,
    public ITfTextEditSink,
    public ITfKeyEventSink,
    public ITfThreadFocusSink,
    public ITfFunction,
    public ITfCompositionSink {

public:
    IMECore() noexcept;
    ~IMECore();

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
    STDMETHODIMP OnSetFocus(_In_ ITfDocumentMgr *pDocMgrFocus, _In_ ITfDocumentMgr *pDocMgrPrevFocus);
    STDMETHODIMP OnPushContext(_In_ ITfContext *pContext);
    STDMETHODIMP OnPopContext(_In_ ITfContext *pContext);

    // ITfTextEditSink
    STDMETHODIMP OnEndEdit(__RPC__in_opt ITfContext *pContext, TfEditCookie ecReadOnly, __RPC__in_opt ITfEditRecord *pEditRecord);

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

    // ITfFunction
    STDMETHODIMP GetDisplayName(_Out_ BSTR *pbstrDisplayName);

    // Inherited via ITfCompositionSink
    STDMETHODIMP OnCompositionTerminated(TfEditCookie ecWrite, ITfComposition * pComposition);

    void _StartComposition(_In_ ITfContext *pContext);
    void _EndComposition(_In_opt_ ITfContext *pContext);
    void _TerminateComposition(TfEditCookie ec, _In_ ITfContext *pContext, BOOL isCalledFromDeactivate);

    /// <summary>edit session utilities, don't call</summary>
    void _SetComposition(_In_ ITfComposition *pComposition);
    /// <summary>edit session utilities, don't call</summary>
    void _SaveCompositionContext(_In_ ITfContext *pContext);
    /// <summary>edit session utilities, don't call</summary>
    bool _IsComposing() const;
    /// <summary>edit session utilities, don't call</summary>
    void _MoveCaretToEnd(TfEditCookie ec);

    /// <summary>edit session utilities, don't call</summary>
    BOOL _IsRangeCovered(TfEditCookie ec, _In_ ITfRange *pRangeTest, _In_ ITfRange *pRangeCover);
    /// <summary>edit session utilities, don't call</summary>
    STDMETHODIMP _SetCompositionText(TfEditCookie ec, _In_ ITfContext *pContext, std::vector<wchar_t> content);

    // CClassFactory factory callback
    static HRESULT CreateInstance(_In_ IUnknown *pUnkOuter, REFIID riid, _Outptr_ void **ppvObj);

    BOOL _IsSecureMode(void) {
        return (_dwActivateFlags & TF_TMAE_SECUREMODE) ? TRUE : FALSE;
    }
    BOOL _IsComLess(void) {
        return (_dwActivateFlags & TF_TMAE_COMLESS) ? TRUE : FALSE;
    }
    BOOL _IsStoreAppMode(void) {
        return (_dwActivateFlags & TF_TMF_IMMERSIVEMODE) ? TRUE : FALSE;
    };

private:
    BOOL _InitThreadMgrEventSink();
    void _UninitThreadMgrEventSink();

    BOOL _InitTextEditSink(_In_ ITfDocumentMgr *pDocMgr);

    BOOL _InitKeyEventSink();
    void _UninitKeyEventSink();

    BOOL _InitThreadFocusSink();
    void _UninitThreadFocusSink();

    HRESULT _CallKeyEdit(ITfContext *pContext, WPARAM wParam, LPARAM lParam, PBYTE keyState);

private:
    ITfThreadMgr * _pThreadMgr;
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

    ITfComposition* _pComposition;

    Telex::TelexEngine _engine;
    BOOL _disabled;

    LONG _refCount;

private:
    IMECore(const IMECore &) = delete;
    IMECore& operator=(const IMECore &) = delete;
};
