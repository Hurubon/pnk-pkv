// Copyright (C) 2025 Hrvoje 'Hurubon' Žohar
// See the end of the file for extended copyright information.
#ifndef PNK_C23_COMPATIBILITY
#define PNK_C23_COMPATIBILITY

    #if __STDC_VERSION__ >= 202311L
        #define PNK_NODISCARD                [[nodiscard]]
        #define PNK_NODISCARD_REASON(reason) [[nodiscard(reason)]]
    #elif defined(__GNUC__) || defined(__clang__)
        #define PNK_NODISCARD           __attribute__((warn_unused_result))
        #define PNK_NODISCARD_REASON(_) __attribute__((warn_unused_result))
    #elif defined(_MSC_VER)
        #include <sal.h>
        #define PNK_NODISCARD           _Check_return
        #define PNK_NODISCARD_REASON(_) _Check_return
    #else
        #define PNK_NODISCARD
        #define PNK_NODISCARD_REASON(_)
    #endif

    #if __STDC_VERSION__ >= 202311L
        #define PNK_MAYBE_UNUSED [[maybe_unused]]
    #elif defined(__GNUC__) || defined(__clang__)
        #define PNK_MAYBE_UNUSED __attribute__((unused))
    #else
        #define PNK_MAYBE_UNUSED
    #endif

#endif /* PNK_C23_COMPATIBILITY */
/* pnk-pkv - a terminal ASCII video player
** Copyright (C) 2025 Hrvoje 'Hurubon' Žohar
**
** pnk-pkv is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** pnk-pkv is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program. If not, see <https://www.gnu.org/licenses/>. */
