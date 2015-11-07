/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "unicode/ucnv.h"

U_CAPI UConverter* U_EXPORT2
ucnv_open_51 (const char *name,
                       UErrorCode * err) {
  return ucnv_open(name, err);
}

U_CAPI void  U_EXPORT2
ucnv_close_51 (UConverter * converter) {
  ucnv_close (converter);
}

U_CAPI int32_t U_EXPORT2
ucnv_toUChars_51(UConverter *cnv,
              UChar *dest, int32_t destCapacity,
              const char *src, int32_t srcLength,
              UErrorCode *pErrorCode) {
  return ucnv_toUChars(cnv, dest, destCapacity, src, srcLength, pErrorCode);
}

U_CAPI int32_t U_EXPORT2
ucnv_fromUChars_51(UConverter *cnv,
                char *dest, int32_t destCapacity,
                const UChar *src, int32_t srcLength,
                UErrorCode *pErrorCode) {
  return ucnv_fromUChars(cnv, dest, destCapacity, src, srcLength, pErrorCode);
}

