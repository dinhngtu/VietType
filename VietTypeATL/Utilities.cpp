#include "Utilities.h"
#include "Compartment.h"

HRESULT VietType::IsContextEmpty(ITfContext * context, TfClientId clientid, bool * isempty) {
    HRESULT hr;

    SmartComObjPtr<Compartment> compEmpty;
    hr = compEmpty.CreateInstance();
    HRESULT_CHECK_RETURN(hr, L"%s", L"compEmpty.CreateInstance failed");

    hr = compEmpty->Initialize(context, clientid, GUID_COMPARTMENT_EMPTYCONTEXT);
    HRESULT_CHECK_RETURN(hr, L"%s", L"compEmpty->Initialize failed");

    long contextEmpty;
    hr = compEmpty->GetValue(&contextEmpty);
    HRESULT_CHECK_RETURN(hr, L"%s", L"compDisabled->GetValue failed");

    *isempty = hr == S_OK && contextEmpty;
    return hr;
}
