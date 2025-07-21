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

#if defined(__GNUC__) || defined(__clang__)
#  define QACTIONKIT_PRINTF_FORMAT(fmtpos, attrpos)                                                \
      __attribute__((__format__(__printf__, fmtpos, attrpos)))
#else
#  define QACTIONKIT_PRINTF_FORMAT(fmtpos, attrpos)
#endif

namespace QAK {

    enum ActionElement {
        AE_Layouts,
        AE_Texts,
        AE_Keymap,
        AE_Icons,
    };

}

#endif // QAKGLOBAL_H