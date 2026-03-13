// SPDX-FileCopyrightText: Copyright (c) 2026 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Common.h"
#include "Telex.h"
#include "EditSession.h"
#include "SinkAdvisor.h"

namespace VietType {

class ContextManager;

class Context : public CComObjectRootEx<CComSingleThreadModel>, public ITfCompositionSink, public ITfTextEditSink {
public:
    Context() = default;
    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;
    ~Context() = default;

    DECLARE_NO_REGISTRY()
    DECLARE_NOT_AGGREGATABLE(Context)
    BEGIN_COM_MAP(Context)
    COM_INTERFACE_ENTRY(ITfCompositionSink)
    COM_INTERFACE_ENTRY(ITfTextEditSink)
    END_COM_MAP()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    // Inherited via ITfCompositionSink
    virtual STDMETHODIMP OnCompositionTerminated(
        _In_ TfEditCookie ecWrite, __RPC__in_opt ITfComposition* pComposition) override;

    // Inherited via ITfTextEditSink
    virtual STDMETHODIMP OnEndEdit(
        __RPC__in_opt ITfContext* pic, TfEditCookie ecReadOnly, __RPC__in_opt ITfEditRecord* pEditRecord) override;

    HRESULT Initialize(
        _In_ ContextManager* parent,
        _In_ ITfContext* context,
        _In_ const Telex::TelexConfig& config,
        _In_ TfGuidAtom displayAtom);
    HRESULT Uninitialize();
    void FinalRelease() {
        Uninitialize();
    }

    TfClientId GetClientId() const;
    ITfContext* GetContext() const {
        return _context;
    }
    Telex::ITelexEngine* GetEngine() const {
        return _engine.get();
    }

    constexpr bool IsBlocked() const {
        return _blocked;
    }
    bool IsTransitory() const {
        return _isTransitory;
    }
    bool IsCuas() const {
        return _isCuas;
    }
    void UpdateStatus();

    HRESULT StartComposition();
    HRESULT EndComposition();
    ITfComposition* GetComposition() const {
        return _composition;
    }

    // edit session initiators
    HRESULT UpdateBlocked(_Out_ HRESULT* hrSession);
    HRESULT RequestEditKey(
        _Out_ HRESULT* hrSession,
        _In_ WPARAM wParam,
        _In_ LPARAM lParam,
        _In_reads_(256) const BYTE* keyState,
        _In_ bool synchronous,
        _In_ wchar_t appendChar = L'\0') {
        DWORD flags = TF_ES_READWRITE;
        if (synchronous)
            flags |= TF_ES_SYNC;
        else
            flags |= TF_ES_ASYNCDONTCARE;
        return RequestEditSessionEx(EditKey, flags, hrSession, wParam, lParam, keyState, appendChar);
    }
    HRESULT RequestEditLastWord(_In_ int ignore, _In_ wchar_t push);

private:
    template <typename... Args>
    _Check_return_ HRESULT RequestEditSessionEx(
        _In_ HRESULT (*callback)(TfEditCookie ec, Context* context, Args... args),
        _In_ DWORD flags,
        _Out_ HRESULT* hrSession,
        Args... args) {

        if (GetClientId() == TF_CLIENTID_NULL || !_context) {
            return E_FAIL;
        }

        CComPtr<EditSession<Context*, Args...>> session;
        HRESULT hr = CreateInitialize(&session, callback, this, args...);
        HRESULT_CHECK_RETURN(hr, L"CreateInitialize(&session) failed");

        hr = _context->RequestEditSession(GetClientId(), session, flags, hrSession);
        HRESULT_CHECK_RETURN(hr, L"context->RequestEditSession failed");

        return hr;
    }

    HRESULT StartCompositionNow(_In_ TfEditCookie ec);
    HRESULT EndCompositionNow(_In_ TfEditCookie ec);
    HRESULT SetCompositionText(_In_ TfEditCookie ec, _In_z_ LPCWSTR str, _In_ LONG length);
    HRESULT EmptyCompositionText(_In_ TfEditCookie ec);
    HRESULT MoveCaretToEnd(_In_ TfEditCookie ec);
    HRESULT EnsureCompositionText(_In_ TfEditCookie ec, _In_z_ LPCWSTR str, _In_ LONG length);
    // return S_FALSE if nothing to commit
    HRESULT CommitCompositionText(_In_ TfEditCookie ec);

    HRESULT SetRangeDisplayAttribute(_In_ TfEditCookie ec, _In_ ITfRange* range);
    HRESULT ClearRangeDisplayAttribute(_In_ TfEditCookie ec, _In_ ITfRange* range);

    HRESULT EditNextState(_In_ TfEditCookie ec, _In_ Telex::TelexStates state);
    HRESULT EditCommit(_In_ TfEditCookie ec, _In_ wchar_t appendChar = L'\0');

    HRESULT DoUpdateBlocked(_Out_ HRESULT* hrSession);

    // edit sessions
    static HRESULT _StartComposition(TfEditCookie ec, Context* context) {
        return context->StartCompositionNow(ec);
    }
    static HRESULT _EndComposition(TfEditCookie ec, Context* context) {
        return context->EndCompositionNow(ec);
    }
    static HRESULT EditBlocked(_In_ TfEditCookie ec, _In_ Context* context);
    static HRESULT EditKey(
        _In_ TfEditCookie ec,
        _In_ Context* context,
        _In_ WPARAM wParam,
        _In_ LPARAM lParam,
        _In_reads_(256) const BYTE* keyState,
        _In_ wchar_t appendChar);

    static HRESULT SelectLastWord(
        _In_ TfEditCookie ec,
        _In_ ITfContext* context,
        _In_ int ignore,
        _COM_Outptr_ ITfRange** outRange,
        _Out_ std::wstring* word);
    static HRESULT EditLastWord(_In_ TfEditCookie ec, _In_ Context* context, _In_ int ignore, _In_ wchar_t push);
    static HRESULT KillLastWordInFullContext(
        _In_ TfEditCookie ec, _In_ ITfContext* fullContext, _In_ int ignore, _Out_ std::wstring* word);
    static HRESULT BackconvertWord(
        _In_ TfEditCookie ec,
        _In_ Context* context,
        _In_opt_ ITfRange* range,
        _In_ const std::wstring* word,
        _In_ wchar_t push);

private:
    ContextManager* _parent = nullptr;
    CComPtr<ITfContext> _context;
    bool _isTransitory = false;
    bool _isCuas = false;
    TfGuidAtom _displayAtom = TF_INVALID_GUIDATOM;
    bool _blocked = false;

    std::unique_ptr<Telex::ITelexEngine> _engine;
    SinkAdvisor<ITfTextEditSink> _textEditSinkAdvisor;
    CComPtr<ITfComposition> _composition;
};

} // namespace VietType
