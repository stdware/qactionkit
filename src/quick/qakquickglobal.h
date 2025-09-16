// Copyright (C) 2023-2024 Stdware Collections (https://www.github.com/stdware)
// Copyright (C) 2021-2023 wangwenx190 (Yuhang Zhao)
// SPDX-License-Identifier: Apache-2.0

#ifndef QAKQUICKGLOBAL_H
#define QAKQUICKGLOBAL_H

#include <QtCore/QtGlobal>

#ifndef QAK_QUICK_EXPORT
#  ifdef QAK_QUICK_STATIC
#    define QAK_QUICK_EXPORT
#  else
#    ifdef QAK_QUICK_LIBRARY
#      define QAK_QUICK_EXPORT Q_DECL_EXPORT
#    else
#      define QAK_QUICK_EXPORT Q_DECL_IMPORT
#    endif
#  endif
#endif

#endif // QAKQUICKGLOBAL_H
