#include <cctype>

#include "VirtualKeys.h"

VK_CATEGORIES ClassifyVirtualKey(WPARAM wParam, LPARAM lParam) {
#ifdef _DEBUG
    if (wParam == 49) {
        // map these virtual keys to "0" in debug mode
        return VK_CATEGORIES::DEBUG;
    }
#endif

    if (wParam >= VK_SHIFT && wParam <= VK_MENU) {
        return VK_CATEGORIES::MODIFIER;
    } else if (wParam >= 48 && wParam <= 57) {
        // 0-9
        //return KCVK_RELAYOUT;
        return VK_CATEGORIES::UNCATEGORIZED;
    } else if (wParam == 187 || wParam == 189 || wParam == 219 || wParam == 221) {
        // -=[]
        //return KCVK_RELAYOUT;
        return VK_CATEGORIES::UNCATEGORIZED;
    } else if (wParam >= 65 && wParam <= 90) {
        return VK_CATEGORIES::CHARACTER;
    } else if (wParam >= 33 && wParam <= 40) {
        return VK_CATEGORIES::EDIT;
    } else if (wParam == 8 || wParam == 9 || wParam == 13 || wParam == 46) {
        return VK_CATEGORIES::EDIT;
    }

    return VK_CATEGORIES::UNCATEGORIZED;
}

