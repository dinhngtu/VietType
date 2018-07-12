#pragma once

#include "Common.h"

namespace VietType {

class VirtualDocument {
public:
    static HRESULT GetVirtualDocumentMgr(ITfDocumentMgr *dim, ITfDocumentMgr **pdim);
    static HRESULT GetVirtualDocumentContext(ITfContext *context, ITfContext **pContext);

private:
    DISALLOW_IMPLICIT_CONSTRUCTORS(VirtualDocument);
};

}
