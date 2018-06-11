#pragma once

#include "stdafx.h"

enum class VK_CATEGORIES {
    UNCATEGORIZED,
    CHARACTER,
    RELAYOUT,
    MODIFIER,
    EDIT,
    DEBUG,
};

VK_CATEGORIES ClassifyVirtualKey(WPARAM wParam, LPARAM lParam);
