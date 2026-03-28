// SPDX-FileCopyrightText: Copyright (c) 2026 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Common.h"
#include "SinkAdvisor.h"
#include "EditSession.h"
#include "KeyTranslator.h"
#include "Telex.h"

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
        _In_ uint64_t configVersion,
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
    bool UpdateConfig(_In_ const Telex::TelexConfig& config, _In_ uint64_t configVersion) {
        if (configVersion > _configVersion) {
            _engine->SetConfig(config);
            _configVersion = configVersion;
            return true;
        }
        return false;
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

    HRESULT StartComposition();
    HRESULT EndComposition();

    // edit session initiators
    HRESULT UpdateBlocked(_Out_ HRESULT* hrSession);
    HRESULT RequestEditKey(
        _Out_ HRESULT* hrSession,
        _In_ bool sync,
        _In_ KeyResult keyResult,
        _In_ wchar_t push,
        _In_ bool newComposition = false) {
        DWORD flags = TF_ES_READWRITE;
        if (sync)
            flags |= TF_ES_SYNC;
        else
            flags |= TF_ES_ASYNC;
        return RequestEditSessionEx(EditKey, flags, hrSession, keyResult, push, newComposition);
    }
    HRESULT RequestEditLastWord(_In_ int ignore, _In_ wchar_t push);
    HRESULT RequestQueryCompositionSync(_Out_ HRESULT* hrSession) {
        return RequestEditSessionEx(_QueryComposition, TF_ES_READ | TF_ES_SYNC, hrSession);
    }

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

    HRESULT GetComposition(_In_ TfEditCookie ec, _COM_Outptr_result_maybenull_ ITfComposition** composition);
    HRESULT StartCompositionNow(_In_ TfEditCookie ec, _COM_Outptr_opt_ ITfComposition** composition);
    HRESULT EndCompositionNow(_In_ TfEditCookie ec, _In_opt_ ITfComposition* composition);
    HRESULT EnsureComposition(_In_ TfEditCookie ec, CComPtr<ITfComposition>& composition);
    HRESULT SetCompositionText(
        _In_ TfEditCookie ec, _In_ ITfComposition* composition, _In_z_ LPCWSTR str, _In_ LONG length);
    HRESULT EmptyCompositionText(_In_ TfEditCookie ec, _In_ ITfComposition* composition);
    HRESULT MoveCaretToEnd(_In_ TfEditCookie ec, _In_ ITfComposition* composition, bool collapse = false);

    HRESULT SetRangeDisplayAttribute(_In_ TfEditCookie ec, _In_ ITfRange* range);
    HRESULT ClearRangeDisplayAttribute(_In_ TfEditCookie ec, _In_ ITfRange* range);

    HRESULT DoEditNextState(
        _In_ TfEditCookie ec,
        _In_ Telex::TelexStates state,
        _In_ wchar_t nonEngineAppend,
        _In_opt_ ITfComposition* existingComposition = nullptr,
        _In_ bool newComposition = false,
        _In_ bool resetAnyway = false);

    // edit sessions
    static HRESULT _StartComposition(TfEditCookie ec, Context* context) {
        return context->StartCompositionNow(ec, nullptr);
    }
    static HRESULT _EndComposition(TfEditCookie ec, Context* context) {
        CComPtr<ITfComposition> composition;
        HRESULT hr = context->GetComposition(ec, &composition);
        if (SUCCEEDED(hr) && composition) {
            return context->EndCompositionNow(ec, composition);
        }
        return S_OK;
    }
    static HRESULT _QueryComposition(_In_ TfEditCookie ec, _In_ Context* context) {
        CComPtr<ITfComposition> composition;
        HRESULT hr = context->GetComposition(ec, &composition);
        HRESULT_CHECK_RETURN(hr, L"context->GetComposition failed");
        return composition ? S_OK : S_FALSE;
    }
    static HRESULT EditBlocked(_In_ TfEditCookie ec, _In_ Context* context);
    static HRESULT EditKey(
        _In_ TfEditCookie ec,
        _In_ Context* context,
        _In_ KeyResult keyResult,
        _In_ wchar_t push,
        _In_ bool newComposition);

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
    uint64_t _configVersion = 0;
    SinkAdvisor<ITfTextEditSink> _textEditSinkAdvisor;
};

} // namespace VietType
