/*
 * Copyright (C) 2013 The Android Open Source Project
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


#ifndef ANDROID_INCLUDE_BT_GATT_TYPES_H
#define ANDROID_INCLUDE_BT_GATT_TYPES_H

#include "bt_utils.h"

/**
 * GATT Service types
 */
#define BTGATT_SERVICE_TYPE_PRIMARY 0
#define BTGATT_SERVICE_TYPE_SECONDARY 1

/**
 * Invalid application ID. The valid ID range: 1 - 40 (MAX_CLIENT_NUM + MAX_SERVER_NUM)
 */
#define BTGATT_APP_IF_INVALID           0xFF
#define BTGATT_MAX_CLIENT_APP_NUM       20
#define BTGATT_MAX_SERVER_APP_NUM       20

/** Preferred physical Transport for GATT connection */
typedef enum {
    GATT_TRANSPORT_AUTO,
    GATT_TRANSPORT_BREDR,
    GATT_TRANSPORT_LE
} btgatt_transport_t;


#endif /* ANDROID_INCLUDE_BT_GATT_TYPES_H */
