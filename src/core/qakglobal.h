// Copyright (C) 2024-2025 Stdware Collections (https://www.github.com/stdware)
// SPDX-License-Identifier: Apache-2.0

#ifndef QAKGLOBAL_H
#define QAKGLOBAL_H

#include <QtCore/QtGlobal>

#ifndef QAK_CORE_EXPORT
#  ifdef QAK_CORE_STATIC
#    define QAK_CORE_EXPORT
#  else
#    ifdef QAK_CORE_LIBRARY
#      define QAK_CORE_EXPORT Q_DECL_EXPORT
#    else
#      define QAK_CORE_EXPORT Q_DECL_IMPORT
#    endif
#  endif
#endif

#define QAK_ACTION_EXTENSION_VERSION "1.0"

#endif // QAKGLOBAL_H