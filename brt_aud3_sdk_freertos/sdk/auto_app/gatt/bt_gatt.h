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


#ifndef ANDROID_INCLUDE_BT_GATT_H
#define ANDROID_INCLUDE_BT_GATT_H

#include "btdatatype.h"
#include <stdint.h>
#include <stdlib.h>
////#include "bt_bluedroid_types.h"
#include "bt_gatt_client.h"
#include "bt_gatt_server.h"


/** BT-GATT callbacks */
typedef struct {
	/** Set to sizeof(btgatt_callbacks_t) */
	size_t size;

	/** GATT Client callbacks */
	const btgatt_client_callbacks_t *client;

	/** GATT Server callbacks */
	const btgatt_server_callbacks_t *server;
} btgatt_callbacks_t;

#endif /* ANDROID_INCLUDE_BT_GATT_H */
