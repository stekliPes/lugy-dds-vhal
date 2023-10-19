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
#include <utils/SystemClock.h>
#include <vector>

#include <vhal_v2_0/DefaultVehicleHal.h>
#include <vhal_v2_0/VehicleHal.h>
#include <vhal_v2_0/VehicleHalManager.h>


using ::android::hardware::automotive::vehicle::V2_0::VehicleHalManager;
using ::android::hardware::automotive::vehicle::V2_0::VehiclePropertyStore;
using ::android::hardware::automotive::vehicle::V2_0::impl::DefaultVehicleHal;

namespace vhal_v2_0 = android::hardware::automotive::vehicle::V2_0;
namespace android {
namespace hardware {
namespace automotive {
namespace vehicle {
namespace V2_0 {
namespace impl {

class LugyVhalClient : public VehicleHalClient {

public: 
    LugyVhalClient()
    {
        ALOGI("Client constructor start");
        mPropConfigs.resize(4);
        mPropConfigs[0].prop = (int32_t)(VehicleProperty::PERF_VEHICLE_SPEED);
        mPropConfigs[0].access = VehiclePropertyAccess::READ_WRITE;
        mPropConfigs[0].changeMode = VehiclePropertyChangeMode::ON_CHANGE;
        speed.areaId = (int32_t)VehicleArea::GLOBAL;
        speed.status = VehiclePropertyStatus::AVAILABLE;
        speed.prop = (int32_t)VehicleProperty::PERF_VEHICLE_SPEED;
        speed.value.floatValues = {0};
        speed.timestamp = elapsedRealtimeNano();
        mPropConfigs[1].prop = (int32_t)VehicleProperty::PARKING_BRAKE_ON;
        mPropConfigs[1].access = VehiclePropertyAccess::READ_WRITE;
        mPropConfigs[1].changeMode = VehiclePropertyChangeMode::ON_CHANGE;
        pb.areaId = (int32_t)VehicleArea::GLOBAL;
        pb.status = VehiclePropertyStatus::AVAILABLE;
        pb.prop = (int32_t)VehicleProperty::PARKING_BRAKE_ON;
        pb.value.int32Values = {0};
        pb.timestamp = elapsedRealtimeNano();

        mPropConfigs[2].prop = (int32_t)VehicleProperty::OBD2_LIVE_FRAME;
        mPropConfigs[2].access = VehiclePropertyAccess::READ;
        mPropConfigs[2].changeMode = VehiclePropertyChangeMode::ON_CHANGE;
        mPropConfigs[2].configArray ={0,0};
        mPropConfigs[3].prop = (int32_t)VehicleProperty::OBD2_FREEZE_FRAME;
        mPropConfigs[3].access = VehiclePropertyAccess::READ;
        mPropConfigs[3].changeMode = VehiclePropertyChangeMode::ON_CHANGE;
        mPropConfigs[3].configArray ={0,0};
        
        

    }

    virtual std::vector<VehiclePropConfig> getAllPropertyConfig() const
    {

        ALOGI("Get PCs");
        return mPropConfigs;
    }

    virtual StatusCode setProperty(const VehiclePropValue& value, bool /*updateStatus*/)
    {

        ALOGI("Set property");
        switch (value.prop)
        {
            case (int32_t)VehicleProperty::PERF_VEHICLE_SPEED:
            speed = value;
            //publish it to somewhere...
            break;
            case (int32_t)VehicleProperty::PARKING_BRAKE_ON:
            pb = value;
            //publish it to somewhere...
            break;
            default:
            return StatusCode::INVALID_ARG;
            break;
        }
        return StatusCode::INVALID_ARG;
    }

    virtual void triggerSendAllValues()
    {

        ALOGI("Trigger");
        onPropertyValue(speed,true);
        onPropertyValue(pb,true);

        ALOGI("Trigger done");
    }

private:
    std::vector<VehiclePropConfig> mPropConfigs;
    VehiclePropValue speed;
    VehiclePropValue pb;

};

}}}}}
}

using ::android::hardware::automotive::vehicle::V2_0::impl::LugyVhalClient;

int main(int /* argc */, char* /* argv */ []) {
    auto store = std::make_unique<VehiclePropertyStore>();
    auto client = std::make_unique<LugyVhalClient>();
    auto hal = std::make_unique<DefaultVehicleHal>(store.get(),client.get());
    auto service = std::make_unique<VehicleHalManager>(hal.get());

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

    //VehiclePropValue value;

    for (uint i=0; i<properties.size(); i++)
    {
	    const vhal_v2_0::VehiclePropConfig prop = properties[i];
	    ALOGI("Property #%d: configString %s, access %d, changeMode %d, ID %d",i,prop.configString.c_str(),prop.access,prop.changeMode,prop.prop);

        
    }
    android::hardware::joinRpcThreadpool();

    return 0;
}
