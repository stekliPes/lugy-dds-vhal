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

#include <vhal_v2_0/DefaultVehicleConnector.h>
#include <vhal_v2_0/VehicleHal.h>
#include <vhal_v2_0/VehicleHalManager.h>

using ::android::hardware::automotive::vehicle::V2_0::VehicleHalManager;
using ::android::hardware::automotive::vehicle::V2_0::VehiclePropertyStore;
using ::android::hardware::automotive::vehicle::V2_0::impl::DefaultVehicleConnector;

namespace vhal_v2_0 = android::hardware::automotive::vehicle::V2_0;
namespace android {
namespace hardware {
namespace automotive {
namespace vehicle {
namespace V2_0 {
namespace impl {

class LugyVhalClient : public VehicleHalClient {

virtual std::vector<VehiclePropConfig> getAllPropertyConfig() const
{

}

virtual StatusCode setProperty(const VehiclePropValue& value, bool updateStatus)
{

}

virtual void onPropertyValue(const VehiclePropValue& value, bool updateStatus)
{
    
}


};

class LugyVhal : public VehicleHal
{
    public:

    LugyVhal(VehiclePropertyStore* propStore/*, VehicleHalClient* client*/)
    :
    mPropStore(propStore)/*,
    mVehicleClient(client)*/
    {
        
    }

    virtual std::vector<VehiclePropConfig> listProperties()
    {
        return mPropStore->getAllConfigs();
    }
    virtual VehiclePropValuePtr get(const VehiclePropValue& requestedPropValue,StatusCode* outStatus)
    {
        auto propId = requestedPropValue.prop;
        ALOGV("get(%d)", propId);

        VehiclePropValuePtr v = nullptr;
       /* if (propId == OBD2_FREEZE_FRAME) {
            v = getValuePool()->obtainComplex();
            *outStatus = fillObd2FreezeFrame(mPropStore, requestedPropValue, v.get());
            return addTimestamp(std::move(v));
        }

        if (propId == OBD2_FREEZE_FRAME_INFO) {
            v = getValuePool()->obtainComplex();
            *outStatus = fillObd2DtcInfo(mPropStore, v.get());
            return addTimestamp(std::move(v));
        }*/

        auto internalPropValue = mPropStore->readValueOrNull(requestedPropValue);
        if (internalPropValue != nullptr) {
            v = getValuePool()->obtain(*internalPropValue);
        }

        if (!v) {
            *outStatus = StatusCode::INVALID_ARG;
        } else if (v->status == VehiclePropertyStatus::AVAILABLE) {
            *outStatus = StatusCode::OK;
        } else {
            *outStatus = StatusCode::TRY_AGAIN;
        }
        v->timestamp = elapsedRealtimeNano();
        return std::move(v);
    }

    virtual StatusCode set(const VehiclePropValue& propValue)
    {
        if (propValue.status != VehiclePropertyStatus::AVAILABLE) 
        {
            return StatusCode::INVALID_ARG;
        }
        
        int32_t property = propValue.prop;
        const VehiclePropConfig* config = mPropStore->getConfigOrNull(property);
        if (config == nullptr) {
            ALOGW("no config for prop 0x%x", property);
            return StatusCode::INVALID_ARG;
        }

        auto currentPropValue = mPropStore->readValueOrNull(propValue);
        if (currentPropValue && currentPropValue->status != VehiclePropertyStatus::AVAILABLE) {
            // do not allow Android side to set() a disabled/error property
            return StatusCode::NOT_AVAILABLE;
        }

        // Send the value to the vehicle server, the server will talk to the (real or emulated) car
        
        // publish this value to CAN/DDS,ehternet, LIN, whatever... and return 
        
        VehiclePropValuePtr updatedPropValue = getValuePool()->obtain(propValue);

        if (mPropStore->writeValue(*updatedPropValue, true)) {
            doHalEvent(std::move(updatedPropValue));
        }

        return StatusCode::OK;
    }

    virtual StatusCode subscribe(int32_t /*property*/, float /*sampleRate*/)
    {
        ALOGE("subscribe not implemented in LugyVHAL");
        return StatusCode::INTERNAL_ERROR;
    };

    virtual StatusCode unsubscribe(int32_t /*property*/)
    {
        return StatusCode::OK;
    };

    private:

    VehiclePropertyStore *mPropStore;
    //VehicleHalClient *mVehicleClient;

};
}}}}}
}

using ::android::hardware::automotive::vehicle::V2_0::impl::LugyVhal;
andr

int main(int /* argc */, char* /* argv */ []) {
    ALOGI("VHAL startup");
    auto store = std::make_unique<VehiclePropertyStore>();
    //auto connector = std::make_unique<DefaultVehicleConnector>();
    auto hal = std::make_unique<LugyVhal>(store.get()/*,connector.get()*/);
    auto service = std::make_unique<VehicleHalManager>(hal.get());
    //ALOGI("Setting value pool");
    //connector->setValuePool(hal->getValuePool());

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
