// SPDX-FileCopyrightText: Copyright (c) 2026 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Common.h"
#include "Telex.h"
#include "EditSession.h"

namespace VietType {

class CompositionManager;

class Context : public CComObjectRootEx<CComSingleThreadModel>, public ITfCompositionSink {
public:
    Context() = default;
    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;
    ~Context() = default;

    DECLARE_NO_REGISTRY()
    DECLARE_NOT_AGGREGATABLE(Context)
    BEGIN_COM_MAP(Context)
    COM_INTERFACE_ENTRY(ITfCompositionSink)
    END_COM_MAP()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    // Inherited via ITfCompositionSink
    virtual STDMETHODIMP OnCompositionTerminated(
        _In_ TfEditCookie ecWrite, __RPC__in_opt ITfComposition* pComposition) override;

    HRESULT Initialize(
        _In_ TfClientId clientId,
        _In_ ITfContext* context,
        _In_ const Telex::TelexConfig& config,
        _In_ TfGuidAtom displayAtom);
    HRESULT Uninitialize();
    void FinalRelease() {
        Uninitialize();
    }

    constexpr TfClientId GetClientId() const {
        return _clientId;
    }
    ITfContext* GetContext() const {
        return _context;
    }
    Telex::ITelexEngine* GetEngine() const {
        return _engine.get();
    }

    constexpr bool IsBlocked() const {
        return _blocked;
    }

    HRESULT StartComposition();
    HRESULT EndComposition();
    ITfComposition* GetComposition() const {
        return _composition;
    }

    // edit session initiators
    HRESULT RequestEditBlocked(_Out_ HRESULT* hrSession) {
        return RequestEditSessionEx(EditBlocked, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, hrSession);
    }
    HRESULT RequestEditSurroundingWord(_Out_ HRESULT* hrSession, _In_ int ignore) {
        return RequestEditSessionEx(EditSurroundingWord, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, hrSession, ignore);
    }
    HRESULT RequestEditSurroundingWordAndPush(_Out_ HRESULT* hrSession, _In_ int ignore, _In_ wchar_t push) {
        return RequestEditSessionEx(
            EditSurroundingWordAndPush, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, hrSession, ignore, push);
    }
    HRESULT RequestEditKey(
        _Out_ HRESULT* hrSession, _In_ WPARAM wParam, _In_ LPARAM lParam, _In_reads_(256) const BYTE* keyState) {
        return RequestEditSessionEx(
            EditKey, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, hrSession, wParam, lParam, keyState);
    }

private:
    template <typename... Args>
    _Check_return_ HRESULT RequestEditSessionEx(
        _In_ HRESULT (*callback)(TfEditCookie ec, Context* context, Args... args),
        _In_ DWORD flags,
        _Out_ HRESULT* hrSession,
        Args... args) {

        if (_clientId == TF_CLIENTID_NULL || !_context) {
            return E_FAIL;
        }

        CComPtr<EditSession<Context*, Args...>> session;
        HRESULT hr = CreateInitialize(&session, callback, this, args...);
        HRESULT_CHECK_RETURN(hr, L"CreateInitialize(&session) failed");

        hr = _context->RequestEditSession(_clientId, session, flags, hrSession);
        HRESULT_CHECK_RETURN(hr, L"context->RequestEditSession failed");

        return hr;
    }

    HRESULT StartCompositionNow(_In_ TfEditCookie ec);
    HRESULT EndCompositionNow(_In_ TfEditCookie ec);
    HRESULT SetCompositionText(_In_ TfEditCookie ec, _In_z_ LPCWSTR str, _In_ LONG length);
    HRESULT EmptyCompositionText(_In_ TfEditCookie ec);
    HRESULT MoveCaretToEnd(_In_ TfEditCookie ec);
    HRESULT EnsureCompositionText(_In_ TfEditCookie ec, _In_z_ LPCWSTR str, _In_ LONG length);

    HRESULT SetRangeDisplayAttribute(_In_ TfEditCookie ec, _In_ ITfRange* range);
    HRESULT ClearRangeDisplayAttribute(_In_ TfEditCookie ec, _In_ ITfRange* range);

    HRESULT DoEditSurroundingWord(_In_ TfEditCookie ec, _In_ int ignore);
    HRESULT EditNextState(_In_ TfEditCookie ec, _In_ Telex::TelexStates state);
    HRESULT EditCommit(_In_ TfEditCookie ec);

    // edit sessions
    static HRESULT _StartComposition(TfEditCookie ec, Context* context) {
        return context->StartCompositionNow(ec);
    }
    static HRESULT _EndComposition(TfEditCookie ec, Context* context) {
        return context->EndCompositionNow(ec);
    }
    static HRESULT EditBlocked(_In_ TfEditCookie ec, _In_ Context* context);
    static HRESULT EditSurroundingWord(_In_ TfEditCookie ec, _In_ Context* context, _In_ int ignore);
    static HRESULT EditSurroundingWordAndPush(
        _In_ TfEditCookie ec, _In_ Context* context, _In_ int ignore, _In_ wchar_t push);
    static HRESULT EditKey(
        _In_ TfEditCookie ec,
        _In_ Context* context,
        _In_ WPARAM wParam,
        _In_ LPARAM lParam,
        _In_reads_(256) const BYTE* keyState);

private:
    TfClientId _clientId = TF_CLIENTID_NULL;
    CComPtr<ITfContext> _context;
    TfGuidAtom _displayAtom = TF_INVALID_GUIDATOM;
    bool _blocked = false;

    std::unique_ptr<Telex::ITelexEngine> _engine;
    CComPtr<ITfComposition> _composition;
};

} // namespace VietType
