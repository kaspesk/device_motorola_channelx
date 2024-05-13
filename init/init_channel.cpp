/*
   Copyright (c) 2016, The CyanogenMod Project
   Copyright (c) 2019, The LineageOS Project

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
   ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
   BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <cstdlib>
#include <fstream>
#include <string.h>
#include <sys/sysinfo.h>
#include <unistd.h>

#include <android-base/properties.h>
#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>
#include <init/DeviceLibinit.h>

#include "vendor_init.h"
#include "property_service.h"

using android::base::GetProperty;
using android::base::SetProperty;

char const *heapstartsize;
char const *heapgrowthlimit;
char const *heapsize;
char const *heapminfree;
char const *heapmaxfree;
char const *heaptargetutilization;

void property_override(char const prop[], char const value[], bool add = true)
{
    prop_info *pi;

    pi = (prop_info*) __system_property_find(prop);
    if (pi)
    __system_property_update(pi, value, strlen(value));
    else if (add)
    __system_property_add(prop, strlen(prop), value, strlen(value));
}

void check_device()
{
    struct sysinfo sys;

    sysinfo(&sys);

    if (sys.totalram > 5072ull * 1024 * 1024) {
        // from - phone-xhdpi-6144-dalvik-heap.mk
        heapstartsize = "16m";
        heapgrowthlimit = "256m";
        heapsize = "512m";
        heaptargetutilization = "0.5";
        heapminfree = "8m";
        heapmaxfree = "32m";
    } else if (sys.totalram > 3072ull * 1024 * 1024) {
        // from - phone-xxhdpi-4096-dalvik-heap.mk
        heapstartsize = "8m";
        heapgrowthlimit = "256m";
        heapsize = "512m";
        heaptargetutilization = "0.6";
        heapminfree = "8m";
        heapmaxfree = "16m";
    } else {
        // from - phone-xhdpi-2048-dalvik-heap.mk
        heapstartsize = "8m";
        heapgrowthlimit = "192m";
        heapsize = "512m";
        heaptargetutilization = "0.75";
        heapminfree = "512k";
        heapmaxfree = "8m";
    }
}

void vendor_load_properties()
{
    check_device();

    SetProperty("dalvik.vm.heapstartsize", heapstartsize);
    SetProperty("dalvik.vm.heapgrowthlimit", heapgrowthlimit);
    SetProperty("dalvik.vm.heapsize", heapsize);
    SetProperty("dalvik.vm.heaptargetutilization", heaptargetutilization);
    SetProperty("dalvik.vm.heapminfree", heapminfree);
    SetProperty("dalvik.vm.heapmaxfree", heapmaxfree);

    // Setting carrier prop
    std::string carrier = GetProperty("ro.boot.carrier", "unknown");
    property_override("ro.carrier", carrier.c_str());

    vendor_load_device_properties();
}

#include <vector>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

#include <android-base/properties.h>
#include <android-base/logging.h>
#include <init/DeviceLibinit.h>

#include "property_service.h"
#include "vendor_init.h"

using android::base::GetProperty;
using android::base::SetProperty;

std::vector<std::string> ro_props_default_source_order = {
    "",
    "odm.",
    "product.",
    "system.",
    "system_ext.",
    "vendor.",
};

void property_override_device(char const prop[], char const value[], bool add = true)
{
    prop_info *pi;

    pi = (prop_info*) __system_property_find(prop);
    if (pi)
        __system_property_update(pi, value, strlen(value));
    else if (add)
        __system_property_add(prop, strlen(prop), value, strlen(value));
}


void vendor_load_device_properties()
{
    std::string bootsku;
    std::string device;

    const auto set_ro_build_prop = [](const std::string &source,
            const std::string &prop, const std::string &value) {
        auto prop_name = "ro." + source + "build." + prop;
        property_override_device(prop_name.c_str(), value.c_str(), false);
    };

    const auto set_ro_product_prop = [](const std::string &source,
            const std::string &prop, const std::string &value) {
        auto prop_name = "ro.product." + source + prop;
        property_override_device(prop_name.c_str(), value.c_str(), false);
    };

    bootsku = GetProperty("ro.boot.hardware.sku", "");
    if (bootsku == "XT1952-T") {
        /* T-Mobile REVVLRY */
        property_override_device("ro.build.description", "channel_revvl-user 10 QPY30.85-18 6572f release-keys");
        property_override_device("persist.vendor.radio.customer_mbns", "tmo_usa_ims_default.mbn;sprint_usa_ims.mbn");
        property_override_device("persist.vendor.radio.data_con_rprt", "1");
        property_override_device("persist.vendor.ims.playout_delay", "10");
        property_override_device("persist.vendor.ims.cam_sensor_delay", "20");
        property_override_device("persist.vendor.ims.display_delay", "40");
        for (const auto &source : ro_props_default_source_order) {
            set_ro_build_prop(source, "fingerprint", "motorola/channel_revvl/channel:10/QPY30.85-18/6572f:user/release-keys");
            set_ro_product_prop(source, "device", "channel");
            set_ro_product_prop(source, "model", "REVVLRY");
            set_ro_product_prop(source, "name", "channel_revvl");
        }
    } else {
        /* moto g(7) play */
        property_override_device("ro.build.description", "channel_retail-user 10 QPYS30.85-23-8-2 c00f57 release-keys");
        for (const auto &source : ro_props_default_source_order) {
            set_ro_build_prop(source, "fingerprint", "motorola/channel_retail/channel:10/QPYS30.85-23-8-2/c00f57:user/release-keys");
            set_ro_product_prop(source, "device", "channel");
            set_ro_product_prop(source, "model", "moto g(7) play");
        }
    }

    device = GetProperty("ro.product.device", "");
    LOG(ERROR) << "Found bootsku '" << bootsku.c_str() << "' setting build properties for '" << device.c_str() << "' device\n";
}
