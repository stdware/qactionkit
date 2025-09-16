// Copyright (C) 2023-2024 Stdware Collections (https://www.github.com/stdware)
// Copyright (C) 2021-2023 wangwenx190 (Yuhang Zhao)
// SPDX-License-Identifier: Apache-2.0

#ifndef QAKWIDGETSGLOBAL_H
#define QAKWIDGETSGLOBAL_H

#include <QtCore/QtGlobal>

#ifndef QAK_WIDGETS_EXPORT
#  ifdef QAK_WIDGETS_STATIC
#    define QAK_WIDGETS_EXPORT
#  else
#    ifdef QAK_WIDGETS_LIBRARY
#      define QAK_WIDGETS_EXPORT Q_DECL_EXPORT
#    else
#      define QAK_WIDGETS_EXPORT Q_DECL_IMPORT
#    endif
#  endif
#endif

#endif // QAKWIDGETSGLOBAL_H
