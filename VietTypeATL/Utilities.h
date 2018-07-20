#pragma once

#include "Common.h"

namespace VietType {

HRESULT IsContextEmpty(_In_ ITfContext *context, TfClientId clientid, _Out_ bool *isempty);

}
