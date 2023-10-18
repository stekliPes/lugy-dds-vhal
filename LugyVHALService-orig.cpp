/*
 * Copyright (C) 2016 The Android Open Source Project
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

#define LOG_TAG "lugy.vehicle@0.1-service"
#include <android/log.h>
#include <hidl/HidlTransportSupport.h>

#include <iostream>

#include <vhal_v2_0/DefaultVehicleConnector.h>
#include <vhal_v2_0/DefaultVehicleHal.h>
#include <vhal_v2_0/VehicleHalManager.h>

using ::android::hardware::automotive::vehicle::V2_0::VehicleHalManager;
using ::android::hardware::automotive::vehicle::V2_0::VehiclePropertyStore;
using ::android::hardware::automotive::vehicle::V2_0::impl::DefaultVehicleConnector;
using ::android::hardware::automotive::vehicle::V2_0::impl::DefaultVehicleHal;

namespace vhal_v2_0 = android::hardware::automotive::vehicle::V2_0;

int main(int /* argc */, char* /* argv */ []) {
    ALOGI("VHAL startup");
    auto store = std::make_unique<VehiclePropertyStore>();
    auto connector = std::make_unique<DefaultVehicleConnector>();
    auto hal = std::make_unique<DefaultVehicleHal>(store.get(), connector.get());
    auto service = std::make_unique<VehicleHalManager>(hal.get());
    ALOGI("Setting value pool");
    connector->setValuePool(hal->getValuePool());

    ALOGI("Configuring RPC threadpool");
    android::hardware::configureRpcThreadpool(4, true /* callerWillJoin */);

    ALOGI("Registering as service...");
    android::status_t status = service->registerAsService();

    if (status != android::OK) {
        ALOGE("Unable to register Lugy's vehicle service (%d)", status);
        return 1;
    }

    ALOGI("Ready, getting properties");
    std::vector<vhal_v2_0::VehiclePropConfig> properties = hal->listProperties();

    ALOGI("Properties count: %d",properties.size());

    for (uint i=0; i<properties.size(); i++)
    {
	const vhal_v2_0::VehiclePropConfig prop = properties[i];
	ALOGI("Property #%d: configString %s, access %d, changeMode %d, ID %d",i,prop.configString.c_str(),prop.access,prop.changeMode,prop.prop);
    }
    android::hardware::joinRpcThreadpool();

    return 0;
}
